# ADR-0006 — Producto doméstico, sin botón físico de paro

**Estado:** Aceptada

## Contexto
Uso doméstico, por personas no técnicas y posiblemente sin supervisión. No habrá botón físico de paro: todo se controla desde la pantalla.

## Consecuencia
Si el HMI o el control se cuelgan, el usuario sólo puede abrir la puerta o desenchufar. Ninguna de esas fallas puede dejar el calentador funcionando sin límite.

## Mitigaciones en software

| ID | Mitigación | Nodo |
|---|---|---|
| M-01 | Abrir la puerta es el paro: el SIS corta el PTC (SIF-02) y los ventiladores siguen | SIS |
| M-02 | Sin enlace con el HMI > 10 s → el control cancela y enfría | Control |
| M-03 | Tope de ciclo fijo (120 min) | Control + SIS (SIF-07) |
| M-04 | Inicio sólo con puerta cerrada y autotest correcto | HMI + Control + SIS |
| M-05 | Tras un corte de energía no se reanuda el ciclo | Control |
| M-06 | Indicación clara de ciclo activo y de falla | HMI |
| M-07 | Pausa máxima de 5 min | Control |
