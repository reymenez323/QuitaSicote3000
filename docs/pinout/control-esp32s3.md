# Pinout — ESP32-S3 (nodo de CONTROL)

> Regla de máxima prioridad: este controlador hace **sólo control** ([ADR-0012](../decisiones/ADR-0012-regla-mega-control-nano-sis.md)).

Placa asumida: **ESP32-S3-DevKitC-1** [VERIFICAR modelo exacto, A-13]. Lógica a **3,3 V**: ningún pin tolera 5 V.
Sólo módulos y dispositivos, sin componentes sueltos ([ADR-0013](../decisiones/ADR-0013-solo-modulos-y-dispositivos.md)). Estado: **[PROPUESTA]** salvo lo marcado.
Las constantes van en `firmware/control/src/config/ctrl_pins.h`.

## 1. Alimentación y pines de sistema

| Pin de la placa | Conexión | Notas |
|---|---|---|
| **5V** | ← salida de **REG_A** (5V_A) | El regulador de 3,3 V de la placa alimenta el ESP32-S3. Comprobar si la placa aísla el USB del pin 5V [VERIFICAR]; si no, no conectar el USB a la vez |
| **GND** | ← GND de REG_A → estrella de tierra | Común con el SIS y el HMI (obligatorio para los UART) |
| **3V3** | → VCC de M1, M2 (MAX6675), S1 (SHT31), S2 (SGP40) | Así todas sus señales son de 3,3 V |
| Puerto USB nativo (GPIO19/20) | Consola de servicio y registro CSV | No usar GPIO19/20 para otra cosa |
| Puerto "UART" (GPIO43/44) | Sin uso | No usar GPIO43/44 |

## 2. Pines usados

| GPIO | Dir. | Señal | Conecta a (componente.terminal) | Lógica | Constante |
|---|---|---|---|---|---|
| 4 | Entrada (pull-up interno) | Puerta NA | SW1.**NA** (la misma línea va al SIS GPIO32) | BAJO = contacto cerrado | `PIN_DOOR_NO` |
| 5 | Entrada (pull-up interno) | Puerta NC | SW1.**NC** (la misma línea va al SIS GPIO33) | BAJO = contacto cerrado | `PIN_DOOR_NC` |
| 6 | Salida | Calentar PTC | **SSR1.Entrada(+)**; SSR1.Entrada(−) → GND | ALTO = calentar | `PIN_SSR1` |
| 7 | Salida | Ventilador de circulación | **SSR2.Entrada(+)**; SSR2.Entrada(−) → GND | ALTO = encender | `PIN_FAN_C` |
| 21 | Salida | Apagar ventilador del PTC | **RL2.IN** (módulo con disparo ALTO; FAN_P en COM–NC, en paralelo con RL3 del SIS) | **ALTO = pedir apagado** | `PIN_FAN_P_OFF` |
| 8 | E/S | I2C SDA | S1.SDA, S2.SDA | I2C 100 kHz | `PIN_I2C_SDA` |
| 9 | Salida | I2C SCL | S1.SCL, S2.SCL | I2C | `PIN_I2C_SCL` |
| 12 | Salida | SPI SCK | M1.SCK, M2.SCK | SPI modo 0, 1 MHz | `PIN_SPI_SCK` |
| 13 | Entrada | SPI MISO | M1.SO, M2.SO | SPI | `PIN_SPI_MISO` |
| 10 | Salida | CS de TC1 | M1 (MAX6675).CS | BAJO = seleccionado | `PIN_CS_TC1` |
| 14 | Salida | CS de TC2 | M2 (MAX6675).CS | BAJO = seleccionado | `PIN_CS_TC2` |
| 17 | Salida | UART1 TX → SIS | SIS **GPIO16 (RX2)** | UART 115200 8N1 | `PIN_SIS_TX` |
| 18 | Entrada | UART1 RX ← SIS | SIS **GPIO17 (TX2)** | UART | `PIN_SIS_RX` |
| 15 | Salida | UART2 TX → HMI | HMI **IO32** | UART 115200 8N1 | `PIN_HMI_TX` |
| 16 | Entrada | UART2 RX ← HMI | HMI **IO25** | UART | `PIN_HMI_RX` |
| 48 (o 38) | Salida | LED RGB de la placa | LED integrado (WS2812) | Estado | `PIN_LED_RGB` [VERIFICAR versión de placa] |

SW1.COM va a GND. MAX6675 no recibe datos: no se conecta MOSI (`SPI.begin(12, 13, -1)`).

## 3. Pines que no se usan a propósito

| GPIO | Motivo |
|---|---|
| 0, 3, 45, 46 | Pines de arranque (*strapping*) |
| 19, 20 | USB nativo |
| 26–32 | Memoria flash interna |
| 33–37 | PSRAM octal (en placas N8R8/N16R8) |
| 43, 44 | UART0 hacia el conversor USB de la placa |

Libres para ampliaciones: 1, 2, 11, 39, 40, 41, 42, 47 (y 38 o 48, el que no tenga el LED).

## 4. Estado durante el arranque

Mientras el ESP32-S3 arranca, los GPIO usados quedan en alta impedancia:
- GPIO6 al aire ⇒ SSR1 debe quedar apagado [VERIFICAR en F3]. Además, el SIS retira el permiso (RL1) al dejar de recibir `HB_CTRL`.
- GPIO7 al aire ⇒ SSR2 apagado (FAN_C sólo si el SIS lo fuerza con RL4).
- GPIO21 al aire ⇒ RL2 sin energizar ⇒ **FAN_P encendido**.

## 5. `ctrl_pins.h` (contenido esperado)

```cpp
constexpr int PIN_DOOR_NO   = 4;
constexpr int PIN_DOOR_NC   = 5;
constexpr int PIN_SSR1      = 6;
constexpr int PIN_FAN_C     = 7;
constexpr int PIN_FAN_P_OFF = 21;   // ALTO = pedir apagado de FAN_P (RL2)
constexpr int PIN_I2C_SDA   = 8;
constexpr int PIN_I2C_SCL   = 9;
constexpr int PIN_SPI_SCK   = 12;
constexpr int PIN_SPI_MISO  = 13;
constexpr int PIN_CS_TC1    = 10;
constexpr int PIN_CS_TC2    = 14;
constexpr int PIN_SIS_TX    = 17;   // UART1
constexpr int PIN_SIS_RX    = 18;
constexpr int PIN_HMI_TX    = 15;   // UART2
constexpr int PIN_HMI_RX    = 16;
constexpr int PIN_LED_RGB   = 48;   // 38 en algunas versiones de la placa
```
