# Comunicación entre nodos

Contrato en `firmware/compartido/protocolo/` (única fuente de verdad, incluida por los tres firmwares).

## Enlaces

| Enlace | Puertos | Notas |
|---|---|---|
| Control ↔ SIS | Mega `Serial1` ↔ Nano `Serial` (D0/D1) | El UART del Nano es el del USB: desconectar el enlace para programarlo. |
| Control ↔ HMI | Mega `Serial2` (D16/D17) ↔ ESP32 UART2 reasignado a IO32 (RX) / IO25 (TX), en el conector I2C de la placa | Convertidor de nivel 5 V ↔ 3,3 V (LS1, decisión abierta A-9). UART0 queda libre para depurar por USB. |

Velocidad propuesta: 57600 8N1 en ambos enlaces (115200 tiene ≈ 2 % de error a 16 MHz en AVR). El HMI no habla con el SIS.
Cableado exacto: [pinout/](pinout/README.md). Formato de bytes de cada mensaje: [PLAN-MAESTRO §6](PLAN-MAESTRO.md#6-firmware-común).

## Principio

La comunicación es **informativa**. Nada que viaje por ella puede abrir el permiso del PTC ni cambiar umbrales del SIS.

## Trama (borrador)

```
[0xAA][LEN][TIPO][SEQ][PAYLOAD…][CRC16]
```

## Control ↔ SIS

| Mensaje | Sentido | Contenido | Periodo |
|---|---|---|---|
| `HB_CTRL` | Control → SIS | Contador, estado del ciclo | 10 Hz |
| `HB_SIS` | SIS → Control | Contador, estado SIS, causas de disparo (bitmask), TC3, puerta, termostato, permiso real | 10 Hz |
| `REQ_RESET` | Control → SIS | Solicitud de rearme (el SIS valida) | evento |
| `EVENTO` | SIS → Control | Disparo, rearme, falla de autotest | evento |

## Control ↔ HMI

| Mensaje | Sentido | Contenido |
|---|---|---|
| `SOLICITA_CICLO` | HMI → Mega | `calzado_id`, `intensidad_id`, `duracion_id` |
| `RESPUESTA_CICLO` | Mega → HMI | Aceptado / rechazado + motivo (puerta abierta, falla, id inválido) |
| `PAUSAR` / `REANUDAR` / `CANCELAR` / `RECONOCER` | HMI → Mega | Solicitudes; la Mega decide |
| `ESTADO` | Mega → HMI | Estado del ciclo, puerta, tiempo restante, T, HR, VOC, falla activa, estado del SIS, segundos de pausa restantes |
| `LOG` | Mega → HMI | Registro periódico (si se guarda en la SD) |

El HMI nunca envía temperaturas ni minutos.

## Fallas del enlace

| Falla | Reacción |
|---|---|
| Control sin `HB_SIS` > 500 ms | Cancela el ciclo, PTC off, muestra falla |
| SIS sin `HB_CTRL` > 2 s | SIS sigue operando; ver SIF-06 |
| Control sin HMI > 10 s | Cancela el ciclo y enfría |
| HMI sin `ESTADO` > 2 s | Muestra "Sin comunicación"; no asume valores |
| CRC inválido | Descarta y cuenta errores |

## Pendiente

- Comportamiento exacto de SIF-06 (disparo o sólo alarma).
- Campo de versión del protocolo en los heartbeats.
