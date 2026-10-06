// =============================================================================
//  QuitaSicote3000 — SIS (sistema de seguridad) · ESP32 DevKit V1
// =============================================================================
//  Este controlador hace SÓLO seguridad (ADR-0012). Decide tres relés:
//
//    RL1  permiso del PTC ............ sin él, el calefactor no puede encender
//    RL3  permiso de apagar FAN_P .... sin él, el ventilador del PTC sigue girando
//    RL4  forzar FAN_C ............... enciende el ventilador de circulación
//
//  No tiene termopar (ADR-0004): sus entradas son la puerta (contactos NA y NC)
//  y lo que informa el control. Lo que llega del control SÓLO PUEDE RESTRINGIR:
//  puede quitar el permiso o provocar un disparo, nunca conceder nada. Sin
//  temperatura, "frío" se estima por tiempo sin permiso del PTC.
//
//  Dos tareas de FreeRTOS:
//
//    taskSafety  cada 10 ms   puerta -> funciones de seguridad -> relés
//    taskLink    cada 10 ms   habla con el control y guarda en memoria
//
//  Las dos comparten las variables de la sección "Estado compartido", siempre
//  con el mutex tomado (ver la clase Lock).
// =============================================================================
#include <Arduino.h>
#include <Preferences.h>
#include <esp_task_wdt.h>

#include "config.h"
#include "qs_protocol.h"

using namespace qs;

// =============================================================================
//  Estado compartido entre las tareas
// =============================================================================

SemaphoreHandle_t stateMutex;

// Toma el mutex al crearse y lo suelta al salir del bloque:
//     { Lock lock;  ...leer o escribir el estado compartido...  }
struct Lock {
  Lock() { xSemaphoreTake(stateMutex, portMAX_DELAY); }
  ~Lock() { xSemaphoreGive(stateMutex); }
};

// --- Estado del SIS y sus salidas ---
SisState state = SisState::Boot;
uint16_t tripMask = 0;           // causas del disparo (bits TripBit)
Door door = Door::Invalid;
bool ptcPermit = false;          // RL1
bool fanPOffPermit = false;      // RL3
bool fanCForce = false;          // RL4

// --- Lo que informa el control (HB_CTRL) ---
bool heartbeatSeen = false;
uint32_t lastHeartbeatMs = 0;
uint8_t ctrlFlags = 0;           // bits CtrlFlag del último latido

// --- Temporizadores de la tarea de seguridad ---
uint32_t heatingMs = 0;          // tiempo acumulado con el permiso concedido (SIF-07)
uint32_t permitOffMs = 0;        // tiempo seguido con el permiso retirado

// --- Datos que sobreviven a un corte de energía (NVS) ---
uint8_t thermalEvents = 0;       // eventos térmicos ocurridos
uint16_t latchedTripMask = 0;    // disparo térmico pendiente de rearmar
bool lockout = false;            // equipo bloqueado
bool memoryInvalid = false;      // los datos guardados estaban corruptos
bool saveRequested = false;      // taskLink debe guardar

// --- Procedimiento de servicio (desbloqueo) ---
bool serviceActive = false;
uint32_t serviceStartMs = 0;
bool serviceDoorWasOpen = false;
uint8_t serviceDoorToggles = 0;

// Avisos para el control. La tarea de seguridad los encola; taskLink los envía.
QueueHandle_t eventQueue;

void emitEvent(uint8_t event, uint8_t detail = 0) {
  SisEventMsg message = {event, tripMask, detail};
  xQueueSend(eventQueue, &message, 0);  // si la cola está llena, se pierde: nunca se espera
}

// =============================================================================
//  Memoria no volátil
// =============================================================================
//  Un bloque de 8 bytes en NVS:  'S' 'Q' | versión | eventos | máscara (2) | bloqueo | CRC-8

constexpr size_t RECORD_SIZE = 8;

uint8_t crc8(const uint8_t* data, size_t len) {
  uint8_t crc = 0xFF;
  for (size_t i = 0; i < len; i++) {
    crc ^= data[i];
    for (int bit = 0; bit < 8; bit++) crc = (crc & 0x80) ? (crc << 1) ^ 0x31 : (crc << 1);
  }
  return crc;
}

// Se llama una vez en setup(), antes de crear las tareas.
void loadMemory() {
  Preferences nvs;
  nvs.begin("sis", true);
  uint8_t record[RECORD_SIZE];
  const size_t length = nvs.isKey("rec") ? nvs.getBytes("rec", record, sizeof(record)) : 0;
  nvs.end();

  if (length == 0) {  // equipo nuevo: nada guardado todavía
    saveRequested = true;
    return;
  }

  const bool good = length == RECORD_SIZE && record[0] == 'S' && record[1] == 'Q' &&
                    record[2] == 1 && record[6] <= 1 && crc8(record, 7) == record[7];
  if (!good) {
    // Datos corruptos: se arranca disparado y, por prudencia, se supone que ya
    // hubo un evento térmico (el siguiente bloquea).
    memoryInvalid = true;
    thermalEvents = 1;
    tripMask |= TRIP_MEMORY_INVALID;
    return;
  }

  thermalEvents = record[3];
  latchedTripMask = record[4] | (record[5] << 8);
  lockout = record[6] == 1;

  // Lo que estaba enclavado antes del corte sigue enclavado.
  if (latchedTripMask != 0 || lockout) tripMask |= latchedTripMask | TRIP_FROM_MEMORY;
  if (lockout) state = SisState::Locked;
}

void saveMemory() {
  uint8_t record[RECORD_SIZE] = {'S', 'Q', 1};
  {
    Lock lock;
    saveRequested = false;
    record[3] = thermalEvents;
    record[4] = latchedTripMask & 0xFF;
    record[5] = latchedTripMask >> 8;
    record[6] = lockout ? 1 : 0;
  }
  record[7] = crc8(record, 7);

  Preferences nvs;  // la escritura en flash es lenta: se hace sin el mutex
  nvs.begin("sis", false);
  nvs.putBytes("rec", record, sizeof(record));
  nvs.end();
}

// =============================================================================
//  Disparo, rearme y servicio
// =============================================================================

// Enclava el disparo. Las salidas se abren en updateOutputs() porque el estado
// deja de ser OK.
void trip(uint16_t causes) {
  tripMask |= causes;
  if (state == SisState::Tripped || state == SisState::Locked) return;  // ya estaba disparado

  // Un evento térmico se cuenta sólo al pasar de OK a DISPARADO: las causas que
  // se sumen después pertenecen al mismo incidente.
  const bool thermalEvent = state == SisState::Ok && (causes & TRIP_THERMAL_MASK) != 0;
  if (thermalEvent) {
    if (thermalEvents < 255) thermalEvents++;
    latchedTripMask = tripMask & ~(TRIP_FROM_MEMORY | TRIP_MEMORY_INVALID);
    if (thermalEvents >= THERMAL_EVENTS_LOCKOUT) lockout = true;
    saveRequested = true;
  }

  state = lockout ? SisState::Locked : SisState::Tripped;
  emitEvent(EVT_TRIP, tripMask & 0xFF);
  if (lockout) emitEvent(EVT_LOCKOUT, thermalEvents);
}

// El control pide rearmar (REQ_RESET). El SIS decide si se puede.
void handleResetRequest(const ReqReset& request) {
  const bool accepted = state == SisState::Tripped             // bloqueado no se rearma así
                        && request.magic == RESET_MAGIC
                        && request.trip_mask_ack == tripMask   // el control sabe qué está rearmando
                        && permitOffMs >= COOLED_AFTER_MS      // ya se enfrió
                        && door != Door::Invalid;
  if (!accepted) {
    emitEvent(EVT_RESET_REJECTED);
    return;
  }

  tripMask = 0;
  if (latchedTripMask != 0 || memoryInvalid) {
    latchedTripMask = 0;
    memoryInvalid = false;
    saveRequested = true;
  }
  heatingMs = 0;
  state = SisState::Ok;
  emitEvent(EVT_RESET_OK);
}

// El técnico pide desbloquear (REQ_SERVICE). La solicitud sólo abre una ventana
// de 30 s: el desbloqueo exige estar frente al equipo y mover la puerta.
void handleServiceRequest(const ReqService& request) {
  if (state != SisState::Locked) return;
  if (request.magic1 != SERVICE_MAGIC1 || request.magic2 != SERVICE_MAGIC2) return;
  if (request.op != SERVICE_OP_UNLOCK) return;

  serviceActive = true;
  serviceStartMs = millis();
  serviceDoorWasOpen = false;
  serviceDoorToggles = 0;
}

void serviceStep(uint32_t now) {
  if (!serviceActive) return;
  if (state != SisState::Locked || now - serviceStartMs > SERVICE_WINDOW_MS) {
    serviceActive = false;
    return;
  }

  // Cuenta cada "abrir y volver a cerrar".
  if (door == Door::Open) serviceDoorWasOpen = true;
  if (door == Door::Closed && serviceDoorWasOpen) {
    serviceDoorWasOpen = false;
    serviceDoorToggles++;
  }

  if (serviceDoorToggles >= SERVICE_DOOR_TOGGLES && permitOffMs >= COOLED_AFTER_MS) {
    serviceActive = false;
    lockout = false;
    thermalEvents = 0;
    latchedTripMask = 0;
    saveRequested = true;
    tripMask &= ~TRIP_FROM_MEMORY;
    state = SisState::Tripped;  // queda disparado: todavía hace falta el rearme normal
    emitEvent(EVT_SERVICE_UNLOCK);
  }
}

// =============================================================================
//  Funciones de seguridad (SIF)
// =============================================================================

// Devuelve las causas de disparo que se cumplen ahora mismo (0 = ninguna).
// La puerta (SIF-02) no está aquí: no enclava, sólo quita el permiso.
uint16_t activeTripCauses(bool controlAlive) {
  uint16_t causes = 0;

  // SIF-06: el control dejó de hablar con el permiso concedido.
  if (ptcPermit && !controlAlive) causes |= TRIP_HEARTBEAT;

  // SIF-07: demasiado tiempo calentando.
  if (heatingMs > SIF07_MAX_HEAT_MS) causes |= TRIP_MAX_HEAT_TIME;

  return causes;
}

// Autotest de arranque: hace falta una puerta legible.
void selfTestStep(uint32_t now) {
  static bool failureReported = false;

  if (door != Door::Invalid) {
    if (tripMask != 0) {
      trip(tripMask);  // había un disparo guardado de antes del corte
    } else {
      state = SisState::Ok;
    }
    return;
  }
  if (now < SELFTEST_MS || failureReported) return;

  // Con la puerta ilegible no se enclava nada: se espera en ARRANQUE (con el
  // PTC sin permiso) hasta que vuelva a leerse bien.
  failureReported = true;
  emitEvent(EVT_SELFTEST_FAILED, 2);  // 2 = puerta
}

// Las ecuaciones de los tres relés (PLAN-MAESTRO §7.5).
void updateOutputs(bool controlAlive) {
  const bool ok = state == SisState::Ok;
  const bool heatRequest = controlAlive && (ctrlFlags & CTRL_HEAT_REQUEST);
  const bool cold = permitOffMs >= COOLED_AFTER_MS;  // estimado: sin termopar no se mide

  // RL1: TODAS las condiciones a la vez. heat_request sólo puede quitarlo.
  ptcPermit = ok && door == Door::Closed && heatRequest;

  // RL3: FAN_P sólo puede apagarse con todo en orden y tras enfriarse.
  fanPOffPermit = ok && !ptcPermit && cold;

  // RL4: FAN_C se fuerza ante cualquier duda.
  fanCForce = ptcPermit || !ok || !controlAlive || !cold;
}

// Una vuelta de la lógica de seguridad. Se llama con el mutex tomado.
void safetyStep(uint32_t now) {
  static uint32_t previousMs = 0;
  const uint32_t elapsed = now - previousMs;
  previousMs = now;

  const bool controlAlive = heartbeatSeen && now - lastHeartbeatMs <= HB_CTRL_TIMEOUT_MS;

  // 1. Temporizadores (sobre el permiso decidido en la vuelta anterior).
  if (ptcPermit) {
    heatingMs += elapsed;
    permitOffMs = 0;
  } else {
    permitOffMs = min(permitOffMs + elapsed, SIF07_COOL_OFF_MS);  // tope: no hace falta contar más
    if (permitOffMs >= SIF07_COOL_OFF_MS) heatingMs = 0;
  }

  // 2. Procedimiento de servicio, si está en curso.
  serviceStep(now);

  // 3. Máquina de estados.
  switch (state) {
    case SisState::Boot:
      selfTestStep(now);
      break;
    case SisState::Ok: {
      const uint16_t causes = activeTripCauses(controlAlive);
      if (causes != 0) trip(causes);
      break;
    }
    case SisState::Tripped:
    case SisState::Locked:
      tripMask |= activeTripCauses(controlAlive);  // se anotan las causas nuevas
      break;
  }

  // 4. Salidas.
  updateOutputs(controlAlive);
}

// =============================================================================
//  Tarea de seguridad (10 ms)
// =============================================================================

// Lee la puerta. Cerrada = NA en BAJO y NC en ALTO. Si los dos contactos dicen
// lo mismo (cable cortado, corto, switch dañado) la lectura es inválida, y una
// puerta inválida se trata como abierta. Un cambio se acepta tras 30 ms estable.
Door readDoor(uint32_t now) {
  static Door stable = Door::Invalid;
  static Door candidate = Door::Invalid;
  static uint32_t candidateSinceMs = 0;

  const bool noClosed = digitalRead(PIN_DOOR_NO) == LOW;
  const bool ncClosed = digitalRead(PIN_DOOR_NC) == LOW;
  Door reading = Door::Invalid;
  if (noClosed && !ncClosed) reading = Door::Closed;
  if (!noClosed && ncClosed) reading = Door::Open;

  if (reading != candidate) {
    candidate = reading;
    candidateSinceMs = now;
  } else if (now - candidateSinceMs >= DOOR_DEBOUNCE_MS) {
    stable = reading;
  }
  return stable;
}

// LED de la placa: fijo = OK, lento = arrancando, rápido = disparado,
// destello corto = bloqueado.
void updateLed(uint32_t now, SisState shownState) {
  bool on = false;
  switch (shownState) {
    case SisState::Ok:      on = true; break;
    case SisState::Boot:    on = (now / 500) % 2 == 0; break;
    case SisState::Tripped: on = (now / 100) % 2 == 0; break;
    case SisState::Locked:  on = (now / 100) % 10 == 0; break;
  }
  digitalWrite(PIN_LED, on);
}

void taskSafety(void*) {
  esp_task_wdt_add(nullptr);  // si esta tarea se cuelga, el ESP32 se reinicia (relés en reposo)
  TickType_t wakeTime = xTaskGetTickCount();

  for (;;) {
    SisState shownState;
    {
      Lock lock;
      const uint32_t now = millis();
      door = readDoor(now);
      safetyStep(now);

      digitalWrite(PIN_PTC_PERMIT, ptcPermit);
      digitalWrite(PIN_FAN_P_OFF_PERMIT, fanPOffPermit);
      digitalWrite(PIN_FAN_C_FORCE, fanCForce);
      shownState = state;
    }
    updateLed(millis(), shownState);

    esp_task_wdt_reset();
    vTaskDelayUntil(&wakeTime, pdMS_TO_TICKS(SAFETY_PERIOD_MS));
  }
}

// =============================================================================
//  Tarea del enlace con el control
// =============================================================================

HardwareSerial& controlPort = Serial2;

// Sólo tres mensajes tienen efecto. Cualquier otro se ignora, y ninguno cambia umbrales.
void handleFrame(const Frame& frame) {
  Lock lock;
  switch (frame.type) {
    case MSG_HB_CTRL: {
      HbCtrl heartbeat;
      if (!frame.as(heartbeat) || heartbeat.proto_ver != PROTO_VERSION) break;
      heartbeatSeen = true;
      lastHeartbeatMs = millis();
      ctrlFlags = heartbeat.ctrl_flags;
      break;
    }
    case MSG_REQ_RESET: {
      ReqReset request;
      if (frame.as(request)) handleResetRequest(request);
      break;
    }
    case MSG_REQ_SERVICE: {
      ReqService request;
      if (frame.as(request)) handleServiceRequest(request);
      break;
    }
  }
}

void sendHeartbeat() {
  HbSis heartbeat = {};
  {
    Lock lock;
    heartbeat.proto_ver = PROTO_VERSION;
    heartbeat.sis_state = static_cast<uint8_t>(state);
    heartbeat.trip_mask = tripMask;
    heartbeat.thermal_events = thermalEvents;
    if (door == Door::Closed) heartbeat.io_flags |= SIS_IO_DOOR_CLOSED;
    if (door == Door::Invalid) heartbeat.io_flags |= SIS_IO_DOOR_INVALID;
    if (ptcPermit) heartbeat.io_flags |= SIS_IO_PTC_PERMIT;
    if (fanPOffPermit) heartbeat.io_flags |= SIS_IO_FAN_P_OFF_PERMIT;
    if (fanCForce) heartbeat.io_flags |= SIS_IO_FAN_C_FORCED;
  }
  sendMessage(controlPort, MSG_HB_SIS, heartbeat);
}

void taskLink(void*) {
  FrameReader reader;
  Frame frame;
  uint32_t lastHeartbeatSentMs = 0;

  for (;;) {
    // 1. Lo que llega del control.
    while (reader.read(controlPort, frame)) handleFrame(frame);

    // 2. Latido HB_SIS cada 100 ms.
    if (millis() - lastHeartbeatSentMs >= HB_PERIOD_MS) {
      lastHeartbeatSentMs = millis();
      sendHeartbeat();
    }

    // 3. Avisos pendientes (también por el USB, para depurar).
    SisEventMsg event;
    while (xQueueReceive(eventQueue, &event, 0) == pdTRUE) {
      sendMessage(controlPort, MSG_EVENT, event);
      Serial.printf("[SIS] evento %u, causas 0x%04X, detalle %u\n", event.event, event.trip_mask,
                    event.detail);
    }

    // 4. Guardar en memoria si la tarea de seguridad lo pidió. Se hace aquí
    //    para que la seguridad nunca espere a la flash.
    if (saveRequested) saveMemory();

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// =============================================================================
//  Arranque
// =============================================================================

void setup() {
  // Lo primero: relés en reposo (PTC sin permiso, FAN_P encendido). Se fija el
  // nivel antes de hacer salida el pin, para que ningún relé dé un pulso.
  for (int pin : {PIN_PTC_PERMIT, PIN_FAN_P_OFF_PERMIT, PIN_FAN_C_FORCE, PIN_LED}) {
    digitalWrite(pin, LOW);
    pinMode(pin, OUTPUT);
  }
  pinMode(PIN_DOOR_NO, INPUT_PULLUP);
  pinMode(PIN_DOOR_NC, INPUT_PULLUP);

  btStop();  // sin radios: el Wi-Fi nunca se inicia en este firmware

  Serial.begin(115200);  // USB: sólo depuración
  controlPort.begin(LINK_BAUD, SERIAL_8N1, PIN_CTRL_RX, PIN_CTRL_TX);

  stateMutex = xSemaphoreCreateMutex();
  eventQueue = xQueueCreate(8, sizeof(SisEventMsg));
  loadMemory();
  Serial.printf("[SIS] arranque: estado %u, causas 0x%04X, eventos termicos %u\n",
                static_cast<unsigned>(state), tripMask, thermalEvents);

  esp_task_wdt_init(WATCHDOG_S, true);

  // La seguridad tiene la prioridad más alta. Todas en el núcleo 1.
  xTaskCreatePinnedToCore(taskSafety, "safety", 4096, nullptr, 5, nullptr, 1);
  xTaskCreatePinnedToCore(taskLink, "link", 8192, nullptr, 3, nullptr, 1);
}

void loop() {
  vTaskDelete(nullptr);  // todo ocurre en las tareas: loop() no se usa
}
