# Pinout — ESP32 Dev Kit (nodo SIS)

> Regla de máxima prioridad: este controlador hace **sólo seguridad** ([ADR-0012](../decisiones/ADR-0012-regla-mega-control-nano-sis.md)). No maneja ventiladores: sólo **veta** el apagado de FAN_P (RL3) y **fuerza** FAN_C (RL4) con sus propios relés.

Placa asumida: **ESP32 Dev Kit con ESP32-WROOM-32** (DevKitC o DOIT, 30 o 38 pines) [VERIFICAR modelo exacto, A-13]. Todos los pines elegidos existen en ambas versiones. Lógica a **3,3 V**: ningún pin tolera 5 V.
Sólo módulos y dispositivos, sin componentes sueltos ([ADR-0013](../decisiones/ADR-0013-solo-modulos-y-dispositivos.md)). Estado: **[PROPUESTA]** salvo lo marcado.
Las constantes van en `firmware/sis/src/config/sis_pins.h`.

## 1. Alimentación y pines de sistema

| Pin de la placa | Conexión | Notas |
|---|---|---|
| **5V / VIN** | ← salida de **REG_B** (5V_B) | Regulador independiente del del control [CONFIRMADO]. Comprobar si la placa aísla el USB de este pin [VERIFICAR]; si no, no conectar el USB a la vez |
| **GND** | ← GND de REG_B → estrella de tierra | Común con el control |
| **3V3** | → VCC de M3 (MAX6675) | |
| USB (UART0, GPIO1/3) | Programación y depuración | Wi-Fi y Bluetooth apagados en el firmware |

## 2. Pines usados

| GPIO | Dir. | Señal | Conecta a (componente.terminal) | Lógica | Constante |
|---|---|---|---|---|---|
| 32 | Entrada (pull-up interno) | Puerta NA | SW1.**NA** (la misma línea va al control GPIO4) | BAJO = contacto cerrado | `PIN_DOOR_NO` |
| 33 | Entrada (pull-up interno) | Puerta NC | SW1.**NC** (la misma línea va al control GPIO5) | BAJO = contacto cerrado | `PIN_DOOR_NC` |
| 25 | Salida | **Permiso del PTC** | **RL1.IN** (módulo de 30 A, disparo ALTO, contacto NA en serie con el PTC). Con la opción T2: GPIO25 → TH1 → RL1.IN | **ALTO = permitido** | `PIN_PTC_PERMIT` |
| 26 | Salida | Permiso de apagado de FAN_P | **RM1 canal 1 (RL3).IN**; contacto NC en paralelo con RL2 del control | **ALTO = se permite apagar** | `PIN_FAN_P_OFF_PERMIT` |
| 27 | Salida | Forzar FAN_C | **RM1 canal 2 (RL4).IN**; contacto NA en paralelo con la salida de SSR2 | ALTO = forzar encendido | `PIN_FAN_C_FORCE` |
| 18 | Salida | SPI SCK | M3.SCK | SPI modo 0, 1 MHz | `PIN_SPI_SCK` |
| 19 | Entrada | SPI MISO | M3.SO | SPI | `PIN_SPI_MISO` |
| 23 | Salida | CS de TC3 | M3 (MAX6675).CS | BAJO = seleccionado | `PIN_CS_TC3` |
| 16 | Entrada | UART2 RX ← control | Control **GPIO17 (TX)** | UART 115200 8N1 | `PIN_CTRL_RX` |
| 17 | Salida | UART2 TX → control | Control **GPIO18 (RX)** | UART | `PIN_CTRL_TX` |
| 4 | Entrada (pull-down interno) | Lectura del termostato | **Sólo con la opción T2**: lado de RL1.IN de TH1 | ALTO = termostato cerrado | `PIN_TH_SENSE` |
| 2 | Salida | LED de la placa | LED integrado | Estado / falla | `PIN_LED` |

SW1.COM va a GND. MAX6675 no recibe datos: `SPI.begin(18, 19, -1)`.

### Módulos de salida (conexión completa)

| Módulo | VCC | GND | IN | Contacto |
|---|---|---|---|---|
| RL1 (permiso del PTC, 30 A) | 12 V | GND común | GPIO25 (T1) o GPIO25 → TH1 (T2) | COM ← TH1 (T1) o 12 V (T2); **NA** → PTC(+) |
| RM1 canal 1 = RL3 | 12 V (o 5 V según el módulo) | GND común | GPIO26 | COM ← 12 V; **NC** → FAN_P(+) (en paralelo con RL2.NC) |
| RM1 canal 2 = RL4 | ídem | GND común | GPIO27 | COM ← FAN_C(−); **NA** → GND (en paralelo con la salida de SSR2) |

Disparo **ALTO** en los tres; deben activarse con 3,3 V y quedar desactivados con la entrada al aire [VERIFICAR en F3].

## 3. Pines que no se usan a propósito

| GPIO | Motivo |
|---|---|
| 0, 5, 12, 15 | Pines de arranque (*strapping*) o con pulsos al arrancar |
| 1, 3 | UART0 (USB) |
| 6–11 | Memoria flash |
| 14 | Emite pulsos al arrancar |
| 34–39 | Sólo entrada y sin pull-up interno |

Libres: 13, 21, 22 (y 4 con la opción T1).

## 4. Estado durante el arranque

Mientras el ESP32 arranca, GPIO25, 26 y 27 quedan en alta impedancia:
- RL1 desactivado ⇒ **PTC sin permiso**.
- RL3 desactivado ⇒ contacto NC cerrado ⇒ **FAN_P encendido**.
- RL4 desactivado ⇒ FAN_C sin forzar (sigue lo que mande el control).

## 5. `sis_pins.h` (contenido esperado)

```cpp
constexpr int PIN_DOOR_NO          = 32;
constexpr int PIN_DOOR_NC          = 33;
constexpr int PIN_PTC_PERMIT       = 25;  // ALTO = permitido (RL1)
constexpr int PIN_FAN_P_OFF_PERMIT = 26;  // ALTO = se permite apagar FAN_P (RL3)
constexpr int PIN_FAN_C_FORCE      = 27;  // ALTO = forzar FAN_C (RL4)
constexpr int PIN_SPI_SCK          = 18;
constexpr int PIN_SPI_MISO         = 19;
constexpr int PIN_CS_TC3           = 23;
constexpr int PIN_CTRL_RX          = 16;  // UART2
constexpr int PIN_CTRL_TX          = 17;
constexpr int PIN_TH_SENSE         = 4;   // sólo con la opción T2
constexpr int PIN_LED              = 2;
```
