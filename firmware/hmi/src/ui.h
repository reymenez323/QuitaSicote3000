// =============================================================================
//  HMI — Pantallas (interfaz entre ui.cpp y main.cpp)
// =============================================================================
#pragma once
#include "qs_protocol.h"

// Lo que las pantallas necesitan saber. Todo viene del control.
struct UiModel {
  bool linkOk = false;         // se recibe STATUS con normalidad
  bool startRejected = false;  // el control rechazó el último "Iniciar"
  qs::Status status = {};      // último STATUS recibido
};

// --- Lo que ofrece ui.cpp ---
void uiBegin();
// Elige la pantalla que toca y refresca sus textos. Llamar a menudo desde la
// tarea de la interfaz (nunca desde otra tarea: LVGL no lo permite).
void uiUpdate(const UiModel& model);

// --- Lo que ui.cpp necesita de main.cpp: enviar solicitudes al control ---
// Las pantallas sólo solicitan; el control decide.
void requestStart(uint8_t shoe, uint8_t intensity, uint8_t duration);
void requestAction(qs::MsgType action);  // MSG_REQ_PAUSE, _RESUME, _CANCEL, _ACK o _REARM
