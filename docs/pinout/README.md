# Pinout por controlador

> Regla de máxima prioridad: **control = ESP32-S3, SIS = ESP32 Dev Kit** ([ADR-0012](../decisiones/ADR-0012-regla-mega-control-nano-sis.md)). Sólo módulos y dispositivos ([ADR-0013](../decisiones/ADR-0013-solo-modulos-y-dispositivos.md)).

Un documento independiente por cada controlador:

| Controlador | Nodo | Qué maneja | Documento |
|---|---|---|---|
| ESP32-S3 | **Control** | SSR1 (PTC), SSR2 (FAN_C), RL2 (apagar FAN_P), TC1, TC2, SHT31, SGP40, puerta | [control-esp32s3.md](control-esp32s3.md) |
| ESP32 Dev Kit | **SIS** | RL1 (permiso del PTC), RL3 (veto del apagado de FAN_P), RL4 (forzado de FAN_C), TC3, puerta | [sis-esp32.md](sis-esp32.md) |
| ESP32-32E (pantalla 3.2") | HMI | Nada del proceso: pantalla, táctil y enlace | [esp32-hmi.md](esp32-hmi.md) |

Los tres trabajan a **3,3 V**: todos los cables entre ellos son **directos**. Contexto eléctrico completo: [PLAN-MAESTRO §5](../PLAN-MAESTRO.md#5-electrónica).
Los números de pin se fijarán definitivamente cuando se confirmen los modelos de placa (decisión abierta A-13).

## Cables entre controladores

| Señal | Desde | Hacia |
|---|---|---|
| UART control → SIS | ESP32-S3 GPIO17 (TX1) | ESP32 GPIO16 (RX2) |
| UART SIS → control | ESP32 GPIO17 (TX2) | ESP32-S3 GPIO18 (RX1) |
| UART control → HMI | ESP32-S3 GPIO15 (TX2) | HMI IO32 (RX2) |
| UART HMI → control | HMI IO25 (TX2) | ESP32-S3 GPIO16 (RX2) |
| Puerta NA (compartida) | SW1.NA | ESP32-S3 GPIO4 y ESP32 GPIO32 |
| Puerta NC (compartida) | SW1.NC | ESP32-S3 GPIO5 y ESP32 GPIO33 |
| GND común | GND de las tres placas | Estrella de tierra |

## Combinación por contactos (no hay cable entre controladores)

| Ventilador | Control | SIS | Combinación |
|---|---|---|---|
| FAN_P | RL2 (GPIO21) | RL3 (GPIO26) | Contactos NC en paralelo: se apaga sólo si ambos están energizados |
| FAN_C | SSR2 (GPIO7) | RL4 (GPIO27) | Contacto NA en paralelo con SSR2: se enciende si cualquiera lo activa |

## Polaridades que el firmware debe respetar

| Señal | Controlador | ALTO significa |
|---|---|---|
| `PIN_SSR1` | Control | Calentar |
| `PIN_FAN_C` | Control | Encender ventilador de circulación |
| `PIN_FAN_P_OFF` | Control | **Pedir apagado** del ventilador del PTC |
| `PIN_PTC_PERMIT` | SIS | PTC permitido (RL1 cerrado) |
| `PIN_FAN_P_OFF_PERMIT` | SIS | **Permitir** apagar el ventilador del PTC |
| `PIN_FAN_C_FORCE` | SIS | Forzar ventilador de circulación |
| Puerta NA / NC | Control y SIS | Contacto abierto. Puerta cerrada = NA BAJO + NC ALTO |
| LED RGB del HMI | HMI | **Apagado** (ánodo común) |
