# ADR-0013 — Sólo módulos y dispositivos; sin componentes sueltos

**Estado:** Aceptada (preferencia del usuario). Compatible con la regla de máxima prioridad de la [ADR-0012](ADR-0012-regla-mega-control-nano-sis.md).

## Decisión
- Todos los componentes son **módulos o dispositivos** (placas, módulos de relé, SSR, módulos sensores, reguladores). **No** se usan resistencias, condensadores, diodos, transistores ni circuitos integrados sueltos.
- **No** se usan módulos optoacopladores ni convertidores de nivel lógico [CONFIRMADO].
- **No** se usan fusibles ni caja de fusibles [CONFIRMADO].

## Cómo se cumple sin romper la ADR-0012

| Necesidad | Solución |
|---|---|
| Ventilador del PTC: se apaga sólo si el control lo pide **y** el SIS lo permite (AND) | Dos módulos de relé con sus **contactos NC en paralelo**: RL2 (control) y RL3 (SIS) |
| Ventilador de circulación: encendido si el control lo pide **o** el SIS lo fuerza (OR) | **Contacto NA de RL4 (SIS) en paralelo** con la salida del SSR2 (control) |
| Enlaces entre controladores | Todos son ESP32 a 3,3 V: conexión directa, sin convertidor de nivel |
| Polaridad inversa | Conector con polaridad (J1) |

## Lo que se pierde
Sin optoacoplador ni divisores, **el SIS no puede leer nodos de 12 V**:
- no lee el termostato (se infiere: [ADR-0008](ADR-0008-termostato-rearme-automatico.md));
- no detecta un contacto soldado de RL1 ni un SSR1 en corto por medición eléctrica;
- no sabe si el ventilador del PTC tiene tensión.

Se compensa con funciones basadas en TC3 (subida brusca, "calentamiento sin efecto") y con la reacción del control ante sobretemperatura. Opción para recuperar esas lecturas: decisión abierta A-12 (módulo sensor de voltaje).

## Otras consecuencias
- Sin pull-downs: **cada módulo de relé y cada SSR debe quedar desactivado con la entrada al aire** (así queda mientras su controlador arranca). Se comprueba en F3; un módulo que no lo cumpla se cambia.
- Sin fusibles: la única protección contra cortocircuitos es la de la fuente; el cableado principal debe soportar su corriente de protección (riesgo aceptado).
