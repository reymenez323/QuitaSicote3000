# ADR-0011 — Sin componentes auxiliares pequeños

**Estado:** **Revocada** por el usuario (2026-09-30). Sustituida por la ADR-0012.

## Qué decía
No usar compuertas, transistores, diodos, resistencias ni fusibles sueltos.

## Por qué se revocó
Sin compuertas, la única forma de que el SIS conservara la capacidad de vetar el apagado del ventilador del PTC y de forzar el de circulación era que el Nano manejara los ventiladores. Eso convertía al Nano en controlador de proceso, en contra de la regla de máxima prioridad de la ADR-0012: **control = Mega, SIS = Nano**.

## Relación con la ADR-0013
La preferencia del usuario por usar sólo módulos y dispositivos se recoge en la [ADR-0013](ADR-0013-solo-modulos-y-dispositivos.md), que la resuelve **sin** mover funciones entre la Mega y el Nano: la lógica AND/OR se hace con contactos de módulos de relé y el sensado con un módulo optoacoplador.

## Lo que sigue vigente de esa etapa
- Dos reguladores de 5 V independientes (decisión del usuario, se mantiene).
- El permiso del PTC con un módulo de relé (ADR-0010, se mantiene).
