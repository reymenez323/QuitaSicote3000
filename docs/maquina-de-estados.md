# Máquina de estados del ciclo (control, ESP32-S3)

Resumen. Tabla completa con acciones y transiciones en [PLAN-MAESTRO §8.4](PLAN-MAESTRO.md#84-máquina-de-estados). Esta máquina corre en el control (ESP32-S3), que también maneja los ventiladores; el SIS sólo puede vetar el apagado del ventilador del PTC o forzar el de circulación.

```mermaid
stateDiagram-v2
  [*] --> AUTOTEST
  AUTOTEST --> LISTO: sensores OK + SIS OK
  AUTOTEST --> FALLA: sensor o SIS en fallo
  LISTO --> PRECALENTAMIENTO: Iniciar + puerta cerrada
  PRECALENTAMIENTO --> TRATAMIENTO: T objetivo alcanzada
  TRATAMIENTO --> ENFRIAMIENTO: duración cumplida
  ENFRIAMIENTO --> COMPLETO: T segura
  COMPLETO --> LISTO: usuario acepta / abre la puerta
  PRECALENTAMIENTO --> PAUSA: puerta abierta o Pausar
  TRATAMIENTO --> PAUSA: puerta abierta o Pausar
  PAUSA --> PRECALENTAMIENTO: puerta cerrada + Reanudar
  PAUSA --> ENFRIAMIENTO: 5 min sin reanudar
  PRECALENTAMIENTO --> ENFRIAMIENTO: Cancelar
  TRATAMIENTO --> ENFRIAMIENTO: Cancelar
  PRECALENTAMIENTO --> FALLA: falla / timeout
  TRATAMIENTO --> FALLA: falla
  FALLA --> ENFRIAMIENTO: falla reconocida
```

| Estado | PTC | Ventiladores | Notas |
|---|---|---|---|
| AUTOTEST | Off | Ventilador PTC on | Lecturas de sensores, enlace con el SIS, puerta válida, prueba del relé del ventilador del PTC |
| LISTO | Off | Off sólo si está frío (el SIS debe permitir apagar el del PTC) | Espera al usuario |
| PRECALENTAMIENTO | Control | On | Con tiempo límite; si no alcanza la T, FALLA |
| TRATAMIENTO | Control | On | Cuenta la duración sólo dentro de la banda de T |
| PAUSA | Off | On | Cuenta regresiva de 5 min |
| ENFRIAMIENTO | Off | On | Hasta T segura |
| COMPLETO | Off | On hasta enfriar | Aviso al usuario |
| FALLA | Off | On | Muestra causa; el SIS puede estar disparado |

## Reglas de puerta y pausa

- **Inicio**: sólo desde LISTO, con la puerta cerrada, autotest correcto y SIS en OK.
- **Puerta abierta durante el ciclo**: el SIS inhibe el PTC al instante (sin enclavar) y el control pasa a PAUSA.
- **Al cerrar la puerta** el PTC no vuelve solo: hace falta pulsar "Reanudar".
- **Pausa de más de 5 min**: el ciclo se cancela y pasa a ENFRIAMIENTO; hay que volver a elegir el perfil.

## Otras reglas

- Tope global de 120 min (también en el SIS).
- Tras un corte de energía: AUTOTEST → LISTO; nunca retoma un ciclo.
- Si el control pierde el enlace con el HMI más de 10 s durante un ciclo: cancela y enfría.

El SIS tiene su propia máquina ([firmware/sis](../firmware/sis/README.md)) y no depende de ésta.
