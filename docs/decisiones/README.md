# Decisiones de diseño (ADR)

| ID | Decisión | Estado |
|---|---|---|
| [ADR-0001](ADR-0001-separacion-control-sis.md) | Control y SIS en MCU independientes | Aceptada |
| [ADR-0002](ADR-0002-mcu-del-sis.md) | SIS en Arduino Nano, control en Mega | Aceptada |
| ADR-0003 | (Retirada; reemplazada por la ADR-0010) | — |
| [ADR-0004](ADR-0004-asignacion-termopares.md) | TC1/TC2 al control, TC3 exclusivo del SIS | Propuesta |
| [ADR-0005](ADR-0005-hmi-esp32.md) | Pantalla ESP32-32E como nodo HMI sólo de comunicación | Aceptada |
| [ADR-0006](ADR-0006-producto-domestico-sin-paro-fisico.md) | Producto doméstico, sin paro físico | Aceptada |
| [ADR-0007](ADR-0007-perfiles-en-el-control.md) | Perfiles en la Mega; SIS con límite único | Propuesta |
| [ADR-0008](ADR-0008-termostato-rearme-automatico.md) | Enclavado en EEPROM y bloqueo tras 2 eventos | Aceptada; reformulación pendiente (A-11) |
| [ADR-0009](ADR-0009-ventilador-ptc-con-rele.md) | Ventilador del PTC por relé NC con disparo alto, manejado por el SIS | Aceptada |
| [ADR-0010](ADR-0010-permiso-ptc-modulo-rele.md) | Permiso del PTC con un módulo de relé en serie | Aceptada |
| [ADR-0011](ADR-0011-sin-componentes-auxiliares.md) | Sin componentes auxiliares pequeños; dos reguladores | Aceptada |

## Decisiones abiertas

Detalle, opciones y recomendación en [PLAN-MAESTRO §4.2](../PLAN-MAESTRO.md#42-abiertas):

| ID | Pregunta |
|---|---|
| A-9 | Cómo bajar a 3,3 V la señal de la Mega hacia el ESP32 (recomendado: módulo convertidor de nivel) |
| A-10 | ¿Sin fusibles? (recomendado: al menos uno general y en las ramas de cable fino) |
| A-11 | Aplicar el bloqueo tras 2 eventos a los eventos térmicos que el SIS sí detecta |
| A-4 | Pérdida del heartbeat del control: ¿disparo enclavado o sólo retirar el permiso? |
| A-5 | Procedimiento de servicio tras el bloqueo |
| A-6 | ¿VOC/HR como criterio de fin anticipado? |
| A-7 | Registro de ciclos: USB de la Mega o SD del HMI |
| A-8 | Nombres definitivos de los 4 calzados |
