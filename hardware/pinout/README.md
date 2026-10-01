# Pinout

Un archivo por nodo, cuando se defina: `control.md` (Mega), `sis.md` (Nano), `hmi.md` (ESP32-32E).

## Restricciones ya verificadas

**Arduino Mega**: SPI 50 (MISO), 51 (MOSI), 52 (SCK), 53 (SS); I2C 20 (SDA), 21 (SCL); `Serial1` 18/19, `Serial2` 16/17. Pines de sobra.

**Arduino Nano**: SPI D11 (MOSI), D12 (MISO), D13 (SCK), D10 (SS); UART D0/D1 (compartido con USB); I2C A4/A5 (sin usar); A6/A7 sólo analógicos. Interrupciones externas: D2, D3.

**ESP32-32E (HMI)**: no se cablea a ninguna señal de proceso. Sólo UART IO3/IO1 hacia la Mega.

## Señales a asignar

| Señal | Mega (control) | Nano (SIS) | ESP32 (HMI) |
|---|---|---|---|
| SPI SCK/MISO + CS TC1, CS TC2 | ✔ | — | — |
| SPI SCK/MISO + CS TC3 | — | ✔ | — |
| I2C (SHT31, SGP40) | ✔ | — | — |
| Salida SSR PTC | ✔ | — | — |
| Salida SSR ventilador | ✔ | forzado (OR) | — |
| Salida permiso PTC | — | ✔ | — |
| Realimentación del permiso | — | ✔ | — |
| Limit switch de puerta | ✔ | ✔ | — |
| Estado de termostatos | opcional | ✔ | — |
| UART Control↔SIS | Serial1 | Serial | — |
| UART Control↔HMI | Serial2 | — | UART0 |
| Reinicio manual / buzzer / LED de falla | — | ✔ | — |
