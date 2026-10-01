# Validación del SIS

Cada SIF se prueba **inyectando la falla**, no esperando a que ocurra. Registrar fecha, versión de firmware, resultado y tiempo de respuesta en `datos/logs/`.

| Prueba | Método | Resultado esperado |
|---|---|---|
| V-01 (SIF-01) | Simular T alta en TC3 (calentar el termopar con secador de aire / ventana de prueba, o inyectar lectura en banco) | Permiso abre antes de T_SIS_MAX + margen; ventilador ON |
| V-02 (SIF-02) | Abrir puerta con PTC energizado | Permiso abre en < 200 ms [VERIFICAR] |
| V-03 (SIF-03) | Apagar el ventilador de circulación con PTC encendido (a baja potencia/bajo duty) | Disparo por pendiente o termostato |
| V-04 (SIF-04) | Desconectar TC3; cubrir con lectura congelada | Disparo; no "ignorar" |
| V-05 (SIF-05) | Forzar fallo del permiso (puentearlo en banco) | Autotest/realimentación detecta discrepancia |
| V-06 (SIF-06) | Desconectar UART con ciclo en curso | Comportamiento según decisión final |
| V-07 | Cortar alimentación del SIS | PTC sin energía |
| V-08 | Colgar el control (bucle infinito) | PTC se apaga por SIF-06/termostato; SIS sigue vivo |
| V-09 | Desconectar termostatos | PTC sin energía |
| V-10 | Prueba funcional de termostatos con fuente de calor controlada | Apertura dentro del rango de la hoja de datos |

Frecuencia de repetición (prueba periódica): antes de cada modificación de firmware o hardware y, como mínimo, [VERIFICAR].
