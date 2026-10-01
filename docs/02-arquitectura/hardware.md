# Arquitectura de hardware

## 1. Inventario

| Cant. | Componente | Función | Conectado a |
|---|---|---|---|
| 1 | Arduino Mega 2560 | MCU de control | — |
| 1 | Arduino Nano (ATmega328P) | MCU del SIS | — |
| 1 | Módulo ESP32-32E con pantalla 3.2" ([wiki](https://www.lcdwiki.com/3.2inch_ESP32-32E_Display)) | Nodo HMI, sólo comunicación (ADR-0005) | UART a la Mega |
| 3 | Termopar tipo K | Temperatura | MAX6675 |
| 3 | MAX6675 | Conversión termopar → SPI | 2 al control, 1 al SIS |
| 1 | SHT31 | Temperatura y humedad interior | I2C, control |
| 1 | SGP40 (el usuario escribió "SG40") **[VERIFICAR]** | VOC | I2C, control |
| 1 | SSR DC (PTC) | Conmuta el PTC | Control |
| 1 | SSR DC (ventilador) | Conmuta el ventilador de circulación | Control (+ forzado SIS) |
| 1 | Módulo de relé 12 V | Conmuta el ventilador del PTC | Control + permiso de apagado del SIS |
| 1 | Ventilador de la cámara de circuitos (12 V, 2 cables) | Enfría la electrónica | Directo a 12 V, siempre encendido |
| 1 | PTC 100 W / 12 V (≈ 8,3 A) con ventilador integrado | Fuente de calor | Ver §4 y [alimentacion-y-potencia.md](alimentacion-y-potencia.md) |
| 1 | Ventilador de circulación | Flujo de aire en recámara | SSR ventilador |
| 1 | Termostato térmico 80 °C (NC) | Protección independiente | En serie con PTC (ver [alimentacion-y-potencia.md](alimentacion-y-potencia.md) §4) |
| 1 | Limit switch de puerta | Puerta abierta/cerrada | Control y SIS |

## 2. Asignación propuesta de termopares

| TC | Ubicación sugerida | Lo lee | Uso |
|---|---|---|---|
| TC1 | Aire dentro de la recámara, cerca del calzado | Control | Variable de proceso del PID |
| TC2 | Salida del PTC / entrada al ducto | Control | Límite de gradiente, diagnóstico de flujo |
| TC3 | Zona más caliente (junto al PTC) | **SIS** | Límite de seguridad independiente |

TC3 debe estar en el punto donde una falla produzca la temperatura más alta, no en el punto más representativo del proceso.

## 3. Datos relevantes del MAX6675 para el diseño

- Sólo termopar tipo K, resolución 0,25 °C, tiempo de conversión ≈ 0,22 s → lectura máxima ≈ 4 Hz; el lazo y el SIS deben muestrear más lento que eso.
- Entrega un bit de **termopar abierto**: el SIS debe tratarlo como falla.
- Ruido de conmutación de SSR/ventilador puede corromper lecturas: ver §6 (cableado).
- Compensación de unión fría sólo de la propia placa: mantener el módulo lejos del PTC.

## 4. Ruta de potencia del PTC (propuesta)

```
Fuente DC (+) ──► Termostato(s) NC ──► Contacto de permiso SIS ──► SSR-DC (control) ──► PTC ──► (−)
```

- Los tres elementos en serie: cualquiera abre ⇒ PTC sin energía.
- **Contacto de permiso del SIS**: relé electromecánico o segundo SSR comandado por el SIS con lógica "energizado = permitido" **[ver ADR-0003]**.
- El ventilador integrado del PTC tiene **alimentación separada** y lo conmuta un **módulo de relé de 12 V** (decisión del usuario), cableado por **NC** para que el reposo sea "encendido" y con permiso de apagado del SIS: [ADR-0009](../decisiones/ADR-0009-ventilador-ptc-con-rele.md).
- El termostato es de rearme automático: el SIS lo vigila y enclava ([ADR-0008](../decisiones/ADR-0008-termostato-rearme-automatico.md)).
- Un SSR puede fallar en cortocircuito: por eso el permiso del SIS no debe ser el mismo tipo de dispositivo que el SSR de control (diversidad).

## 5. Ruta del ventilador de circulación

SSR-DC comandado por el control. El SIS puede **forzar ON** mediante un OR de hardware (diodos o compuerta) sobre la entrada del SSR, para enfriar tras un disparo.

## 6. Cableado y compatibilidad electromagnética

- Separar físicamente cableado de potencia (PTC, ventiladores) del de señal (termopares, I2C, SPI).
- Cables de termopar: par trenzado/apantallado, mantener el material K hasta el módulo (extensión K, no cobre).
- Tierra común única, en estrella, entre fuente, MCU 1, MCU 2 y módulos.
- Diodo de rueda libre (flyback) en el ventilador si es de motor DC con escobillas.
- Pull-ups I2C acordes a la longitud del cable al SHT31/SGP40.
- Limit switch de puerta (usa NA y NC): común a GND, NA y NC a entradas con pull-up en Mega y Nano. Sólo la combinación complementaria "NA bajo + NC alto" es puerta cerrada; todo lo demás es puerta abierta/falla (ver [funciones-de-seguridad.md](../03-seguridad/funciones-de-seguridad.md)).
- Fusible en la línea de potencia del PTC dimensionado para la corriente máxima del PTC en frío (el PTC tiene pico de arranque).

## 7. Riesgos de hardware identificados

| ID | Riesgo | Mitigación prevista |
|---|---|---|
| H-01 | ~~Pocos GPIO en el módulo de pantalla~~ | Cerrado: el control es la Mega y el HMI no usa I/O de proceso |
| H-10 | Niveles lógicos: Mega/Nano a 5 V, HMI a 3,3 V | Divisor en Mega TX → HMI RX. SHT31 y SGP40 toleran 5 V (confirmado por el usuario): sin adaptación en I2C |
| H-11 | Mega y Nano comparten familia AVR (causa común) | Ver mitigaciones en ADR-0002 |
| H-12 | Nano: UART único compartido con USB; bootloader viejo y WDT | Ver ADR-0002 |
| H-13 | Reguladores de Mega/Nano si el bus DC es de 12–24 V | Usar convertidor buck a 5 V; reguladores independientes para Mega y Nano |
| H-14 | Ventilador de la cámara de circuitos sin detección de falla | Aire exterior filtrado, medir temperatura interior en ensayo (alimentacion-y-potencia.md §5) |
| H-02 | SSR-DC con falla en cortocircuito | Contacto de permiso diverso + termostatos |
| H-03 | Ruido en MAX6675 por el PWM del PTC | Conmutación lenta (time-proportional), filtros, blindaje |
| H-04 | Humedad / condensación sobre sensores y electrónica | Ubicación fuera del flujo directo, conformal coating en módulos |
| H-05 | Vapores o compuestos volátiles del calzado cerca del PTC | Revisar temperatura máxima de la superficie del PTC (autoinflamación del material) |
