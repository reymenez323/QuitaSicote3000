# ADR-0002 — SIS en Arduino Nano, control en Arduino Mega

**Estado:** Aceptada

## Decisión
- SIS: **Arduino Nano** (ATmega328P, 16 MHz, 32 KB flash, 2 KB SRAM).
- Control: **Arduino Mega 2560** (8 KB SRAM, 4 UART).

## Implicaciones para el firmware
- **Watchdog del Nano**: limpiar `MCUSR` y llamar a `wdt_disable()` al arrancar, para que funcione con cualquier bootloader (original o clon).
- **Memoria del Nano**: sin `String`, sin asignación dinámica, sin `SoftwareSerial`.
- **UART único del Nano**, compartido con el USB: el enlace con la Mega usa D0/D1 y hay que desconectarlo para programar. Depuración por LED/buzzer o por el propio enlace.
- **Baudios**: 57600, o 115200 con U2X (a 16 MHz, 115200 normal tiene ≈ 2 % de error).
- **Causa común**: ambos son AVR con el mismo compilador y core. Mitigación: el SIS no usa bibliotecas de terceros (MAX6675 y UART directos) y su validación se repite tras cualquier cambio de compilador o core.
