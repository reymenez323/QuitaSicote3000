# ADR-0006 — Producto doméstico, sin botón físico de paro

**Estado:** Aceptada por el usuario; riesgos y mitigaciones obligatorias

## Contexto
- El dispositivo es de **uso doméstico**: lo opera una persona no técnica, posiblemente sin supervisión, con niños o mascotas cerca.
- **No habrá botón físico de paro.** Todo el control es por la pantalla.

## Consecuencia principal
Si el HMI o la Mega se cuelgan, el usuario no tiene forma inmediata de parar el ciclo salvo abrir la puerta o desenchufar. El diseño debe garantizar que **ninguna de esas dos fallas deje el calentador funcionando sin límite**.

## Mitigaciones obligatorias

| ID | Mitigación | Dónde |
|---|---|---|
| M-01 | **Abrir la puerta es el paro de emergencia**: el SIS corta el PTC (SIF-02) y el ventilador sigue para enfriar | SIS |
| M-02 | Pérdida de enlace con el HMI > 10 s [VERIFICAR] → la Mega aborta el ciclo y pasa a enfriamiento | Control |
| M-03 | Tiempo máximo de ciclo fijo en firmware, no editable por el usuario más allá de un tope | Control + SIS (SIF-07) |
| M-04 | El SIS sigue dependiendo sólo de sus propias lecturas y de los termostatos | SIS / hardware |
| M-05 | Inicio de ciclo sólo con puerta cerrada y autotest correcto | Control + SIS (el SIS no concede permiso con la puerta abierta) + HMI (Iniciar deshabilitado) |
| M-06 | Alimentación mediante **fuente externa certificada** de baja tensión; el dispositivo no tiene tensión de red expuesta | Hardware |
| M-07 | Tras un corte de energía el ciclo **no se reanuda solo**: vuelve a LISTO | Control |
| M-08 | Superficies accesibles con límite de temperatura y puerta/rejilla que impidan contacto con el PTC | Mecánica |
| M-09 | Indicación visible y sonora de ciclo activo y de falla | HMI / SIS |

## Riesgo residual aceptado
Una persona no puede detener el calentamiento *instantáneamente* si la pantalla falla y no abre la puerta. Se recomienda reconsiderar un paro físico (aunque sea un interruptor que corte la alimentación del PTC) antes de cualquier uso fuera del banco de pruebas.

## Referencias normativas a consultar [VERIFICAR; no se declara conformidad]
- IEC 60335-1 (seguridad de aparatos electrodomésticos) y parte particular aplicable a secadores/calentadores.
- IEC 60730-1 / Anexo H (controles automáticos eléctricos con funciones de seguridad).
- Normas de fuente de alimentación externa (IEC 62368-1 / 61558) y de compatibilidad electromagnética según el mercado.
