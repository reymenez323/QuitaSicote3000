# Firmware

> Regla de máxima prioridad: **control = ESP32-S3, SIS = ESP32 Dev Kit** ([ADR-0012](../docs/decisiones/ADR-0012-regla-mega-control-nano-sis.md)).

Tres proyectos PlatformIO independientes, todos con framework Arduino-ESP32:

| Carpeta | Placa | Rol |
|---|---|---|
| [control/](control/README.md) | ESP32-S3 | Ciclo, sensores, SSR1 y ventiladores |
| [sis/](sis/README.md) | ESP32 Dev Kit | Seguridad |
| [hmi/](hmi/README.md) | ESP32-32E con pantalla 3.2" | Interfaz LVGL; sólo comunicación |
| [compartido/protocolo/](compartido/protocolo/README.md) | — | Ids, mensajes, CRC: contrato común |

Estructura de cada proyecto: `src/`, `include/`, `lib/`, `test/`. Especificación completa: [docs/PLAN-MAESTRO.md](../docs/PLAN-MAESTRO.md).

## Reglas

- `sis/` no incluye código de `control/` ni de `hmi/`; sólo `compartido/protocolo/`.
- `compartido/` contiene definiciones, no lógica (salvo el CRC y el parser).
- La lógica se separa del hardware para probarla en PC (`pio test -e native`).
- Cada cambio del SIS, o de la versión del core Arduino-ESP32, repite su [validación](../docs/seguridad-sis.md#validación).

## Pruebas

| Nivel | Qué | Dónde |
|---|---|---|
| Unitarias | Funciones SIF, PID, máquina de estados, parser/CRC del protocolo | PC (`native`) |
| HMI con simulador | Todas las pantallas con un controlador simulado | ESP32-32E ([PLAN](hmi/PLAN.md)) |
| Integración | Enlaces UART, heartbeats, fallas de enlace | Placas en banco |
| Validación SIS | Inyección de fallas V-01…V-13 | Banco y equipo |
| Perfiles | Las 36 combinaciones calzado × intensidad × duración | Equipo completo |
