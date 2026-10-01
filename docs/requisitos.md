# Requisitos del software

## Funcionales

| ID | Requisito | Nodo |
|---|---|---|
| RF-01 | El usuario inicia, pausa, reanuda y cancela un ciclo desde la pantalla táctil. | HMI |
| RF-02 | El usuario elige tipo de calzado (4 opciones), intensidad (3) y duración (3). Sin valores numéricos ni porcentajes. Ver [perfiles](perfiles-de-tratamiento.md). | HMI / Control |
| RF-03 | Un ciclo sólo puede iniciarse con la puerta cerrada: el HMI deshabilita Iniciar, el control rechaza la solicitud y el SIS no concede permiso al PTC. | Todos |
| RF-04 | El control calienta la recámara hasta la temperatura del perfil y la mantiene durante la duración elegida. | Control |
| RF-05 | El control maneja el ventilador de circulación y el del PTC según el ciclo. El SIS puede forzar el de circulación y vetar el apagado del ventilador del PTC. | Control / SIS |
| RF-06 | Al terminar o cancelar, el sistema enfría (ventilación sin calor) antes de volver a "listo". | Control |
| RF-07 | La pantalla muestra estado, tiempo restante, temperatura, humedad, VOC y fallas. | HMI |
| RF-08 | Se registran los datos de cada ciclo (destino por definir: SD del HMI o salida serie). | Control / HMI |

## Seguridad

| ID | Requisito |
|---|---|
| RS-01 | El PTC nunca se energiza con la puerta abierta. |
| RS-02 | El PTC nunca supera el límite de seguridad aunque falle el control. |
| RS-03 | El PTC nunca queda energizado sin flujo de aire. |
| RS-04 | Ante pérdida de alimentación, de comunicación o de lectura de sensor, el calentador queda apagado. |
| RS-05 | Un fallo del control no puede desactivar al SIS. |
| RS-06 | Tras un disparo del SIS, el rearme exige una acción deliberada y condiciones seguras. |

## Por ser producto doméstico ([ADR-0006](decisiones/ADR-0006-producto-domestico-sin-paro-fisico.md))

| ID | Requisito |
|---|---|
| RD-01 | Operable por una persona no técnica sólo desde la pantalla, con el dedo o un stylus. |
| RD-02 | Sin botón físico de paro: abrir la puerta detiene el calentamiento. |
| RD-03 | Si el control pierde el enlace con el HMI, aborta el ciclo y enfría. |
| RD-04 | Tras un corte de energía el ciclo no se reanuda solo. |
| RD-05 | Tope de tiempo de ciclo no modificable por el usuario. |

## Parámetros provisionales (constantes de firmware, a fijar con ensayos)

| Parámetro | Valor | Dónde |
|---|---|---|
| Temperatura objetivo (TC1) | 35–50 °C según perfil | Control |
| Límite de software | objetivo + 5 °C | Control |
| Disparo por sobretemperatura (TC3) | ≈ 65–70 °C | SIS |
| Termostato de hardware (referencia para el margen) | 80 °C | — |
| Tope global de ciclo | 120 min | Control y SIS |
| Pausa máxima con puerta abierta / sin reanudar | 5 min (decidido) | Control |
| Pérdida de enlace con el HMI | 10 s | Control |
| Pérdida de heartbeat del SIS (visto por el control) | 500 ms | Control |
| Pérdida de heartbeat del control (visto por el SIS) | 2 s | SIS |
| Temperatura "fría" para apagar el ventilador del PTC | 35 °C | Control (TC2) y SIS (TC3) |
| Tiempo máximo de precalentamiento | ? | Control |
| Criterio de fin anticipado por VOC/HR | ? (por decidir si se usa) | Control |
