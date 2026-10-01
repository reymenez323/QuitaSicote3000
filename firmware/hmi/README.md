# Firmware del HMI (ESP32-32E, pantalla 3.2")

Interfaz táctil del equipo, hecha con **LVGL**. No lee entradas ni maneja salidas del proceso: sólo dibuja, lee su táctil y se comunica con la Mega ([ADR-0005](../../docs/decisiones/ADR-0005-hmi-esp32.md)).

Plan de implementación inicial: **[PLAN.md](PLAN.md)**.

## Placa (verificado en la [wiki](https://www.lcdwiki.com/3.2inch_ESP32-32E_Display))

| Recurso | Detalle |
|---|---|
| MCU | ESP32-32E (ESP32-WROOM-32E), 240 MHz, 520 KB SRAM, **sin PSRAM**, 4 MB flash |
| Pantalla | ST7789P3, 240×320 nativa, IPS, RGB565; **se usa en horizontal (320×240)** |
| Bus de pantalla (SPI) | CS IO15, DC IO2, SCLK IO14, MOSI IO13, MISO IO12; RST compartido con EN |
| Retroiluminación | IO27 (ALTO = encendida) |
| Táctil | XPT2046 resistivo, **mismo bus SPI**; CS IO33, IRQ IO36 |
| UART0 | IO3 (RX) / IO1 (TX), conversor USB CH340C: depuración |
| Enlace con la Mega | UART2 reasignado a IO32 (RX) / IO25 (TX), conector I2C. Pinout completo: [docs/pinout/esp32-hmi.md](../../docs/pinout/esp32-hmi.md) |
| Otros (sin uso por ahora) | SD (IO5/18/19/23), altavoz (IO4/IO26), LED RGB (IO22/16/17), BOOT (IO0) |

## Reglas

- No toma decisiones de proceso ni de seguridad. Todo lo que muestra viene de la Mega; todo lo que envía es una solicitud.
- Si deja de recibir `ESTADO` de la Mega durante más de 2 s, muestra "Sin comunicación" y no asume valores.
- Envía sólo identificadores de perfil, nunca temperaturas ni minutos.
