# Pinout por controlador

> Regla de máxima prioridad: **control y SIS en dos ESP32 DevKit V1 distintos** ([ADR-0012](../decisiones/ADR-0012-regla-mega-control-nano-sis.md)). Sólo módulos y dispositivos ([ADR-0013](../decisiones/ADR-0013-solo-modulos-y-dispositivos.md)).

Un documento independiente por cada controlador:

| Controlador | Nodo | Qué maneja | Documento |
|---|---|---|---|
| ESP32 DevKit V1 n.º 1 | **Control** | SSR1 (PTC), SSR2 (FAN_C), RL2 (apagar FAN_P), TC1, TC2, SHT31, SGP40, puerta | [control-esp32.md](control-esp32.md) |
| ESP32 DevKit V1 n.º 2 | **SIS** | RL1 (permiso del PTC), RL3 (veto del apagado de FAN_P), RL4 (forzado de FAN_C), TC3, puerta | [sis-esp32.md](sis-esp32.md) |
| ESP32-32E (pantalla 3.2") | HMI | Nada del proceso: pantalla, táctil y enlace | [esp32-hmi.md](esp32-hmi.md) |

Los tres trabajan a **3,3 V**: todos los cables entre ellos son **directos**. Contexto eléctrico completo: [PLAN-MAESTRO §5](../PLAN-MAESTRO.md#5-electrónica).
Las dos placas son del mismo modelo: **etiquetarlas** ("CONTROL" y "SIS"). Usan los mismos números de pin para la puerta, las salidas, el SPI y el UART entre ellas.

## Cables entre controladores

| Señal | Desde | Hacia |
|---|---|---|
| UART control → SIS | Control GPIO17 (TX2) | SIS GPIO16 (RX2) |
| UART SIS → control | SIS GPIO17 (TX2) | Control GPIO16 (RX2) |
| UART control → HMI | Control GPIO4 (TX1) | HMI IO32 (RX2) |
| UART HMI → control | HMI IO25 (TX2) | Control GPIO35 (RX1) |
| Puerta NA (compartida) | SW1.NA | Control GPIO32 y SIS GPIO32 |
| Puerta NC (compartida) | SW1.NC | Control GPIO33 y SIS GPIO33 |
| GND común | GND de las tres placas | Estrella de tierra |

## Combinación por contactos (no hay cable entre controladores)

| Ventilador | Control | SIS | Combinación |
|---|---|---|---|
| FAN_P | RL2 (GPIO27) | RL3 (GPIO26) | Contactos NC en paralelo: se apaga sólo si ambos están energizados |
| FAN_C | SSR2 (GPIO26) | RL4 (GPIO27) | Contacto NA en paralelo con SSR2: se enciende si cualquiera lo activa |

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
