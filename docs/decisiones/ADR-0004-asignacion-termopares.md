# ADR-0004 — Asignación de termopares

**Estado:** Propuesta

## Decisión propuesta
- TC1 (aire de recámara) y TC2 (salida del PTC) → MCU de control.
- TC3 → **exclusivo del SIS**, ubicado en la zona más caliente.

## Razón
Un MAX6675 es un periférico SPI de sólo lectura que no debe compartirse. Si el SIS leyera el mismo sensor que el control, un fallo de ese sensor o de su bus afectaría a ambas capas (fallo de causa común).

## Consecuencia
El SIS depende de un único termopar; su validez se vigila con SIF-04 y los termostatos cubren su fallo.
