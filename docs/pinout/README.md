# Pinout por controlador

Un documento independiente por cada controlador:

| Controlador | Nodo | Qué maneja | Documento |
|---|---|---|---|
| Arduino Mega 2560 | Control | SSR1 (PTC), sensores de proceso, puerta | [mega.md](mega.md) |
| Arduino Nano | SIS | RL1 (permiso del PTC), RL2 (ventilador del PTC), SSR2 (ventilador de circulación), TC3, puerta | [nano.md](nano.md) |
| ESP32-32E (pantalla 3.2") | HMI | Nada del proceso: pantalla, táctil y enlace | [esp32-hmi.md](esp32-hmi.md) |

Contexto eléctrico (fuente, reguladores, potencia): [PLAN-MAESTRO §5](../PLAN-MAESTRO.md#5-electrónica). Sin componentes auxiliares (resistencias, diodos, compuertas, fusibles) [CONFIRMADO].

## Cables entre controladores

| Señal | Desde | Hacia | En la línea |
|---|---|---|---|
| UART control → SIS | Mega D18 (TX1) | Nano D0 (RX) | Directo |
| UART SIS → control | Nano D1 (TX) | Mega D19 (RX1) | Directo |
| UART control → HMI | Mega D16 (TX2) | ESP32 IO32 | **LS1** (5 V → 3,3 V) [ABIERTO A-9] |
| UART HMI → control | ESP32 IO25 | Mega D17 (RX2) | LS1 |
| Puerta NA (compartida) | SW1.NA | Mega D22 y Nano D2 | Directo |
| Puerta NC (compartida) | SW1.NC | Mega D23 y Nano D3 | Directo |
| GND común | GND de la Mega, del Nano y del HMI | Estrella de tierra | — |

## Polaridades que el firmware debe respetar

| Señal | Controlador | ALTO significa |
|---|---|---|
| `PIN_SSR1` | Mega | Calentar |
| `PIN_PTC_PERMIT` | Nano | PTC permitido (RL1 cerrado) |
| `PIN_FAN_C` | Nano | Ventilador de circulación encendido |
| `PIN_FAN_P_OFF` | Nano | **Ventilador del PTC apagado** (RL2 energizado, contacto NC abierto) |
| Puerta NA / NC | Mega y Nano | Contacto abierto. Puerta cerrada = NA BAJO + NC ALTO |
| LED RGB | ESP32 | **Apagado** (ánodo común) |
