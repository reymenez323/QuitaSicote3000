# ADR-0001 — Control y SIS en MCU independientes

**Estado:** Aceptada

## Contexto
El equipo calienta aire cerca de materiales combustibles. Un fallo de software del control no debe poder dejar el calentador encendido.

## Decisión
Dos MCU con firmwares separados: **control** (proceso) y **SIS** (seguridad). El SIS tiene autoridad sobre el calentador y no acepta órdenes del control.

## Consecuencias
- (+) Un bug en el PID, el ciclo o la UI no afecta la seguridad.
- (+) El SIS se mantiene pequeño y verificable.
- (−) Hace falta un protocolo entre ambos.
- (−) Riesgo de causa común por usar el mismo toolchain (ver ADR-0002).
