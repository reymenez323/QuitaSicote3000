# ADR-0009 — Ventilador del PTC por relé, manejado por el SIS

**Estado:** Aceptada (modificada por la ADR-0011: sin compuerta AND ni sensado)

## Contexto
El ventilador del PTC se conmuta con un módulo de relé. Eso permite que quede apagado con el PTC caliente, una de las causas principales de sobrecalentamiento.

## Decisión
- **Contacto NC y disparo por nivel ALTO**: pin en BAJO, al aire o MCU reiniciando ⇒ relé sin energizar ⇒ **ventilador encendido**.
- **Lo maneja sólo el Nano** (D6). La Mega lo pide con `fan_p_off_request` en `HB_CTRL`.
- El SIS lo apaga **sólo** si lo pide la Mega **y** se cumple todo esto: estado OK, permiso del PTC retirado desde hace al menos 60 s, sin disparos y TC3 < 35 °C. Si el SIS no está vivo, el ventilador no se puede apagar.
- El ventilador de circulación (SSR2) también lo maneja el Nano: lo enciende si lo pide la Mega o si lo exige la seguridad.

## Limitación
No hay sensado: no se sabe si el ventilador tiene tensión ni si gira (2 cables, sin tacómetro). Un relé trabado o un ventilador detenido sólo se detecta por la subida de TC3 (SIF-03), por TC2 en la Mega y, en última instancia, por el termostato.
