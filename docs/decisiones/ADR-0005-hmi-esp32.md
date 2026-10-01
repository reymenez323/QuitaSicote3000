# ADR-0005 — La pantalla ESP32-32E es un nodo HMI sólo de comunicación

**Estado:** Aceptada

## Contexto
La pantalla 3.2" ESP32-32E ([wiki](https://www.lcdwiki.com/3.2inch_ESP32-32E_Display)) es una placa completa con su propio ESP32; no se conecta al control como un shield.

## Decisión
- Es un tercer nodo, **HMI**, unido al control por UART.
- **No lee entradas ni maneja salidas del proceso** (decisión del usuario): sólo dibuja, lee su táctil y se comunica.
- La interfaz se implementa con **LVGL** ([plan](../../firmware/hmi/PLAN.md)).

## Consecuencias
- (+) La UI no consume recursos del control y un cuelgue de la UI no afecta al control ni al SIS.
- (−) Tercer firmware.
- El enlace usa UART2 reasignado a IO25 (TX) / IO32 (RX), en el conector I2C de la placa [PROPUESTA]. UART0 queda para depurar por USB ([pinout](../pinout/esp32-hmi.md)).
- La UI no es una función de seguridad: "Cancelar" es una solicitud, el paro real es la puerta/SIS.
