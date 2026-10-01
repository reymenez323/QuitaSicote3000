# ADR-0008 — Termostato de rearme automático: el SIS lo vigila y enclava

**Estado:** Aceptada por el usuario (enclavado en EEPROM, bloqueo tras 2 eventos)

## Contexto
El termostato de 80 °C (único, NC, en serie con el PTC) es de **rearme automático**. Si abre y el aire se enfría, vuelve a cerrar y el PTC recibiría energía otra vez. Que haya llegado a 80 °C significa que las capas 1 y 2 fallaron o que hay una condición grave (falla de ventilador, incendio): no es un evento a "reintentar".

## Decisión propuesta
1. El SIS **lee el estado del termostato** (nodo de 12 V justo después del termostato, con divisor resistivo hacia un pin del Nano, ≈ 18 kΩ / 10 kΩ, más resistencia serie y pull-down; **[VERIFICAR valores]**). Nodo en alto = termostato cerrado; bajo = abierto.
2. Si lo ve abierto con el PTC ordenado (o simplemente abierto cuando debería estar cerrado): **SIF-08** → disparo **enclavado**; el permiso del PTC permanece abierto aunque el termostato se rearme.
3. El enclavamiento se guarda en **EEPROM del Nano**, para que desenchufar y volver a enchufar no lo borre.
4. Salir del enclavamiento: enfriamiento completo, autotest correcto y una confirmación deliberada desde el HMI (que sólo *solicita*; el SIS decide).
5. Contador de eventos en EEPROM: tras **2 eventos** el equipo queda bloqueado hasta un procedimiento de servicio (secuencia especial) y el HMI recomienda revisar el ventilador del PTC y el montaje.

## Consecuencias
- (+) Evita que el termostato cicle el PTC en un ciclo apertura/cierre alrededor de 80 °C.
- (+) Convierte el evento en una falla visible para el usuario.
- (−) Una falla persistente en EEPROM puede inutilizar el equipo si el usuario no sabe qué hacer; el mensaje del HMI debe ser claro.
- (−) Añade un pin y un divisor al Nano (no es un dispositivo nuevo: son dos resistencias).
- Pendiente de decidir: mientras el termostato está abierto, ¿el SIS fuerza ON el ventilador de circulación? Propuesta: sí, como en cualquier disparo.
