# Firmware del SIS (Arduino Nano)

**Pequeño, determinista, aburrido.** Especificación completa: [PLAN-MAESTRO §7](../../docs/PLAN-MAESTRO.md#7-firmware-del-sis-arduino-nano). Pines: [docs/pinout/nano.md](../../docs/pinout/nano.md).

## Reglas

- Sin RTOS, sin asignación dinámica, sin `float`, sin bibliotecas de terceros.
- Lazo cíclico de 10 ms con watchdog de 250 ms (limpiar `MCUSR` + `wdt_disable()` al arrancar; ver [ADR-0002](../../docs/decisiones/ADR-0002-mcu-del-sis.md)).
- Umbrales como constantes de compilación en `config/`. Ninguno se recibe por la comunicación.
- Lo que llega de la Mega **sólo puede restringir**.
- Cada condición de disparo es una función pura probada en PC.

## Responsabilidades

1. Leer TC3 y la puerta (NA/NC).
2. Evaluar las [funciones de seguridad](../../docs/seguridad-sis.md).
3. Manejar **RL1** (permiso del PTC, nunca con la puerta abierta), **RL2** (ventilador del PTC) y **SSR2** (ventilador de circulación), combinando sus condiciones con las peticiones de la Mega.
4. Informar estado y causas de disparo al control.

## Módulos (`lib/` lógica pura, `src/` hardware)

```
lib/sis_logic/   door, tc_validator, trend (pendiente y "sin efecto"), sif, sis_fsm, persist
src/             main (lazo + WDT), hw_io, hw_max6675, hw_eeprom, link, config/
```

## Estados

```
ARRANQUE ──autotest OK──► OK ──disparo──► DISPARADO ──rearme válido──► OK
    └──autotest falla──────────────────────────┘
DISPARADO ──2.º evento térmico──► BLOQUEADO (servicio)
```

- En ARRANQUE, DISPARADO y BLOQUEADO, RL1 está abierto y los ventiladores encendidos.
- Los eventos térmicos y su contador se guardan en EEPROM ([ADR-0008](../../docs/decisiones/ADR-0008-termostato-rearme-automatico.md)).
- Rearme: causa desaparecida, enfriamiento y solicitud `REQ_RESET` que el SIS valida.
