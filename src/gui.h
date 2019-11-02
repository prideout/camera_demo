#pragma once

#include <sokol/sokol_app.h>

typedef struct GuiImpl Gui;
struct App;

Gui* gui_create(struct App* app, int sidebar_width);
void gui_destroy(Gui* gui);
bool gui_handle(Gui* gui, const sapp_event* event);
void gui_draw(Gui* gui);
