# Firmware del HMI (ESP32-32E, pantalla 3.2")

Ubicación: `firmware/hmi/`

## Hardware verificado
ESP32-32E (3,3 V), ST7789P3 240×320 (SPI: CS IO15, DC IO2, CLK IO14, MOSI IO13, MISO IO12), táctil XPT2046 (CS IO33, IRQ IO36), SD (CS IO5, MOSI IO23, CLK IO18, MISO IO19), altavoz (IO4/IO26), LED RGB (IO22/IO16/IO17). Serie expuesto: IO3/IO1.

## Alcance (decidido por el usuario)
El módulo de pantalla **no maneja ninguna salida ni lee ninguna entrada del proceso**: sin sensores, sin SSR, sin puerta, sin termostatos. Sólo intercambia información con los demás dispositivos por un protocolo de comunicación. (Su pantalla y táctil son la interfaz con la persona, no I/O de proceso; la SD y los pines I2C/SPI expuestos de la placa quedan sin usar para el proceso.)

## Responsabilidades
1. Mostrar estado, temperaturas, humedad, VOC, tiempo restante y fallas.
2. Captar órdenes del usuario (inicio, pausa, cancelar, parámetros) y enviarlas como **solicitudes** a la Mega.
3. Guardar en SD el registro del ciclo recibido de la Mega.
4. Avisos sonoros / LED RGB.

## Flujo de pantallas
Tipo de calzado → Intensidad (3 botones) → Duración (3 botones) → Confirmación → Estado del ciclo → Fin. Detalle y tablas: [perfiles-de-tratamiento.md](perfiles-de-tratamiento.md). Sólo botones grandes y discretos; sin campos numéricos.

## Reglas
- No toma decisiones de proceso ni de seguridad. Todo lo que muestra viene de la Mega; todo lo que envía es una petición que la Mega puede rechazar.
- Si pierde el enlace con la Mega, muestra "sin comunicación" y no asume valores.
- Biblioteca gráfica a decidir [VERIFICAR: LVGL o TFT_eSPI]. Cuidado: al usar el UART0 para el enlace, **no** usar `Serial` para depuración mientras esté conectado.
- Nivel lógico 3,3 V: ver divisor en [ADR-0005](../decisiones/ADR-0005-hmi-esp32.md).
