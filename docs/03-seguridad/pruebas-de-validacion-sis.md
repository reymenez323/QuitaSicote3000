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
| V-10 | Prueba funcional del termostato con fuente de calor controlada | Apertura dentro del rango de la hoja de datos (75–85 °C aprox.) |
| V-11 (SIF-08) | Abrir el termostato (o puentear el nodo a tierra) y luego cerrarlo | SIS detecta, enclava; el PTC no se reenergiza al rearmar |
| V-12 | Tras V-11, quitar y volver a dar alimentación | El enclavamiento persiste (EEPROM) |
| V-14 | Intentar iniciar un ciclo con la puerta abierta (HMI, y solicitud forzada por UART) | HMI deshabilita Iniciar; la Mega rechaza; el SIS no concede permiso |
| V-15 | Cortar el cable del común, del NA o del NC; desconectar el conector; puentear NA con NC | Combinación inválida detectada; se trata como puerta abierta con falla; PTC inhibido |
| V-16 | Cerrar la puerta tras una pausa | El PTC no se reactiva hasta que el usuario pulsa Reanudar |
| V-13 | Con el PTC energizado, apagar el ventilador del PTC (a baja potencia) | Subida de TC3 detectada (SIF-03) antes de llegar a 80 °C |

Frecuencia de repetición (prueba periódica): antes de cada modificación de firmware o hardware y, como mínimo, [VERIFICAR].
