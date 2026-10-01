# Funciones instrumentadas de seguridad (SIF)

Todas las acciones de disparo: **abrir permiso del PTC**, **forzar ventilador ON**, **latch de disparo**, **avisar al control**.

| ID | Condición de disparo | Sensores | Umbral | Reset |
|---|---|---|---|---|
| SIF-01 | Sobretemperatura | TC3 | T_SIS_MAX [VERIFICAR] con histéresis | T < T_SIS_RESET + reinicio manual |
| SIF-02 | Puerta abierta (con PTC energizado o antes de iniciar) | Limit switch | Abierta o lectura inválida | **No enclava**: el permiso sigue a la puerta. Al cerrar, el PTC sólo vuelve con orden explícita del usuario (Reanudar / Iniciar) |
| SIF-03 | Calentamiento sin flujo de aire | TC3 vs. tendencia (ΔT/Δt alto) y nodo de alimentación del ventilador del PTC sin tensión con el PTC energizado ([ADR-0009](../decisiones/ADR-0009-ventilador-ptc-con-rele.md)) | Pendiente > X °C/s | Reinicio manual |
| SIF-04 | Sensor inválido | MAX6675 (bit abierto, valor fuera de rango, lectura congelada) | N lecturas consecutivas malas | Sensor válido + reinicio |
| SIF-05 | Autodiagnóstico fallido | Realimentación del permiso, autotest | Discrepancia | Reinicio completo |
| SIF-06 | Pérdida de heartbeat del control con calentamiento | UART | > T_HB [VERIFICAR] | Heartbeat restablecido + reinicio |
| SIF-07 | Tiempo máximo de calentamiento continuo | Temporizador interno | T_MAX_CALENT [VERIFICAR] | Reinicio manual |
| SIF-08 | Termostato de 80 °C abierto (rearme automático) | Divisor tras el termostato | Nodo bajo con 12 V presentes | Enclavado en EEPROM; enfriamiento + autotest + confirmación; ver [ADR-0008](../decisiones/ADR-0008-termostato-rearme-automatico.md) |

## Matriz causa–efecto

| Causa \ Efecto | Permiso PTC abierto | Ventilador forzado ON | Alarma sonora/LED | Aviso al control |
|---|---|---|---|---|
| SIF-01 | ✔ | ✔ | ✔ | ✔ |
| SIF-02 | ✔ | — | — | ✔ |
| SIF-03 | ✔ | ✔ | ✔ | ✔ |
| SIF-04 | ✔ | ✔ | ✔ | ✔ |
| SIF-05 | ✔ | — | ✔ | ✔ |
| SIF-06 | ✔ | ✔ | ✔ | — |
| SIF-07 | ✔ | ✔ | ✔ | ✔ |
| SIF-08 | ✔ (aunque el termostato rearme) | ✔ | ✔ | ✔ |

## Escalonado de temperaturas (provisional)

| Nivel | Valor | Quién actúa |
|---|---|---|
| Objetivo (aire, TC1) | 35–50 °C según perfil | Control |
| Límite de software | objetivo + 5 °C | Control |
| Disparo SIS (TC3) | ≈ 65–70 °C **[VERIFICAR]** | SIS |
| Termostato | 80 °C (puede abrir entre 75 y 85 °C) | Hardware |

TC3 está junto al PTC y leerá más que TC1 en operación normal; el umbral del SIS debe quedar por encima de ese valor normal (medido en ensayo) y por debajo del rango de apertura del termostato.

```
T objetivo  <  límite de software del control  <  disparo SIS (TC3)  <  apertura de termostatos  <  límite del material/PTC
```

Debe haber margen suficiente entre cada nivel para que la capa inferior **no actúe en operación normal** y la superior **sí actúe si la inferior falla**.

## Requisitos de implementación

- Detección **latcheada**: un disparo permanece hasta reinicio deliberado (excepto SIF-02, puerta, que inhibe sin enclavar y exige orden explícita para reanudar).
- **Limit switch con doble contacto (NA y NC)**: el común va a GND; el contacto **NA** a una entrada con pull-up y el **NC** a otra. Las dos señales deben ser **complementarias**:

  | Puerta | Entrada NA | Entrada NC | Interpretación |
  |---|---|---|---|
  | Cerrada | BAJA (cerrado) | ALTA (abierto) | Puerta cerrada válida |
  | Abierta | ALTA | BAJA | Puerta abierta válida |
  | — | BAJA | BAJA | **Inválido** (cortocircuito entre líneas, switch dañado) |
  | — | ALTA | ALTA | **Inválido** (cable roto, común desconectado, conector suelto) |

  Cualquier combinación inválida se trata como **puerta abierta con falla**: inhibe el PTC y el HMI avisa. Sólo "NA bajo + NC alto" cuenta como cerrada. Ambas MCU leen las dos entradas; el SIS aplica además un antirrebote de decenas de ms. Limitación: con un solo switch de un polo no se detectaría un contacto soldado mecánicamente.
- Tiempo de respuesta total (sensor → permiso abierto) documentado y medido.
- Sin dependencia de la comunicación para disparar (excepto SIF-06, cuya causa es la propia comunicación).
- Documentar cómo se prueba cada SIF: [pruebas-de-validacion-sis.md](pruebas-de-validacion-sis.md).
