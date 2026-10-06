# ADR-0004 — Asignación de termopares

**Estado:** Aceptada (revisada el 2026-10-06)

## Decisión
El equipo tiene **2 termopares** (se quitó el TC3):
- TC1 (aire de recámara) y TC2 (salida del PTC) → **MCU de control**.
- El **SIS no tiene termopar**.

## Razón
Sólo hay dos sensores. Se prefirió dejarlos al control antes que compartirlos con el SIS: un MAX6675 no debe leerse desde dos nodos, y compartirlo crearía un fallo de causa común.

## Consecuencias
- El SIS pierde SIF-01 (sobretemperatura), SIF-03 (subida brusca), SIF-04 (sensor inválido) y SIF-08 (calentamiento sin efecto). Le quedan: puerta (SIF-02), latido del control (SIF-06) y tiempo máximo de calentamiento (SIF-07).
- **Ya no hay protección de temperatura independiente del control.** La última barrera ante un control que falle "caliente" son el tiempo máximo (125 min) y el **termostato bimetálico de 80 °C**. El límite por temperatura (TC2_MAX y la sobretemperatura suave) vive sólo en el control.
- "Frío" lo estima el SIS **por tiempo**: 10 min sin permiso del PTC (`COOLED_AFTER_MS`) para rearmar, desbloquear, apagar FAN_P y soltar FAN_C.
- El único evento térmico que queda es SIF-07; al 2.º el equipo se bloquea.
- Reponer un TC3 en el SIS devolvería SIF-01/03/04/08 (están descritos en el PLAN-MAESTRO §7). Los bits 0, 2, 3 y 7 de `trip_mask` quedan reservados.
