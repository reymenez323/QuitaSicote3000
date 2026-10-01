# Pinout — Arduino Mega 2560 (nodo de control)

Lógica a **5 V**. Estado: **[PROPUESTA]** salvo lo marcado. Sin componentes auxiliares (resistencias, diodos, compuertas) [CONFIRMADO].
Designadores: [PLAN-MAESTRO §5](../PLAN-MAESTRO.md#5-electrónica). Las constantes van en `firmware/control/src/config/ctrl_pins.h`.

La Mega sólo maneja una salida de potencia, **SSR1 (PTC)**. Los ventiladores los maneja el Nano; la Mega los pide por la comunicación.

## 1. Alimentación y pines de sistema

| Pin de la placa | Conexión | Notas |
|---|---|---|
| **5V** | ← salida de **REG_A** (5V_A) | Salta el regulador lineal de la placa. **No conectar el USB a la vez**. En banco: alimentar sólo por USB y dejar REG_A desconectado |
| **GND** (cualquiera) | ← GND de REG_A → estrella de tierra | Común con el Nano y el HMI (obligatorio para los UART) |
| VIN | **No conectar** | |
| 3.3V | No usado | |
| IOREF | No usado | |
| AREF | **No conectar** | No se usa el ADC |
| RESET | Libre | |
| USB-B | Programación y consola de servicio (115200) | Sólo en banco o con REG_A desconectado |

## 2. Pines usados

| Pin | Función del pin | Dir. | Señal | Conecta a (componente.terminal) | Lógica | Constante |
|---|---|---|---|---|---|---|
| D0 | RX0 (USB) | — | Consola USB | Conversor USB de la placa | — | — (no conectar nada) |
| D1 | TX0 (USB) | — | Consola USB | Conversor USB de la placa | — | — (no conectar nada) |
| D8 | Digital | Salida | Calentar PTC | **SSR1.Entrada(+)**. SSR1.Entrada(−) → GND | ALTO = calentar | `PIN_SSR1` |
| D13 | LED de la placa | Salida | LED de estado | LED integrado | ALTO = encendido | `PIN_LED_STATUS` |
| D16 | TX2 | Salida | UART al HMI | **LS1** canal 1, lado 5 V (HV1) → LS1 LV1 → ESP32 **IO32** | UART 57600 8N1 | `Serial2` |
| D17 | RX2 | Entrada | UART desde el HMI | **LS1** canal 2, lado 5 V (HV2) ← LS1 LV2 ← ESP32 **IO25** | UART | `Serial2` |
| D18 | TX1 | Salida | UART al SIS | Nano **D0 (RX)**, directo | UART 57600 8N1 | `Serial1` |
| D19 | RX1 | Entrada | UART desde el SIS | Nano **D1 (TX)**, directo | UART | `Serial1` |
| D20 | SDA | E/S | I2C | S1 (SHT31).SDA, S2 (SGP40).SDA. La Mega ya trae pull-ups de 10 kΩ | I2C 100 kHz | `Wire` |
| D21 | SCL | Salida | I2C | S1.SCL, S2.SCL | I2C | `Wire` |
| D22 | Digital | Entrada (pull-up interno) | Puerta NA | SW1.**NA**, directo (la misma línea va al Nano D2) | BAJO = contacto cerrado | `PIN_DOOR_NO` |
| D23 | Digital | Entrada (pull-up interno) | Puerta NC | SW1.**NC**, directo (la misma línea va al Nano D3) | BAJO = contacto cerrado | `PIN_DOOR_NC` |
| D49 | Digital | Salida | CS de TC2 | M2 (MAX6675).CS | BAJO = seleccionado | `PIN_CS_TC2` |
| D50 | MISO | Entrada | Datos de los termopares | M1.SO, M2.SO | SPI modo 0, 1 MHz | `SPI` |
| D51 | MOSI | — | — | **No conectar** (el MAX6675 no recibe datos) | — | — |
| D52 | SCK | Salida | Reloj SPI | M1.SCK, M2.SCK | SPI | `SPI` |
| D53 | SS | Salida | CS de TC1 | M1 (MAX6675).CS. Debe ser salida para que el SPI sea maestro | BAJO = seleccionado | `PIN_CS_TC1` |

SW1.COM va a GND.
**LS1** depende de la decisión abierta A-9. Mientras no esté, **no conectar D16 al ESP32**: 5 V puede dañarlo.

## 3. Periféricos alimentados desde 5V_A

Se cablean desde la salida de REG_A (en estrella), no desde los pines de la Mega:

| Componente | VCC | GND |
|---|---|---|
| M1, M2 (MAX6675) | 5V_A | GND |
| S1 (SHT31), S2 (SGP40) | 5V_A (VIN del módulo) | GND |
| LS1, lado de 5 V (HV) | 5V_A | GND |
| Placa HMI | 5V_A por USB-C (o VCC del conector serie [VERIFICAR]) | GND |

## 4. Pines libres

D2–D7, D9–D12, D14 (TX3), D15 (RX3), D24–D48, A0–A15.
Si hiciera falta otro UART: `Serial3` (D14/D15).

## 5. Estado durante el reinicio

Mientras la Mega arranca (bootloader ≈ 1–2 s), **todos sus pines son entradas en alta impedancia**. D8 queda al aire: el SSR1 debe quedar apagado con la entrada al aire [VERIFICAR en F3]. Además, el SIS retira el permiso del PTC (RL1) al dejar de recibir `HB_CTRL`.

## 6. `ctrl_pins.h` (contenido esperado)

```cpp
constexpr uint8_t PIN_SSR1        = 8;
constexpr uint8_t PIN_LED_STATUS  = 13;
constexpr uint8_t PIN_DOOR_NO     = 22;
constexpr uint8_t PIN_DOOR_NC     = 23;
constexpr uint8_t PIN_CS_TC2      = 49;
constexpr uint8_t PIN_CS_TC1      = 53;
// Serial1 = SIS (D18/D19), Serial2 = HMI (D16/D17 vía LS1), Wire = D20/D21, SPI = D50/D52
```
