# Pinout — ESP32 DevKit V1 (nodo de CONTROL)

> Regla de máxima prioridad: este controlador hace **sólo control** ([ADR-0012](../decisiones/ADR-0012-regla-mega-control-nano-sis.md)).

Placa: **ESP32 DevKit V1** (DOIT, ESP32-WROOM-32, 30 pines) [CONFIRMADO, 2026-10-06]. Es el mismo modelo que la placa del SIS: **etiquetar las dos placas** ("CONTROL" y "SIS") para no intercambiarlas. Lógica a **3,3 V**: ningún pin tolera 5 V.
Sólo módulos y dispositivos, sin componentes sueltos ([ADR-0013](../decisiones/ADR-0013-solo-modulos-y-dispositivos.md)). Estado: **[PROPUESTA]** salvo lo marcado.
Las constantes están en `firmware/control/src/config.h`.

## 1. Alimentación y pines de sistema

| Pin de la placa | Conexión | Notas |
|---|---|---|
| **VIN** | ← salida de **REG_A** (5V_A) | El regulador de 3,3 V de la placa alimenta el ESP32. Comprobar si la placa aísla el USB de VIN [VERIFICAR]; si no, no conectar el USB a la vez |
| **GND** | ← GND de REG_A → estrella de tierra | Común con el SIS y el HMI (obligatorio para los UART) |
| **3V3** | → VCC de M1, M2 (MAX6675), S1 (SHT31), S2 (SGP40) | Así todas sus señales son de 3,3 V |
| USB (UART0, GPIO1/3) | Programación, consola de servicio y registro CSV | No usar GPIO1/3 para otra cosa |

## 2. Pines usados

| GPIO | Dir. | Señal | Conecta a (componente.terminal) | Lógica | Constante |
|---|---|---|---|---|---|
| 32 | Entrada (pull-up interno) | Puerta NA | SW1.**NA** (la misma línea va al SIS GPIO32) | BAJO = contacto cerrado | `PIN_DOOR_NO` |
| 33 | Entrada (pull-up interno) | Puerta NC | SW1.**NC** (la misma línea va al SIS GPIO33) | BAJO = contacto cerrado | `PIN_DOOR_NC` |
| 25 | Salida | Calentar PTC | **SSR1.Entrada(+)**; SSR1.Entrada(−) → GND | ALTO = calentar | `PIN_SSR1` |
| 26 | Salida | Ventilador de circulación | **SSR2.Entrada(+)**; SSR2.Entrada(−) → GND | ALTO = encender | `PIN_FAN_C` |
| 27 | Salida | Apagar ventilador del PTC | **RL2.IN** (módulo con disparo ALTO; FAN_P en COM–NC, en paralelo con RL3 del SIS) | **ALTO = pedir apagado** | `PIN_FAN_P_OFF` |
| 21 | E/S | I2C SDA | S1.SDA, S2.SDA | I2C 100 kHz | `PIN_I2C_SDA` |
| 22 | Salida | I2C SCL | S1.SCL, S2.SCL | I2C | `PIN_I2C_SCL` |
| 18 | Salida | SPI SCK | M1.SCK, M2.SCK | SPI modo 0, 1 MHz | `PIN_SPI_SCK` |
| 19 | Entrada | SPI MISO | M1.SO, M2.SO | SPI | `PIN_SPI_MISO` |
| 23 | Salida | CS de TC1 | M1 (MAX6675).CS | BAJO = seleccionado | `PIN_CS_TC1` |
| 13 | Salida | CS de TC2 | M2 (MAX6675).CS | BAJO = seleccionado | `PIN_CS_TC2` |
| 17 | Salida | UART2 TX → SIS | SIS **GPIO16 (RX2)** | UART 115200 8N1 | `PIN_SIS_TX` |
| 16 | Entrada | UART2 RX ← SIS | SIS **GPIO17 (TX2)** | UART | `PIN_SIS_RX` |
| 4 | Salida | UART1 TX → HMI | HMI **IO32** | UART 115200 8N1 | `PIN_HMI_TX` |
| 35 | Entrada (sólo entrada) | UART1 RX ← HMI | HMI **IO25** | UART | `PIN_HMI_RX` |
| 2 | Salida | LED de la placa | LED integrado | Estado / falla | `PIN_LED` |

SW1.COM va a GND. MAX6675 no recibe datos: no se conecta MOSI (`SPI.begin(18, 19, -1, -1)`).
UART1 se reasigna a GPIO4/GPIO35 (sus pines por defecto, 9/10, son de la memoria flash).
GPIO35 no tiene pull-up interno: con el HMI desconectado la línea queda al aire y puede llegar ruido; el CRC lo descarta.

El control y el SIS usan **los mismos números de pin** para la puerta (32/33), las salidas (25/26/27), el SPI (18/19/23) y el UART entre ellos (16/17, cruzados): TX de uno (17) al RX del otro (16).

## 3. Pines que no se usan a propósito

| GPIO | Motivo |
|---|---|
| 0, 5, 12, 15 | Pines de arranque (*strapping*) o con pulsos al arrancar |
| 1, 3 | UART0 (USB) |
| 6–11 | Memoria flash |
| 14 | Emite pulsos al arrancar |
| 34, 36, 39 | Sólo entrada y sin pull-up interno (libres) |

Libres para ampliaciones: 34, 36 (VP), 39 (VN), sólo como entradas.

## 4. Estado durante el arranque

Mientras el ESP32 arranca, los GPIO usados quedan en alta impedancia:
- GPIO25 al aire ⇒ SSR1 debe quedar apagado [VERIFICAR en F3]. Además, el SIS retira el permiso (RL1) al dejar de recibir `HB_CTRL`.
- GPIO26 al aire ⇒ SSR2 apagado (FAN_C sólo si el SIS lo fuerza con RL4).
- GPIO27 al aire ⇒ RL2 sin energizar ⇒ **FAN_P encendido**.

## 5. Constantes en `config.h`

```cpp
constexpr int PIN_DOOR_NO = 32;
constexpr int PIN_DOOR_NC = 33;
constexpr int PIN_SSR1 = 25;
constexpr int PIN_FAN_C = 26;
constexpr int PIN_FAN_P_OFF = 27;  // ALTO = pedir apagado de FAN_P (RL2)
constexpr int PIN_I2C_SDA = 21;
constexpr int PIN_I2C_SCL = 22;
constexpr int PIN_SPI_SCK = 18;
constexpr int PIN_SPI_MISO = 19;
constexpr int PIN_CS_TC1 = 23;
constexpr int PIN_CS_TC2 = 13;
constexpr int PIN_SIS_RX = 16;     // UART2
constexpr int PIN_SIS_TX = 17;
constexpr int PIN_HMI_RX = 35;     // UART1 reasignado
constexpr int PIN_HMI_TX = 4;
constexpr int PIN_LED = 2;
```
