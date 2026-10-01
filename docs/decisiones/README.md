# Decisiones de diseño (ADR)

| ID | Decisión | Estado |
|---|---|---|
| [ADR-0001](ADR-0001-separacion-control-sis.md) | Dos MCU independientes: control y SIS | Aceptada |
| [ADR-0002](ADR-0002-mcu-del-sis.md) | SIS en Arduino Nano (control en Mega) | Aceptada con riesgos |
| [ADR-0005](ADR-0005-hmi-esp32.md) | Pantalla ESP32-32E como nodo HMI sólo de comunicación | Propuesta |
| [ADR-0006](ADR-0006-producto-domestico-sin-paro-fisico.md) | Producto doméstico, sin paro físico | Aceptada, con mitigaciones |
| [ADR-0007](ADR-0007-perfiles-en-el-control.md) | Perfiles en la Mega; SIS con límite único | Propuesta |
| [ADR-0003](ADR-0003-permiso-en-serie.md) | Permiso del SIS en serie con el SSR del PTC | Propuesta |
| [ADR-0004](ADR-0004-asignacion-termopares.md) | TC1/TC2 al control, TC3 exclusivo del SIS | Propuesta |

## Preguntas abiertas (necesito tu respuesta)

1. ~~Termostatos~~ → uno, 80 °C. Falta: ¿rearme automático o manual? ¿dónde se montará (carcasa del PTC)? ¿corriente DC nominal?
2. **Sensor VOC**: ¿es un **SGP40**?
3. ~~Tensión y potencia~~ → 12 V, PTC 100 W (≈ 8,3 A). Falta: corriente de arranque del PTC y de los ventiladores.
4. **¿Cómo se alimenta hoy el ventilador integrado del PTC?** ¿Queda encendido cuando el PTC está apagado, y viceversa?
5. ~~MCU del SIS~~ → Nano (ADR-0002). Falta: ¿tu Nano tiene bootloader Optiboot o antiguo? ¿Es original o clon?
6. ~~MCU de control~~ → Mega; la pantalla ESP32 es sólo HMI (ADR-0005).
9. ¿Qué **protocolo** prefieres entre nodos? Propuesta: UART (Mega↔Nano en Serial1, Mega↔HMI en Serial2).
10. ~~SHT31/SGP40 a 5 V~~ → toleran 5 V.
11. ~~Botón físico de paro~~ → No habrá (ADR-0006). La puerta abierta hace de paro.
12. ~~Fuente~~ → 12 V / 20 A. Falta: ¿es adaptador certificado?
13. ¿Cuántos **tipos de calzado** ofrecerá el HMI? (propuse 4; intensidad y duración son 3 opciones cada una).
14. ~~Ventiladores~~ → ambos de 12 V. Falta su corriente (medir).
15. ~~Fusible térmico~~ → no se añadirá (riesgo residual aceptado; ver alimentacion-y-potencia.md §4).
7. **Temperatura objetivo** de tratamiento y tiempo máximo de ciclo.
8. ¿El ventilador de circulación es de motor DC con escobillas o sin escobillas (tiene electrónica propia)?
