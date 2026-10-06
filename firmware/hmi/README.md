# Firmware del HMI (ESP32-32E, pantalla 3.2")

Interfaz táctil del equipo, hecha con **LVGL 9** y **LovyanGFX**. No lee entradas ni maneja salidas del proceso: sólo dibuja, lee su táctil y se comunica con el control ([ADR-0005](../../docs/decisiones/ADR-0005-hmi-esp32.md)).

Diseño de las pantallas y criterios de uso: [PLAN.md](PLAN.md). Pines: [docs/pinout/esp32-hmi.md](../../docs/pinout/esp32-hmi.md).

## Archivos

| Archivo | Contenido |
|---|---|
| [include/lv_conf.h](include/lv_conf.h) | Configuración de LVGL (memoria, fuentes) |
| [src/display.h](src/display.h) | Pines de la placa y configuración de la pantalla y el táctil |
| [src/ui.cpp](src/ui.cpp) | Las pantallas: textos → colores → piezas comunes → botones → pantallas → selección |
| [src/ui.h](src/ui.h) | Lo que se comparten `ui.cpp` y `main.cpp` |
| [src/main.cpp](src/main.cpp) | Las dos tareas, el enlace con el control y la unión de LVGL con la pantalla |

## Cómo funciona

- `taskLink` recibe `STATUS` del control (cada 200 ms), envía el latido (cada 500 ms) y las solicitudes de los botones.
- `taskUi` muestra la pantalla que corresponde al estado que informa el control. Sólo el asistente de selección (Calzado → Intensidad → Duración → Confirmar) es estado local.

Reglas:

- No toma decisiones de proceso ni de seguridad. Todo lo que muestra viene del control; todo lo que envía es una solicitud.
- Envía sólo números de opción, nunca temperaturas ni minutos.
- Sin `STATUS` durante 2 s muestra "Sin comunicación" y no asume valores.

## Primera puesta en marcha

1. En el primer arranque pide **calibrar el táctil** (tocar las 4 esquinas). Para repetirlo: mantener el dedo en la pantalla 3 s al encender.
2. Comprobar los colores y la orientación; si no son correctos, ajustar `invert`, `rgb_order` o `SCREEN_ROTATION` en [src/display.h](src/display.h).

## Pendiente

- **Fuentes con acentos** (PLAN.md §2): las fuentes incluidas en LVGL sólo traen ASCII, así que por ahora los textos de `ui.cpp` van sin acentos ("Duracion"). Hay que generarlas con `lv_font_conv`.
- Prueba en la placa real y prueba de usabilidad.
