// =============================================================================
//  HMI — Pines y configuración de la pantalla (placa ESP32-32E 3.2", E32R32P)
// =============================================================================
//  La pantalla (ST7789) y el táctil resistivo (XPT2046) comparten el bus SPI.
//  Los pines son fijos de la placa: docs/pinout/esp32-hmi.md
//
//  A comprobar en la primera puesta en marcha (PLAN.md §1):
//    · colores correctos (si no: cambiar `invert` o `rgb_order`),
//    · orientación (si queda al revés: SCREEN_ROTATION = 3).
// =============================================================================
#pragma once
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

// Pantalla y táctil
constexpr int PIN_LCD_CS = 15;
constexpr int PIN_LCD_DC = 2;
constexpr int PIN_SPI_SCLK = 14;
constexpr int PIN_SPI_MOSI = 13;
constexpr int PIN_SPI_MISO = 12;
constexpr int PIN_LCD_BACKLIGHT = 27;  // ALTO = encendida
constexpr int PIN_TOUCH_CS = 33;
constexpr int PIN_TOUCH_IRQ = 36;

// LED RGB de la placa (BAJO = encendido). No se usa: se deja apagado.
constexpr int PIN_LED_R = 22;
constexpr int PIN_LED_G = 16;
constexpr int PIN_LED_B = 17;

// Enlace con el control: UART2 en el conector "I2C" de la placa
constexpr int PIN_LINK_TX = 25;  // -> control GPIO35
constexpr int PIN_LINK_RX = 32;  // <- control GPIO4

// Pantalla en horizontal
constexpr int SCREEN_W = 320;
constexpr int SCREEN_H = 240;
constexpr int SCREEN_ROTATION = 1;  // 1 o 3, según cómo quede montada

class Display : public lgfx::LGFX_Device {
 public:
  Display() {
    auto bus = bus_.config();
    bus.spi_host = HSPI_HOST;
    bus.freq_write = 40000000;
    bus.freq_read = 16000000;
    bus.pin_sclk = PIN_SPI_SCLK;
    bus.pin_mosi = PIN_SPI_MOSI;
    bus.pin_miso = PIN_SPI_MISO;
    bus.pin_dc = PIN_LCD_DC;
    bus_.config(bus);
    panel_.setBus(&bus_);

    auto panel = panel_.config();
    panel.pin_cs = PIN_LCD_CS;
    panel.pin_rst = -1;  // el reset de la pantalla va unido al del ESP32
    panel.panel_width = 240;
    panel.panel_height = 320;
    panel.invert = true;
    panel.rgb_order = false;
    panel.bus_shared = true;  // el táctil usa el mismo bus
    panel_.config(panel);

    auto light = light_.config();
    light.pin_bl = PIN_LCD_BACKLIGHT;
    light.freq = 12000;
    light.pwm_channel = 7;
    light_.config(light);
    panel_.setLight(&light_);

    auto touch = touch_.config();
    touch.spi_host = HSPI_HOST;
    touch.freq = 1000000;
    touch.pin_sclk = PIN_SPI_SCLK;
    touch.pin_mosi = PIN_SPI_MOSI;
    touch.pin_miso = PIN_SPI_MISO;
    touch.pin_cs = PIN_TOUCH_CS;
    touch.pin_int = PIN_TOUCH_IRQ;
    touch.bus_shared = true;
    touch.x_min = 300;  // valores de partida; la calibración los corrige
    touch.x_max = 3900;
    touch.y_min = 200;
    touch.y_max = 3700;
    touch_.config(touch);
    panel_.setTouch(&touch_);

    setPanel(&panel_);
  }

 private:
  lgfx::Panel_ST7789 panel_;
  lgfx::Bus_SPI bus_;
  lgfx::Light_PWM light_;
  lgfx::Touch_XPT2046 touch_;
};
