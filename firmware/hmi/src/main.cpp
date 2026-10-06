// =============================================================================
//  QuitaSicote3000 — HMI (pantalla táctil) · ESP32-32E 3.2"
// =============================================================================
//  La pantalla NO decide nada del proceso ni de la seguridad (ADR-0005):
//    · muestra lo que el control informa en el mensaje STATUS,
//    · envía solicitudes (iniciar, pausar, cancelar…) que el control acepta o no.
//  Nunca envía temperaturas ni minutos: sólo números de opción.
//
//  Dos tareas de FreeRTOS:
//
//    taskUi     cada 5 ms    dibuja con LVGL y lee el táctil   (pantallas: ui.cpp)
//    taskLink   cada 10 ms   recibe STATUS, envía el latido y las solicitudes
//
//  Se comunican así:
//    taskLink -> taskUi : la variable `model`, protegida por un mutex
//    taskUi -> taskLink : la cola `requestQueue` (un botón pulsado = una solicitud)
// =============================================================================
#include <Arduino.h>
#include <Preferences.h>
#include <lvgl.h>

#include "display.h"
#include "qs_protocol.h"
#include "ui.h"

using namespace qs;

// =============================================================================
//  Comunicación entre las dos tareas
// =============================================================================

SemaphoreHandle_t modelMutex;
UiModel model;                 // lo último que informó el control
uint32_t lastStatusMs = 0;
bool statusSeen = false;

// Una solicitud pendiente de enviar al control.
struct Request {
  MsgType type;
  ReqStart start;  // sólo se usa con MSG_REQ_START
};
QueueHandle_t requestQueue;

// Las llaman los botones de ui.cpp (desde taskUi). Sólo encolan: quien usa el
// puerto serie es taskLink.
void requestStart(uint8_t shoe, uint8_t intensity, uint8_t duration) {
  Request request = {MSG_REQ_START, {shoe, intensity, duration}};
  xQueueSend(requestQueue, &request, 0);
}

void requestAction(MsgType action) {
  Request request = {action, {}};
  xQueueSend(requestQueue, &request, 0);
}

// =============================================================================
//  Tarea del enlace con el control
// =============================================================================

HardwareSerial& controlPort = Serial2;

void handleFrame(const Frame& frame) {
  xSemaphoreTake(modelMutex, portMAX_DELAY);
  if (frame.type == MSG_STATUS) {
    Status status;
    if (frame.as(status) && status.proto_ver == PROTO_VERSION) {
      model.status = status;
      statusSeen = true;
      lastStatusMs = millis();
    }
  } else if (frame.type == MSG_RESP_START) {
    RespStart response;
    if (frame.as(response)) model.startRejected = response.result != START_OK;
  }
  xSemaphoreGive(modelMutex);
}

void taskLink(void*) {
  FrameReader reader;
  Frame frame;
  uint32_t lastHeartbeatMs = 0;

  for (;;) {
    // 1. Lo que llega del control.
    while (reader.read(controlPort, frame)) handleFrame(frame);

    // 2. Si STATUS deja de llegar 2 s, la pantalla muestra "Sin comunicación".
    xSemaphoreTake(modelMutex, portMAX_DELAY);
    model.linkOk = statusSeen && millis() - lastStatusMs <= STATUS_TIMEOUT_MS;
    xSemaphoreGive(modelMutex);

    // 3. Latido cada 500 ms: si el control deja de recibirlo 10 s, cancela el ciclo.
    if (millis() - lastHeartbeatMs >= HMI_HB_PERIOD_MS) {
      lastHeartbeatMs = millis();
      HmiHb heartbeat = {PROTO_VERSION, 0};
      sendMessage(controlPort, MSG_HMI_HB, heartbeat);
    }

    // 4. Solicitudes de los botones.
    Request request;
    while (xQueueReceive(requestQueue, &request, 0) == pdTRUE) {
      if (request.type == MSG_REQ_START) {
        xSemaphoreTake(modelMutex, portMAX_DELAY);
        model.startRejected = false;  // se olvida el rechazo anterior
        xSemaphoreGive(modelMutex);
        sendMessage(controlPort, MSG_REQ_START, request.start);
      } else {
        sendFrame(controlPort, request.type);  // las demás no llevan datos
      }
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// =============================================================================
//  Pantalla y táctil (unión de LVGL con LovyanGFX)
// =============================================================================

Display lcd;

// LVGL entrega un trozo de imagen ya dibujado; aquí se envía a la pantalla.
void flushToScreen(lv_display_t* display, const lv_area_t* area, uint8_t* pixels) {
  const int width = lv_area_get_width(area);
  const int height = lv_area_get_height(area);
  lcd.startWrite();
  lcd.setAddrWindow(area->x1, area->y1, width, height);
  lcd.writePixels(reinterpret_cast<lgfx::rgb565_t*>(pixels), width * height);
  lcd.endWrite();
  lv_display_flush_ready(display);
}

// LVGL pregunta dónde está el dedo.
void readTouch(lv_indev_t*, lv_indev_data_t* data) {
  uint16_t x = 0, y = 0;
  if (lcd.getTouch(&x, &y)) {
    data->point.x = x;
    data->point.y = y;
    data->state = LV_INDEV_STATE_PRESSED;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

uint32_t lvglTick() { return millis(); }

// Calibración del táctil (tocar las 4 esquinas). Se hace en el primer arranque
// y se guarda; para repetirla, mantener el dedo en la pantalla 3 s al encender.
void calibrateTouch() {
  Preferences nvs;
  nvs.begin("hmi", false);
  uint16_t calibration[8];
  bool saved = nvs.getBytes("touch", calibration, sizeof(calibration)) == sizeof(calibration);

  if (saved) {
    uint16_t x, y;
    const uint32_t start = millis();
    bool fingerDown = lcd.getTouch(&x, &y);
    while (fingerDown && millis() - start < 3000) {
      delay(20);
      fingerDown = lcd.getTouch(&x, &y);
    }
    if (fingerDown) saved = false;  // dedo mantenido 3 s: calibrar de nuevo
  }

  if (saved) {
    lcd.setTouchCalibrate(calibration);
  } else {
    lcd.fillScreen(TFT_WHITE);
    lcd.setTextColor(TFT_BLACK);
    lcd.setTextDatum(lgfx::middle_center);
    lcd.setFont(&lgfx::fonts::FreeSans9pt7b);
    lcd.drawString("Toca las esquinas marcadas", SCREEN_W / 2, SCREEN_H / 2);
    lcd.calibrateTouch(calibration, TFT_BLACK, TFT_WHITE, 24);
    nvs.putBytes("touch", calibration, sizeof(calibration));
    lcd.fillScreen(TFT_WHITE);
  }
  nvs.end();
}

void startDisplayAndLvgl() {
  lcd.init();
  lcd.setRotation(SCREEN_ROTATION);
  lcd.setBrightness(255);
  lcd.fillScreen(TFT_WHITE);
  calibrateTouch();

  lv_init();
  lv_tick_set_cb(lvglTick);

  // Sin PSRAM no cabe la pantalla entera en memoria: LVGL dibuja por franjas
  // de 30 líneas, con dos búferes.
  constexpr size_t BUFFER_BYTES = SCREEN_W * 30 * 2;
  void* buffer1 = heap_caps_malloc(BUFFER_BYTES, MALLOC_CAP_DMA);
  void* buffer2 = heap_caps_malloc(BUFFER_BYTES, MALLOC_CAP_DMA);
  lv_display_t* display = lv_display_create(SCREEN_W, SCREEN_H);
  lv_display_set_flush_cb(display, flushToScreen);
  lv_display_set_buffers(display, buffer1, buffer2, BUFFER_BYTES, LV_DISPLAY_RENDER_MODE_PARTIAL);

  lv_indev_t* touch = lv_indev_create();
  lv_indev_set_type(touch, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(touch, readTouch);
}

// =============================================================================
//  Tarea de la interfaz
// =============================================================================
//  Todo lo que toca LVGL ocurre en esta tarea (LVGL no admite llamadas desde
//  varias tareas a la vez).

void taskUi(void*) {
  startDisplayAndLvgl();
  uiBegin();

  for (;;) {
    xSemaphoreTake(modelMutex, portMAX_DELAY);
    const UiModel snapshot = model;  // copia: se dibuja sin tener el mutex tomado
    xSemaphoreGive(modelMutex);

    uiUpdate(snapshot);
    lv_timer_handler();
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}

// =============================================================================
//  Arranque
// =============================================================================

void setup() {
  for (int pin : {PIN_LED_R, PIN_LED_G, PIN_LED_B}) {  // LED RGB apagado
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH);
  }

  Serial.begin(115200);  // USB: sólo depuración
  controlPort.begin(LINK_BAUD, SERIAL_8N1, PIN_LINK_RX, PIN_LINK_TX);

  modelMutex = xSemaphoreCreateMutex();
  requestQueue = xQueueCreate(8, sizeof(Request));

  xTaskCreatePinnedToCore(taskUi, "ui", 8192, nullptr, 2, nullptr, 1);
  xTaskCreatePinnedToCore(taskLink, "link", 4096, nullptr, 3, nullptr, 1);
}

void loop() {
  vTaskDelete(nullptr);  // todo ocurre en las tareas: loop() no se usa
}
