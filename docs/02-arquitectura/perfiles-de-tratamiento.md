# Perfiles de tratamiento

El usuario no ajusta temperaturas, porcentajes ni minutos. En el HMI elige entre **opciones discretas**: **tipo de calzado**, **intensidad** (3 opciones) y **duración** (3 opciones). La Mega traduce esa selección a parámetros de proceso.

## Flujo en el HMI

1. Tipo de calzado.
2. Intensidad: **Suave / Media / Intensa**.
3. Duración: **Corta / Media / Larga**.
4. Resumen y confirmación.
5. Cerrar la puerta e iniciar. **El botón Iniciar sólo se habilita con la puerta cerrada** (estado informado por la Mega).

## Reglas

- El HMI envía sólo **identificadores**: `calzado_id`, `intensidad_id` (0–2), `duracion_id` (0–2). No envía temperaturas ni minutos.
- Cualquier identificador fuera de rango es rechazado por la Mega.
- La tabla vive en el firmware de la Mega (`firmware/control/src/config/`), no en el HMI. Los nombres de las opciones y los identificadores están en `firmware/compartido/protocolo/`.
- Intensidad y duración son **independientes**: la intensidad fija la temperatura según el tipo de calzado; la duración fija el tiempo de tratamiento.
- La duración cuenta el **tiempo de tratamiento a temperatura**. El precalentamiento tiene su propio tiempo límite; si no se alcanza la temperatura, se declara falla (PTC o ventilador defectuoso).
- El SIS **no conoce** la selección: tiene un único límite fijo ([ADR-0007](../decisiones/ADR-0007-perfiles-en-el-control.md)).
- Como las opciones son un conjunto cerrado, toda combinación posible puede probarse completa: 4 tipos × 3 × 3 = 36 casos (ver [plan de pruebas](../04-pruebas/plan-de-pruebas.md)).

## Temperatura del aire de recámara (TC1) según tipo e intensidad **[VERIFICAR con ensayos]**

| Tipo | Suave | Media | Intensa |
|---|---|---|---|
| Cuero / piel | 35 °C | 40 °C | 45 °C |
| Tela / deportivo / malla | 40 °C | 45 °C | 50 °C |
| Bota forrada / acolchada gruesa | 40 °C | 45 °C | 50 °C |
| Goma / sintético con adhesivos | 38 °C | 42 °C | 48 °C |

## Duración de tratamiento **[VERIFICAR con ensayos]**

| Opción | Tiempo |
|---|---|
| Corta | 20 min |
| Media | 40 min |
| Larga | 70 min |
| **Tope global (constante en Mega y SIS, SIF-07)** | **120 min** |

Son hipótesis iniciales, no resultados. Se validan en los niveles 5 y 6 del plan de pruebas. El número de tipos de calzado (4) está confirmado por el usuario; los nombres y las temperaturas siguen siendo propuestas.
