# ADR-0012 — Regla estricta: un controlador para el control y otro para el SIS

**Estado:** Aceptada. **Decisión del usuario de máxima prioridad: debe mantenerse estrictamente en todo el proyecto.**

## Asignación vigente (2026-10-06)

| Función | Controlador |
|---|---|
| **Control** | **ESP32 DevKit V1** (placa n.º 1, etiquetada "CONTROL") |
| **SIS** | **ESP32 DevKit V1** (placa n.º 2, etiquetada "SIS") |
| Interfaz (HMI) | ESP32-32E con pantalla 3.2" |

Historial: la primera asignación fue control = Arduino Mega 2560 y SIS = Arduino Nano. Después fue control = ESP32-S3 y SIS = ESP32 Dev Kit. El 2026-10-06 el usuario fijó **un ESP32 DevKit V1 para cada uno**: siguen siendo dos controladores físicamente separados. El nombre del archivo conserva la versión original para no romper enlaces.

## Regla
1. **Todas las funciones de control** (ciclo, perfiles, regulación de temperatura, manejo de ventiladores, sensores de proceso) se ejecutan en el **controlador de control (ESP32 DevKit V1 de control)**.
2. **Todas las funciones de seguridad** (funciones SIF, permiso del PTC, vetos y forzados de seguridad, enclavamientos) se ejecutan en el **controlador SIS (ESP32 Dev Kit)**.
3. El SIS **no** hace control de proceso. El control **no** hace funciones de seguridad (sus límites de software son límites de proceso, la capa 1, no funciones SIF).
4. El HMI no hace ni control ni seguridad: sólo interfaz.
5. Cuando una salida necesite a la vez la orden del control y la condición de seguridad (p. ej. ventiladores), se combina **por hardware** con contactos de módulos de relé ([ADR-0013](ADR-0013-solo-modulos-y-dispositivos.md)). Nunca se resuelve moviendo la función de un controlador al otro.

## Consecuencias
- Cualquier cambio futuro que mueva una función entre el control y el SIS es inválido sin una decisión explícita del usuario.
- Los tres controladores son de Espressif con el mismo toolchain: riesgo de falla de causa común (ver [ADR-0002](ADR-0002-mcu-del-sis.md)).
