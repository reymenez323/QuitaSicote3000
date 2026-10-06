// =============================================================================
//  HMI — Pantallas (LVGL 9, 320 × 240 en horizontal)
// =============================================================================
//  Orden de este archivo:
//    1. Textos            todos juntos, para corregirlos sin tocar las pantallas
//    2. Colores y medidas
//    3. Piezas comunes    etiqueta, botón grande, cabecera, diálogo de confirmación
//    4. Botones           qué hace cada uno al pulsarlo
//    5. Pantallas         una función build...() por pantalla
//    6. Selección         qué pantalla toca según el estado que informa el control
//
//  Criterios de uso con el dedo (PLAN.md §2): zonas táctiles de 56 px o más,
//  texto de 20 px o más, un toque por paso, sin gestos ni scroll.
// =============================================================================
#include "ui.h"

#include <lvgl.h>

using namespace qs;

// =============================================================================
//  1. Textos
// =============================================================================
//  PENDIENTE: van sin acentos porque las fuentes incluidas en LVGL sólo traen
//  ASCII. Al generar las fuentes con acentos (PLAN.md §2) se corrigen aquí.

const char* const TXT_APP = "QuitaSicote 3000";
const char* const TXT_BOOTING = "Iniciando...";
const char* const TXT_NO_LINK = "Sin comunicacion con el controlador";

const char* const TXT_DOOR_CLOSED = "Puerta cerrada";
const char* const TXT_DOOR_OPEN = "Puerta abierta";
const char* const TXT_DOOR_INVALID = "Revisa la puerta";
const char* const TXT_READY = "Equipo listo";
const char* const TXT_BEGIN = "EMPEZAR";

const char* const TXT_SHOE = "Calzado";
const char* const TXT_INTENSITY = "Intensidad";
const char* const TXT_DURATION = "Duracion";
const char* const TXT_CONFIRM = "Confirmar";
const char* const SHOE_NAMES[SHOE_COUNT] = {"Cuero", "Deportivo", "Bota", "Sintetico"};
const char* const INTENSITY_NAMES[INTENSITY_COUNT] = {"Suave", "Media", "Intensa"};
const char* const DURATION_NAMES[DURATION_COUNT] = {"Corta", "Media", "Larga"};

const char* const TXT_START = "INICIAR";
const char* const TXT_CLOSE_DOOR = "Cierra la puerta";
const char* const TXT_START_REJECTED = "No se pudo iniciar";

const char* const TXT_PREHEATING = "Calentando";
const char* const TXT_TREATING = "Tratando";
const char* const TXT_TEMPERATURE = "Temp.";
const char* const TXT_HUMIDITY = "Humedad";
const char* const TXT_ODOR = "Olor";
const char* const TXT_PAUSE = "PAUSAR";
const char* const TXT_CANCEL = "CANCELAR";

const char* const TXT_PAUSED_TITLE = "En pausa";
const char* const TXT_PAUSED_BY_USER = "Pausado";
const char* const TXT_PAUSE_COUNTDOWN = "Se cancela en %u:%02u";
const char* const TXT_RESUME = "REANUDAR";

const char* const TXT_CANCEL_QUESTION = "Cancelar el tratamiento?";
const char* const TXT_NO = "No";
const char* const TXT_YES_CANCEL = "Si, cancelar";

const char* const TXT_COOLING = "Enfriando";
const char* const TXT_COOLING_HINT = "Puedes abrir la puerta cuando termine";
const char* const TXT_DONE_TITLE = "Completo";
const char* const TXT_DONE = "Listo! Retira el calzado";
const char* const TXT_ACCEPT = "ACEPTAR";

const char* const TXT_FAULT_TITLE = "Falla";
const char* const TXT_FAULT_CODE = "Codigo %u";
const char* const TXT_TRIP_TITLE = "Proteccion activada";
const char* const TXT_TRIP_THERMAL = "Se detecto sobrecalentamiento. Revisa los ventiladores.";
const char* const TXT_TRIP_OTHER = "Proteccion de seguridad activada.";
const char* const TXT_TRIP_CAUSE = "Causa 0x%04X";
const char* const TXT_REARM = "REARMAR";
const char* const TXT_REARM_QUESTION = "Rearmar la proteccion?";
const char* const TXT_YES_REARM = "Si, rearmar";
const char* const TXT_LOCKED_TITLE = "Equipo bloqueado";
const char* const TXT_LOCKED = "Equipo bloqueado. Requiere servicio tecnico.";

// Mensaje en lenguaje llano para cada código de falla del control.
const char* faultMessage(uint8_t faultCode) {
  switch (faultCode) {
    case FAULT_TC1_INVALID:     return "Falla del sensor de temperatura de la recamara";
    case FAULT_TC2_INVALID:     return "Falla del sensor de temperatura del calefactor";
    case FAULT_PREHEAT_TIMEOUT: return "El equipo no alcanzo la temperatura. Revisa los ventiladores";
    case FAULT_SOFT_OVERTEMP:   return "Temperatura demasiado alta";
    case FAULT_SIS_NO_LINK:     return "Falla interna de comunicacion";
    case FAULT_SIS_TRIP:        return "Proteccion de seguridad activada";
    case FAULT_DOOR_INVALID:    return "Falla del sensor de la puerta";
    default:                    return "Falla del equipo";
  }
}

// =============================================================================
//  2. Colores y medidas
// =============================================================================

// Alto contraste: fondo claro, texto casi negro, blanco sólo sobre colores oscuros.
const lv_color_t COLOR_BACKGROUND = LV_COLOR_MAKE(0xFF, 0xFF, 0xFF);
const lv_color_t COLOR_SURFACE = LV_COLOR_MAKE(0xEE, 0xF1, 0xF5);   // cabecera y tarjetas
const lv_color_t COLOR_TEXT = LV_COLOR_MAKE(0x11, 0x14, 0x18);
const lv_color_t COLOR_TEXT_SOFT = LV_COLOR_MAKE(0x4A, 0x52, 0x60);
const lv_color_t COLOR_PRIMARY = LV_COLOR_MAKE(0x0B, 0x4F, 0xA8);   // opciones y navegación
const lv_color_t COLOR_GO = LV_COLOR_MAKE(0x1B, 0x6E, 0x35);        // iniciar, reanudar
const lv_color_t COLOR_DANGER = LV_COLOR_MAKE(0xB3, 0x26, 0x1E);    // cancelar, fallas
const lv_color_t COLOR_NEUTRAL = LV_COLOR_MAKE(0x4A, 0x52, 0x60);
const lv_color_t COLOR_DISABLED = LV_COLOR_MAKE(0xD5, 0xD9, 0xE0);
const lv_color_t COLOR_DISABLED_TEXT = LV_COLOR_MAKE(0x4E, 0x56, 0x62);
const lv_color_t COLOR_WHITE = LV_COLOR_MAKE(0xFF, 0xFF, 0xFF);

const lv_font_t* const FONT_SMALL = &lv_font_montserrat_14;
const lv_font_t* const FONT_BODY = &lv_font_montserrat_20;
const lv_font_t* const FONT_BUTTON = &lv_font_montserrat_24;
const lv_font_t* const FONT_BIG = &lv_font_montserrat_28;
const lv_font_t* const FONT_CLOCK = &lv_font_montserrat_48;

// Medidas en píxeles (1 px ≈ 0,2 mm)
constexpr int MARGIN = 8;
constexpr int GAP = 12;
constexpr int HEADER_H = 56;
constexpr int CONTENT_Y = HEADER_H + MARGIN;     // 64: donde empieza el contenido
constexpr int CONTENT_W = 320 - 2 * MARGIN;      // 304
constexpr int CONTENT_H = 240 - CONTENT_Y - MARGIN;  // 168
constexpr int BUTTON_H = 72;
constexpr int BUTTON_Y = 240 - MARGIN - BUTTON_H;    // 160: fila de botones de abajo
constexpr int HALF_W = (CONTENT_W - GAP) / 2;        // 146: dos botones lado a lado
constexpr int RIGHT_X = MARGIN + HALF_W + GAP;       // 166: x del botón derecho

// =============================================================================
//  Estado de la interfaz
// =============================================================================

enum class Screen : uint8_t {
  None,
  Boot, NoLink,
  Home, Shoe, Intensity, Duration, Summary,  // asistente de selección (estado local del HMI)
  Running, Paused, Cooling, Done,
  Fault, Trip, Locked,
};

static Screen currentScreen = Screen::None;
static Screen wizardStep = Screen::Home;  // en qué paso del asistente está el usuario
static int chosenShoe = -1;
static int chosenIntensity = -1;
static int chosenDuration = -1;
static uint16_t shownDetail = 0;          // código de falla o causa con que se dibujó la pantalla
static lv_obj_t* dialog = nullptr;

// Elementos de la pantalla actual cuyo texto cambia mientras se muestra.
static lv_obj_t* titleLabel = nullptr;
static lv_obj_t* line1Label = nullptr;
static lv_obj_t* line2Label = nullptr;
static lv_obj_t* clockLabel = nullptr;
static lv_obj_t* progressBar = nullptr;
static lv_obj_t* valueLabels[3] = {};
static lv_obj_t* doorButton = nullptr;    // botón que sólo funciona con la puerta cerrada

// =============================================================================
//  3. Piezas comunes
// =============================================================================

// Cambia el texto sólo si es distinto (evita redibujar sin necesidad).
static void setText(lv_obj_t* label, const char* text) {
  if (label != nullptr && strcmp(lv_label_get_text(label), text) != 0) lv_label_set_text(label, text);
}

// Etiqueta de texto. Con `width` > 0 el texto se ajusta a ese ancho en varias líneas.
static lv_obj_t* addLabel(lv_obj_t* parent, const char* text, const lv_font_t* font, lv_color_t color,
                          int x, int y, int width = 0, lv_text_align_t align = LV_TEXT_ALIGN_LEFT) {
  lv_obj_t* label = lv_label_create(parent);
  lv_label_set_text(label, text);
  lv_obj_set_style_text_font(label, font, 0);
  lv_obj_set_style_text_color(label, color, 0);
  lv_obj_set_pos(label, x, y);
  if (width > 0) {
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(label, width);
    lv_obj_set_style_text_align(label, align, 0);
  }
  return label;
}

// Rectángulo liso (cabecera, tarjeta, fondo del diálogo).
static lv_obj_t* addPanel(lv_obj_t* parent, int x, int y, int width, int height, lv_color_t color) {
  lv_obj_t* panel = lv_obj_create(parent);
  lv_obj_remove_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_pos(panel, x, y);
  lv_obj_set_size(panel, width, height);
  lv_obj_set_style_bg_color(panel, color, 0);
  lv_obj_set_style_border_width(panel, 0, 0);
  lv_obj_set_style_radius(panel, 0, 0);
  lv_obj_set_style_pad_all(panel, 0, 0);
  return panel;
}

// Botón grande. `onClick` se ejecuta al SOLTAR el dedo sobre el botón, así se
// puede corregir arrastrando fuera. `number` llega a onClick (ver clickedNumber).
static lv_obj_t* addButton(lv_obj_t* parent, const char* text, int x, int y, int width, int height,
                           lv_color_t color, lv_event_cb_t onClick, int number = 0,
                           const lv_font_t* font = FONT_BUTTON) {
  lv_obj_t* button = lv_button_create(parent);
  lv_obj_set_pos(button, x, y);
  lv_obj_set_size(button, width, height);
  lv_obj_set_style_radius(button, 8, 0);
  lv_obj_set_style_shadow_width(button, 0, 0);
  lv_obj_set_style_pad_all(button, 4, 0);
  lv_obj_set_style_bg_color(button, color, 0);
  lv_obj_set_style_bg_color(button, lv_color_darken(color, LV_OPA_40), LV_STATE_PRESSED);
  lv_obj_set_style_bg_color(button, COLOR_DISABLED, LV_STATE_DISABLED);
  lv_obj_set_style_bg_opa(button, LV_OPA_COVER, LV_STATE_DISABLED);
  lv_obj_set_style_text_color(button, COLOR_WHITE, 0);
  lv_obj_set_style_text_color(button, COLOR_DISABLED_TEXT, LV_STATE_DISABLED);
  lv_obj_add_event_cb(button, onClick, LV_EVENT_CLICKED, reinterpret_cast<void*>(static_cast<intptr_t>(number)));

  lv_obj_t* label = lv_label_create(button);
  lv_label_set_text(label, text);
  lv_obj_set_style_text_font(label, font, 0);
  lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
  lv_obj_set_width(label, width - 12);
  lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_center(label);
  return button;
}

// El número que se le dio al botón pulsado en addButton().
static int clickedNumber(lv_event_t* event) {
  return static_cast<int>(reinterpret_cast<intptr_t>(lv_event_get_user_data(event)));
}

// Botón que depende de la puerta: abierta, queda gris y dice "Cierra la puerta"
// (un botón deshabilitado siempre explica por qué).
static void showDoorButton(bool doorClosed, const char* normalText) {
  if (doorButton == nullptr) return;
  lv_obj_t* label = lv_obj_get_child(doorButton, 0);
  if (doorClosed) {
    lv_obj_remove_state(doorButton, LV_STATE_DISABLED);
    lv_obj_set_style_text_font(label, FONT_BUTTON, 0);
    setText(label, normalText);
  } else {
    lv_obj_add_state(doorButton, LV_STATE_DISABLED);
    lv_obj_set_style_text_font(label, FONT_BODY, 0);
    setText(label, TXT_CLOSE_DOOR);
  }
}

// Marca la opción elegida antes (se ve al volver con "Atrás").
static void markIfChosen(lv_obj_t* button, bool chosen) {
  if (!chosen) return;
  lv_obj_set_style_border_width(button, 5, 0);
  lv_obj_set_style_border_color(button, COLOR_TEXT, 0);
}

static void onBack(lv_event_t*);

// Cabecera de 56 px. "Atrás" va siempre arriba a la izquierda. En rojo para las fallas.
static void addHeader(lv_obj_t* screen, const char* title, bool withBack, bool red = false) {
  lv_obj_t* bar = addPanel(screen, 0, 0, 320, HEADER_H, red ? COLOR_DANGER : COLOR_SURFACE);
  int titleX = MARGIN;
  if (withBack) {
    addButton(bar, LV_SYMBOL_LEFT, 0, 0, 72, HEADER_H, COLOR_PRIMARY, onBack);
    titleX = 72 + GAP;
  }
  titleLabel = addLabel(bar, title, FONT_BUTTON, red ? COLOR_WHITE : COLOR_TEXT, titleX, 14);
}

static void closeDialog() {
  if (dialog == nullptr) return;
  lv_obj_delete_async(dialog);  // se borra al terminar el evento en curso
  dialog = nullptr;
}

static void onDialogNo(lv_event_t*) { closeDialog(); }

// Diálogo de confirmación, sólo para acciones con consecuencias (cancelar, rearmar).
static void openDialog(const char* question, const char* yesText, lv_event_cb_t onYes) {
  closeDialog();
  dialog = addPanel(lv_layer_top(), 0, 0, 320, 240, LV_COLOR_MAKE(0, 0, 0));
  lv_obj_set_style_bg_opa(dialog, LV_OPA_50, 0);  // oscurece la pantalla de atrás

  lv_obj_t* box = addPanel(dialog, 16, 32, 288, 176, COLOR_BACKGROUND);
  lv_obj_set_style_radius(box, 8, 0);
  addLabel(box, question, FONT_BUTTON, COLOR_TEXT, 12, 16, 264, LV_TEXT_ALIGN_CENTER);
  addButton(box, TXT_NO, 8, 104, 130, 64, COLOR_NEUTRAL, onDialogNo);
  addButton(box, yesText, 150, 104, 130, 64, COLOR_DANGER, onYes, 0, FONT_BODY);
}

// =============================================================================
//  4. Botones: qué hace cada uno
// =============================================================================
//  Un botón sólo cambia el paso del asistente o envía una solicitud al control.
//  El cambio de pantalla lo hace uiUpdate() en la siguiente vuelta.

static void onBack(lv_event_t*) {
  if (wizardStep == Screen::Shoe) wizardStep = Screen::Home;
  else if (wizardStep == Screen::Intensity) wizardStep = Screen::Shoe;
  else if (wizardStep == Screen::Duration) wizardStep = Screen::Intensity;
  else if (wizardStep == Screen::Summary) wizardStep = Screen::Duration;
}

static void onBegin(lv_event_t*) { wizardStep = Screen::Shoe; }

// Al elegir una opción se avanza solo al paso siguiente (un toque por paso).
static void onShoeChosen(lv_event_t* event) {
  chosenShoe = clickedNumber(event);
  wizardStep = Screen::Intensity;
}
static void onIntensityChosen(lv_event_t* event) {
  chosenIntensity = clickedNumber(event);
  wizardStep = Screen::Duration;
}
static void onDurationChosen(lv_event_t* event) {
  chosenDuration = clickedNumber(event);
  wizardStep = Screen::Summary;
}

static void onStart(lv_event_t*) { requestStart(chosenShoe, chosenIntensity, chosenDuration); }
static void onPause(lv_event_t*) { requestAction(MSG_REQ_PAUSE); }
static void onResume(lv_event_t*) { requestAction(MSG_REQ_RESUME); }
static void onAccept(lv_event_t*) { requestAction(MSG_REQ_ACK); }

static void onCancelConfirmed(lv_event_t*) {
  requestAction(MSG_REQ_CANCEL);
  closeDialog();
}
static void onCancel(lv_event_t*) { openDialog(TXT_CANCEL_QUESTION, TXT_YES_CANCEL, onCancelConfirmed); }

static void onRearmConfirmed(lv_event_t*) {
  requestAction(MSG_REQ_REARM);
  closeDialog();
}
static void onRearm(lv_event_t*) { openDialog(TXT_REARM_QUESTION, TXT_YES_REARM, onRearmConfirmed); }

// =============================================================================
//  5. Pantallas
// =============================================================================

// Pantalla de sólo texto: arranque, sin comunicación, bloqueado.
static void buildMessage(lv_obj_t* screen, const char* title, const char* message, bool red = false) {
  addHeader(screen, title, false, red);
  addLabel(screen, message, FONT_BUTTON, COLOR_TEXT, MARGIN, CONTENT_Y + 8, CONTENT_W);
}

// Inicio: estado de la puerta y un botón grande.
static void buildHome(lv_obj_t* screen) {
  addHeader(screen, TXT_APP, false);
  lv_obj_t* card = addPanel(screen, MARGIN, CONTENT_Y, CONTENT_W, 56, COLOR_SURFACE);
  lv_obj_set_style_radius(card, 8, 0);
  line1Label = addLabel(card, "", FONT_BODY, COLOR_TEXT, 10, 4);
  addLabel(card, TXT_READY, FONT_BODY, COLOR_TEXT_SOFT, 10, 28);
  addButton(screen, TXT_BEGIN, MARGIN, 132, CONTENT_W, 100, COLOR_PRIMARY, onBegin, 0, FONT_BIG);
}

// Calzado: cuadrícula de 2 × 2.
static void buildShoe(lv_obj_t* screen) {
  addHeader(screen, TXT_SHOE, true);
  const int cellH = (CONTENT_H - GAP) / 2;  // 78
  for (int i = 0; i < SHOE_COUNT; i++) {
    const int x = (i % 2 == 0) ? MARGIN : RIGHT_X;
    const int y = CONTENT_Y + (i / 2) * (cellH + GAP);
    lv_obj_t* button = addButton(screen, SHOE_NAMES[i], x, y, HALF_W, cellH, COLOR_PRIMARY, onShoeChosen, i);
    markIfChosen(button, chosenShoe == i);
  }
}

// Intensidad y duración: tres columnas (apiladas no llegarían a 56 px de alto).
static void buildThreeOptions(lv_obj_t* screen, const char* title, const char* const* names,
                              lv_event_cb_t onChosen, int chosen) {
  addHeader(screen, title, true);
  const int columnW = (CONTENT_W - 2 * GAP) / 3;  // 93
  for (int i = 0; i < 3; i++) {
    const int x = MARGIN + i * (columnW + GAP);
    lv_obj_t* button = addButton(screen, names[i], x, CONTENT_Y, columnW, CONTENT_H, COLOR_PRIMARY,
                                 onChosen, i, FONT_BODY);
    markIfChosen(button, chosen == i);
  }
}

// Confirmar: resumen de lo elegido y botón INICIAR.
static void buildSummary(lv_obj_t* screen) {
  addHeader(screen, TXT_CONFIRM, true);
  const char* names[3] = {TXT_SHOE, TXT_INTENSITY, TXT_DURATION};
  const char* values[3] = {SHOE_NAMES[chosenShoe], INTENSITY_NAMES[chosenIntensity],
                           DURATION_NAMES[chosenDuration]};
  for (int row = 0; row < 3; row++) {
    const int y = CONTENT_Y + row * 30;
    addLabel(screen, names[row], FONT_BODY, COLOR_TEXT_SOFT, MARGIN, y);
    addLabel(screen, values[row], FONT_BODY, COLOR_TEXT, 160, y, 152, LV_TEXT_ALIGN_RIGHT);
  }
  doorButton = addButton(screen, TXT_START, MARGIN, BUTTON_Y, CONTENT_W, BUTTON_H, COLOR_GO, onStart);
}

// En ciclo: tiempo restante en grande, barra de avance y tres lecturas.
static void buildRunning(lv_obj_t* screen) {
  addHeader(screen, TXT_PREHEATING, false);
  clockLabel = addLabel(screen, "", FONT_CLOCK, COLOR_TEXT, MARGIN, CONTENT_Y, 150, LV_TEXT_ALIGN_CENTER);

  progressBar = lv_bar_create(screen);
  lv_obj_set_pos(progressBar, MARGIN, 128);
  lv_obj_set_size(progressBar, 150, 16);
  lv_bar_set_range(progressBar, 0, 100);
  lv_obj_set_style_bg_color(progressBar, COLOR_PRIMARY, LV_PART_INDICATOR);

  const char* names[3] = {TXT_TEMPERATURE, TXT_HUMIDITY, TXT_ODOR};
  for (int row = 0; row < 3; row++) {
    const int y = CONTENT_Y + 4 + row * 28;
    addLabel(screen, names[row], FONT_BODY, COLOR_TEXT_SOFT, 170, y);
    valueLabels[row] = addLabel(screen, "", FONT_BODY, COLOR_TEXT, 232, y, 80, LV_TEXT_ALIGN_RIGHT);
  }
  addButton(screen, TXT_PAUSE, MARGIN, BUTTON_Y, HALF_W, BUTTON_H, COLOR_PRIMARY, onPause);
  addButton(screen, TXT_CANCEL, RIGHT_X, BUTTON_Y, HALF_W, BUTTON_H, COLOR_DANGER, onCancel);
}

// Pausa: motivo, cuenta atrás de 5 min, Reanudar y Cancelar.
static void buildPaused(lv_obj_t* screen) {
  addHeader(screen, TXT_PAUSED_TITLE, false);
  line1Label = addLabel(screen, "", FONT_BUTTON, COLOR_TEXT, MARGIN, CONTENT_Y + 4);
  line2Label = addLabel(screen, "", FONT_BODY, COLOR_TEXT_SOFT, MARGIN, CONTENT_Y + 40);
  doorButton = addButton(screen, TXT_RESUME, MARGIN, BUTTON_Y, HALF_W, BUTTON_H, COLOR_GO, onResume);
  addButton(screen, TXT_CANCEL, RIGHT_X, BUTTON_Y, HALF_W, BUTTON_H, COLOR_DANGER, onCancel);
}

static void buildCooling(lv_obj_t* screen) {
  addHeader(screen, TXT_COOLING, false);
  lv_obj_t* spinner = lv_spinner_create(screen);
  lv_obj_set_pos(spinner, 124, CONTENT_Y + 4);
  lv_obj_set_size(spinner, 72, 72);
  lv_obj_set_style_arc_color(spinner, COLOR_PRIMARY, LV_PART_INDICATOR);
  addLabel(screen, TXT_COOLING_HINT, FONT_BODY, COLOR_TEXT, MARGIN, 160, CONTENT_W, LV_TEXT_ALIGN_CENTER);
}

static void buildDone(lv_obj_t* screen) {
  addHeader(screen, TXT_DONE_TITLE, false);
  addLabel(screen, TXT_DONE, FONT_BUTTON, COLOR_TEXT, MARGIN, CONTENT_Y + 16, CONTENT_W, LV_TEXT_ALIGN_CENTER);
  addButton(screen, TXT_ACCEPT, MARGIN, BUTTON_Y, CONTENT_W, BUTTON_H, COLOR_GO, onAccept);
}

// Falla del control: mensaje llano, código pequeño y Aceptar.
static void buildFault(lv_obj_t* screen, uint8_t faultCode) {
  addHeader(screen, TXT_FAULT_TITLE, false, true);
  addLabel(screen, faultMessage(faultCode), FONT_BODY, COLOR_TEXT, MARGIN, CONTENT_Y, CONTENT_W);
  char code[24];
  snprintf(code, sizeof(code), TXT_FAULT_CODE, faultCode);
  addLabel(screen, code, FONT_SMALL, COLOR_TEXT_SOFT, MARGIN, 138);
  addButton(screen, TXT_ACCEPT, MARGIN, BUTTON_Y, CONTENT_W, BUTTON_H, COLOR_PRIMARY, onAccept);
}

// Disparo del SIS: mensaje y Rearmar (con confirmación).
static void buildTrip(lv_obj_t* screen, uint16_t tripMask) {
  addHeader(screen, TXT_TRIP_TITLE, false, true);
  const bool thermal = (tripMask & TRIP_THERMAL_MASK) != 0;
  addLabel(screen, thermal ? TXT_TRIP_THERMAL : TXT_TRIP_OTHER, FONT_BODY, COLOR_TEXT, MARGIN, CONTENT_Y,
           CONTENT_W);
  char cause[24];
  snprintf(cause, sizeof(cause), TXT_TRIP_CAUSE, tripMask);
  addLabel(screen, cause, FONT_SMALL, COLOR_TEXT_SOFT, MARGIN, 138);
  addButton(screen, TXT_REARM, MARGIN, BUTTON_Y, CONTENT_W, BUTTON_H, COLOR_DANGER, onRearm);
}

// =============================================================================
//  6. Selección de pantalla y refresco
// =============================================================================

// Qué pantalla corresponde a lo que informa el control.
static Screen screenFor(const UiModel& model) {
  if (!model.linkOk) return Screen::NoLink;

  const Status& status = model.status;
  switch (static_cast<CycleState>(status.cycle_state)) {
    case CycleState::SelfTest:  return Screen::Boot;
    case CycleState::Ready:     return wizardStep;  // el asistente sólo existe en LISTO
    case CycleState::Preheat:
    case CycleState::Treatment: return Screen::Running;
    case CycleState::Paused:    return Screen::Paused;
    case CycleState::Cooling:   return Screen::Cooling;
    case CycleState::Complete:  return Screen::Done;
    case CycleState::Fault:
      if (status.sis_state == static_cast<uint8_t>(SisState::Locked)) return Screen::Locked;
      if (status.sis_state == static_cast<uint8_t>(SisState::Tripped)) return Screen::Trip;
      return Screen::Fault;
  }
  return Screen::NoLink;
}

// Las pantallas de falla se redibujan si cambia su código o su causa.
static uint16_t detailFor(Screen screen, const Status& status) {
  if (screen == Screen::Fault) return status.fault_code;
  if (screen == Screen::Trip) return status.trip_mask;
  return 0;
}

// Borra la pantalla actual y construye la nueva (nunca hay dos vivas a la vez).
static void showScreen(Screen screen, const Status& status) {
  closeDialog();
  titleLabel = line1Label = line2Label = clockLabel = progressBar = doorButton = nullptr;
  valueLabels[0] = valueLabels[1] = valueLabels[2] = nullptr;

  lv_obj_t* previous = lv_screen_active();
  lv_obj_t* fresh = lv_obj_create(nullptr);
  lv_obj_remove_flag(fresh, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_color(fresh, COLOR_BACKGROUND, 0);
  lv_obj_set_style_pad_all(fresh, 0, 0);

  switch (screen) {
    case Screen::Home:      buildHome(fresh); break;
    case Screen::Shoe:      buildShoe(fresh); break;
    case Screen::Intensity: buildThreeOptions(fresh, TXT_INTENSITY, INTENSITY_NAMES, onIntensityChosen, chosenIntensity); break;
    case Screen::Duration:  buildThreeOptions(fresh, TXT_DURATION, DURATION_NAMES, onDurationChosen, chosenDuration); break;
    case Screen::Summary:   buildSummary(fresh); break;
    case Screen::Running:   buildRunning(fresh); break;
    case Screen::Paused:    buildPaused(fresh); break;
    case Screen::Cooling:   buildCooling(fresh); break;
    case Screen::Done:      buildDone(fresh); break;
    case Screen::Fault:     buildFault(fresh, status.fault_code); break;
    case Screen::Trip:      buildTrip(fresh, status.trip_mask); break;
    case Screen::Locked:    buildMessage(fresh, TXT_LOCKED_TITLE, TXT_LOCKED, true); break;
    case Screen::Boot:      buildMessage(fresh, TXT_APP, TXT_BOOTING); break;
    default:                buildMessage(fresh, TXT_APP, TXT_NO_LINK, true); break;
  }

  lv_screen_load(fresh);
  if (previous != nullptr) lv_obj_delete(previous);
  currentScreen = screen;
  shownDetail = detailFor(screen, status);
}

// Actualiza los textos que cambian mientras la pantalla está a la vista.
static void refreshTexts(const UiModel& model) {
  const Status& status = model.status;
  const bool doorClosed = status.door == static_cast<uint8_t>(Door::Closed);
  const bool doorOpen = status.door == static_cast<uint8_t>(Door::Open);
  char text[32];

  switch (currentScreen) {
    case Screen::Home:
      setText(line1Label, doorClosed ? TXT_DOOR_CLOSED : doorOpen ? TXT_DOOR_OPEN : TXT_DOOR_INVALID);
      break;

    case Screen::Summary:
      showDoorButton(doorClosed, TXT_START);
      if (model.startRejected) setText(titleLabel, TXT_START_REJECTED);
      break;

    case Screen::Running: {
      const bool preheating = status.cycle_state == static_cast<uint8_t>(CycleState::Preheat);
      setText(titleLabel, preheating ? TXT_PREHEATING : TXT_TREATING);

      snprintf(text, sizeof(text), "%u:%02u", status.remaining_s / 60, status.remaining_s % 60);
      setText(clockLabel, text);

      int percent = 0;
      if (status.total_s > 0) percent = (status.total_s - status.remaining_s) * 100 / status.total_s;
      if (lv_bar_get_value(progressBar) != percent) lv_bar_set_value(progressBar, percent, LV_ANIM_OFF);

      // Sin dato se muestra "--": la pantalla no inventa valores.
      if (status.t_chamber_x10 == TEMP_INVALID) strcpy(text, "--");
      else snprintf(text, sizeof(text), "%d \xC2\xB0""C", (status.t_chamber_x10 + 5) / 10);
      setText(valueLabels[0], text);

      if (status.rh_x10 == U16_INVALID) strcpy(text, "--");
      else snprintf(text, sizeof(text), "%u %%", (status.rh_x10 + 5) / 10);
      setText(valueLabels[1], text);

      const bool odorKnown = status.voc_index != U16_INVALID && !(status.warn_flags & WARN_VOC_LEARNING);
      if (!odorKnown) strcpy(text, "--");
      else snprintf(text, sizeof(text), "%u", status.voc_index);
      setText(valueLabels[2], text);
      break;
    }

    case Screen::Paused:
      setText(line1Label, doorClosed ? TXT_PAUSED_BY_USER : TXT_DOOR_OPEN);
      snprintf(text, sizeof(text), TXT_PAUSE_COUNTDOWN, status.pause_left_s / 60, status.pause_left_s % 60);
      setText(line2Label, text);
      showDoorButton(doorClosed, TXT_RESUME);
      break;

    default:
      break;
  }
}

void uiBegin() {
  UiModel nothingYet;
  showScreen(Screen::Boot, nothingYet.status);
}

void uiUpdate(const UiModel& model) {
  // El asistente es estado local del HMI y sólo vive mientras el control está en LISTO.
  const bool ready = model.linkOk && model.status.cycle_state == static_cast<uint8_t>(CycleState::Ready);
  if (!ready) {
    wizardStep = Screen::Home;
    chosenShoe = chosenIntensity = chosenDuration = -1;
  }

  const Screen wanted = screenFor(model);
  if (wanted != currentScreen || detailFor(wanted, model.status) != shownDetail) {
    showScreen(wanted, model.status);
  }
  refreshTexts(model);
}
