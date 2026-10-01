# Plan de pruebas

Niveles, de menor a mayor riesgo. No se pasa al siguiente sin cerrar el anterior.

| Nivel | Qué se prueba | Dónde | Calentador |
|---|---|---|---|
| 0. Unitario (PC) | Lógica pura: PID, filtros, funciones SIF, protocolo (CRC, parser) | `firmware/*/test/` con PlatformIO `native` | — |
| 1. Banco de sensores | Drivers: MAX6675, SHT31, SGP40, puerta | Placa suelta | Sin carga |
| 2. Banco de actuadores | SSR PTC y ventilador con carga ficticia (resistencia/LED/ventilador solo) | Placa + SSR | Carga ficticia |
| 3. Enlace entre MCU | Heartbeat, pérdida de enlace, CRC | Ambos MCU | Sin carga |
| 4. Validación SIS | Tabla V-01…V-10 | Banco | Carga ficticia |
| 5. Integración en recámara | Ciclo completo con PTC real, sin calzado | Equipo completo | Real, T baja |
| 5b. Matriz de perfiles | Cada combinación tipo × intensidad × duración (≈ 36) con calzado simulado, midiendo T máx. en TC1, TC2 y TC3 | Equipo completo | Real |
| 6. Ensayo con calzado | Eficacia (VOC, humedad, tiempo), repetibilidad | Equipo completo | Real |

Resultados y registros: `datos/logs/` (nombre `AAAA-MM-DD_nivel_descripcion.csv`).
