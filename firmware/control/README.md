# Firmware de control (Arduino Mega 2560)

Especificación completa: [PLAN-MAESTRO §8](../../docs/PLAN-MAESTRO.md#8-firmware-de-control-arduino-mega). Pines: [docs/pinout/mega.md](../../docs/pinout/mega.md).

## Responsabilidades

1. Leer TC1 y TC2 (MAX6675), SHT31, SGP40 (compensado con T/HR del SHT31) y la puerta.
2. Ejecutar la [máquina de estados](../../docs/maquina-de-estados.md) y traducir el [perfil](../../docs/perfiles-de-tratamiento.md) elegido.
3. Regular la temperatura con el **SSR1** (conmutación lenta, ventana de 2 s).
4. **Pedir** los ventiladores al SIS (`fan_c_on_request`, `fan_p_off_request` en `HB_CTRL`); el SIS los maneja.
5. Enlaces con el SIS (`Serial1`) y con el HMI (`Serial2`); consola y CSV por USB.

## Módulos (`lib/` lógica pura, `src/` hardware)

```
lib/ctrl_logic/  cycle_fsm, profiles, heat_ctrl, fan_requests, faults, door
src/             main, scheduler, sensors/, actuators/ssr1, links/, console/, config/
```

## Reglas

- Sin RTOS: planificador cooperativo con `millis()`, sin `delay()`.
- Sin `String`; evitar `malloc` (8 KB de SRAM).
- Watchdog de 1 s.
- Cada lectura lleva calidad (válida / obsoleta / falla); no se usan lecturas inválidas.
- SSR1 sólo se activa con `heat_request`, la puerta cerrada y el permiso del SIS concedido.
- MAX6675: ≥ 220 ms entre lecturas. SGP40: muestreo regular a 1 Hz (biblioteca de Sensirion).
