# Firmware

> Regla de máxima prioridad: **el control y el SIS corren en dos ESP32 DevKit V1 distintos** ([ADR-0012](../docs/decisiones/ADR-0012-regla-mega-control-nano-sis.md)).

Tres proyectos PlatformIO (framework Arduino-ESP32). **Toda la lógica corre en tareas de FreeRTOS** y cada proyecto cabe en muy pocos archivos, pensados para leerse de arriba abajo.

```
firmware/
├─ compartido/
│  └─ qs_protocol.h     El "contrato": identificadores, mensajes y tramas. Lo incluyen los tres.
├─ sis/                 ESP32 DevKit V1 n.º 2 — sólo seguridad
│  └─ src/  config.h (pines y umbrales) · main.cpp
├─ control/             ESP32 DevKit V1 n.º 1 — sólo control
│  └─ src/  config.h (pines, parámetros y perfiles) · sensors.h · main.cpp
└─ hmi/                 ESP32-32E con pantalla 3.2" — sólo interfaz
   ├─ include/lv_conf.h (configuración de LVGL)
   └─ src/  display.h (pines y pantalla) · ui.h · ui.cpp (pantallas) · main.cpp
```

## Tareas de cada nodo

| Nodo | Tarea | Periodo | Qué hace |
|---|---|---|---|
| SIS | `taskSafety` | 10 ms | Puerta → funciones de seguridad → relés RL1, RL3, RL4. Vigilada por el watchdog (1 s) |
| SIS | `taskLink` | 10 ms | Habla con el control; guarda en memoria no volátil |
| Control | `taskCycle` | 10 ms | Puerta → máquina de estados → SSR1 y ventiladores. Vigilada por el watchdog (2 s) |
| Control | `taskSensors` | 250 ms | TC1 y TC2 alternados; cada segundo, humedad y olor |
| Control | `taskSisLink` | 10 ms | Recibe `HB_SIS`; envía `HB_CTRL`, rearmes y servicio |
| Control | `taskHmiLink` | 10 ms | Recibe las solicitudes de la pantalla; envía `STATUS` |
| Control | `taskConsole` | 100 ms | Comandos por USB y registro CSV |
| HMI | `taskUi` | 5 ms | LVGL: dibuja y lee el táctil |
| HMI | `taskLink` | 10 ms | Recibe `STATUS`; envía el latido y las solicitudes |

En cada nodo, las tareas comparten unas pocas variables globales protegidas por **un mutex** (clase `Lock`). Donde una tarea le encarga algo a otra se usa **una cola** (avisos del SIS, botones del HMI).

## Compilar y cargar

Desde la carpeta de cada proyecto (`sis`, `control` o `hmi`):

```bash
pio run -t upload
```

```bash
pio device monitor
```

Si `pio` no está en el PATH: `%USERPROFILE%\.platformio\penv\Scripts\pio.exe`.

Las dos placas DevKit V1 son iguales: **etiquétalas** ("CONTROL" y "SIS") y carga en cada una su firmware.

## Estado

Los tres proyectos compilan. **Nada se ha probado todavía en hardware.** No hay pruebas automáticas: la validación es en banco, siguiendo [docs/seguridad-sis.md](../docs/seguridad-sis.md#validación) (inyección de fallas V-01…V-13). Repetirla tras cada cambio del SIS.
