# ADR-0005 — La pantalla ESP32-32E es un nodo HMI independiente

**Estado:** Propuesta

## Contexto
La pantalla elegida (3.2" ESP32-32E, verificada en la [wiki](https://www.lcdwiki.com/3.2inch_ESP32-32E_Display)) **no es un periférico**: es una placa con su propio ESP32 (3,3 V), ST7789P3 de 240×320, táctil resistivo XPT2046, SD, altavoz y LED RGB. No se puede conectar a una Arduino Mega como un shield TFT.

## Decisión propuesta
Tratarla como un tercer nodo, **HMI**, conectado a la Mega por UART. La Mega sigue siendo el control; el ESP32 sólo dibuja, lee el táctil y (opcionalmente) escribe el log en la SD.

**Restricción confirmada por el usuario:** el HMI no maneja salidas ni recibe entradas de proceso; sólo se comunica con los demás dispositivos. Por tanto ningún GPIO del ESP32 se cablea a sensores o actuadores.

## Detalles verificados
- Puerto serie expuesto: **IO3 (RX0) / IO1 (TX0)**, 3,3 V. Son los mismos pines del USB de programación.
- I2C expuesto en IO25/IO32 (no se usará; los sensores cuelgan de la Mega).
- La placa se alimenta a 5 V por USB-C.
- Lógica 3,3 V: **Mega TX (5 V) → ESP32 RX necesita divisor resistivo** (p. ej. 1 kΩ/2 kΩ). ESP32 TX → Mega RX funciona directo (3,3 V supera el umbral alto de 5 V).

## Consecuencias
- (+) La UI no consume ciclos ni RAM de la Mega.
- (+) Un cuelgue de la UI no afecta al control ni al SIS.
- (+) Tarjeta SD disponible para el registro sin shield extra.
- (−) Tres firmwares y un protocolo más (`firmware/hmi/`).
- (−) La UI **no es una función de seguridad**: el botón "Parar" es comodidad. El paro seguro es el del SIS/termostatos; se recomienda un **paro físico** independiente de la pantalla.
- Alternativa: sustituir esta pantalla por una TFT compatible con Mega y eliminar el nodo HMI.
