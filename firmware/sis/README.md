# Firmware del SIS (ESP32 Dev Kit)

> **Sólo seguridad.** Ninguna función de control se implementa aquí ([ADR-0012](../../docs/decisiones/ADR-0012-regla-mega-control-nano-sis.md)).

**Pequeño, determinista, aburrido.** Especificación completa: [PLAN-MAESTRO §7](../../docs/PLAN-MAESTRO.md#7-firmware-del-sis-esp32-dev-kit). Pines: [docs/pinout/sis-esp32.md](../../docs/pinout/sis-esp32.md).

## Reglas

- **Wi-Fi y Bluetooth apagados** desde el arranque.
- Un único lazo de 10 ms en `loop()`; Task Watchdog de 1 s sobre ese lazo; detector de caída de tensión activo.
- Sin bibliotecas de terceros; sin asignación dinámica tras el arranque; aritmética entera.
- Umbrales como constantes de compilación en `config/`. Ninguno se recibe por la comunicación.
- Lo que llega del control **sólo puede restringir**.
- Cada condición de disparo es una función pura probada en PC.

## Responsabilidades

1. Leer TC3 y la puerta (NA/NC).
2. Evaluar las [funciones de seguridad](../../docs/seguridad-sis.md).
3. Manejar **RL1** (permiso del PTC, nunca con la puerta abierta), **RL3** (permiso de apagado del ventilador del PTC) y **RL4** (forzado del ventilador de circulación). Los ventiladores los maneja el control; el SIS sólo veta o fuerza con sus propios relés.
4. Informar estado y causas de disparo al control.

## Módulos (`lib/` lógica pura, `src/` hardware)

```
lib/sis_logic/   door, tc_validator, trend (pendiente y "sin efecto"), sif, sis_fsm, persist
src/             main (lazo + watchdog), hw_io, hw_max6675, hw_store (NVS), link, config/
```

## Estados

```
ARRANQUE ──autotest OK──► OK ──disparo──► DISPARADO ──rearme válido──► OK
    └──autotest falla──────────────────────────┘
DISPARADO ──2.º evento térmico──► BLOQUEADO (servicio)
```

- En ARRANQUE, DISPARADO y BLOQUEADO, RL1 está abierto.
- Los eventos térmicos y su contador se guardan en NVS ([ADR-0008](../../docs/decisiones/ADR-0008-termostato-rearme-automatico.md)).
- Rearme: causa desaparecida, enfriamiento y solicitud `REQ_RESET` que el SIS valida.
