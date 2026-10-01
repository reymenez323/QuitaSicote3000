# Pinout — Arduino Nano (nodo SIS)

Lógica a **5 V**. Estado: **[PROPUESTA]** salvo lo marcado. Sin componentes auxiliares (resistencias, diodos, compuertas) [CONFIRMADO].
Designadores: [PLAN-MAESTRO §5](../PLAN-MAESTRO.md#5-electrónica). Las constantes van en `firmware/sis/src/config/sis_pins.h`.

El Nano maneja **las tres salidas de seguridad**: el permiso del PTC (RL1), el ventilador del PTC (RL2) y el ventilador de circulación (SSR2). La Mega pide los ventiladores por la comunicación y el SIS decide (ver [PLAN-MAESTRO §7.5](../PLAN-MAESTRO.md#75-funciones-de-seguridad-pseudocódigo)).

## 1. Alimentación y pines de sistema

| Pin de la placa | Conexión | Notas |
|---|---|---|
| **5V** | ← salida de **REG_B** (5V_B) | Regulador independiente del de la Mega [CONFIRMADO]. **No conectar el USB a la vez** |
| **GND** (ambos) | ← GND de REG_B → estrella de tierra | Común con la Mega |
| VIN | **No conectar** | |
| 3V3 | No usado | |
| AREF | **No conectar** | No se usa el ADC |
| RST | Libre | |
| Mini-USB | Programación | **Desconectar el enlace con la Mega (D0/D1) para programar** |

## 2. Pines usados

| Pin | Función del pin | Dir. | Señal | Conecta a (componente.terminal) | Lógica | Constante |
|---|---|---|---|---|---|---|
| D0 | RX | Entrada | UART desde la Mega | Mega **D18 (TX1)**, directo | UART 57600 8N1 | `Serial` |
| D1 | TX | Salida | UART a la Mega | Mega **D19 (RX1)**, directo | UART | `Serial` |
| D2 | Digital | Entrada (pull-up interno) | Puerta NA | SW1.**NA**, directo (la misma línea va a la Mega D22) | BAJO = contacto cerrado | `PIN_DOOR_NO` |
| D3 | Digital | Entrada (pull-up interno) | Puerta NC | SW1.**NC**, directo (la misma línea va a la Mega D23) | BAJO = contacto cerrado | `PIN_DOOR_NC` |
| D4 | Digital | Salida | **Permiso del PTC** | **RL1.IN** (módulo con disparo **ALTO**, contacto **NA** en serie con el PTC) | **ALTO = permitido** | `PIN_PTC_PERMIT` |
| D5 | Digital | Salida | Ventilador de circulación | **SSR2.Entrada(+)**. SSR2.Entrada(−) → GND | ALTO = encendido | `PIN_FAN_C` |
| D6 | Digital | Salida | Ventilador del PTC | **RL2.IN** (módulo con disparo **ALTO**, FAN_P en **COM–NC**) | **ALTO = FAN_P apagado** | `PIN_FAN_P_OFF` |
| D10 | SS | Salida | CS de TC3 | M3 (MAX6675).CS. Debe ser salida para el SPI maestro | BAJO = seleccionado | `PIN_CS_TC3` |
| D11 | MOSI | — | — | **No conectar** | — | — |
| D12 | MISO | Entrada | Datos de TC3 | M3.SO | SPI modo 0, 1 MHz | `SPI` |
| D13 | SCK | Salida | Reloj SPI | M3.SCK (el LED de la placa parpadea con el reloj: normal) | SPI | `SPI` |

Corriente de salida: cada entrada de módulo de relé o de SSR toma unos 5–15 mA, dentro de los 20 mA recomendados por pin del ATmega328P [VERIFICAR con los módulos reales en F3].

## 3. Módulos de salida (conexión completa)

| Módulo | VCC | GND | IN | Contacto / salida |
|---|---|---|---|---|
| RL1 (permiso del PTC) | 12 V | GND común | Nano D4 | COM ← TH1; **NA** → PTC(+). Configurar disparo **ALTO** |
| RL2 (ventilador del PTC) | 12 V | GND común | Nano D6 | COM ← 12 V; **NC** → FAN_P(+). Configurar disparo **ALTO** |
| SSR2 (ventilador de circulación) | — | — | Entrada(+) ← Nano D5; Entrada(−) → GND | Salida(+) ← FAN_C(−); Salida(−) → GND |
| M3 (MAX6675) | 5V_B | GND | — | SO → D12, SCK ← D13, CS ← D10 |

## 4. Pines libres

D7, D8, D9, A0–A5 (también digitales), A6, A7 (sólo analógicas).

## 5. Estado durante el reinicio

Mientras el Nano arranca, sus pines son entradas en alta impedancia:
- D4 al aire ⇒ RL1 desactivado ⇒ **PTC sin permiso**.
- D6 al aire ⇒ RL2 desactivado ⇒ **FAN_P encendido** (contacto NC).
- D5 al aire ⇒ SSR2 apagado ⇒ FAN_C apagado.

Todo esto depende de que los módulos queden **desactivados con la entrada al aire**. Comprobarlo en F3 con cada módulo real; si alguno no lo cumple, hay que cambiar de módulo (no se añaden resistencias).

## 6. `sis_pins.h` (contenido esperado)

```cpp
constexpr uint8_t PIN_DOOR_NO     = 2;
constexpr uint8_t PIN_DOOR_NC     = 3;
constexpr uint8_t PIN_PTC_PERMIT  = 4;   // ALTO = permitido (RL1)
constexpr uint8_t PIN_FAN_C       = 5;   // ALTO = encendido (SSR2)
constexpr uint8_t PIN_FAN_P_OFF   = 6;   // ALTO = FAN_P apagado (RL2, contacto NC)
constexpr uint8_t PIN_CS_TC3      = 10;
// Serial = Mega (D0/D1), SPI = D12/D13
```
