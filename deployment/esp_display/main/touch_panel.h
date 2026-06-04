#pragma once

#include "board.h"
#include "esp_lcd_touch.h"

esp_lcd_touch_handle_t touch_panel_startup(void);

bool touch_event_pending_get(void);
void touch_event_pending_clear(void);