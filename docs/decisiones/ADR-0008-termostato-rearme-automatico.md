# ADR-0008 — Termostato de rearme automático: enclavado y bloqueo tras 2 eventos

**Estado:** Aceptada (enclavado en EEPROM, bloqueo tras 2 eventos). **Reformulación pendiente** por la ADR-0011 (decisión abierta A-11).

## Contexto
El termostato de 80 °C en serie con el PTC se rearma solo al enfriarse. Llegar a 80 °C indica una falla grave (p. ej. ventilador parado): no debe reintentarse.

## Decisión original
El SIS leía el estado del termostato y, si lo veía abierto, disparaba con enclavado en EEPROM; al 2.º evento, bloqueo hasta servicio.

## Por qué hay que reformularla
Sin divisor de resistencias (ADR-0011), el Nano no puede leer un nodo de 12 V: **el termostato no se puede leer**.

## Reformulación propuesta (A-11)
- Se cuentan como **evento térmico** los disparos que el SIS sí detecta y que corresponden a la situación del termostato:
  - SIF-01: sobretemperatura en TC3 (montado junto al termostato, actúa antes que él);
  - SIF-03: subida brusca de TC3;
  - SIF-08: calentamiento sin efecto (el PTC recibe orden pero TC3 baja: posible termostato abierto).
- 1.er evento: disparo enclavado, guardado en **EEPROM**; rearme con enfriamiento, autotest y confirmación desde el HMI.
- 2.º evento: **bloqueo** hasta un procedimiento de servicio.

## Consecuencias
- El HMI necesita pantallas claras para "sobrecalentamiento" y "equipo bloqueado".
- Pendiente: definir el procedimiento de servicio (A-5).
