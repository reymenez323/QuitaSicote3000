# ADR-0008 — Termostato de rearme automático: enclavado y bloqueo tras 2 eventos

**Estado:** Aceptada (enclavado persistente, bloqueo tras 2 eventos). **Reformulación pendiente** (decisión abierta A-11), porque el SIS ya no puede leer el termostato ([ADR-0013](ADR-0013-solo-modulos-y-dispositivos.md)).

## Contexto
El termostato (TH1) es un **bimetálico de 2 cables** [CONFIRMADO]: un contacto que abre a 80 °C y se rearma solo al enfriarse. Llegar a 80 °C indica una falla grave (p. ej. ventilador parado): no debe reintentarse.

Al ser un único contacto, hay dos formas de cablearlo (decisión abierta A-11 en el [plan maestro](../PLAN-MAESTRO.md#42-abiertas)):

| Opción | Cableado | Ventaja | Inconveniente |
|---|---|---|---|
| **T1** (actual, recomendada) | En serie con la alimentación de 12 V del PTC | Corta el PTC aunque fallen a la vez RL1 y SSR1: barrera totalmente independiente | El SIS no puede leerlo (no hay módulo para leer 12 V) |
| **T2** | En serie con la señal de 3,3 V del SIS hacia RL1; un pin del SIS (con pull-down interno) lee el lado de RL1 | El SIS lo lee y lo enclava (decisión original). Corta el PTC aunque el software del SIS falle | Si el contacto de RL1 se suelda, el termostato ya no corta. Contacto de baja corriente: comprobar que el bimetálico conduce bien a 3,3 V y pocos mA |

## Decisión original
El SIS leía el estado del termostato y, si lo veía abierto, disparaba con enclavado persistente; al 2.º evento, bloqueo hasta servicio.

## Reformulación propuesta con la opción T1 (A-11)
- Se cuentan como **evento térmico** los disparos que el SIS sí detecta:
  - SIF-01: sobretemperatura en TC3 (TC3 montado junto al termostato, para actuar antes que él);
  - SIF-03: subida brusca de TC3;
  - SIF-08: calentamiento sin efecto (el control informa SSR1 encendido pero TC3 baja: posible termostato abierto).
- 1.er evento: disparo enclavado y guardado en memoria persistente (NVS); rearme con enfriamiento, autotest y confirmación desde el HMI.
- 2.º evento: **bloqueo** hasta un procedimiento de servicio.
- Con la opción T2 (o con un módulo sensor de voltaje, A-12) se vuelve a la decisión original: leer el termostato directamente.

## Consecuencias
- El HMI necesita pantallas claras para "sobrecalentamiento" y "equipo bloqueado".
- Pendiente: definir el procedimiento de servicio (A-5).
