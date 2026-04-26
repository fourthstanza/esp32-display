#include "lvgl_display.h"
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_lcd_panel_ops.h"
#include "lvgl.h"
#include "board.h"
#include "display.h"

static const char *TAG = "lvgl";

/** full single frame buffer height. Reduce this if we are having memory issues */
#define LVGL_BUF_LINES          240
#define LVGL_BUF_PIXELS         (LCD_H_RES * LVGL_BUF_LINES)
#define LVGL_BUF_SIZE           (LVGL_BUF_PIXELS * 2)
#define LVGL_REFRESH_PERIOD_MS  30

static uint8_t s_lvgl_buf1[LVGL_BUF_SIZE] __attribute__((aligned(4)));
static esp_timer_handle_t s_lvgl_tick_timer;

static void lvgl_tick_cb(void *arg)
{
    (void)arg;
    lv_tick_inc(1);
}

static void lvgl_flush_cb(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    esp_lcd_panel_handle_t panel = (esp_lcd_panel_handle_t)lv_display_get_user_data(disp);
    const int32_t x1 = area->x1;
    const int32_t y1 = area->y1;
    const int32_t x2 = area->x2;
    const int32_t y2 = area->y2;
    const int w = (int)(x2 - x1 + 1);
    const int h = (int)(y2 - y1 + 1);
    uint16_t *src = (uint16_t *)px_map;
    static uint16_t line_buf[LCD_H_RES];
    
    // fixes incorrect bit order on gc901a
    lv_draw_sw_rgb565_swap(src, LCD_H_RES * LCD_V_RES);

    for (int row = 0; row < h; row++) {
        for (int col = 0; col < w; col++) {
            line_buf[col] = src[row * w + col];
        }
        esp_err_t err = esp_lcd_panel_draw_bitmap(panel, x1, y1 + row, x2 + 1, y1 + row + 1, line_buf);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "draw_bitmap failed: %s", esp_err_to_name(err));
        }
    }
    
    lv_display_flush_ready(disp);
}

static void lvgl_splashscreen(void)
{
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(label, &lv_font_montserrat_30, 0);
    lv_label_set_text(label, "Booting...");
    lv_obj_center(label);
}

/**
 * Run all LVGL calls in this task
 */
static void lvgl_port_task(void *arg)
{
    esp_lcd_panel_handle_t panel = (esp_lcd_panel_handle_t)arg;

    lv_init();

    lv_display_t *disp = lv_display_create(LCD_H_RES, LCD_V_RES);
    if (disp == NULL) {
        ESP_LOGE(TAG, "lv_display_create failed");
        vTaskDelete(NULL);
        return;
    }

    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
    lv_display_set_user_data(disp, panel);
    lv_display_set_flush_cb(disp, lvgl_flush_cb);
    lv_display_set_buffers(disp, s_lvgl_buf1, NULL, sizeof(s_lvgl_buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_default(disp);

    const esp_timer_create_args_t tick_args = {
        .callback = &lvgl_tick_cb,
        .name = "lvgl_tick",
    };
    ESP_ERROR_CHECK(esp_timer_create(&tick_args, &s_lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(s_lvgl_tick_timer, 1000));

    lvgl_splashscreen();

    ESP_LOGI(TAG, "LVGL running (%dx%d)", LCD_H_RES, LCD_V_RES);

    while (1) {
        uint32_t delay_ms = lv_timer_handler();
        if (delay_ms > 500) {
            delay_ms = 500;
        }
        if (delay_ms < 30) {
            delay_ms = 30;
        }
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }
}

void lvgl_display_init(esp_lcd_panel_handle_t panel)
{
    const BaseType_t ok = xTaskCreate(lvgl_port_task, "lvgl", 8192, panel, 5, NULL);
    if (ok != pdPASS) {
        ESP_LOGE(TAG, "xTaskCreate(lvgl) failed");
    }
}
