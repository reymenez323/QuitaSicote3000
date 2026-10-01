# Pinout — ESP32-32E con pantalla 3.2" (nodo HMI)

Placa E32R32P ([wiki](https://www.lcdwiki.com/3.2inch_ESP32-32E_Display)). Lógica a **3,3 V** (las entradas **no** toleran 5 V).
El HMI **no se conecta a nada del proceso** [CONFIRMADO]: sólo alimentación y el enlace con la Mega.
Las constantes van en `firmware/hmi/src/hal/board_pins.h` y en la configuración de LovyanGFX.

## 1. Pines internos de la placa (fijos, verificados en la wiki)

No se reasignan; el firmware sólo los configura.

| GPIO | Uso en la placa | Uso en el firmware | Notas |
|---|---|---|---|
| IO15 | LCD CS | LovyanGFX panel `pin_cs` | Pin de arranque (strapping) |
| IO2 | LCD DC | `pin_dc` | Pin de arranque |
| IO14 | SPI SCLK (LCD y táctil) | `pin_sclk` | |
| IO13 | SPI MOSI (LCD y táctil) | `pin_mosi` | |
| IO12 | SPI MISO (LCD y táctil) | `pin_miso` | Pin de arranque: no forzarlo a ALTO al encender |
| EN | Reset del ESP32 y del LCD | `pin_rst = -1` | |
| IO27 | Retroiluminación | `Light_PWM`, ALTO = encendida | |
| IO33 | Táctil XPT2046 CS | `Touch_XPT2046 pin_cs` | Mismo bus SPI que el LCD (`bus_shared = true`) |
| IO36 | Táctil IRQ | `pin_int` | Sólo entrada; BAJO = toque |
| IO5, IO18, IO19, IO23 | Tarjeta microSD (CS, SCLK, MISO, MOSI) | Sin uso por ahora | IO18/19/23 también en el conector SPI |
| IO4, IO26 | Altavoz: habilitación y DAC | Sin uso por ahora | |
| IO22, IO16, IO17 | LED RGB (rojo, verde, azul), **ánodo común: BAJO = encendido** | Poner los tres en ALTO al arrancar (apagado) | IO16/IO17 son los pines por defecto de UART2: **por eso UART2 se reasigna** (§2) |
| IO34 | ADC de batería | Sin uso | Sólo entrada |
| IO0 | Botón BOOT | Sin uso (arranque en modo descarga) | Pin de arranque |
| IO1, IO3 | UART0 (TX0/RX0) → conversor USB **CH340C** | Consola de depuración por USB-C | |

## 2. Conexiones externas usadas

| Conector de la placa | Pin | Dir. | Señal | Conecta a | Elementos en la línea | Constante |
|---|---|---|---|---|---|---|
| **I2C (1,25 mm, 4 pines)** | **IO25** | Salida | UART2 TX → Mega | Mega **D17 (RX2)** | **LS1** canal 2 (LV2 → HV2) | `PIN_LINK_TX = 25` |
| **I2C (1,25 mm, 4 pines)** | **IO32** | Entrada | UART2 RX ← Mega | Mega **D16 (TX2)** | **LS1** canal 1 (HV1 → LV1). **Nunca 5 V directo** | `PIN_LINK_RX = 32` |
| I2C (1,25 mm, 4 pines) | 3V3 | — | Alimentación del lado de 3,3 V de LS1 (LV) | LS1.LV | — | — |
| I2C (1,25 mm, 4 pines) | GND | — | Tierra del enlace | GND común | — | — |
| USB-C | 5 V / GND | Entrada | Alimentación | **5V_A** (REG_A) | Por cable USB-C o pigtail | — |

Configuración del enlace en el firmware (el orden de argumentos es RX y luego TX):

```cpp
Serial2.begin(57600, SERIAL_8N1, /*rx=*/32, /*tx=*/25);
```

**[VERIFICAR con multímetro]** el orden de los 4 pines del conector I2C y que incluya GND y 3V3. Si no incluyera GND, tomarla del conector serie.

**LS1** es un módulo convertidor de nivel lógico (5 V ↔ 3,3 V) y depende de la decisión abierta A-9 ([PLAN-MAESTRO §4.2](../PLAN-MAESTRO.md#42-abiertas)). El ESP32 **no tolera 5 V**: sin LS1 no se debe conectar la salida TX2 de la Mega. Durante el prototipo del HMI (que usa un simulador) no hace falta.

**[PROPUESTA]** Usar UART2 en el conector I2C en vez de UART0 tiene dos ventajas: UART0 queda libre para depurar por USB y se evita la contención con el CH340C, que también maneja IO3.

## 3. Conectores sin uso

| Conector | Pines | Uso |
|---|---|---|
| Serie (1,25 mm, 4P) | RXD0 (IO3), TXD0 (IO1), GND, VCC | **Sin señales.** Su VCC podría servir de entrada de 5 V en lugar del USB-C **[VERIFICAR]** |
| SPI periférico (1,25 mm, 4P) | IO21 (CS), IO18, IO19, IO23 | Sin uso |
| Entradas de expansión (1,25 mm, 2P) | IO35, IO39 | Sin uso (sólo entrada) |
| Batería (1,25 mm, 2P) | — | Sin uso |
| Altavoz (1,25 mm, 2P) | — | Sin uso por ahora |
| microSD | — | Sin uso por ahora (registro en SD: decisión abierta A-7) |

## 4. Estado durante el reinicio

El HMI no maneja nada del proceso: un reinicio sólo deja la pantalla en negro. Si dura más de 10 s con un ciclo activo, la Mega cancela y enfría.

## 5. `board_pins.h` (contenido esperado)

```cpp
// Pantalla y táctil (fijos de la placa)
constexpr int PIN_LCD_CS   = 15;
constexpr int PIN_LCD_DC   = 2;
constexpr int PIN_SPI_SCLK = 14;
constexpr int PIN_SPI_MOSI = 13;
constexpr int PIN_SPI_MISO = 12;
constexpr int PIN_LCD_RST  = -1;   // compartido con EN
constexpr int PIN_LCD_BL   = 27;   // ALTO = encendida
constexpr int PIN_TOUCH_CS = 33;
constexpr int PIN_TOUCH_IRQ = 36;
// LED RGB, ánodo común (BAJO = encendido)
constexpr int PIN_LED_R = 22;
constexpr int PIN_LED_G = 16;
constexpr int PIN_LED_B = 17;
// Enlace con la Mega (UART2 reasignado al conector I2C)
constexpr int PIN_LINK_TX = 25;
constexpr int PIN_LINK_RX = 32;
constexpr uint32_t LINK_BAUD = 57600;
```
