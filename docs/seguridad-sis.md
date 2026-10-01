# Funciones de seguridad del SIS

Resumen. El pseudocódigo exacto y los parámetros están en [PLAN-MAESTRO §7](PLAN-MAESTRO.md#7-firmware-del-sis-arduino-nano).

Entradas del SIS: **TC3**, **puerta (NA/NC)** y lo que informa la Mega en `HB_CTRL`. Sin componentes auxiliares, el SIS no lee nodos de 12 V ([ADR-0011](decisiones/ADR-0011-sin-componentes-auxiliares.md)).

Acciones de todo disparo: **abrir RL1 (permiso del PTC)**, **mantener FAN_P y encender FAN_C**, **enclavar** e **informar al control**.

| ID | Condición | Entradas | Rearme |
|---|---|---|---|
| SIF-01 | Sobretemperatura | TC3 ≥ T_SIS_MAX | T < T_SIS_RESET + rearme. Evento térmico |
| SIF-02 | Puerta abierta o lectura inválida | Contactos NA/NC | **No enclava**: el permiso sigue a la puerta; el PTC sólo vuelve con orden del usuario |
| SIF-03 | Calentamiento sin flujo de aire | Subida brusca de TC3 con el permiso concedido | Rearme. Evento térmico |
| SIF-04 | Sensor inválido | MAX6675 abierto, fuera de rango o congelado | Sensor válido + rearme |
| SIF-05 | (Retirada: no hay realimentación eléctrica) | — | — |
| SIF-06 | Control sin heartbeat con el permiso concedido | `HB_CTRL` > 2 s | Heartbeat + rearme (pendiente A-4) |
| SIF-07 | Tiempo máximo de calentamiento | Permiso continuo > 125 min | Rearme |
| SIF-08 | Calentamiento sin efecto (posible termostato abierto) | La Mega informa SSR1 encendido casi todo el tiempo y TC3 baja | Rearme. Evento térmico |

Eventos térmicos (SIF-01, 03, 08): enclavado en EEPROM; al 2.º, **bloqueo** hasta servicio ([ADR-0008](decisiones/ADR-0008-termostato-rearme-automatico.md), pendiente A-11).

Escalonado de temperaturas: objetivo (35–50 °C) < límite del control (+5 °C) < SIF-01 (≈ 65–70 °C) < termostato (80 °C ± 5).

## Lógica de la puerta

| NA | NC | Resultado |
|---|---|---|
| BAJO | ALTO | Cerrada |
| ALTO | BAJO | Abierta |
| BAJO | BAJO | Inválida → abierta con falla |
| ALTO | ALTO | Inválida → abierta con falla |

## Reglas

- Lo que llega de la Mega **sólo puede restringir**: retirar el permiso, provocar un disparo o encender un ventilador.
- Ningún disparo depende de la comunicación, salvo SIF-06 (cuya causa es la comunicación) y SIF-08 (que usa el dato sólo para disparar).
- Cada SIF es una función pura probada en PC.

## Validación

Se prueba **inyectando la falla**. Repetir tras cada cambio del firmware del SIS.

| Prueba | Acción | Esperado |
|---|---|---|
| V-01 | TC3 por encima del umbral | RL1 abierto, ventiladores encendidos |
| V-02 | Abrir la puerta con el PTC permitido | RL1 abierto en < 200 ms |
| V-03 | Desconectar FAN_P con el PTC calentando (ciclo de trabajo bajo) | SIF-03 antes de que TC3 llegue a T_SIS_MAX |
| V-04 | Desconectar TC3 / lectura congelada | SIF-04 |
| V-05 | Cortar el UART durante un ciclo | Según la decisión A-4 |
| V-06 | Colgar el firmware del control | El SIS retira el permiso y mantiene los ventiladores |
| V-07 | Abrir la rama del PTC (simular termostato abierto) con SSR1 al 100 % | SIF-08 |
| V-08 | Dos eventos térmicos seguidos | Bloqueo; persiste tras cortar la alimentación |
| V-09 | Iniciar con la puerta abierta (HMI y solicitud forzada) | HMI deshabilita, Mega rechaza, SIS no concede |
| V-10 | Cortar NA, NC o común; puentear NA con NC | Puerta inválida → PTC sin permiso |
| V-11 | Cerrar la puerta tras una pausa | El PTC no vuelve hasta "Reanudar" |
| V-12 | Reiniciar el Nano durante un ciclo | RL1 abierto y FAN_P encendido durante el reinicio |
| Periódica | Comprobar que RL1 abre de verdad (medir continuidad con el permiso retirado) | Abierto: el contacto no está soldado |
