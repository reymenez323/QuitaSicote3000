// =============================================================================
//  Control — Pines, parámetros y perfiles (ESP32 DevKit V1)
// =============================================================================
//  Todo lo que se puede ajustar del control está aquí.
//  Pinout completo: docs/pinout/control-esp32.md · Parámetros: PLAN-MAESTRO §12
//
//  Estos límites son de PROCESO (capa 1). Las funciones de seguridad viven
//  sólo en el SIS.
// =============================================================================
#pragma once
#include <stdint.h>

// ----------------------------- Pines -----------------------------------------
constexpr int PIN_DOOR_NO = 32;    // puerta, contacto NA  (BAJO = contacto cerrado)
constexpr int PIN_DOOR_NC = 33;    // puerta, contacto NC  (BAJO = contacto cerrado)
constexpr int PIN_SSR1 = 25;       // SSR1: ALTO = calentar el PTC
constexpr int PIN_FAN_C = 26;      // SSR2: ALTO = encender el ventilador de circulación
constexpr int PIN_FAN_P_OFF = 27;  // RL2:  ALTO = pedir que se apague el ventilador del PTC
constexpr int PIN_I2C_SDA = 21;    // SHT31 y SGP40
constexpr int PIN_I2C_SCL = 22;
constexpr int PIN_SPI_SCK = 18;    // MAX6675 de TC1 y TC2
constexpr int PIN_SPI_MISO = 19;
constexpr int PIN_CS_TC1 = 23;     // TC1: aire de la recámara
constexpr int PIN_CS_TC2 = 13;     // TC2: salida del PTC
constexpr int PIN_SIS_RX = 16;     // UART2 <- SIS (su GPIO17)
constexpr int PIN_SIS_TX = 17;     // UART2 -> SIS (su GPIO16)
constexpr int PIN_HMI_RX = 35;     // UART1 <- HMI (su IO25)
constexpr int PIN_HMI_TX = 4;      // UART1 -> HMI (su IO32)
constexpr int PIN_LED = 2;         // LED de la placa

// ----------------------------- Tiempos de las tareas -------------------------
constexpr uint32_t CYCLE_PERIOD_MS = 10;    // tarea del ciclo
constexpr uint32_t TC_PERIOD_MS = 250;      // se lee un termopar cada 250 ms, alternando TC1 y TC2
constexpr uint32_t WATCHDOG_S = 2;          // reinicio si la tarea del ciclo se cuelga
constexpr uint32_t DOOR_DEBOUNCE_MS = 30;
constexpr uint8_t SENSOR_BAD_READS = 3;     // lecturas malas seguidas para dar un sensor por perdido

// ----------------------------- Perfiles --------------------------------------
// Valores de partida, a ajustar con ensayos (fase F9). El HMI sólo envía el
// número de calzado, intensidad y duración: las temperaturas y los minutos
// viven aquí.

// Temperatura objetivo del aire de la recámara (TC1), en °C
//                                       Suave  Media  Intensa
constexpr int16_t SETPOINT_C[4][3] = {
    /* 0 Cuero     */ {35, 40, 45},
    /* 1 Deportivo */ {40, 45, 50},
    /* 2 Bota      */ {40, 45, 50},
    /* 3 Sintético */ {38, 42, 48},
};

// Duración del tratamiento a temperatura, en minutos:  Corta  Media  Larga
constexpr uint32_t DURATION_MIN[3] = {5, 10, 25};

// ----------------------------- Regulación de temperatura ---------------------
// Etapa 1: histéresis sobre TC1. Las temperaturas van en décimas de grado (x10).
constexpr int16_t HYSTERESIS_X10 = 5;          // enciende por debajo de consigna − 0,5 °C; apaga por encima de + 0,5 °C
constexpr uint32_t HEATER_MIN_STATE_MS = 5000; // tiempo mínimo encendido o apagado (conmutación lenta del SSR)
constexpr int16_t TREAT_BAND_X10 = 30;         // "a temperatura" = TC1 a menos de 3 °C de la consigna
constexpr int16_t OVERTEMP_MARGIN_X10 = 50;    // TC1 más de 5 °C sobre la consigna = falla
constexpr int16_t TC2_MAX_X10 = 700;           // salida del PTC a más de 70 °C: se corta el calor... [POR MEDIR]
constexpr int16_t TC2_RESUME_X10 = 650;        // ...hasta que baje de 65 °C

// ----------------------------- Tiempos del ciclo -----------------------------
constexpr uint32_t SIS_BOOT_WAIT_MS = 5000;          // espera al SIS en el arranque
constexpr uint32_t PREHEAT_TIMEOUT_MS = 15 * 60000u; // sin llegar a temperatura = falla
constexpr uint32_t PAUSE_MAX_MS = 5 * 60000u;        // una pausa más larga cancela el ciclo
constexpr uint32_t CYCLE_MAX_MS = 120 * 60000u;      // tope global de un ciclo
constexpr uint32_t COOL_MAX_MS = 15 * 60000u;        // tope del enfriamiento

// ----------------------------- Enfriamiento y ventiladores -------------------
constexpr int16_t COOL_END_TC1_X10 = 350;      // el enfriamiento termina con TC1 < 35 °C...
constexpr int16_t COOL_END_TC2_X10 = 400;      // ...y TC2 < 40 °C
constexpr int16_t COLD_X10 = 350;              // en reposo, los ventiladores se apagan con TC2 < 35 °C...
constexpr uint32_t FAN_OFF_HOLD_MS = 60000;    // ...mantenido durante 1 min
