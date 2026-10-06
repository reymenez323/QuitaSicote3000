// Configuración de LVGL 9 para el HMI (ESP32 sin PSRAM, 320 × 240, RGB565).
// Lo que no se define aquí toma el valor por defecto de LVGL.
#ifndef LV_CONF_H
#define LV_CONF_H

#define LV_COLOR_DEPTH 16

// Memoria propia de LVGL. Cada pantalla se crea al entrar y se borra al salir.
#define LV_USE_STDLIB_MALLOC LV_STDLIB_BUILTIN
#define LV_MEM_SIZE (48U * 1024U)

#define LV_DEF_REFR_PERIOD 20
#define LV_DPI_DEF 125

#define LV_USE_LOG 0
#define LV_USE_ASSERT_NULL 1
#define LV_USE_ASSERT_MALLOC 1

// Fuentes. Las incluidas en LVGL sólo traen ASCII y "°": las fuentes con
// acentos (font_es_*) están pendientes de generar con lv_font_conv (PLAN.md §2).
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_24 1
#define LV_FONT_MONTSERRAT_28 1
#define LV_FONT_MONTSERRAT_48 1
#define LV_FONT_DEFAULT &lv_font_montserrat_20

#define LV_USE_THEME_DEFAULT 1
#define LV_THEME_DEFAULT_DARK 0

#define LV_USE_SYSMON 0
#define LV_BUILD_EXAMPLES 0

#endif  // LV_CONF_H
