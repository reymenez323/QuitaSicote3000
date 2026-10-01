# ADR-0003 — Permiso del SIS en serie con el SSR del PTC

**Estado:** Propuesta

## Contexto
Un SSR puede fallar en cortocircuito; el control puede ordenar encendido erróneamente.

## Decisión propuesta
Alimentación del PTC a través de, en serie: termostato(s) NC → **contacto de permiso gobernado por el SIS** → SSR-DC comandado por el control. El permiso es de otro tipo de dispositivo que el SSR (relé electromecánico o SSR de otra tecnología) y funciona con lógica de "energizado = permitido". Se añade realimentación del estado real del permiso al SIS.

## Alternativas
- AND por compuerta lógica sobre la señal del SSR (más simple, pero no protege contra SSR en corto).
- Sólo termostatos (sin capa programable).

## Consecuencias
- (+) Protege incluso con SSR del control en corto.
- (−) Un componente más en la ruta de potencia; hay que dimensionarlo para la corriente del PTC.
