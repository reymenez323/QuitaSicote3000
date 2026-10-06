// =============================================================================
//  QuitaSicote3000 — Protocolo de comunicación (común a control, SIS y HMI)
// =============================================================================
//  Este archivo es el "contrato" entre los tres nodos. Contiene:
//    1. Los identificadores compartidos (estados, fallas, causas de disparo).
//    2. Los mensajes (qué bytes viajan en cada uno).
//    3. Cómo se arma, se envía y se recibe una trama.
//
//  Formato de una trama:
//
//      0xAA | LEN | TYPE | SEQ | PAYLOAD (LEN bytes) | CRC16 (bajo, alto)
//
//  El CRC (CRC-16/CCITT-FALSE) cubre LEN, TYPE, SEQ y el payload.
//  Especificación completa: docs/PLAN-MAESTRO.md §6.
// =============================================================================
#pragma once
#include <Arduino.h>

namespace qs {

// -----------------------------------------------------------------------------
//  1. Identificadores compartidos
// -----------------------------------------------------------------------------

constexpr uint8_t PROTO_VERSION = 1;
constexpr uint32_t LINK_BAUD = 115200;

// Periodos y tiempos de espera de los enlaces (ms)
constexpr uint32_t HB_PERIOD_MS = 100;          // HB_CTRL y HB_SIS
constexpr uint32_t HB_SIS_TIMEOUT_MS = 500;     // el control da por perdido al SIS
constexpr uint32_t HB_CTRL_TIMEOUT_MS = 2000;   // el SIS da por perdido al control
constexpr uint32_t STATUS_PERIOD_MS = 200;      // STATUS del control al HMI
constexpr uint32_t STATUS_TIMEOUT_MS = 2000;    // el HMI muestra "Sin comunicación"
constexpr uint32_t HMI_HB_PERIOD_MS = 500;      // latido del HMI
constexpr uint32_t HMI_LINK_TIMEOUT_MS = 10000; // el control cancela el ciclo

// Estado del ciclo (lo decide el control)
enum class CycleState : uint8_t {
  SelfTest = 0,   // AUTOTEST
  Ready = 1,      // LISTO
  Preheat = 2,    // PRECALENTAMIENTO
  Treatment = 3,  // TRATAMIENTO
  Paused = 4,     // PAUSA
  Cooling = 5,    // ENFRIAMIENTO
  Complete = 6,   // COMPLETO
  Fault = 7,      // FALLA
};

// Estado del SIS
enum class SisState : uint8_t {
  Boot = 0,     // ARRANQUE
  Ok = 1,
  Tripped = 2,  // DISPARADO: hace falta rearmar
  Locked = 3,   // BLOQUEADO: hace falta servicio técnico
};

enum class Door : uint8_t { Closed = 0, Open = 1, Invalid = 2 };

enum class PauseReason : uint8_t { None = 0, Door = 1, User = 2 };

// Perfiles: el HMI sólo envía estos números, nunca temperaturas ni minutos.
constexpr uint8_t SHOE_COUNT = 4;       // 0 Cuero, 1 Deportivo, 2 Bota, 3 Sintético
constexpr uint8_t INTENSITY_COUNT = 3;  // 0 Suave, 1 Media, 2 Intensa
constexpr uint8_t DURATION_COUNT = 3;   // 0 Corta, 1 Media, 2 Larga
constexpr uint8_t ID_NONE = 0xFF;       // sin ciclo

// Códigos de falla del control
enum FaultCode : uint8_t {
  FAULT_NONE = 0,
  FAULT_TC1_INVALID = 1,      // sensor de la recámara
  FAULT_TC2_INVALID = 2,      // sensor del calefactor
  FAULT_PREHEAT_TIMEOUT = 5,  // no alcanzó la temperatura
  FAULT_SOFT_OVERTEMP = 6,    // temperatura demasiado alta
  FAULT_SIS_NO_LINK = 7,      // sin comunicación con el SIS
  FAULT_SIS_TRIP = 8,         // el SIS disparó
  FAULT_DOOR_INVALID = 10,    // sensor de la puerta
  FAULT_LOCKOUT = 13,         // el SIS está bloqueado
};

// Causas de disparo del SIS (bits de trip_mask)
enum TripBit : uint16_t {
  TRIP_OVERTEMP = 1u << 0,         // SIF-01: TC3 por encima del límite
  TRIP_FAST_RISE = 1u << 2,        // SIF-03: TC3 sube demasiado rápido (sin flujo de aire)
  TRIP_SENSOR = 1u << 3,           // SIF-04: TC3 inválido
  TRIP_HEARTBEAT = 1u << 5,        // SIF-06: el control dejó de hablar mientras calentaba
  TRIP_MAX_HEAT_TIME = 1u << 6,    // SIF-07: demasiado tiempo calentando
  TRIP_NO_EFFECT = 1u << 7,        // SIF-08: se calienta pero TC3 baja (termostato abierto)
  TRIP_MEMORY_INVALID = 1u << 11,  // los datos guardados están corruptos
  TRIP_FROM_MEMORY = 1u << 15,     // el disparo viene de antes de un corte de energía
};
// Los "eventos térmicos" se cuentan y se guardan: al segundo, el SIS se bloquea.
constexpr uint16_t TRIP_THERMAL_MASK = TRIP_OVERTEMP | TRIP_FAST_RISE | TRIP_NO_EFFECT;

// Bits de HbCtrl.ctrl_flags (informativos: el SIS sólo los usa para restringir)
enum CtrlFlag : uint8_t {
  CTRL_HEAT_REQUEST = 1u << 0,   // el control quiere calentar
  CTRL_FAN_P_OFF_CMD = 1u << 1,  // el control pide apagar el ventilador del PTC
  CTRL_FAN_C_CMD = 1u << 2,      // el control enciende el ventilador de circulación
  CTRL_SSR1_CMD = 1u << 3,       // el control está ordenando conducir al SSR1
};

// Bits de HbSis.io_flags
enum SisIoFlag : uint8_t {
  SIS_IO_DOOR_CLOSED = 1u << 0,
  SIS_IO_DOOR_INVALID = 1u << 1,
  SIS_IO_PTC_PERMIT = 1u << 2,        // RL1 cerrado: el PTC puede calentar
  SIS_IO_FAN_P_OFF_PERMIT = 1u << 3,  // RL3: se permite apagar el ventilador del PTC
  SIS_IO_FAN_C_FORCED = 1u << 4,      // RL4: ventilador de circulación forzado
};

// Bits de Status.warn_flags y Status.act_flags
enum WarnFlag : uint8_t { WARN_SHT31 = 1u << 0, WARN_SGP40 = 1u << 1, WARN_VOC_LEARNING = 1u << 2 };
enum ActFlag : uint8_t { ACT_PTC = 1u << 0, ACT_FAN_C = 1u << 1, ACT_FAN_P = 1u << 2 };

// Eventos que el SIS avisa al control
enum SisEvent : uint8_t {
  EVT_TRIP = 1,
  EVT_RESET_OK = 2,
  EVT_RESET_REJECTED = 3,
  EVT_SELFTEST_FAILED = 4,
  EVT_LOCKOUT = 5,
  EVT_SERVICE_UNLOCK = 6,
};

// Respuesta del control a "Iniciar"
enum StartResult : uint8_t {
  START_OK = 0,
  START_DOOR_OPEN = 1,
  START_NOT_READY = 2,
  START_INVALID_ID = 3,
  START_FAULT_ACTIVE = 4,
};

// Valores que significan "sin dato"
constexpr int16_t TEMP_INVALID = INT16_MIN;
constexpr uint16_t U16_INVALID = UINT16_MAX;

// Números mágicos de las solicitudes delicadas
constexpr uint8_t RESET_MAGIC = 0x5A;
constexpr uint8_t SERVICE_MAGIC1 = 0xC3;
constexpr uint8_t SERVICE_MAGIC2 = 0x3C;
constexpr uint8_t SERVICE_OP_UNLOCK = 1;

// -----------------------------------------------------------------------------
//  2. Mensajes
// -----------------------------------------------------------------------------

enum MsgType : uint8_t {
  // Control <-> SIS
  MSG_HB_CTRL = 0x01,      // Control -> SIS, cada 100 ms        (HbCtrl)
  MSG_HB_SIS = 0x02,       // SIS -> Control, cada 100 ms        (HbSis)
  MSG_REQ_RESET = 0x03,    // Control -> SIS: rearmar            (ReqReset)
  MSG_EVENT = 0x04,        // SIS -> Control: aviso              (SisEventMsg)
  MSG_REQ_SERVICE = 0x05,  // Control -> SIS: desbloqueo         (ReqService)
  // HMI -> Control
  MSG_HMI_HB = 0x10,       // cada 500 ms                        (HmiHb)
  MSG_REQ_START = 0x11,    // iniciar un ciclo                   (ReqStart)
  MSG_REQ_PAUSE = 0x12,    // sin payload
  MSG_REQ_RESUME = 0x13,   // sin payload
  MSG_REQ_CANCEL = 0x14,   // sin payload
  MSG_REQ_ACK = 0x15,      // aceptar "Completo" o una falla     (sin payload)
  MSG_REQ_REARM = 0x16,    // rearmar el SIS                     (sin payload)
  // Control -> HMI
  MSG_STATUS = 0x20,       // cada 200 ms                        (Status)
  MSG_RESP_START = 0x21,   // respuesta a REQ_START              (RespStart)
};

#define QS_PACKED __attribute__((packed))

struct QS_PACKED HbCtrl {
  uint8_t proto_ver;
  uint8_t cycle_state;  // CycleState
  uint8_t ctrl_flags;   // CtrlFlag
  uint8_t reserved;
};

struct QS_PACKED HbSis {
  uint8_t proto_ver;
  uint8_t sis_state;       // SisState
  uint16_t trip_mask;      // TripBit
  int16_t tc3_q2;          // TC3 en cuartos de grado (68 °C = 272)
  uint8_t io_flags;        // SisIoFlag
  uint8_t thermal_events;  // eventos térmicos guardados
  uint8_t reserved;
};

struct QS_PACKED ReqReset {
  uint8_t magic;           // RESET_MAGIC
  uint16_t trip_mask_ack;  // debe coincidir con el trip_mask vigente
};

struct QS_PACKED SisEventMsg {
  uint8_t event;  // SisEvent
  uint16_t trip_mask;
  uint8_t detail;
};

struct QS_PACKED ReqService {
  uint8_t magic1;  // SERVICE_MAGIC1
  uint8_t magic2;  // SERVICE_MAGIC2
  uint8_t op;      // SERVICE_OP_UNLOCK
};

struct QS_PACKED HmiHb {
  uint8_t proto_ver;
  uint8_t screen_id;
};

struct QS_PACKED ReqStart {
  uint8_t shoe_id;
  uint8_t intensity_id;
  uint8_t duration_id;
};

struct QS_PACKED RespStart {
  uint8_t result;  // StartResult
};

struct QS_PACKED Status {
  uint8_t proto_ver;
  uint8_t cycle_state;    // CycleState
  uint8_t door;           // Door
  uint8_t sis_state;      // SisState
  uint16_t trip_mask;     // TripBit
  uint8_t fault_code;     // FaultCode
  uint8_t warn_flags;     // WarnFlag
  uint8_t shoe_id;        // ID_NONE si no hay ciclo
  uint8_t intensity_id;
  uint8_t duration_id;
  uint8_t pause_reason;   // PauseReason
  uint16_t remaining_s;   // tiempo de tratamiento que falta
  uint16_t total_s;       // duración del tratamiento elegido
  uint16_t pause_left_s;  // lo que queda antes de cancelar la pausa
  int16_t t_chamber_x10;  // TC1 en décimas de grado (45,3 °C = 453)
  uint16_t rh_x10;        // humedad en décimas de %
  uint16_t voc_index;     // índice de olor (1–500)
  uint8_t act_flags;      // ActFlag
  uint8_t thermal_events;
  uint16_t reserved;
};

static_assert(sizeof(HbSis) == 9, "HB_SIS debe medir 9 bytes");
static_assert(sizeof(Status) == 28, "STATUS debe medir 28 bytes");

// -----------------------------------------------------------------------------
//  3. Tramas: envío y recepción
// -----------------------------------------------------------------------------

constexpr uint8_t SOF = 0xAA;
constexpr uint8_t MAX_PAYLOAD = 32;
constexpr size_t MAX_FRAME = 6 + MAX_PAYLOAD;  // SOF, LEN, TYPE, SEQ, payload, CRC16

// CRC-16/CCITT-FALSE. Comprobación: "123456789" -> 0x29B1.
inline uint16_t crc16(const uint8_t* data, size_t len) {
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < len; i++) {
    crc ^= static_cast<uint16_t>(data[i]) << 8;
    for (int bit = 0; bit < 8; bit++) {
      crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : (crc << 1);
    }
  }
  return crc;
}

// Envía una trama por el puerto. Sin payload: sendFrame(port, MSG_REQ_PAUSE).
inline void sendFrame(Stream& port, uint8_t type, const void* payload = nullptr, uint8_t len = 0) {
  static uint8_t sequence = 0;
  if (len > MAX_PAYLOAD) return;

  uint8_t frame[MAX_FRAME];
  frame[0] = SOF;
  frame[1] = len;
  frame[2] = type;
  frame[3] = sequence++;
  if (len > 0) memcpy(&frame[4], payload, len);
  const uint16_t crc = crc16(&frame[1], 3 + len);
  frame[4 + len] = crc & 0xFF;
  frame[5 + len] = crc >> 8;
  port.write(frame, 6 + len);
}

// Envía un mensaje completo: sendMessage(Serial2, MSG_HB_SIS, hb);
template <typename Message>
void sendMessage(Stream& port, uint8_t type, const Message& message) {
  sendFrame(port, type, &message, sizeof(Message));
}

// Una trama recibida y ya comprobada.
struct Frame {
  uint8_t type = 0;
  uint8_t len = 0;
  uint8_t payload[MAX_PAYLOAD] = {};

  // Copia el payload al mensaje sólo si el tamaño coincide exactamente.
  template <typename Message>
  bool as(Message& message) const {
    if (len != sizeof(Message)) return false;
    memcpy(&message, payload, sizeof(Message));
    return true;
  }
};

// Recibe tramas de un puerto. Uso:
//     qs::FrameReader reader;
//     qs::Frame frame;
//     while (reader.read(Serial2, frame)) { ... }
class FrameReader {
 public:
  // Lee los bytes disponibles. Devuelve true cada vez que completa una trama válida.
  bool read(Stream& port, Frame& frame) {
    while (port.available() > 0) {
      buffer_[count_++] = static_cast<uint8_t>(port.read());
      if (extract(frame)) return true;
    }
    return false;
  }

 private:
  // Busca una trama completa al principio del búfer. Si la longitud o el CRC
  // no cuadran, descarta sólo el primer byte y sigue buscando el próximo 0xAA.
  bool extract(Frame& frame) {
    while (count_ > 0) {
      if (buffer_[0] != SOF) { discard(1); continue; }
      if (count_ < 2) return false;  // falta LEN

      const uint8_t len = buffer_[1];
      if (len > MAX_PAYLOAD) { discard(1); continue; }

      const size_t total = 6 + len;
      if (count_ < total) return false;  // la trama aún no llegó entera

      const uint16_t received = buffer_[4 + len] | (buffer_[5 + len] << 8);
      if (crc16(&buffer_[1], 3 + len) != received) { discard(1); continue; }

      frame.type = buffer_[2];
      frame.len = len;
      memcpy(frame.payload, &buffer_[4], len);
      discard(total);
      return true;
    }
    return false;
  }

  void discard(size_t bytes) {
    count_ -= bytes;
    memmove(buffer_, buffer_ + bytes, count_);
  }

  uint8_t buffer_[MAX_FRAME];
  size_t count_ = 0;
};

}  // namespace qs
