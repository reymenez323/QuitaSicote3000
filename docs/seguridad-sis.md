# Funciones de seguridad del SIS (ESP32 Dev Kit)

Resumen. El pseudocódigo exacto y los parámetros están en [PLAN-MAESTRO §7](PLAN-MAESTRO.md#7-firmware-del-sis-esp32-dev-kit). Todas estas funciones corren **sólo en el SIS** ([ADR-0012](decisiones/ADR-0012-regla-mega-control-nano-sis.md)).

Entradas del SIS: **TC3**, **puerta (NA/NC)** y lo que informa el control en `HB_CTRL`. Sin módulos de lectura de 12 V, el SIS no ve el termostato (con la opción T1), el contacto de RL1 ni la tensión de los ventiladores ([ADR-0013](decisiones/ADR-0013-solo-modulos-y-dispositivos.md)).

Acciones de todo disparo: **abrir RL1 (permiso del PTC)**, **retirar el permiso de apagado de FAN_P (RL3)**, **forzar FAN_C (RL4)**, **enclavar** e **informar al control**.

| ID | Condición | Entradas | Rearme |
|---|---|---|---|
| SIF-01 | Sobretemperatura | TC3 ≥ T_SIS_MAX | T < T_SIS_RESET + rearme. Evento térmico |
| SIF-02 | Puerta abierta o lectura inválida | Contactos NA/NC | **No enclava**: el permiso sigue a la puerta; el PTC sólo vuelve con orden del usuario |
| SIF-03 | Calentamiento sin flujo de aire | Subida brusca de TC3 con el permiso concedido | Rearme. Evento térmico |
| SIF-04 | Sensor inválido | MAX6675 abierto, fuera de rango o congelado | Sensor válido + rearme |
| SIF-05 | (Retirada: no hay realimentación eléctrica) | — | — |
| SIF-06 | Control sin heartbeat con el permiso concedido | `HB_CTRL` > 2 s | Heartbeat + rearme (pendiente A-4) |
| SIF-07 | Tiempo máximo de calentamiento | Permiso continuo > 125 min | Rearme |
| SIF-08 | Termostato abierto: inferido por "calentamiento sin efecto" (T1) o leído (T2) | TC3 + `ssr1_cmd` del control (T1) / pin de lectura (T2) | Rearme. Evento térmico |

Eventos térmicos (SIF-01, 03, 08): enclavado persistente; al 2.º, **bloqueo** hasta servicio ([ADR-0008](decisiones/ADR-0008-termostato-rearme-automatico.md), pendiente A-11).

Escalonado de temperaturas: objetivo (35–50 °C) < límite del control (+5 °C) < SIF-01 (≈ 65–70 °C) < termostato (80 °C ± 5).

## Lógica de la puerta

| NA | NC | Resultado |
|---|---|---|
| BAJO | ALTO | Cerrada |
| ALTO | BAJO | Abierta |
| BAJO | BAJO | Inválida → abierta con falla |
| ALTO | ALTO | Inválida → abierta con falla |

## Reglas

- Lo que llega del control **sólo puede restringir**: retirar el permiso o provocar un disparo.
- Ningún disparo depende de la comunicación, salvo SIF-06 (cuya causa es la comunicación) y SIF-08 con T1 (que usa el dato sólo para disparar).
- Cada SIF es una función pura probada en PC.

## Validación

Se prueba **inyectando la falla**. Repetir tras cada cambio del firmware del SIS o de la versión del core.

| Prueba | Acción | Esperado |
|---|---|---|
| V-01 | TC3 por encima del umbral | RL1 abierto, FAN_C forzado, FAN_P encendido |
| V-02 | Abrir la puerta con el PTC permitido | RL1 abierto en < 200 ms |
| V-03 | Desconectar FAN_P con el PTC calentando (ciclo de trabajo bajo) | SIF-03 antes de que TC3 llegue a T_SIS_MAX |
| V-04 | Desconectar TC3 / lectura congelada | SIF-04 |
| V-05 | Cortar el UART durante un ciclo | Según la decisión A-4 |
| V-06 | Colgar el firmware del control | El SIS retira el permiso y fuerza FAN_C |
| V-07 | Abrir el termostato (o simularlo) con SSR1 al 100 % | SIF-08 |
| V-08 | Dos eventos térmicos seguidos | Bloqueo; persiste tras cortar la alimentación |
| V-09 | Iniciar con la puerta abierta (HMI y solicitud forzada) | HMI deshabilita, control rechaza, SIS no concede |
| V-10 | Cortar NA, NC o común; puentear NA con NC | Puerta inválida → PTC sin permiso |
| V-11 | Cerrar la puerta tras una pausa | El PTC no vuelve hasta "Reanudar" |
| V-12 | Reiniciar el SIS durante un ciclo | RL1 abierto y FAN_P encendido durante el arranque |
| V-13 | Puentear SSR1 (en corto) | El control detecta sobretemperatura y retira `heat_request` ⇒ RL1 abre; si no, SIF-01 |
| Periódica | Comprobar que RL1 abre de verdad (medir continuidad con el permiso retirado) | Abierto: el contacto no está soldado |
