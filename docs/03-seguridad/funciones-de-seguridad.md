# Funciones instrumentadas de seguridad (SIF)

Todas las acciones de disparo: **abrir permiso del PTC**, **forzar ventilador ON**, **latch de disparo**, **avisar al control**.

| ID | Condición de disparo | Sensores | Umbral | Reset |
|---|---|---|---|---|
| SIF-01 | Sobretemperatura | TC3 | T_SIS_MAX [VERIFICAR] con histéresis | T < T_SIS_RESET + reinicio manual |
| SIF-02 | Puerta abierta con PTC energizado | Limit switch | Abierta | Puerta cerrada + reinicio |
| SIF-03 | Calentamiento sin flujo de aire | TC3 vs. tendencia (ΔT/Δt alto) y/o realimentación del ventilador [VERIFICAR] | Pendiente > X °C/s | Reinicio manual |
| SIF-04 | Sensor inválido | MAX6675 (bit abierto, valor fuera de rango, lectura congelada) | N lecturas consecutivas malas | Sensor válido + reinicio |
| SIF-05 | Autodiagnóstico fallido | Realimentación del permiso, autotest | Discrepancia | Reinicio completo |
| SIF-06 | Pérdida de heartbeat del control con calentamiento | UART | > T_HB [VERIFICAR] | Heartbeat restablecido + reinicio |
| SIF-07 | Tiempo máximo de calentamiento continuo | Temporizador interno | T_MAX_CALENT [VERIFICAR] | Reinicio manual |

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

- Detección **latcheada**: un disparo permanece hasta reinicio deliberado.
- Tiempo de respuesta total (sensor → permiso abierto) documentado y medido.
- Sin dependencia de la comunicación para disparar (excepto SIF-06, cuya causa es la propia comunicación).
- Documentar cómo se prueba cada SIF: [pruebas-de-validacion-sis.md](pruebas-de-validacion-sis.md).
