# Firmware del SIS (MCU 2)

Ubicación: `firmware/sis/`

## Filosofía

**Pequeño, determinista, aburrido.** Menos código = más fácil de probar y de confiar.

- Sin RTOS, sin asignación dinámica, sin bibliotecas gráficas ni de red.
- Lazo cíclico de periodo fijo con watchdog por hardware.
- Umbrales como constantes de compilación (`config/umbrales.h`).
- Ningún parámetro de seguridad se acepta por la comunicación.
- Cada condición de disparo es una función pura probable en PC.

## Responsabilidades

1. Leer TC3 (MAX6675 exclusivo), puerta, **estado del termostato** (divisor, SIF-08) y retroalimentación del permiso.
2. Evaluar las funciones de seguridad ([funciones-de-seguridad.md](../03-seguridad/funciones-de-seguridad.md)).
3. Gobernar el permiso en serie del PTC y el forzado del ventilador. **Nunca concede el permiso con la puerta abierta**, tampoco antes de iniciar un ciclo.
4. Reportar estado y causa de disparo al control.
5. Autodiagnóstico: verificar que el permiso realmente abre (retroalimentación) y que el termopar responde.

## Módulos propuestos

```
src/
├─ main.cpp           Lazo cíclico + watchdog
├─ sensores/          max6675, puerta, termostatos (lectura + validación)
├─ seguridad/         Funciones SIF: sobretemperatura, puerta, sensor inválido,
│                     calentamiento sin flujo, pérdida de heartbeat, tiempo máximo
├─ estado/            Estados SIS: ARRANQUE → OK → DISPARADO → (reinicio manual)
├─ salidas/           Permiso del PTC, forzado de ventilador, indicadores (LED/buzzer)
├─ comunicacion/      Enlace con el control (sólo informa y recibe heartbeat)
└─ config/            Pines y umbrales (constantes)
```

## Estados del SIS

```
ARRANQUE ──autotest OK──► OK ──condición──► DISPARADO ──condición segura + reset──► OK
    │                                              ▲
    └── autotest falla ─────────────────────────────┘
```

- En `ARRANQUE` y `DISPARADO` el permiso del PTC está **abierto**.
- Los disparos graves (SIF-08, y los que el SIS defina) se guardan en **EEPROM** y sobreviven a un corte de energía. Escribir sólo en eventos, no cíclicamente (desgaste).
- Salir de `DISPARADO` exige: causa desaparecida, tiempo mínimo de enfriamiento y un reinicio explícito (botón físico o confirmación en UI como *solicitud*, que el SIS valida).

## Elección de MCU

Pendiente — ver [ADR-0002](../decisiones/ADR-0002-mcu-del-sis.md). Criterio: familia o toolchain **distinta** a la del control para evitar fallos de causa común, con watchdog independiente y pocos pines.
