# QuitaSicote3000

Sistema para eliminar el mal olor del calzado mediante **circulación de aire forzado y calentamiento controlado** de una recámara, con una capa de seguridad independiente (SIS).

Proyecto de DPC. Producto de **uso doméstico**, controlado únicamente desde su pantalla (sin paro físico; ver [ADR-0006](docs/decisiones/ADR-0006-producto-domestico-sin-paro-fisico.md)).

Repositorio: https://github.com/reymenez323/QuitaSicote3000

## Mapa del repositorio

```
QuitaSicote3000/
├─ docs/                  Documentación (requisitos, arquitectura, seguridad, pruebas, ADRs)
│  ├─ 01-requisitos/      Qué debe hacer el sistema y con qué límites
│  ├─ 02-arquitectura/    Cómo está diseñado (hardware, firmware, comunicación, estados)
│  ├─ 03-seguridad/       Análisis de peligros, funciones de seguridad (SIS), validación
│  ├─ 04-pruebas/         Plan de pruebas y registros de resultados
│  └─ decisiones/         ADRs: una decisión de diseño por archivo
├─ hardware/              Todo lo físico
│  ├─ bom/                Lista de materiales
│  ├─ esquematicos/       Esquemáticos eléctricos
│  ├─ cableado/           Diagramas de conexión y arneses
│  ├─ pinout/             Asignación de pines por microcontrolador
│  ├─ mecanica/           Recámara, ductos, soportes, ubicación de sensores
│  └─ datasheets/         Hojas de datos de componentes
├─ firmware/
│  ├─ control/            Arduino Mega: control, sensado ambiental, actuadores
│  ├─ sis/                Arduino Nano: Safety Instrumented System
│  ├─ hmi/                ESP32-32E con pantalla: UI, sólo comunicación
│  └─ compartido/         Contrato entre ambos MCU (protocolo, constantes)
├─ tools/                 Scripts auxiliares (análisis de logs, simuladores, utilidades)
└─ datos/                 Logs de ensayos y datos de calibración
```

## Arquitectura en una frase

Dos microcontroladores **independientes**: el de **control** decide *qué hacer* (ciclo, PID, UI); el **SIS** decide *si está permitido* y puede cortar el calentador sin pedir permiso. Encima de ambos, **termostatos térmicos en serie** con la alimentación del PTC actúan como última barrera sin software.

Detalle completo en [docs/02-arquitectura/vision-general.md](docs/02-arquitectura/vision-general.md).

## Por dónde empezar

1. [Requisitos](docs/01-requisitos/requisitos.md)
2. [Visión general](docs/02-arquitectura/vision-general.md)
3. [Hardware](docs/02-arquitectura/hardware.md)
4. [Funciones de seguridad](docs/03-seguridad/funciones-de-seguridad.md)
5. [Decisiones abiertas](docs/decisiones/README.md)

## Convenciones

- Idioma: documentación y comentarios en español; identificadores de código en inglés.
- Firmware: PlatformIO, un proyecto por MCU (`firmware/control`, `firmware/sis`). Ver [firmware/README.md](firmware/README.md).
- Cada decisión de diseño importante se registra como ADR en `docs/decisiones/`.
