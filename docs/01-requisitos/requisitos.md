# Requisitos

## 1. Objetivo

Eliminar o reducir el mal olor del calzado haciendo circular aire caliente y seco a través de una recámara, verificando el resultado con un sensor de compuestos orgánicos volátiles (VOC).

## 2. Requisitos funcionales

| ID | Requisito | Origen |
|---|---|---|
| RF-01 | El usuario inicia, pausa y cancela un ciclo desde la pantalla táctil. | UI |
| RF-02 | El sistema calienta la recámara hasta una temperatura objetivo mediante el PTC. | Control |
| RF-03 | El sistema fuerza la circulación de aire con un ventilador dedicado (distinto del ventilador integrado al PTC). | Control |
| RF-04 | El sistema mide temperatura en 3 puntos (termopares K), humedad (SHT31) y VOC (SGP40). | Sensado |
| RF-05 | El ciclo termina al cumplirse la duración elegida (Corta/Media/Larga) de tratamiento a temperatura. VOC y humedad se muestran y registran; su uso como criterio de fin anticipado queda por decidir. | Control |
| RF-10 | **Un ciclo sólo puede iniciarse con la puerta cerrada.** El botón Iniciar se habilita únicamente con la puerta cerrada, la Mega rechaza la solicitud si está abierta, y el SIS no concede permiso al PTC con la puerta abierta. | Control / SIS / UI |
| RF-09 | El usuario elige tipo de calzado, intensidad (3 opciones) y duración (3 opciones); sin valores numéricos ni porcentajes. Ver [perfiles-de-tratamiento.md](../02-arquitectura/perfiles-de-tratamiento.md). | UI |
| RF-06 | Al terminar o abortar, el sistema ejecuta un enfriamiento (ventilador sin calor) antes de declarar "listo". | Control |
| RF-07 | La pantalla muestra estado, temperaturas, humedad, índice VOC, tiempo restante y fallas activas. | UI |
| RF-08 | Se registran datos del ciclo (CSV/serie) para análisis posterior. | Datos |

## 3. Requisitos de seguridad (resumen; detalle en [03-seguridad](../03-seguridad/funciones-de-seguridad.md))

| ID | Requisito |
|---|---|
| RS-01 | El PTC nunca debe energizarse con la puerta abierta. |
| RS-02 | El PTC nunca debe superar el límite de temperatura de seguridad aunque falle el MCU de control. |
| RS-03 | El PTC nunca debe permanecer energizado sin flujo de aire de la recámara. |
| RS-04 | Ante pérdida de alimentación, de comunicación o de lectura de sensor, el estado resultante debe ser calentador apagado. |
| RS-05 | Un fallo del MCU de control no debe poder desactivar al SIS. |
| RS-06 | Tras un disparo del SIS, el reinicio requiere acción deliberada y condiciones seguras. |

## 3.1 Requisitos por ser producto doméstico ([ADR-0006](../decisiones/ADR-0006-producto-domestico-sin-paro-fisico.md))

| ID | Requisito |
|---|---|
| RD-01 | Operable por una persona no técnica sólo desde la pantalla. |
| RD-02 | Sin botón físico de paro: abrir la puerta detiene el calentamiento. |
| RD-03 | Si se pierde el enlace con la pantalla, el ciclo se aborta y se enfría. |
| RD-04 | Tras un corte de energía no se reanuda solo. |
| RD-05 | Alimentación por fuente externa certificada de baja tensión. |
| RD-06 | Superficies y salida de aire accesibles sin riesgo de quemadura. |
| RD-07 | Tiempo máximo de ciclo con tope no modificable por el usuario. |
| RD-08 | Debe poder funcionar sin supervisión sin riesgo de incendio (ver análisis de peligros). |

## 4. Parámetros por definir [VERIFICAR]

| Parámetro | Valor | Nota |
|---|---|---|
| Temperatura objetivo de operación | 35–50 °C según perfil | Hipótesis inicial ([perfiles](../02-arquitectura/perfiles-de-tratamiento.md)). |
| Temperatura límite SIS (disparo) | ≈ 65–70 °C en TC3 | Provisional; depende de la lectura normal de TC3 en ensayo. |
| Temperatura de apertura del termostato | 80 °C (dato del usuario) | Un solo termostato; tolerancia típica ±5 °C. |
| Tiempo máximo de ciclo | 120 min (tope global) | Provisional. |
| Tensión del bus DC | 12 V (dato del usuario) | Fuente externa, ≥ 15 A recomendado. |
| Potencia del PTC | 100 W a 12 V ≈ 8,3 A (dato del usuario) | Ver [alimentacion-y-potencia.md](../02-arquitectura/alimentacion-y-potencia.md). |
| Criterio de fin por VOC | ? | Índice SGP40 por debajo de un umbral relativo. |
