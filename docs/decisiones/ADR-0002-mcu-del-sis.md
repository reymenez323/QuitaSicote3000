# ADR-0002 — Controladores: control en ESP32-S3, SIS en ESP32 Dev Kit

**Estado:** Aceptada (decisión del usuario, 2026-09-30). Sustituye a la versión anterior (control en Arduino Mega 2560, SIS en Arduino Nano).

## Decisión
- **Control: ESP32-S3** (placa de desarrollo; modelo exacto por confirmar, se asume ESP32-S3-DevKitC-1).
- **SIS: ESP32 Dev Kit** (módulo ESP32-WROOM-32; modelo exacto por confirmar, se asume DevKitC/DOIT de 30 o 38 pines).
- HMI: sin cambios (ESP32-32E con pantalla).

## Implicaciones para el electrónico
- **Todo a 3,3 V**: los tres controladores se conectan entre sí directamente. Ninguna entrada de un ESP32 tolera 5 V: los módulos que se conecten a sus pines deben trabajar o entregar señales de 3,3 V.
- **Sensores a 3,3 V**: MAX6675 (3,0–5,5 V), SHT31 y SGP40 se alimentan desde el pin 3V3 de cada placa, para que sus salidas sean de 3,3 V.
- **Módulos de relé y SSR manejados con 3,3 V**: hay que comprobar que cada módulo se activa de forma fiable con 3,3 V en su entrada (disparo por nivel alto) [VERIFICAR en F3]. No usar módulos con disparo por nivel bajo alimentados a 5 V: con 3,3 V en la entrada podrían quedar a medio activar.
- **Pines con comportamiento especial al arrancar**: se evitan para las salidas (en el ESP32: GPIO 0, 2, 5, 12, 15; 1/3 son UART0; 6–11 son la flash; 34–39 sólo entrada y sin pull-up. En el ESP32-S3: GPIO 0, 3, 45, 46; 19/20 USB; 26–32 flash; 33–37 si la placa tiene PSRAM octal; 43/44 UART0).

## Implicaciones para el firmware
- **SIS**: Wi-Fi y Bluetooth **apagados**; un único lazo; *Task Watchdog* activo; persistencia en **NVS** (`Preferences`) en lugar de EEPROM; sin bibliotecas de terceros; sin asignación dinámica tras el arranque; detector de caída de tensión (*brownout*) activo.
- **Control**: Arduino-ESP32 sobre el ESP32-S3; puede usar FreeRTOS, pero mantiene un planificador simple; consola por el USB nativo.
- Velocidad de los UART: 115200 (los ESP32 no tienen el error de baudios de los AVR).

## Causa común
Los tres controladores son Espressif con el mismo toolchain. Un fallo del compilador, del core Arduino-ESP32 o del ESP-IDF podría afectarlos a la vez. Mitigación:
- el SIS usa el mínimo de funciones del core y nada de radio;
- la capa 3 (termostato) no depende de ningún controlador;
- la validación del SIS se repite tras cualquier cambio de versión del core.
