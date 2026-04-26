#pragma once

#include <stdint.h>
#include "esp_lcd_panel_ops.h"

esp_lcd_panel_handle_t display_startup(void);
uint16_t display_colorfix(uint16_t color);