#pragma once

#include "esp_lcd_panel_ops.h"
#include "esp_lcd_touch.h"

/**
 * Starts the LVGL port task. That task owns all LVGL calls: init, display registration,
 * 1 ms tick (esp_timer), flush to the panel, and lv_timer_handler(). Call after
 * display_startup().
 */

typedef struct {
    esp_lcd_panel_handle_t panel;
    esp_lcd_touch_handle_t touch;
} lvgl_contxt_t;

void lvgl_display_init(esp_lcd_panel_handle_t panel, esp_lcd_touch_handle_t touch);
void touch_isr_handler(void *arg);
