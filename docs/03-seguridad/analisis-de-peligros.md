# Análisis de peligros (borrador)

Método: lista de desviaciones tipo HAZOP simplificado. Este documento **no** declara un nivel SIL; es un ejercicio de ingeniería para justificar las funciones de seguridad.

| ID | Peligro | Causa(s) | Consecuencia | Función que lo cubre |
|---|---|---|---|---|
| P-01 | Sobretemperatura en la recámara | SSR PTC en corto; PID mal; termopar desprendido | Daño al calzado, incendio | SIF-01, termostatos |
| P-02 | Calentamiento sin flujo de aire | Ventilador falla/apagado; ducto obstruido | Punto caliente en PTC | SIF-03, termostatos |
| P-03 | Contacto del usuario con zona caliente | Puerta abierta con PTC encendido | Quemadura | SIF-02 |
| P-04 | Lectura de temperatura engañosa | Termopar abierto/ruido/MAX6675 congelado | Control a ciegas | SIF-04 |
| P-05 | Control colgado con PTC encendido | Bloqueo del MCU 1 | Calentamiento sin supervisión | SIF-05, SIF-06 |
| P-06 | Pérdida de alimentación parcial | Brownout | Estados indeterminados | Lógica "energizar para operar" |
| P-07 | Concentración de vapores inflamables | Solventes/pegamentos del calzado | Ignición | Límite de T bajo + SGP40 como alarma [VERIFICAR] |
| P-08 | Descarga eléctrica / cortocircuito | Humedad, cableado | Daño, fuego | Fusible, aislamiento, bajo voltaje DC |
| P-09 | Condensación en electrónica | Aire húmedo | Falla de sensores | Diseño mecánico, recubrimiento |
| P-10 | No se puede parar el ciclo (pantalla/HMI colgado, sin paro físico) | Falla del HMI o de la Mega | Calentamiento prolongado sin que el usuario pueda intervenir | Puerta = paro (SIF-02), aborto por pérdida de HMI, tiempo máximo (SIF-07); ADR-0006 |
| P-11 | Uso sin supervisión, con niños o mascotas | Entorno doméstico | Quemadura, contacto con partes calientes | Mecánica (M-08), temperaturas superficiales bajas, indicación de ciclo activo |
| P-12 | Corte y retorno de energía | Red doméstica | Reinicio inesperado del calentamiento | No reanudar automáticamente (M-07) |
| P-13 | Fuente o cableado incorrecto del usuario | Mal uso | Sobrecorriente, calor | Fuente certificada, fusible, conectores no intercambiables |
| P-14 | Termostato de rearme automático cicla el PTC cerca de 80 °C | Ventilador del PTC parado, ducto obstruido | Calentamiento repetido, deterioro, riesgo de incendio | SIF-08 (enclavado en EEPROM), SIF-03, ADR-0008 |
| P-15 | Ventilador del PTC apagado con el PTC energizado | Error de software, relé o cable | PTC sin flujo, sobrecalentamiento | Contacto NC (reposo = encendido), permiso de apagado del SIS, sensado del nodo, SIF-03, termostato, SIF-08 (ADR-0009) |

Pendiente: valorar severidad y probabilidad, y definir la temperatura de disparo con datos de los materiales reales.
