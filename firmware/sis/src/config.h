// =============================================================================
//  SIS — Pines y umbrales (ESP32 DevKit V1)
// =============================================================================
//  Todo lo que se puede ajustar del SIS está aquí. Son constantes de
//  compilación: NINGÚN umbral se recibe por la comunicación.
//  Pinout completo: docs/pinout/sis-esp32.md · Parámetros: PLAN-MAESTRO §12
// =============================================================================
#pragma once
#include <stdint.h>

// ----------------------------- Pines -----------------------------------------
constexpr int PIN_DOOR_NO = 32;           // puerta, contacto NA  (BAJO = contacto cerrado)
constexpr int PIN_DOOR_NC = 33;           // puerta, contacto NC  (BAJO = contacto cerrado)
constexpr int PIN_PTC_PERMIT = 25;        // RL1: ALTO = el PTC puede calentar
constexpr int PIN_FAN_P_OFF_PERMIT = 26;  // RL3: ALTO = se permite apagar el ventilador del PTC
constexpr int PIN_FAN_C_FORCE = 27;       // RL4: ALTO = forzar el ventilador de circulación
constexpr int PIN_SPI_SCK = 18;           // MAX6675 de TC3
constexpr int PIN_SPI_MISO = 19;
constexpr int PIN_CS_TC3 = 23;
constexpr int PIN_CTRL_RX = 16;           // UART2 <- control (su GPIO17)
constexpr int PIN_CTRL_TX = 17;           // UART2 -> control (su GPIO16)
constexpr int PIN_LED = 2;                // LED de la placa

// ----------------------------- Tiempos ---------------------------------------
constexpr uint32_t SAFETY_PERIOD_MS = 10;   // periodo de la tarea de seguridad
constexpr uint32_t TC3_PERIOD_MS = 250;     // periodo de lectura de TC3
constexpr uint32_t WATCHDOG_S = 1;          // reinicio si la tarea de seguridad se cuelga
constexpr uint32_t SELFTEST_MS = 2000;      // plazo del autotest de arranque
constexpr uint32_t DOOR_DEBOUNCE_MS = 30;   // antirrebote de la puerta

// ----------------------------- Temperaturas ----------------------------------
// En cuartos de grado ("Q2"), como las entrega el MAX6675:  °C × 4.
constexpr int16_t T_SIS_MAX_Q2 = 68 * 4;    // SIF-01: límite de seguridad [POR MEDIR en F7; usar 55 °C durante los ensayos]
constexpr int16_t T_SIS_RESET_Q2 = 45 * 4;  // por debajo de esto se acepta el rearme
constexpr int16_t T_COLD_Q2 = 35 * 4;       // "frío": se puede dejar de ventilar
constexpr int16_t TC3_MIN_Q2 = -10 * 4;     // rango creíble del sensor
constexpr int16_t TC3_MAX_Q2 = 150 * 4;

// ----------------------------- Validez de TC3 --------------------------------
constexpr uint8_t TC3_BAD_READS = 3;        // lecturas malas seguidas para darlo por inválido
constexpr uint32_t TC3_FROZEN_MS = 60000;   // misma lectura todo este tiempo, calentando = congelado

// ----------------------------- Funciones de seguridad ------------------------
constexpr uint32_t SIF03_WINDOW_MS = 10000;        // SIF-03: se compara TC3 con el de hace 10 s...
constexpr int16_t SIF03_MAX_RISE_Q2 = 10 * 4;      // ...y dispara si subió 10 °C o más [POR MEDIR]
constexpr uint32_t SIF07_MAX_HEAT_MS = 125 * 60000u;  // SIF-07: tiempo máximo con el permiso concedido
constexpr uint32_t SIF07_COOL_OFF_MS = 10 * 60000u;   // descanso que pone ese contador a cero
constexpr uint32_t SIF08_WINDOW_MS = 60000;        // SIF-08: en cada minuto calentando...
constexpr uint32_t SIF08_MIN_DUTY_PCT = 80;        // ...si el SSR1 estuvo ordenado el 80 % del tiempo...
constexpr int16_t SIF08_DROP_Q2 = 3 * 4;           // ...y aun así TC3 bajó 3 °C, dispara [POR MEDIR]

// ----------------------------- Ventiladores ----------------------------------
constexpr uint32_t FAN_P_OFF_HOLD_MS = 60000;  // tiempo sin calentar antes de permitir apagar FAN_P

// ----------------------------- Bloqueo y servicio ----------------------------
constexpr uint8_t THERMAL_EVENTS_LOCKOUT = 2;   // al 2.º evento térmico el equipo se bloquea
constexpr uint8_t SERVICE_DOOR_TOGGLES = 3;     // aperturas y cierres de puerta para desbloquear
constexpr uint32_t SERVICE_WINDOW_MS = 30000;   // plazo para hacerlos
