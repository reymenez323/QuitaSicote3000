// =============================================================================
//  SIS — Pines y umbrales (ESP32 DevKit V1)
// =============================================================================
//  El SIS NO tiene termopar (ADR-0004): sólo vigila puerta, enlace y tiempo.
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
constexpr int PIN_CTRL_RX = 16;           // UART2 <- control (su GPIO17)
constexpr int PIN_CTRL_TX = 17;           // UART2 -> control (su GPIO16)
constexpr int PIN_LED = 2;                // LED de la placa

// ----------------------------- Tiempos ---------------------------------------
constexpr uint32_t SAFETY_PERIOD_MS = 10;   // periodo de la tarea de seguridad
constexpr uint32_t WATCHDOG_S = 1;          // reinicio si la tarea de seguridad se cuelga
constexpr uint32_t SELFTEST_MS = 2000;      // plazo del autotest de arranque
constexpr uint32_t DOOR_DEBOUNCE_MS = 30;   // antirrebote de la puerta

// ----------------------------- Funciones de seguridad ------------------------
constexpr uint32_t SIF07_MAX_HEAT_MS = 125 * 60000u;  // SIF-07: tiempo máximo con el permiso concedido
constexpr uint32_t SIF07_COOL_OFF_MS = 10 * 60000u;   // descanso que pone ese contador a cero
// Sin termopar, "frío" se estima por tiempo: este descanso sin permiso del PTC
// basta para rearmar, desbloquear, apagar FAN_P y soltar FAN_C. [POR MEDIR en F7]
constexpr uint32_t COOLED_AFTER_MS = SIF07_COOL_OFF_MS;

// ----------------------------- Bloqueo y servicio ----------------------------
constexpr uint8_t THERMAL_EVENTS_LOCKOUT = 2;   // al 2.º evento térmico el equipo se bloquea
constexpr uint8_t SERVICE_DOOR_TOGGLES = 3;     // aperturas y cierres de puerta para desbloquear
constexpr uint32_t SERVICE_WINDOW_MS = 30000;   // plazo para hacerlos
