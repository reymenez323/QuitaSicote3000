# Decisiones de diseño (ADR)

> **Regla de máxima prioridad ([ADR-0012](ADR-0012-regla-mega-control-nano-sis.md))**: el control es el **ESP32-S3** y el SIS es el **ESP32 Dev Kit**. Ninguna función se mueve de uno a otro.
> **Preferencia de hardware ([ADR-0013](ADR-0013-solo-modulos-y-dispositivos.md))**: sólo módulos y dispositivos; sin optoacopladores, convertidores de nivel ni fusibles.

| ID | Decisión | Estado |
|---|---|---|
| [ADR-0012](ADR-0012-regla-mega-control-nano-sis.md) | **Control y SIS en controladores separados, estrictamente** (ESP32-S3 / ESP32 Dev Kit) | **Aceptada, prioritaria** |
| [ADR-0013](ADR-0013-solo-modulos-y-dispositivos.md) | Sólo módulos y dispositivos; lógica con contactos de relé | Aceptada |
| [ADR-0001](ADR-0001-separacion-control-sis.md) | Control y SIS en MCU independientes | Aceptada |
| [ADR-0002](ADR-0002-mcu-del-sis.md) | Control en ESP32-S3, SIS en ESP32 Dev Kit (antes Mega y Nano) | Aceptada |
| ADR-0003 | (Retirada; reemplazada por la ADR-0010) | — |
| [ADR-0004](ADR-0004-asignacion-termopares.md) | TC1/TC2 al control, TC3 exclusivo del SIS | Propuesta |
| [ADR-0005](ADR-0005-hmi-esp32.md) | Pantalla ESP32-32E como nodo HMI sólo de comunicación | Aceptada |
| [ADR-0006](ADR-0006-producto-domestico-sin-paro-fisico.md) | Producto doméstico, sin paro físico | Aceptada |
| [ADR-0007](ADR-0007-perfiles-en-el-control.md) | Perfiles en el control; SIS con límite único | Propuesta |
| [ADR-0008](ADR-0008-termostato-rearme-automatico.md) | Termostato bimetálico: enclavado y bloqueo tras 2 eventos; cableado T1/T2 | Aceptada; cableado pendiente (A-11) |
| [ADR-0009](ADR-0009-ventilador-ptc-con-rele.md) | Ventilador del PTC: relé del control y relé de veto del SIS (NC en paralelo) | Aceptada |
| [ADR-0010](ADR-0010-permiso-ptc-modulo-rele.md) | Permiso del PTC con un módulo de relé en serie | Aceptada |
| [ADR-0011](ADR-0011-sin-componentes-auxiliares.md) | Sin componentes auxiliares (versión que movía los ventiladores al SIS) | **Revocada** |

## Decisiones abiertas

Detalle, opciones y recomendación en [PLAN-MAESTRO §4.2](../PLAN-MAESTRO.md#42-abiertas):

| ID | Pregunta |
|---|---|
| **A-11** | Dónde cablear el termostato bimetálico: en serie con el PTC (T1, recomendado) o en la señal del SIS hacia RL1 (T2, se puede leer) |
| **A-12** | ¿Módulo sensor de voltaje para que el SIS lea los nodos de 12 V? |
| **A-13** | Modelos exactos de las placas ESP32-S3 y ESP32 Dev Kit |
| A-4 | Pérdida del heartbeat del control: ¿disparo enclavado o sólo retirar el permiso? |
| A-5 | Procedimiento de servicio tras el bloqueo |
| A-6 | ¿VOC/HR como criterio de fin anticipado? |
| A-7 | Registro de ciclos: USB del control o SD del HMI |
| A-8 | Nombres definitivos de los 4 calzados |
