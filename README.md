# QuitaSicote3000

Firmware de un equipo doméstico que elimina el mal olor del calzado con aire caliente forzado. Se controla sólo desde la pantalla táctil y tiene una capa de seguridad independiente (SIS).

Repositorio: https://github.com/reymenez323/QuitaSicote3000

Este repositorio documenta **el software**. El hardware (potencia, cableado, mecánica) está documentado fuera de aquí.

## Nodos

| Nodo | Placa | Rol | Carpeta |
|---|---|---|---|
| Control | Arduino Mega 2560 | Ciclo, sensores, actuadores | [firmware/control](firmware/control/) |
| SIS | Arduino Nano | Seguridad: puede cortar el calentador sin pedir permiso | [firmware/sis](firmware/sis/) |
| HMI | ESP32-32E con pantalla 3.2" | Interfaz táctil; sólo comunicación | [firmware/hmi](firmware/hmi/) |

## Estructura

```
QuitaSicote3000/
├─ docs/
│  ├─ PLAN-MAESTRO.md            Plan completo de implementación (electrónica + firmware). Empieza aquí
│  ├─ pinout/                    Pinout de cada controlador: mega.md, nano.md, esp32-hmi.md
│  ├─ requisitos.md              Qué debe hacer el software y parámetros provisionales
│  ├─ arquitectura.md            Nodos, reparto de responsabilidades, principios
│  ├─ maquina-de-estados.md      Ciclo de tratamiento (Mega)
│  ├─ perfiles-de-tratamiento.md Calzado × intensidad × duración
│  ├─ comunicacion.md            Enlaces UART y mensajes
│  ├─ seguridad-sis.md           Funciones de seguridad y su validación
│  └─ decisiones/                ADRs y pendientes
└─ firmware/
   ├─ control/                   Proyecto PlatformIO de la Mega
   ├─ sis/                       Proyecto PlatformIO del Nano
   ├─ hmi/                       Proyecto PlatformIO del ESP32 (LVGL) + PLAN.md
   └─ compartido/protocolo/      Contrato común entre nodos
```

## Convenciones

- Documentación y comentarios en español; identificadores de código en inglés.
- PlatformIO, un proyecto por nodo.
- Decisiones de diseño como ADR en [docs/decisiones](docs/decisiones/README.md).
