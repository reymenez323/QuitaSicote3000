# Comunicación entre MCU

Contrato en `firmware/compartido/protocolo/` (única fuente de verdad, incluida por ambos firmwares).

## Enlace físico

- UART dedicado, 3,3 V, TX/RX cruzados + GND común. Velocidad inicial: 115200 8N1 **[VERIFICAR]**.
- Alternativa descartada por ahora: I2C/SPI (acopla más los buses y complica la independencia).

## Enlaces

| Enlace | Puertos | Notas |
|---|---|---|
| Control ↔ SIS | Mega `Serial1` ↔ Nano `Serial` (D0/D1) | 5 V ambos. Desconectar para programar el Nano. |
| Control ↔ HMI | Mega `Serial2` ↔ ESP32 IO3/IO1 | Divisor 5 V → 3,3 V en el TX de la Mega. |

El HMI **no** se comunica con el SIS. Los mensajes del HMI son solicitudes (iniciar, pausar, cancelar, ajustar parámetros) y el flujo de datos de la Mega al HMI es de estado y telemetría.

## Principio

La comunicación es **informativa**. Nada que viaje por ella puede abrir el permiso del PTC ni modificar umbrales del SIS.

## Tramas (borrador)

```
[0xAA][LEN][TIPO][SEQ][PAYLOAD…][CRC16]
```

| Tipo | Sentido | Contenido | Periodo |
|---|---|---|---|
| `HB_CTRL` | Control → SIS | Contador, estado del ciclo, modo | 10 Hz |
| `HB_SIS` | SIS → Control | Contador, estado SIS, causa de disparo (bitmask), TC3, puerta, termostato, permiso real | 10 Hz |
| `REQ_RESET` | Control → SIS | Solicitud de reinicio tras disparo (el SIS valida) | evento |
| `EVENTO` | SIS → Control | Disparo, rearme, falla de autotest | evento |

## Mensajes Control ↔ HMI (borrador)

| Mensaje | Sentido | Contenido |
|---|---|---|
| `SOLICITA_CICLO` | HMI → Mega | `calzado_id`, `intensidad_id` (0–2), `duracion_id` (0–2) |
| `RESPUESTA_CICLO` | Mega → HMI | Aceptado / rechazado + motivo (puerta abierta, falla, id inválido) |
| `CANCELAR` / `PAUSAR` | HMI → Mega | Solicitudes; la Mega decide |
| `ESTADO` | Mega → HMI | Estado del ciclo, T, HR, VOC, tiempo restante, fallas, estado del SIS |
| `LOG` | Mega → HMI | Registro periódico para guardar en SD |

El HMI nunca envía temperaturas ni minutos.

## Comportamiento ante fallas del enlace

| Falla | Control | SIS |
|---|---|---|
| Control deja de ver HB_SIS (> 500 ms) | Aborta ciclo, PTC off, muestra falla | — |
| SIS deja de ver HB_CTRL (> 2 s) | — | Sigue operando; si había calentamiento, ver SIF-06 |
| Control deja de ver al HMI (> 10 s [VERIFICAR]) | Aborta ciclo, PTC off, enfría (ADR-0006, M-02) | — |
| Trama con CRC inválido | Descarta; cuenta errores | Descarta; cuenta errores |

## Pendiente

- Definir si el SIS usa la ausencia de HB_CTRL como disparo (SIF-06) o sólo como alarma.
- Versionado del protocolo (campo de versión en el heartbeat).
