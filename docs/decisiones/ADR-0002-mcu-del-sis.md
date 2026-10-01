# ADR-0002 — MCU del SIS: Arduino Nano

**Estado:** Aceptada, con riesgos conocidos

## Decisión
SIS en **Arduino Nano (ATmega328P, 5 V, 16 MHz, 32 KB flash, 2 KB SRAM)**. Control en **Arduino Mega 2560** ([ADR-0005](ADR-0005-hmi-esp32.md) para la pantalla).

## Verificación contra los criterios

| Criterio | Resultado |
|---|---|
| Pines: 1 SPI (MAX6675), 1 UART, 4–6 GPIO | Cumple. SPI en D10–D13, UART en D0/D1, quedan D2–D9 y A0–A5. Nota: A6/A7 son **sólo analógicas**, no usarlas como digitales. |
| Watchdog independiente | Cumple: el WDT del ATmega328P corre con su oscilador interno de 128 kHz, independiente del cristal. **Riesgo**: los Nano con bootloader antiguo no limpian el WDT tras reset y entran en bucle de reinicio. Usar Optiboot o limpiar `MCUSR`/`wdt_disable()` al inicio, y probarlo. |
| Memoria | 2 KB SRAM alcanza para un lazo de seguridad pequeño. Sin `String`, sin asignación dinámica, sin `SoftwareSerial`. |
| Toolchain distinta a la del control | **No cumple.** Mega y Nano son AVR de 8 bits con el mismo compilador, avr-libc y core Arduino. Un bug de compilador o de biblioteca común afectaría a ambos. |

## Mitigaciones del fallo de causa común
- Los termostatos NC (capa 3) no dependen de ningún MCU.
- Alimentación del Nano con regulador **propio**, no tomada del 5 V de la Mega.
- El SIS no usa bibliotecas de terceros: lee MAX6675 por SPI directo y UART por registros/`Serial`.
- Las pruebas V-01…V-10 se repiten tras cualquier cambio de compilador o core.

## Consecuencias prácticas
- El Nano tiene **un solo UART**, compartido con el USB (D0/D1). El enlace con la Mega usa ese UART; para programar o depurar hay que desconectar el cable de la Mega. Descartado `SoftwareSerial` por no ser determinista con interrupciones. Depuración: LED/buzzer y mensajes por el propio enlace.
- Reloj de 16 MHz: 115200 baudios tiene ≈2 % de error; usar 57600 [VERIFICAR] o 115200 con U2X.
- Muchos "Nano" son clones con CH340 y regulador mediocre: usar alimentación externa regulada de 5 V directa al pin 5V o con buen regulador.
