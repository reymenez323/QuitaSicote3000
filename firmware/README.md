# Firmware

Tres proyectos PlatformIO independientes:

| Carpeta | Placa | Rol |
|---|---|---|
| [control/](control/README.md) | Arduino Mega 2560 | Ciclo, sensores, actuadores |
| [sis/](sis/README.md) | Arduino Nano | Seguridad |
| [hmi/](hmi/README.md) | ESP32-32E con pantalla 3.2" | Interfaz LVGL; sólo comunicación |
| [compartido/protocolo/](compartido/protocolo/README.md) | — | Ids, mensajes, CRC: contrato común |

Estructura de cada proyecto: `src/`, `include/`, `lib/`, `test/`.

## Reglas

- `sis/` no incluye código de `control/` ni de `hmi/`; sólo `compartido/protocolo/`.
- `compartido/` contiene definiciones, no lógica (salvo el CRC).
- La lógica se separa del hardware para probarla en PC (`pio test -e native`).
- Cada cambio del SIS repite su [validación](../docs/seguridad-sis.md#validación).

## Pruebas

| Nivel | Qué | Dónde |
|---|---|---|
| Unitarias | Funciones SIF, PID, máquina de estados, parser/CRC del protocolo | PC (`native`) |
| HMI con simulador | Todas las pantallas con un controlador simulado | ESP32 ([PLAN](hmi/PLAN.md)) |
| Integración | Enlaces UART, heartbeats, fallas de enlace | Placas en banco |
| Validación SIS | Inyección de fallas V-01…V-13 | Banco |
| Perfiles | Las 36 combinaciones calzado × intensidad × duración | Equipo completo |
