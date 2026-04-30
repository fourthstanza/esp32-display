#pragma once

#include <stdint.h>
#include "esp_lcd_panel_ops.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

typedef struct {
    uint32_t duty;
} bl_msg_t;

extern QueueHandle_t bl_queue;

esp_lcd_panel_handle_t display_startup(void);
uint16_t display_colorfix(uint16_t color);