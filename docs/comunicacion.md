# Comunicación entre nodos

Resumen. El formato byte a byte de la trama y de cada mensaje está en [PLAN-MAESTRO §6](PLAN-MAESTRO.md#6-firmware-común). Contrato de código en `firmware/compartido/qs_protocol.h` (única fuente de verdad, incluido por los tres firmwares).

## Enlaces

Los tres controladores trabajan a 3,3 V: **cables directos**, sin convertidores.

| Enlace | Puertos | Notas |
|---|---|---|
| Control ↔ SIS | Control UART2 (GPIO17 TX / GPIO16 RX) ↔ SIS UART2 (GPIO16 RX / GPIO17 TX), cruzados | La depuración de cada placa va por su USB |
| Control ↔ HMI | Control UART1 reasignado (GPIO4 TX / GPIO35 RX) ↔ HMI UART2 reasignado a IO32 (RX) / IO25 (TX), en el conector I2C de la placa | UART0 del HMI queda libre para depurar por USB |

Velocidad: **115200 8N1** en ambos enlaces. GND común entre los tres nodos. El HMI no habla con el SIS. Pines exactos: [pinout/](pinout/README.md).

## Principio

La comunicación es **informativa**. Nada que viaje por ella puede conceder el permiso del PTC ni cambiar umbrales del SIS: lo que el SIS recibe **sólo puede restringir**.

## Mensajes (resumen)

| Mensaje | Sentido | Periodo | Contenido |
|---|---|---|---|
| `HB_CTRL` | Control → SIS | 100 ms | Estado del ciclo; `heat_request`; estado de las salidas del control |
| `HB_SIS` | SIS → Control | 100 ms | Estado del SIS, causas de disparo, puerta, estado de sus relés, eventos térmicos |
| `REQ_RESET` / `REQ_SERVICE` | Control → SIS | evento | Solicitud de rearme / desbloqueo (el SIS valida) |
| `EVENT` | SIS → Control | evento | Disparo, rearme, bloqueo… |
| `HMI_HB` | HMI → Control | 500 ms | Latido del HMI |
| `REQ_START` | HMI → Control | evento | `shoe_id`, `intensity_id`, `duration_id` |
| `REQ_PAUSE` / `REQ_RESUME` / `REQ_CANCEL` / `REQ_ACK` / `REQ_REARM` | HMI → Control | evento | Solicitudes; el control decide |
| `STATUS` | Control → HMI | 200 ms | Estado del ciclo, puerta, tiempos, T, HR, VOC, falla, estado del SIS |
| `RESP_START` | Control → HMI | evento | Aceptado / rechazado + motivo |

El HMI nunca envía temperaturas ni minutos.

## Fallas del enlace

| Falla | Reacción |
|---|---|
| Control sin `HB_SIS` > 500 ms | Apaga SSR1 y pasa a FALLA |
| SIS sin `HB_CTRL` > 2 s | Retira el permiso; SIF-06 (pendiente A-4) |
| Control sin `HMI_HB` > 10 s | Cancela el ciclo y enfría |
| HMI sin `STATUS` > 2 s | Muestra "Sin comunicación"; no asume valores |
| CRC inválido | Descarta y cuenta errores |
