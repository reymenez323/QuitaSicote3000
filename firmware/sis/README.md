# Firmware del SIS (ESP32 DevKit V1 n.º 2)

> **Sólo seguridad.** Ninguna función de control se implementa aquí ([ADR-0012](../../docs/decisiones/ADR-0012-regla-mega-control-nano-sis.md)).

Especificación: [PLAN-MAESTRO §7](../../docs/PLAN-MAESTRO.md#7-firmware-del-sis-esp32-dev-kit) y [seguridad-sis.md](../../docs/seguridad-sis.md). Pines: [docs/pinout/sis-esp32.md](../../docs/pinout/sis-esp32.md).

## Archivos

| Archivo | Contenido |
|---|---|
| [src/config.h](src/config.h) | Pines y **todos** los umbrales. Ninguno se recibe por la comunicación |
| [src/main.cpp](src/main.cpp) | Todo lo demás, en este orden: estado compartido → memoria → disparo, rearme y servicio → funciones de seguridad → las tres tareas → arranque |

## Cómo funciona

- `taskSafety` (10 ms) lee la puerta, evalúa las funciones de seguridad y escribe los tres relés: **RL1** (permiso del PTC), **RL3** (permiso de apagar el ventilador del PTC) y **RL4** (forzar el ventilador de circulación).
- `taskThermocouple` (250 ms) lee TC3 y decide si es creíble (abierto, fuera de rango o congelado = inválido).
- `taskLink` recibe `HB_CTRL`, `REQ_RESET` y `REQ_SERVICE`; envía `HB_SIS` y los avisos; guarda en NVS para que la tarea de seguridad nunca espere a la flash.
- Lo que llega del control **sólo puede restringir**: `heat_request` puede quitar el permiso, nunca concederlo por sí solo.

## Estados

```
ARRANQUE ──autotest correcto──► OK ──disparo──► DISPARADO ──rearme aceptado──► OK
                                                    │
                                     2.º evento térmico ──► BLOQUEADO ──servicio──► DISPARADO
```

En ARRANQUE, DISPARADO y BLOQUEADO el PTC no tiene permiso. Los eventos térmicos (SIF-01, SIF-03, SIF-08) se cuentan y se guardan: sobreviven a un corte de energía.

LED de la placa: fijo = OK · parpadeo lento = arrancando · rápido = disparado · destello corto = bloqueado.

## Decisiones abiertas aplicadas (cambiar si se decide otra cosa)

- **A-11 → T1**: el termostato va en serie con el PTC y el SIS no lo lee; lo infiere con SIF-08.
- **A-4**: perder el latido del control con el permiso concedido es un disparo enclavado (SIF-06).
- **A-5**: desbloqueo = comando `servicio desbloquear` en la consola del control + abrir y cerrar la puerta 3 veces en 30 s.
