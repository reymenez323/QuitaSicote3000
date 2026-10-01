# ADR-0011 — Sin componentes auxiliares pequeños; dos reguladores de 5 V

**Estado:** Aceptada (decisión del usuario)

## Decisión
- **No** se usan compuertas lógicas, transistores, diodos, resistencias ni fusibles sueltos. Sólo módulos y placas.
- **Dos reguladores de 5 V**: REG_A para la Mega, el HMI y sus sensores; REG_B para el Nano y TC3.

## Consecuencias aplicadas al diseño
1. **El Nano maneja directamente** RL1 (permiso del PTC), RL2 (ventilador del PTC) y SSR2 (ventilador de circulación). Las combinaciones que antes hacía una compuerta (AND/OR entre la Mega y el SIS) se hacen **por software en el SIS**. La Mega pide los ventiladores en `HB_CTRL`; esas peticiones sólo pueden encender un ventilador, nunca apagarlo contra las condiciones del SIS.
2. **El SIS no lee nodos de 12 V** (haría falta un divisor). Se pierden:
   - la lectura directa del termostato (ver ADR-0008);
   - la realimentación de RL1 (contacto soldado) y la detección eléctrica de SSR1 en corto;
   - la detección de tensión en el ventilador del PTC.
   Se compensan con funciones basadas en TC3: subida brusca (SIF-03) y "calentamiento sin efecto" (SIF-08). SIF-05 se retira.
3. **Puerta**: conexión directa con pull-ups internos, sin resistencias en serie.
4. **Salidas al aire durante un reinicio**: no hay pull-downs; se exige que los módulos de relé y los SSR queden desactivados con la entrada al aire (comprobar en F3).
5. **Enlace Mega → ESP32**: el ESP32 no tolera 5 V y no se puede usar un divisor. Decisión abierta A-9 (se recomienda un módulo convertidor de nivel).
6. **Sin fusibles**: decisión abierta A-10 (se recomienda reconsiderar).
