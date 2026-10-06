# Perfiles de tratamiento

El usuario elige **opciones discretas**; nunca ve ni introduce temperaturas, porcentajes o minutos.

| Categoría | Opciones |
|---|---|
| Calzado (4) | Cuero · Deportivo · Bota · Sintético (nombres provisionales) |
| Intensidad (3) | Suave · Media · Intensa |
| Duración (3) | Corta · Media · Larga |

## Reglas

- El HMI envía sólo `calzado_id` (0–3), `intensidad_id` (0–2) y `duracion_id` (0–2). El control rechaza ids fuera de rango.
- La tabla de valores vive en el control ([ADR-0007](decisiones/ADR-0007-perfiles-en-el-control.md)); los ids, en `firmware/compartido/qs_protocol.h`; los valores, en `firmware/control/src/config.h`.
- La intensidad fija la temperatura según el calzado; la duración fija el tiempo de tratamiento a temperatura.
- El SIS no conoce el perfil: tiene un único límite fijo.
- 4 × 3 × 3 = 36 combinaciones: todas se prueban.

## Valores de partida (hipótesis, a validar con ensayos)

Temperatura del aire de recámara (TC1):

| Calzado | Suave | Media | Intensa |
|---|---|---|---|
| Cuero | 35 °C | 40 °C | 45 °C |
| Deportivo (tela/malla) | 40 °C | 45 °C | 50 °C |
| Bota (forrada/gruesa) | 40 °C | 45 °C | 50 °C |
| Sintético (goma/adhesivos) | 38 °C | 42 °C | 48 °C |

Duración del tratamiento:

| Corta | Media | Larga | Tope global |
|---|---|---|---|
| 20 min | 40 min | 70 min | 120 min |
