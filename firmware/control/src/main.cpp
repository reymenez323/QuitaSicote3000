// =============================================================================
//  QuitaSicote3000 — CONTROL · ESP32 DevKit V1
// =============================================================================
//  Este controlador hace SÓLO control (ADR-0012): lleva el ciclo de
//  tratamiento, regula la temperatura y maneja los ventiladores. La seguridad
//  la hace el SIS, en otra placa.
//
//  Salidas:  SSR1 (calentar el PTC) · SSR2 (ventilador de circulación, FAN_C)
//            RL2 (pedir que se apague el ventilador del PTC, FAN_P)
//  Entradas: TC1 (recámara) · TC2 (salida del PTC) · SHT31 · SGP40 · puerta
//
//  Cinco tareas de FreeRTOS:
//
//    taskCycle     cada 10 ms    puerta -> máquina de estados -> salidas
//    taskSensors   cada 250 ms   termopares; una vez por segundo, humedad y olor
//    taskSisLink   cada 10 ms    recibe HB_SIS, envía HB_CTRL y los rearmes
//    taskHmiLink   cada 10 ms    recibe las solicitudes de la pantalla, envía STATUS
//    taskConsole   cada 100 ms   comandos por USB y registro CSV
//
//  Todas comparten las variables de la sección "Estado compartido", siempre
//  con el mutex tomado (ver la clase Lock).
// =============================================================================
#include <Arduino.h>
#include <VOCGasIndexAlgorithm.h>
#include <esp_task_wdt.h>

#include "config.h"
#include "qs_protocol.h"
#include "sensors.h"

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

// --- Lecturas de los sensores (las escribe taskSensors) ---
struct Readings {
  bool tc1Valid = false;        // TC1: aire de la recámara
  bool tc2Valid = false;        // TC2: salida del PTC
  int16_t tc1X10 = 0;           // décimas de grado
  int16_t tc2X10 = 0;
  bool humidityValid = false;   // SHT31
  int16_t ambientX10 = 250;
  uint16_t humidityX10 = 500;   // décimas de %
  bool vocValid = false;        // SGP40
  bool vocLearning = true;      // el índice de olor aún se está estabilizando
  uint16_t vocIndex = 0;
} readings;

// --- Lo que informa el SIS (lo escribe taskSisLink) ---
struct SisInfo {
  bool seen = false;            // llegó al menos un HB_SIS
  uint32_t lastHeartbeatMs = 0;
  uint32_t bootSinceMs = 0;     // desde cuándo dice estar arrancando
  SisState state = SisState::Boot;
  uint16_t tripMask = 0;
  uint8_t ioFlags = 0;
  uint8_t thermalEvents = 0;
} sis;

// --- El ciclo de tratamiento (lo escribe taskCycle; taskHmiLink hace solicitudes) ---
struct Cycle {
  CycleState state = CycleState::SelfTest;
  uint32_t stateSinceMs = 0;
  uint8_t faultCode = FAULT_NONE;

  uint8_t shoe = ID_NONE;       // perfil elegido
  uint8_t intensity = ID_NONE;
  uint8_t duration = ID_NONE;
  int16_t setpointX10 = 0;      // temperatura objetivo de TC1
  uint32_t durationMs = 0;      // tiempo de tratamiento pedido
  uint32_t treatedMs = 0;       // tiempo de tratamiento ya cumplido
  uint32_t startedMs = 0;       // cuándo empezó el ciclo (para el tope global)

  PauseReason pauseReason = PauseReason::None;
  bool finishedNormally = false;  // el enfriamiento termina en COMPLETO (si no, en LISTO)
  bool rearmSent = false;         // en FALLA: ya se pidió rearmar el SIS
} cycle;

// --- Entradas y salidas del control ---
Door door = Door::Invalid;
bool ssr1On = false;            // SSR1: calentando
bool fanCOn = true;             // SSR2: ventilador de circulación
bool fanPOffRequest = false;    // RL2: se pide apagar el ventilador del PTC

// --- Enlace con el HMI ---
bool hmiSeen = false;
uint32_t lastHmiHeartbeatMs = 0;

// --- Encargos para taskSisLink ---
bool resetRequested = false;          // enviar REQ_RESET (rearme)
bool serviceUnlockRequested = false;  // enviar REQ_SERVICE (desbloqueo)

// --- Preguntas frecuentes sobre el estado (llamar con el mutex tomado) ---

bool sisAlive(uint32_t now) { return sis.seen && now - sis.lastHeartbeatMs <= HB_SIS_TIMEOUT_MS; }
bool sisIsOk(uint32_t now) { return sisAlive(now) && sis.state == SisState::Ok; }
bool sisGivesPermit(uint32_t now) { return sisAlive(now) && (sis.ioFlags & SIS_IO_PTC_PERMIT); }
bool hmiAlive(uint32_t now) { return hmiSeen && now - lastHmiHeartbeatMs <= HMI_LINK_TIMEOUT_MS; }

bool isHeating() {
  return cycle.state == CycleState::Preheat || cycle.state == CycleState::Treatment;
}
bool cycleInProgress() { return isHeating() || cycle.state == CycleState::Paused; }

// =============================================================================
//  Fallas
// =============================================================================

// Devuelve la falla más importante que está presente ahora mismo, o FAULT_NONE.
// El orden de las comprobaciones es el orden de importancia.
uint8_t activeFault(uint32_t now) {
  const bool alive = sisAlive(now);

  if (alive && sis.state == SisState::Locked) return FAULT_LOCKOUT;
  if (alive && sis.state == SisState::Tripped) return FAULT_SIS_TRIP;
  if (door == Door::Invalid) return FAULT_DOOR_INVALID;
  if (!alive) return FAULT_SIS_NO_LINK;

  // Un SIS que dice estar arrancando se tolera unos segundos, salvo calentando.
  if (sis.state == SisState::Boot) {
    const bool tooLong = now - sis.bootSinceMs >= SIS_BOOT_WAIT_MS;
    if (isHeating() || tooLong) return FAULT_SIS_NO_LINK;
  }

  if (!readings.tc1Valid) return FAULT_TC1_INVALID;
  if (!readings.tc2Valid) return FAULT_TC2_INVALID;
  return FAULT_NONE;
}

// =============================================================================
//  Cambios de estado
// =============================================================================

void enterState(CycleState newState, uint32_t now) {
  cycle.state = newState;
  cycle.stateSinceMs = now;
  if (newState != CycleState::Paused) cycle.pauseReason = PauseReason::None;
}

void enterReady(uint32_t now) {
  cycle.shoe = cycle.intensity = cycle.duration = ID_NONE;
  cycle.faultCode = FAULT_NONE;
  cycle.treatedMs = 0;
  enterState(CycleState::Ready, now);
}

void enterPause(PauseReason reason, uint32_t now) {
  enterState(CycleState::Paused, now);
  cycle.pauseReason = reason;
}

// finishedNormally: el tratamiento se completó (acabará en COMPLETO).
// Si fue una cancelación o una falla, tras enfriar se vuelve a LISTO.
void enterCooling(bool finishedNormally, uint32_t now) {
  cycle.finishedNormally = finishedNormally;
  enterState(CycleState::Cooling, now);
}

void enterFault(uint8_t faultCode, uint32_t now) {
  cycle.faultCode = faultCode;
  cycle.rearmSent = false;
  enterState(CycleState::Fault, now);
}

// =============================================================================
//  Solicitudes del HMI
// =============================================================================
//  La pantalla sólo pide; aquí se decide. Todas se llaman con el mutex tomado.

uint8_t requestStart(const ReqStart& request, uint32_t now) {
  const bool validIds = request.shoe_id < SHOE_COUNT && request.intensity_id < INTENSITY_COUNT &&
                        request.duration_id < DURATION_COUNT;
  if (!validIds) return START_INVALID_ID;
  if (cycle.state == CycleState::Fault) return START_FAULT_ACTIVE;
  if (cycle.state != CycleState::Ready || !sisIsOk(now)) return START_NOT_READY;
  if (door != Door::Closed) return START_DOOR_OPEN;

  cycle.shoe = request.shoe_id;
  cycle.intensity = request.intensity_id;
  cycle.duration = request.duration_id;
  cycle.setpointX10 = SETPOINT_C[request.shoe_id][request.intensity_id] * 10;
  cycle.durationMs = DURATION_MIN[request.duration_id] * 60000u;
  cycle.treatedMs = 0;
  cycle.startedMs = now;
  enterState(CycleState::Preheat, now);
  return START_OK;
}

void requestPause(uint32_t now) {
  if (isHeating()) enterPause(PauseReason::User, now);
}

// Cerrar la puerta no basta para seguir: hace falta esta solicitud.
void requestResume(uint32_t now) {
  if (cycle.state == CycleState::Paused && door == Door::Closed && sisIsOk(now)) {
    enterState(CycleState::Preheat, now);  // conserva el tiempo de tratamiento ya cumplido
  }
}

void requestCancel(uint32_t now) {
  if (cycleInProgress()) enterCooling(false, now);
}

// "Aceptar": en COMPLETO vuelve a LISTO; en FALLA sólo vale con la causa resuelta.
void requestAck(uint32_t now) {
  if (cycle.state == CycleState::Complete) enterReady(now);
  if (cycle.state == CycleState::Fault && activeFault(now) == FAULT_NONE) enterCooling(false, now);
}

// "Rearmar": se le pide al SIS, que es quien decide si se puede.
void requestRearm(uint32_t now) {
  if (cycle.state == CycleState::Fault && sisAlive(now) && sis.state == SisState::Tripped) {
    cycle.rearmSent = true;
    resetRequested = true;
  }
}

// =============================================================================
//  Máquina de estados del ciclo
// =============================================================================
//
//   AUTOTEST -> LISTO -> PRECALENTAMIENTO -> TRATAMIENTO -> ENFRIAMIENTO -> COMPLETO -> LISTO
//                              ^      \          /
//                              |       v        v
//                              +----- PAUSA (máx. 5 min)
//
//   Desde cualquier estado, una falla lleva a FALLA. Tras aceptarla: ENFRIAMIENTO -> LISTO.

// Lo que se comprueba en cada vuelta mientras se calienta.
void heatingStep(uint32_t now, uint32_t elapsed) {
  const uint32_t timeInState = now - cycle.stateSinceMs;
  const bool atTemperature = readings.tc1X10 >= cycle.setpointX10 - TREAT_BAND_X10;

  if (!hmiAlive(now)) {  // sin pantalla nadie puede parar el equipo: se cancela
    Serial.println("[CTRL] sin enlace con el HMI: ciclo cancelado");
    enterCooling(false, now);
  } else if (door != Door::Closed) {
    enterPause(PauseReason::Door, now);
  } else if (now - cycle.startedMs >= CYCLE_MAX_MS) {
    Serial.println("[CTRL] tope de tiempo del ciclo");
    enterCooling(true, now);
  } else if (readings.tc1X10 > cycle.setpointX10 + OVERTEMP_MARGIN_X10) {
    // Al dejar de calentar se retira heat_request y el SIS abre RL1.
    enterFault(FAULT_SOFT_OVERTEMP, now);
  } else if (cycle.state == CycleState::Preheat) {
    if (atTemperature) {
      enterState(CycleState::Treatment, now);
    } else if (timeInState >= PREHEAT_TIMEOUT_MS) {
      enterFault(FAULT_PREHEAT_TIMEOUT, now);
    }
  } else {  // TRATAMIENTO: el tiempo sólo cuenta estando a temperatura
    if (atTemperature) cycle.treatedMs += elapsed;
    if (cycle.treatedMs >= cycle.durationMs) enterCooling(true, now);
  }
}

// Una vuelta de la máquina de estados. Se llama con el mutex tomado.
void cycleStep(uint32_t now, uint32_t elapsed, bool doorJustOpened) {
  const uint32_t timeInState = now - cycle.stateSinceMs;
  const uint8_t fault = activeFault(now);

  // FALLA se trata aparte: es el único estado del que no se sale por otra falla.
  if (cycle.state == CycleState::Fault) {
    if (fault != FAULT_NONE) cycle.faultCode = fault;  // se muestra la causa presente más importante
    // Un rearme que el SIS aceptó cuenta como "falla aceptada".
    if (fault == FAULT_NONE && cycle.rearmSent && sis.state == SisState::Ok) enterCooling(false, now);
    return;
  }

  // AUTOTEST: se da un plazo a los sensores y al SIS para arrancar.
  if (cycle.state == CycleState::SelfTest) {
    const bool sisAlreadyTripped = fault == FAULT_LOCKOUT || fault == FAULT_SIS_TRIP;
    if (fault == FAULT_NONE && sis.state == SisState::Ok) {
      enterReady(now);
    } else if (sisAlreadyTripped || (fault != FAULT_NONE && timeInState >= SIS_BOOT_WAIT_MS)) {
      enterFault(fault, now);
    }
    return;
  }

  // En el resto de estados, cualquier falla presente lleva a FALLA.
  if (fault != FAULT_NONE) {
    enterFault(fault, now);
    return;
  }

  switch (cycle.state) {
    case CycleState::Ready:
      break;  // espera a que el HMI pida iniciar (requestStart)

    case CycleState::Preheat:
    case CycleState::Treatment:
      heatingStep(now, elapsed);
      break;

    case CycleState::Paused:
      if (!hmiAlive(now) || timeInState >= PAUSE_MAX_MS) enterCooling(false, now);
      break;

    case CycleState::Cooling: {
      const bool cool = readings.tc1X10 < COOL_END_TC1_X10 && readings.tc2X10 < COOL_END_TC2_X10;
      if (cool || timeInState >= COOL_MAX_MS) {
        if (cycle.finishedNormally) {
          enterState(CycleState::Complete, now);
        } else {
          enterReady(now);
        }
      }
      break;
    }

    case CycleState::Complete:
      if (doorJustOpened) enterReady(now);  // abrir la puerta equivale a "Aceptar"
      break;

    default:
      break;
  }
}

// =============================================================================
//  Salidas: calefactor y ventiladores
// =============================================================================

// Regulación por histéresis sobre TC1, con tiempo mínimo en cada estado.
void updateHeater(uint32_t now) {
  static bool wanted = false;        // la regulación quiere calor
  static uint32_t changedMs = 0;
  static bool tc2TooHot = false;     // límite de proceso sobre la salida del PTC

  if (!isHeating()) {
    wanted = false;
    tc2TooHot = false;
    ssr1On = false;
    return;
  }

  bool wantNow = wanted;
  if (readings.tc1X10 < cycle.setpointX10 - HYSTERESIS_X10) wantNow = true;
  if (readings.tc1X10 > cycle.setpointX10 + HYSTERESIS_X10) wantNow = false;
  if (wantNow != wanted && now - changedMs >= HEATER_MIN_STATE_MS) {
    wanted = wantNow;
    changedMs = now;
  }

  if (readings.tc2X10 > TC2_MAX_X10) tc2TooHot = true;
  if (readings.tc2X10 < TC2_RESUME_X10) tc2TooHot = false;

  // SSR1 sólo conduce con la puerta cerrada y el permiso del SIS concedido.
  ssr1On = wanted && !tc2TooHot && door == Door::Closed && sisGivesPermit(now);
}

// Los ventiladores están encendidos siempre, salvo en reposo (LISTO o COMPLETO)
// y con el equipo frío desde hace un minuto. FAN_P nunca se apaga calentando.
void updateFans(uint32_t now) {
  static bool coldTimerRunning = false;
  static uint32_t coldSinceMs = 0;

  const bool resting = cycle.state == CycleState::Ready || cycle.state == CycleState::Complete;
  const bool cold = readings.tc2Valid && readings.tc2X10 < COLD_X10;

  if (!resting || !cold) {
    coldTimerRunning = false;
  } else if (!coldTimerRunning) {
    coldTimerRunning = true;
    coldSinceMs = now;
  }
  const bool fansCanStop = coldTimerRunning && now - coldSinceMs >= FAN_OFF_HOLD_MS;

  fanCOn = !fansCanStop;
  fanPOffRequest = fansCanStop;  // el SIS además debe permitirlo con su relé RL3
}

// =============================================================================
//  Tarea del ciclo (10 ms)
// =============================================================================

// Lee la puerta. Cerrada = NA en BAJO y NC en ALTO. Si los dos contactos dicen
// lo mismo (cable cortado, corto, switch dañado) la lectura es inválida.
// Un cambio se acepta tras 30 ms estable.
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

void taskCycle(void*) {
  esp_task_wdt_add(nullptr);  // si esta tarea se cuelga, el ESP32 se reinicia
  TickType_t wakeTime = xTaskGetTickCount();
  uint32_t previousMs = millis();

  for (;;) {
    {
      Lock lock;
      const uint32_t now = millis();
      const uint32_t elapsed = now - previousMs;
      previousMs = now;

      const Door previousDoor = door;
      door = readDoor(now);
      const bool doorJustOpened = previousDoor == Door::Closed && door == Door::Open;

      cycleStep(now, elapsed, doorJustOpened);
      updateHeater(now);
      updateFans(now);

      digitalWrite(PIN_SSR1, ssr1On);
      digitalWrite(PIN_FAN_C, fanCOn);
      digitalWrite(PIN_FAN_P_OFF, fanPOffRequest);
      digitalWrite(PIN_LED, ssr1On);  // el LED de la placa acompaña al calefactor
    }
    esp_task_wdt_reset();
    vTaskDelayUntil(&wakeTime, pdMS_TO_TICKS(CYCLE_PERIOD_MS));
  }
}

// =============================================================================
//  Tarea de los sensores
// =============================================================================

VOCGasIndexAlgorithm vocAlgorithm;  // algoritmo de Sensirion: señal cruda -> índice 1–500

// Lee un termopar y actualiza su lectura. Una lectura mala suelta se tolera
// (se conserva la anterior); tres seguidas dejan el sensor como inválido.
void updateThermocouple(int csPin, bool& valid, int16_t& valueX10, uint8_t& badReads) {
  int16_t tempX10 = 0;
  const bool readOk = readThermocouple(csPin, tempX10);  // sin el mutex: el SPI tarda

  Lock lock;
  if (readOk) {
    badReads = 0;
    valid = true;
    valueX10 = tempX10;
  } else {
    if (badReads < SENSOR_BAD_READS) badReads++;
    if (badReads >= SENSOR_BAD_READS) valid = false;
  }
}

// Humedad y olor, una vez por segundo.
void updateAirQuality() {
  static uint8_t humidityBadReads = 0;
  static uint8_t vocBadReads = 0;
  static uint16_t vocSamples = 0;

  int16_t ambientX10 = 250;     // sin SHT31, el SGP40 se compensa con 25 °C y 50 %
  uint16_t humidityX10 = 500;
  const bool humidityOk = readSht31(ambientX10, humidityX10);
  uint16_t rawVoc = 0;
  const bool vocOk = readSgp40(ambientX10, humidityX10, rawVoc);
  const int32_t vocIndex = vocOk ? vocAlgorithm.process(rawVoc) : 0;

  Lock lock;
  if (humidityOk) {
    humidityBadReads = 0;
    readings.humidityValid = true;
    readings.ambientX10 = ambientX10;
    readings.humidityX10 = humidityX10;
  } else if (++humidityBadReads >= SENSOR_BAD_READS) {
    humidityBadReads = SENSOR_BAD_READS;
    readings.humidityValid = false;
  }

  if (vocOk) {
    vocBadReads = 0;
    readings.vocValid = true;
    readings.vocIndex = vocIndex;
    if (vocSamples < 60) vocSamples++;
    readings.vocLearning = vocSamples < 60 || vocIndex == 0;  // el primer minuto no es fiable
  } else if (++vocBadReads >= SENSOR_BAD_READS) {
    vocBadReads = SENSOR_BAD_READS;
    readings.vocValid = false;
  }
}

void taskSensors(void*) {
  static uint8_t tc1BadReads = 0;
  static uint8_t tc2BadReads = 0;
  TickType_t wakeTime = xTaskGetTickCount();

  // Cada 250 ms se lee un termopar, alternando (cada uno queda leído cada 500 ms).
  for (uint32_t tick = 0;; tick++) {
    vTaskDelayUntil(&wakeTime, pdMS_TO_TICKS(TC_PERIOD_MS));

    if (tick % 2 == 0) {
      updateThermocouple(PIN_CS_TC1, readings.tc1Valid, readings.tc1X10, tc1BadReads);
    } else {
      updateThermocouple(PIN_CS_TC2, readings.tc2Valid, readings.tc2X10, tc2BadReads);
    }
    if (tick % 4 == 0) updateAirQuality();  // una vez por segundo
  }
}

// =============================================================================
//  Tarea del enlace con el SIS
// =============================================================================

HardwareSerial& sisPort = Serial2;

void handleSisFrame(const Frame& frame) {
  if (frame.type == MSG_HB_SIS) {
    HbSis heartbeat;
    if (!frame.as(heartbeat) || heartbeat.proto_ver != PROTO_VERSION) return;
    if (heartbeat.sis_state > static_cast<uint8_t>(SisState::Locked)) return;

    Lock lock;
    const uint32_t now = millis();
    const SisState newState = static_cast<SisState>(heartbeat.sis_state);
    const bool justEnteredBoot = newState == SisState::Boot && (!sis.seen || sis.state != SisState::Boot);
    if (justEnteredBoot) sis.bootSinceMs = now;

    sis.seen = true;
    sis.lastHeartbeatMs = now;
    sis.state = newState;
    sis.tripMask = heartbeat.trip_mask;
    sis.ioFlags = heartbeat.io_flags;
    sis.thermalEvents = heartbeat.thermal_events;
  } else if (frame.type == MSG_EVENT) {
    SisEventMsg event;
    if (frame.as(event)) {
      Serial.printf("[CTRL] aviso del SIS: evento %u, causas 0x%04X, detalle %u\n", event.event,
                    event.trip_mask, event.detail);
    }
  }
}

// Latido hacia el SIS. heat_request es lo único que el SIS usa para conceder
// el permiso (y sólo si todas SUS condiciones se cumplen).
void sendControlHeartbeat() {
  HbCtrl heartbeat = {};
  {
    Lock lock;
    heartbeat.proto_ver = PROTO_VERSION;
    heartbeat.cycle_state = static_cast<uint8_t>(cycle.state);
    if (isHeating()) heartbeat.ctrl_flags |= CTRL_HEAT_REQUEST;
    if (fanPOffRequest) heartbeat.ctrl_flags |= CTRL_FAN_P_OFF_CMD;
    if (fanCOn) heartbeat.ctrl_flags |= CTRL_FAN_C_CMD;
    if (ssr1On) heartbeat.ctrl_flags |= CTRL_SSR1_CMD;
  }
  sendMessage(sisPort, MSG_HB_CTRL, heartbeat);
}

// Envía al SIS los encargos pendientes: rearme y desbloqueo de servicio.
void sendPendingSisRequests() {
  bool sendReset = false;
  bool sendService = false;
  uint16_t tripMask = 0;
  {
    Lock lock;
    sendReset = resetRequested;
    sendService = serviceUnlockRequested;
    tripMask = sis.tripMask;
    resetRequested = false;
    serviceUnlockRequested = false;
  }
  if (sendReset) {
    ReqReset request = {RESET_MAGIC, tripMask};  // el SIS comprueba que la máscara coincida
    sendMessage(sisPort, MSG_REQ_RESET, request);
  }
  if (sendService) {
    ReqService request = {SERVICE_MAGIC1, SERVICE_MAGIC2, SERVICE_OP_UNLOCK};
    sendMessage(sisPort, MSG_REQ_SERVICE, request);
  }
}

void taskSisLink(void*) {
  FrameReader reader;
  Frame frame;
  uint32_t lastSentMs = 0;

  for (;;) {
    while (reader.read(sisPort, frame)) handleSisFrame(frame);

    if (millis() - lastSentMs >= HB_PERIOD_MS) {
      lastSentMs = millis();
      sendControlHeartbeat();
    }
    sendPendingSisRequests();

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// =============================================================================
//  Tarea del enlace con el HMI
// =============================================================================

HardwareSerial& hmiPort = Serial1;

void handleHmiFrame(const Frame& frame) {
  // REQ_START es la única solicitud con respuesta.
  if (frame.type == MSG_REQ_START) {
    ReqStart request;
    if (!frame.as(request)) return;
    RespStart response;
    {
      Lock lock;
      response.result = requestStart(request, millis());
    }
    sendMessage(hmiPort, MSG_RESP_START, response);
    return;
  }

  Lock lock;
  const uint32_t now = millis();
  switch (frame.type) {
    case MSG_HMI_HB: {
      HmiHb heartbeat;
      if (frame.as(heartbeat) && heartbeat.proto_ver == PROTO_VERSION) {
        hmiSeen = true;
        lastHmiHeartbeatMs = now;
      }
      break;
    }
    case MSG_REQ_PAUSE:  requestPause(now);  break;
    case MSG_REQ_RESUME: requestResume(now); break;
    case MSG_REQ_CANCEL: requestCancel(now); break;
    case MSG_REQ_ACK:    requestAck(now);    break;
    case MSG_REQ_REARM:  requestRearm(now);  break;
  }
}

// Todo lo que la pantalla muestra sale de este mensaje.
void sendStatus() {
  Status status = {};
  {
    Lock lock;
    const uint32_t now = millis();
    const bool alive = sisAlive(now);

    status.proto_ver = PROTO_VERSION;
    status.cycle_state = static_cast<uint8_t>(cycle.state);
    status.door = static_cast<uint8_t>(door);
    status.fault_code = cycle.faultCode;

    // Sin enlace con el SIS no se repite su último estado conocido.
    status.sis_state = static_cast<uint8_t>(alive ? sis.state : SisState::Boot);
    status.trip_mask = alive ? sis.tripMask : 0;
    status.thermal_events = sis.thermalEvents;

    status.shoe_id = cycle.shoe;
    status.intensity_id = cycle.intensity;
    status.duration_id = cycle.duration;
    status.pause_reason = static_cast<uint8_t>(cycle.pauseReason);
    if (cycle.shoe != ID_NONE) status.total_s = cycle.durationMs / 1000;
    if (cycleInProgress() && cycle.treatedMs < cycle.durationMs) {
      status.remaining_s = (cycle.durationMs - cycle.treatedMs + 999) / 1000;
    }
    const uint32_t timeInState = now - cycle.stateSinceMs;
    if (cycle.state == CycleState::Paused && timeInState < PAUSE_MAX_MS) {
      status.pause_left_s = (PAUSE_MAX_MS - timeInState + 999) / 1000;
    }

    // Una lectura inválida viaja como "sin dato": la pantalla no inventa valores.
    status.t_chamber_x10 = readings.tc1Valid ? readings.tc1X10 : TEMP_INVALID;
    status.rh_x10 = readings.humidityValid ? readings.humidityX10 : U16_INVALID;
    status.voc_index = readings.vocValid ? readings.vocIndex : U16_INVALID;
    if (!readings.humidityValid) status.warn_flags |= WARN_SHT31;
    if (!readings.vocValid) status.warn_flags |= WARN_SGP40;
    if (readings.vocLearning) status.warn_flags |= WARN_VOC_LEARNING;

    // Estado real de los actuadores: la orden del control combinada con los relés del SIS.
    const bool fanPOffAllowed = alive && (sis.ioFlags & SIS_IO_FAN_P_OFF_PERMIT);
    const bool fanCForced = alive && (sis.ioFlags & SIS_IO_FAN_C_FORCED);
    if (ssr1On) status.act_flags |= ACT_PTC;
    if (fanCOn || fanCForced) status.act_flags |= ACT_FAN_C;
    if (!(fanPOffRequest && fanPOffAllowed)) status.act_flags |= ACT_FAN_P;
  }
  sendMessage(hmiPort, MSG_STATUS, status);
}

void taskHmiLink(void*) {
  FrameReader reader;
  Frame frame;
  uint32_t lastSentMs = 0;

  for (;;) {
    while (reader.read(hmiPort, frame)) handleHmiFrame(frame);

    if (millis() - lastSentMs >= STATUS_PERIOD_MS) {
      lastSentMs = millis();
      sendStatus();
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// =============================================================================
//  Tarea de la consola (USB): comandos y registro CSV
// =============================================================================
//  Comandos (115200 baudios, terminados en Enter):
//    status                 muestra el estado actual
//    log on | log off       registro CSV permanente (durante un ciclo se registra siempre)
//    servicio desbloquear   pide al SIS abrir la ventana de desbloqueo

bool logAlways = false;

// Convierte décimas a texto: 453 -> "45.3". Sin dato, devuelve un campo vacío.
String tenths(bool valid, int value) {
  if (!valid) return "";
  char text[16];
  snprintf(text, sizeof(text), "%d.%d", value / 10, abs(value % 10));
  return text;
}

// Una línea CSV por segundo, con estas columnas:
const char CSV_HEADER[] =
    "t_ms,state,sp,tc1,tc2,rh,voc,ssr1,permit,fan_c,fan_p_off,door,sis_state,trip_mask,fault";

void printCsvLine() {
  char line[160];
  {
    // La línea se arma con el mutex tomado y se envía después de soltarlo,
    // para que un puerto USB lento nunca frene a las demás tareas.
    Lock lock;
    const uint32_t now = millis();

    snprintf(line, sizeof(line), "%lu,%u,%s,%s,%s,%s,%s,%d,%d,%d,%d,%u,%u,0x%04X,%u",
             static_cast<unsigned long>(now),
             static_cast<unsigned>(cycle.state),
             tenths(cycleInProgress(), cycle.setpointX10).c_str(),
             tenths(readings.tc1Valid, readings.tc1X10).c_str(),
             tenths(readings.tc2Valid, readings.tc2X10).c_str(),
             tenths(readings.humidityValid, readings.humidityX10).c_str(),
             readings.vocValid ? String(readings.vocIndex).c_str() : "",
             ssr1On, sisGivesPermit(now), fanCOn, fanPOffRequest,
             static_cast<unsigned>(door),
             static_cast<unsigned>(sis.state),
             sis.tripMask,
             cycle.faultCode);
  }
  Serial.println(line);
}

void runCommand(const String& command) {
  if (command == "status") {
    Serial.println(CSV_HEADER);
    printCsvLine();
  } else if (command == "log on") {
    logAlways = true;
  } else if (command == "log off") {
    logAlways = false;
  } else if (command == "servicio desbloquear") {
    Lock lock;
    serviceUnlockRequested = true;
    Serial.println("Solicitud enviada. Abre y cierra la puerta 3 veces en 30 s.");
  } else if (command.length() > 0) {
    Serial.println("Comandos: status | log on | log off | servicio desbloquear");
  }
}

void taskConsole(void*) {
  bool wasLogging = false;
  uint32_t lastLogMs = 0;

  for (;;) {
    if (Serial.available() > 0) {
      String command = Serial.readStringUntil('\n');
      command.trim();
      runCommand(command);
    }

    bool logging;
    {
      Lock lock;
      logging = logAlways || cycleInProgress();
    }
    if (logging && millis() - lastLogMs >= 1000) {
      lastLogMs = millis();
      if (!wasLogging) Serial.println(CSV_HEADER);
      printCsvLine();
    }
    wasLogging = logging;

    vTaskDelay(pdMS_TO_TICKS(100));
  }
}

// =============================================================================
//  Arranque
// =============================================================================

void setup() {
  // Lo primero: salidas en reposo (PTC apagado, FAN_P encendido). Se fija el
  // nivel antes de hacer salida el pin, para que nada dé un pulso.
  for (int pin : {PIN_SSR1, PIN_FAN_C, PIN_FAN_P_OFF, PIN_LED}) {
    digitalWrite(pin, LOW);
    pinMode(pin, OUTPUT);
  }
  for (int pin : {PIN_CS_TC1, PIN_CS_TC2}) {
    digitalWrite(pin, HIGH);
    pinMode(pin, OUTPUT);
  }
  pinMode(PIN_DOOR_NO, INPUT_PULLUP);
  pinMode(PIN_DOOR_NC, INPUT_PULLUP);

  btStop();  // sin radios: el Wi-Fi nunca se inicia en este firmware

  Serial.begin(115200);  // USB: consola y registro
  Serial.setTimeout(50);
  SPI.begin(PIN_SPI_SCK, PIN_SPI_MISO, -1, -1);
  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, 100000);
  Wire.setTimeOut(20);  // un sensor ausente no puede colgar la tarea
  sisPort.begin(LINK_BAUD, SERIAL_8N1, PIN_SIS_RX, PIN_SIS_TX);
  hmiPort.begin(LINK_BAUD, SERIAL_8N1, PIN_HMI_RX, PIN_HMI_TX);

  Serial.printf("[CTRL] arranque. Autotest del SGP40: %s\n", selfTestSgp40() ? "correcto" : "FALLO");

  stateMutex = xSemaphoreCreateMutex();
  esp_task_wdt_init(WATCHDOG_S, true);

  // Tras un reinicio o un corte de energía siempre se empieza en AUTOTEST:
  // un ciclo interrumpido nunca se reanuda solo.
  xTaskCreatePinnedToCore(taskCycle, "cycle", 4096, nullptr, 5, nullptr, 1);
  xTaskCreatePinnedToCore(taskSisLink, "sis", 4096, nullptr, 4, nullptr, 1);
  xTaskCreatePinnedToCore(taskHmiLink, "hmi", 4096, nullptr, 3, nullptr, 1);
  xTaskCreatePinnedToCore(taskSensors, "sensors", 4096, nullptr, 2, nullptr, 1);
  xTaskCreatePinnedToCore(taskConsole, "console", 4096, nullptr, 1, nullptr, 1);
}

void loop() {
  vTaskDelete(nullptr);  // todo ocurre en las tareas: loop() no se usa
}
