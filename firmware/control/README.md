# Firmware de control (ESP32-S3)

> **Sólo control.** Ninguna función de seguridad se implementa aquí ([ADR-0012](../../docs/decisiones/ADR-0012-regla-mega-control-nano-sis.md)). Sus límites de software son límites de proceso (capa 1), no funciones SIF.

Especificación completa: [PLAN-MAESTRO §8](../../docs/PLAN-MAESTRO.md#8-firmware-de-control-esp32-s3). Pines: [docs/pinout/control-esp32s3.md](../../docs/pinout/control-esp32s3.md).

## Responsabilidades

1. Leer TC1 y TC2 (MAX6675), SHT31, SGP40 (compensado con T/HR del SHT31) y la puerta.
2. Ejecutar la [máquina de estados](../../docs/maquina-de-estados.md) y traducir el [perfil](../../docs/perfiles-de-tratamiento.md) elegido.
3. Regular la temperatura con el **SSR1** (conmutación lenta, ventana de 2 s).
4. Manejar el **ventilador de circulación** (SSR2) y la orden de **apagar el ventilador del PTC** (RL2). El SIS puede forzar o vetar con sus propios relés; el control lo ve en `HB_SIS`.
5. Enlaces con el SIS (UART1) y con el HMI (UART2); consola y CSV por el USB nativo.

## Módulos (`lib/` lógica pura, `src/` hardware)

```
lib/ctrl_logic/  cycle_fsm, profiles, heat_ctrl, fans, faults, door
src/             main, scheduler, sensors/, actuators/ (ssr1, fan_c, fan_p), links/, console/, config/
```

## Reglas

- Wi-Fi y Bluetooth apagados.
- Planificador cooperativo en `loop()` con `millis()`, sin `delay()`; Task Watchdog activo.
- Cada lectura lleva calidad (válida / obsoleta / falla); no se usan lecturas inválidas.
- SSR1 sólo se activa con `heat_request`, la puerta cerrada y el permiso del SIS concedido.
- Nunca pedir el apagado del ventilador del PTC con el PTC activo o TC2 ≥ T_FRIO.
- MAX6675: ≥ 220 ms entre lecturas. SGP40: muestreo regular a 1 Hz (biblioteca de Sensirion).
