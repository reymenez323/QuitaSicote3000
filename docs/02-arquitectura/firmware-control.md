# Firmware de control (MCU 1: Arduino Mega 2560)

Ubicación: `firmware/control/`

Hardware: ATmega2560, 5 V, 16 MHz, 256 KB flash, 8 KB SRAM, 4 UART, I2C en pines 20/21, SPI en 50–53.

## Responsabilidades

1. Adquirir sensores: TC1, TC2 (MAX6675), SHT31, SGP40 (compensado con T/HR del SHT31), puerta.
2. Ejecutar la máquina de estados del ciclo ([maquina-de-estados.md](maquina-de-estados.md)).
3. Regular temperatura (PID o histéresis) sobre el SSR del PTC con conmutación lenta.
4. Comandar el ventilador de circulación.
5. Enlace con el SIS (Serial1) y con el HMI (Serial2).
6. Mantener los límites de proceso por software.

La interfaz de usuario y el registro en SD viven en el nodo HMI ([firmware-hmi.md](firmware-hmi.md)).

## Módulos propuestos (`src/`)

```
src/
├─ main.cpp
├─ app/          Máquina de estados, orquestación del ciclo
├─ control/      PID / histéresis, perfil de ciclo, criterios de fin
├─ sensores/     max6675, sht31, sgp40 (+ índice VOC), puerta
├─ actuadores/   SSR PTC, SSR ventilador, relé del ventilador del PTC (nunca apagarlo con PTC activo ni caliente)
├─ comunicacion/ Enlace con SIS y con HMI (usa firmware/compartido/protocolo)
└─ config/       Pines y parámetros
```

## Reglas de diseño

- **Sin RTOS.** Planificador cooperativo basado en `millis()`: tareas cortas, sin `delay()` bloqueante, periodos fijos.
- Sin `String` y evitar `malloc` (8 KB de SRAM, fragmentación).
- Watchdog habilitado (con bootloader compatible, ver riesgo en [ADR-0002](../decisiones/ADR-0002-mcu-del-sis.md)).
- Cada actuador pasa por una capa que impone límites de software (T máxima, tiempo máximo de calentamiento continuo).
- Cada lectura de sensor lleva **calidad** (válida / obsoleta / fallo); no se usan lecturas inválidas.
- Parámetros de ciclo ajustables desde el HMI; **los umbrales del SIS no**.
- Los MAX6675 comparten SCK/MISO; cada uno con su CS. Respetar ≥ 220 ms entre lecturas por módulo.
- El índice VOC del SGP40 requiere muestreo regular de 1 Hz; la biblioteca oficial de Sensirion corre en AVR.
- Lógica separada de hardware (HAL) para poder probarla en PC (`native`).

## Pines (resumen; detalle en [hardware/pinout](../../hardware/pinout/README.md))

La Mega tiene pines de sobra, lo que cierra el riesgo H-01 para el control.
