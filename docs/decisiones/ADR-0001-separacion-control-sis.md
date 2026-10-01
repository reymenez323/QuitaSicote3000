# ADR-0001 — Dos MCU independientes: control y SIS

**Estado:** Aceptada

## Contexto
El sistema calienta aire cerca de materiales combustibles (calzado). Un fallo de software del controlador no debe poder dejar el calentador encendido.

## Decisión
Separar en dos MCU con firmware, repositorio de código y toolchain-proyecto distintos: **control** (proceso/UI) y **SIS** (seguridad). El SIS tiene autoridad sobre el calentador y no acepta órdenes del control.

## Consecuencias
- (+) Un bug en la UI, el log o el PID no afecta la seguridad.
- (+) El SIS puede mantenerse pequeño y verificable.
- (−) Más hardware, más cableado, necesidad de un protocolo entre ambos.
- (−) Hay que evitar fallo de causa común (misma fuente, misma tierra ruidosa, mismo toolchain): ver ADR-0002.
