# ADR-0009 — Ventilador del PTC: relé del control y relé de veto del SIS en paralelo

**Estado:** Aceptada. Implementación con módulos según la [ADR-0013](ADR-0013-solo-modulos-y-dispositivos.md).

## Contexto
El ventilador del PTC (FAN_P) se conmuta con un módulo de relé. Eso permite que quede apagado con el PTC caliente, una de las causas principales de sobrecalentamiento. Por la ADR-0012, encender y apagar el ventilador es **control (ESP32-S3)** y vetar su apagado es **seguridad (ESP32 Dev Kit)**.

## Decisión
- FAN_P se alimenta a través de **dos contactos NC en paralelo**:
  - **RL2** (el módulo del usuario), manejado por el **control**. ALTO = pedir apagado.
  - **RL3** (canal de un módulo de 2 relés), manejado por el **SIS**. ALTO = permitir apagado.
- El ventilador se apaga **sólo si los dos relés están energizados**. Con cualquiera de los dos controladores sin alimentación, arrancando o colgado, su relé queda sin energizar y el ventilador gira.
- Ambos módulos con **disparo por nivel ALTO** compatible con 3,3 V, contacto **NC**.
- El control sólo pide el apagado con el PTC apagado y TC2 < 35 °C durante un tiempo.
- El SIS sólo lo permite con su permiso del PTC retirado desde hace al menos 60 s, TC3 < 35 °C y sin disparos.
- El ventilador de circulación (SSR2) lo maneja el control; el SIS puede forzarlo con el contacto NA de **RL4** en paralelo con la salida del SSR2.

## Limitación
No hay sensado: no se sabe si el ventilador tiene tensión ni si gira (2 cables, sin tacómetro, y sin módulos de lectura de 12 V). Un relé trabado o un ventilador detenido sólo se detecta por la subida de TC3 (SIF-03), por TC2 en el control y, en última instancia, por el termostato.
