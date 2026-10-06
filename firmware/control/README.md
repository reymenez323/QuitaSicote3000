# Firmware de control (ESP32 DevKit V1 n.º 1)

> **Sólo control.** Ninguna función de seguridad se implementa aquí ([ADR-0012](../../docs/decisiones/ADR-0012-regla-mega-control-nano-sis.md)). Sus límites son de proceso (capa 1).

Especificación: [PLAN-MAESTRO §8](../../docs/PLAN-MAESTRO.md#8-firmware-de-control-esp32-s3). Pines: [docs/pinout/control-esp32.md](../../docs/pinout/control-esp32.md).

## Archivos

| Archivo | Contenido |
|---|---|
| [src/config.h](src/config.h) | Pines, parámetros y la **tabla de perfiles** (temperaturas y duraciones) |
| [src/sensors.h](src/sensors.h) | Lectura de MAX6675, SHT31 y SGP40 |
| [src/main.cpp](src/main.cpp) | Todo lo demás: estado compartido → fallas → solicitudes del HMI → máquina de estados → calefactor y ventiladores → las cinco tareas → arranque |

## Cómo funciona

- `taskCycle` (10 ms) lee la puerta, avanza la máquina de estados y escribe **SSR1** (PTC), **SSR2** (ventilador de circulación) y **RL2** (pedir apagar el ventilador del PTC).
- `taskSensors` lee TC1 y TC2 alternados cada 250 ms y, una vez por segundo, el SHT31 y el SGP40 (con el algoritmo de índice de olor de Sensirion).
- `taskSisLink` y `taskHmiLink` atienden los dos enlaces serie.
- `taskConsole` atiende el USB.

Reglas que el código respeta:

- SSR1 sólo conduce calentando, con la puerta cerrada **y** con el permiso del SIS concedido.
- Abrir la puerta pausa el ciclo; cerrarla no lo reanuda: hace falta "Reanudar".
- Una pausa de más de 5 min, o perder la pantalla más de 10 s, cancela el ciclo.
- Tras un reinicio o un corte de energía siempre se empieza en AUTOTEST: un ciclo nunca se reanuda solo.
- El ventilador del PTC nunca se pide apagar calentando ni con el equipo caliente.

## Consola (USB, 115200)

| Comando | Efecto |
|---|---|
| `status` | Una línea con el estado actual |
| `log on` / `log off` | Registro CSV permanente (durante un ciclo se registra siempre, una línea por segundo) |
| `servicio desbloquear` | Pide al SIS abrir la ventana de desbloqueo |

## Pendiente

- Regulación en etapa 1 (histéresis). El PI con ventana proporcional queda para la fase F9.
- Valores de los perfiles y límites de TC2: se ajustan con los ensayos (F7 y F9).
