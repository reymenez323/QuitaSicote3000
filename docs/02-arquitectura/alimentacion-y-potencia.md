# Alimentación y potencia

Datos del usuario: fuente externa de **12 V DC**; PTC de **100 W a 12 V**; un termostato de **80 °C**; SHT31 y SGP40 toleran 5 V.

## 1. Presupuesto de corriente (a 12 V)

| Carga | Corriente estimada | Nota |
|---|---|---|
| PTC | **≈ 8,3 A** (100 W / 12 V) en régimen | Pico de arranque en frío mayor **[VERIFICAR hoja de datos]** |
| Ventilador del PTC | [VERIFICAR] A (12 V, vía módulo de relé) | Más bobina del relé ≈ 70 mA |
| Ventilador de circulación | [VERIFICAR] A | |
| Ventilador de la cámara de circuitos | [VERIFICAR] A (12 V, directo, siempre encendido) | |
| Lógica (Mega + Nano + HMI + sensores) | ≈ 0,5 A (≈ 5 W a 5 V tras el buck) | HMI por USB-C a 5 V |
| **Total aproximado** | **≈ 9–10 A** | |

Fuente del usuario: **12 V / 20 A (240 W)**. Alcanza con margen (~50 %) para el arranque del PTC y el envejecimiento. Debe ser de marca certificada. La corriente de arranque del PTC sigue sin conocerse: medirla en el nivel 2 del plan de pruebas. Ambos ventiladores son de 12 V.

## 2. Distribución

```
Fuente 12 V ─► Entrada con protección contra polaridad inversa + TVS + fusible general
   ├─► Rama PTC:  fusible ─► termostato 80 °C ─► relé de permiso (SIS) ─► SSR-DC (control) ─► PTC
   ├─► Ventilador del PTC: módulo de relé 12 V, contacto NC (reposo = encendido) con permiso de apagado del SIS
   ├─► Ventilador de circulación: SSR-DC (control; el SIS puede forzarlo a ON)
   ├─► Ventilador de la cámara de circuitos: directo a 12 V, siempre encendido
   ├─► Buck A 5 V ─► Mega + HMI (USB-C) + SHT31 + SGP40 + MAX6675 TC1/TC2
   └─► Buck B 5 V ─► Nano + MAX6675 TC3 (alimentación independiente del SIS) + bobina del relé vía transistor
```

- **Reguladores independientes** para el control y para el SIS: un fallo o ruido en uno no debe reiniciar al otro.
- Usar **convertidores buck** (conmutados) y no el regulador lineal de la Mega ni del Nano: a 12 V disipan demasiado calor. Alimentar al pin 5 V pasa por alto sus protecciones, así que cada buck lleva su fusible/TVS.
- Condensadores de bulto en cada entrada de buck y **retornos en estrella** hacia un único punto de tierra junto a la fuente; que los 8 A del PTC no compartan cable de retorno con la lógica.

## 3. Componentes de potencia

| Elemento | Especificación mínima propuesta |
|---|---|
| SSR-DC del PTC | De salida MOSFET, **≥ 25 A** a 12–24 V DC, con disipador; respetar polaridad. Entrada 3–32 V: se maneja con 5 V de la Mega. Conmutación **lenta** (periodo ≈ 1–2 s, proporcional al tiempo). |
| Relé de permiso (SIS) | Electromecánico automotriz, bobina 12 V, contactos **≥ 30 A DC**. Activado = permitido. Pull-down en el transistor de la bobina: sin señal del Nano, abre. |
| Termostato 80 °C | NC, **≥ 10 A a 12 V DC** [VERIFICAR: sus datos suelen darse para CA]. Ver §4. |
| Fusible | De acción lenta, 12–15 A **[VERIFICAR contra el arranque del PTC]**; el cable debe soportar más que el fusible. |
| Cableado de potencia | ≥ 14 AWG (2,5 mm²) en la rama del PTC; conectores con ≥ 15 A continuos. |

## 4. El termostato de 80 °C

- **Colocación**: sobre la carcasa del PTC o en la salida inmediata de su flujo, **no** en el aire de la recámara. Si el ventilador falla, el PTC se calienta mucho antes de que el aire de la recámara lo refleje.
- **Tolerancia**: los termostatos bimetálicos suelen tener ±5 °C; puede abrir entre 75 y 85 °C. El disparo del SIS debe quedar claramente por debajo (≈ 65–70 °C, ver [funciones-de-seguridad.md](../03-seguridad/funciones-de-seguridad.md)).
- **Uno solo es un punto único de fallo** de la capa 3 (los contactos pueden quedar pegados). **Decisión del usuario: no se añadirán más dispositivos**, por lo que no habrá fusible térmico. Riesgo residual aceptado: si el termostato falla cerrado y además fallan el SIS y el SSR, no hay otra barrera. El termostato es de **rearme automático** (dato del usuario): al enfriarse volvería a cerrar y a energizar el PTC. Por eso el SIS lo vigila y enclava el disparo ([ADR-0008](../decisiones/ADR-0008-termostato-rearme-automatico.md), SIF-08).
- **Ventilador del PTC (módulo de relé de 12 V)**: va por el contacto **NC** (reposo = encendido), aguas abajo del fusible general y **no** por la rama del PTC (el termostato y el relé de permiso no lo cortan). El SIS concede el "permiso de apagado" y sensa su alimentación con un divisor. Detalle y razones en [ADR-0009](../decisiones/ADR-0009-ventilador-ptc-con-rele.md).
- **Sensado**: el SIS lee un nodo tras el termostato mediante divisor resistivo (12 V → ≤ 5 V) con pull-down; nodo bajo = termostato abierto.

## 5. Ventilador de la cámara de circuitos

Directo a 12 V (decisión del usuario), siempre encendido mientras el equipo esté alimentado, tras la protección de polaridad inversa y el fusible general. Recomendaciones:
- Que tome **aire exterior**, no el de la recámara (caliente, húmedo, con vapores del calzado), y con rejilla/filtro contra polvo.
- Un condensador pequeño en sus terminales para no inyectar ruido al riel.
- Su falla no se detecta (2 cables, sin sensor): el riesgo es sobrecalentar reguladores y electrónica, incluidos los del SIS. Se acepta; se puede revisar con una medición de temperatura en el nivel 5 de pruebas.

## 6. Cosas que tu fuente de 12 V NO resuelve
- La rama del PTC está a 8 A: aunque 12 V sea baja tensión (poco riesgo de descarga), **sí hay riesgo de incendio** por cables, conectores o contactos flojos. Por eso el fusible y la calidad de conectores no son opcionales.
