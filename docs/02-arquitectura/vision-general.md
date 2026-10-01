# Visión general de la arquitectura

## 1. Principio rector: tres capas de protección independientes

```
Capa 3  Termostatos térmicos (NC) en serie con la alimentación del PTC   → hardware puro, sin software
Capa 2  SIS (MCU 2) con sensor propio, puerta y permiso en serie         → software mínimo, independiente
Capa 1  Control (MCU 1): límites por software en el lazo de control      → lógica de proceso
```

Cada capa debe poder apagar el PTC **por sí sola**. Ninguna capa superior depende del correcto funcionamiento de una inferior.

## 2. Diagrama de bloques

```mermaid
flowchart LR
  subgraph CTRL[MCU 1 · Control · Arduino Mega 2560]
    FSM[Máquina de estados + PID]
  end
  subgraph SIS[MCU 2 · SIS · Arduino Nano]
    SAF[Lógica de seguridad]
  end
  subgraph HMI[Nodo HMI · ESP32-32E 3.2" · sólo comunicación]
    UI[UI táctil + registro SD]
  end
  CTRL <-->|UART Serial2 + divisor 5V→3.3V| HMI

  TC1[TC-K 1 · aire recámara] --> M1[MAX6675] --> CTRL
  TC2[TC-K 2 · salida PTC / ducto] --> M2[MAX6675] --> CTRL
  TC3[TC-K 3 · independiente] --> M3[MAX6675] --> SIS
  SHT[SHT31 · T/HR] -->|I2C| CTRL
  SGP[SGP40 · VOC] -->|I2C| CTRL
  DOOR[Limit switch puerta] --> CTRL
  DOOR --> SIS

  CTRL <-->|UART + heartbeat| SIS

  CTRL -->|PWM/ON-OFF| SSR1[SSR-DC PTC]
  CTRL -->|ON/OFF| SSR2[SSR-DC ventilador recámara]
  SIS -->|PERMISO| INT[Interruptor de permiso en serie]
  TH[Termostato 80°C NC · rearme automático] --- INT
  TH -.->|sensado| SIS
  SSR1 --- INT --> PTC[PTC + su ventilador]
  SSR2 --> FAN[Ventilador de circulación]
  SIS -.->|forzar ON| SSR2
  CTRL -->|ON/OFF| RLY[Módulo de relé 12 V · NC]
  RLY --> PFAN[Ventilador del PTC]
  SIS -.->|permiso de apagado| RLY
```

Lectura del diagrama: la alimentación del PTC pasa por **tres contactos en serie**: SSR de control, permiso del SIS y termostatos. Basta que uno abra para que el PTC se apague.

## 3. Reparto de responsabilidades

| Elemento | MCU 1 Control | MCU 2 SIS |
|---|---|---|
| TC1 y TC2 (MAX6675) | Lee | — |
| TC3 (MAX6675) | — | Lee (sensor **exclusivo**) |
| SHT31, SGP40 | Lee | — |
| Limit switch de puerta | Lee (UI, pausa) | Lee (disparo) |
| SSR del PTC | Comanda | Puede inhibirlo vía permiso en serie |
| SSR del ventilador de recámara | Comanda | Puede forzarlo a ON en disparo (enfriamiento) |
| Relé del ventilador del PTC (NC) | Comanda (apagado sólo en frío) | Concede el permiso de apagado y sensa su alimentación |
| Termostato 80 °C | — | Lee estado y enclava el disparo (SIF-08) |
| Pantalla / UI | Vía HMI (UART) | No |
| Decisión de ciclo y setpoints | Sí | No |
| Decisión de disparo | No | **Sí, autoridad única** |

Notas de diseño:

- **Un MAX6675 no se comparte entre MCU.** Es un dispositivo SPI de solo lectura; dos maestros sobre el mismo módulo romperían la independencia. Por eso TC3 es de uso exclusivo del SIS.
- El SIS **no recibe comandos de control**, sólo lee un *modo* informativo y el heartbeat. Puede reportar al control, pero el control no puede desactivarlo ni cambiar sus umbrales en operación.
- Los umbrales del SIS son constantes de compilación (`firmware/sis`), no parámetros recibidos por la red.

## 4. Principios de falla segura

- **Lógica de energizar para operar**: permiso del SIS = señal activa; ausencia de señal (MCU colgado, cable roto, sin alimentación) = PTC sin energía.
- **Watchdog por hardware** en ambos MCU. Heartbeat bidireccional; si el SIS deja de ver al control sigue funcionando (el control no es crítico para seguridad), pero muestra falla; si el control deja de ver al SIS, aborta el ciclo.
- El ventilador **sigue encendido** tras un disparo para evacuar calor residual (el PTC con su propio ventilador mitiga, pero no sustituye).
- Lectura de termopar inválida (circuito abierto, bit de MAX6675, valor fuera de rango, valor congelado) = condición de disparo, no "ignorar la lectura".

## 5. Documentos relacionados

- [hardware.md](hardware.md) — componentes, asignación de sensores, riesgos
- [firmware-control.md](firmware-control.md), [firmware-sis.md](firmware-sis.md) y [firmware-hmi.md](firmware-hmi.md)
- [comunicacion-inter-mcu.md](comunicacion-inter-mcu.md)
- [maquina-de-estados.md](maquina-de-estados.md)
- [../03-seguridad/funciones-de-seguridad.md](../03-seguridad/funciones-de-seguridad.md)
