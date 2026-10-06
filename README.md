# QuitaSicote3000

Equipo doméstico que elimina el mal olor del calzado con aire caliente forzado. Se controla sólo desde la pantalla táctil y tiene una capa de seguridad independiente (SIS).

Repositorio: https://github.com/reymenez323/QuitaSicote3000

**Empieza por [docs/PLAN-MAESTRO.md](docs/PLAN-MAESTRO.md)**: plan completo de electrónica y firmware.

> **Regla de máxima prioridad**: el **control** es el **ESP32 DevKit V1 de control** y el **SIS** es el **ESP32 Dev Kit**, estrictamente en todo el proyecto ([ADR-0012](docs/decisiones/ADR-0012-regla-mega-control-nano-sis.md)).
> **Hardware**: sólo módulos y dispositivos; sin optoacopladores, convertidores de nivel ni fusibles ([ADR-0013](docs/decisiones/ADR-0013-solo-modulos-y-dispositivos.md)).

## Nodos

| Nodo | Placa | Rol | Carpeta |
|---|---|---|---|
| Control | ESP32 DevKit V1 de control | Ciclo, sensores, SSR del PTC y ventiladores | [firmware/control](firmware/control/) |
| SIS | ESP32 Dev Kit | Seguridad: puede cortar el calentador sin pedir permiso | [firmware/sis](firmware/sis/) |
| HMI | ESP32-32E con pantalla 3.2" | Interfaz táctil; sólo comunicación | [firmware/hmi](firmware/hmi/) |

Los tres trabajan a 3,3 V y se conectan entre sí directamente.

## Estructura

```
QuitaSicote3000/
├─ docs/
│  ├─ PLAN-MAESTRO.md            Plan completo de implementación (electrónica + firmware). Empieza aquí
│  ├─ pinout/                    Pinout de cada controlador: control-esp32.md, sis-esp32.md, esp32-hmi.md
│  ├─ requisitos.md              Qué debe hacer el sistema y parámetros provisionales
│  ├─ arquitectura.md            Nodos, reparto de responsabilidades, principios
│  ├─ maquina-de-estados.md      Ciclo de tratamiento (control)
│  ├─ perfiles-de-tratamiento.md Calzado × intensidad × duración
│  ├─ comunicacion.md            Enlaces UART y mensajes
│  ├─ seguridad-sis.md           Funciones de seguridad y su validación
│  └─ decisiones/                ADRs y decisiones abiertas
└─ firmware/
   ├─ control/                   Proyecto PlatformIO del ESP32 DevKit V1 de control
   ├─ sis/                       Proyecto PlatformIO del ESP32 Dev Kit
   ├─ hmi/                       Proyecto PlatformIO del ESP32-32E (LVGL) + PLAN.md
   └─ compartido/qs_protocol.h   Contrato común entre nodos
```

## Convenciones

- Documentación y comentarios en español; identificadores de código en inglés.
- PlatformIO con Arduino-ESP32, un proyecto por nodo.
- Decisiones de diseño como ADR en [docs/decisiones](docs/decisiones/README.md).
