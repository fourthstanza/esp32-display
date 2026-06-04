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
#include "gif/gif.h"
#include "esp_sleep.h"
#include "touch_panel.h"

static const char *TAG = "lvgl";

/** full single frame buffer height. Reduce this if we are having memory issues */
#define LVGL_BUF_LINES          240
#define LVGL_BUF_PIXELS         (LCD_H_RES * LVGL_BUF_LINES)
#define LVGL_BUF_SIZE           (LVGL_BUF_PIXELS * 3)
#define LVGL_REFRESH_PERIOD_MS  30

static esp_timer_handle_t s_lvgl_tick_timer;
static uint8_t *buf1 = NULL;
static uint8_t *buf2 = NULL;

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
    lv_draw_sw_rgb565_swap(src, w * h);

    for (int row = 0; row < h; row++) {
        for (int col = 0; col < w; col++) {
            line_buf[col] = src[row * w + col];
        }
        ESP_ERROR_CHECK(esp_lcd_panel_draw_bitmap(panel, x1, y1 + row, x2 + 1, y1 + row + 1, line_buf));
    }
    
    lv_display_flush_ready(disp);
}

void bl_set(int y) {
    
    int dutyset = y * 255 / 200;
    if (dutyset > 255) {
        dutyset = 255;
    }

    bl_msg_t msg = {
        .duty = dutyset,
    };
    xQueueSend(bl_queue, &msg, portMAX_DELAY);
}

void sleep_set(int x) {
    if (x > 150) {
        ESP_LOGI(TAG, "Entering deep sleep mode");
        vTaskDelay(pdMS_TO_TICKS(1000)); // Give some time for the log to be printed
        esp_deep_sleep_start();
    }
}

static void touch_read_cb(lv_indev_t *indev, lv_indev_data_t *data)
{
    
    esp_lcd_touch_handle_t touch =
        (esp_lcd_touch_handle_t)lv_indev_get_user_data(indev);

    static lv_point_t last_point;

    esp_lcd_touch_point_data_t points[1];  
    uint8_t point_cnt = 0;
    if (touch_event_pending_get() || data->state == LV_INDEV_STATE_PRESSED) {
        touch_event_pending_clear();
        esp_lcd_touch_read_data(touch);
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
        data->point = last_point;
        return;
    }

    esp_err_t err = esp_lcd_touch_get_data(
        touch,
        points,
        &point_cnt,
        1
    );

    if (err == ESP_OK && point_cnt > 0) {
        last_point.x = points[0].x;
        last_point.y = points[0].y;

        data->state = LV_INDEV_STATE_PRESSED;
        ESP_LOGI(TAG, "Touch at (%d, %d)", last_point.x, last_point.y);
        bl_set(last_point.y);
        sleep_set(last_point.x);
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }

    // LVGL requires last known position always returned
    data->point = last_point;
}

static void lvgl_splashscreen(void)
{
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(label, &lv_font_montserrat_30, 0);
    lv_label_set_text(label, "Booting...");
    lv_obj_center(label);
}

static void lvgl_gif_open(void)
{
    LV_IMAGE_DECLARE(gif_bytes);
    lv_obj_t *img = lv_gif_create(lv_screen_active());
    lv_gif_set_color_format(img, LV_COLOR_FORMAT_RGB565);
    lv_gif_set_src(img, &gif_bytes);
    lv_obj_center(img);
}

/**
 * Run all LVGL calls in this task
 */
static void lvgl_port_task(void *arg)
{
    lvgl_contxt_t *ctx = (lvgl_contxt_t *)arg;

    esp_lcd_panel_handle_t panel = ctx->panel;
    esp_lcd_touch_handle_t touch = ctx->touch;
    free(ctx);

    lv_init();

    lv_display_t *disp = lv_display_create(LCD_H_RES, LCD_V_RES);
    if (disp == NULL) {
        ESP_LOGE(TAG, "lv_display_create failed");
        vTaskDelete(NULL);
        return;
    }

    buf1 = malloc(LVGL_BUF_SIZE);
    buf2 = malloc(LVGL_BUF_SIZE);
    if (buf1 == NULL || buf2 == NULL) {
        ESP_LOGE(TAG, "Failed to allocate buffer");
        free(buf1);
        free(buf2);
        vTaskDelete(NULL);
        return;
    }
    
    ESP_LOGE(TAG, "Free heap size: %d", esp_get_free_heap_size());
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
    lv_display_set_user_data(disp, panel);
    lv_display_set_flush_cb(disp, lvgl_flush_cb);
    lv_display_set_buffers(disp, buf1, buf2, LVGL_BUF_SIZE, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_default(disp);

    const esp_timer_create_args_t tick_args = {
        .callback = &lvgl_tick_cb,
        .name = "lvgl_tick",
    };
    ESP_ERROR_CHECK(esp_timer_create(&tick_args, &s_lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(s_lvgl_tick_timer, 1000));

    lv_indev_t * indev_drv = lv_indev_create();
    lv_indev_set_user_data(indev_drv, touch);
    lv_indev_set_type(indev_drv, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev_drv, touch_read_cb);

    lvgl_splashscreen();
    lv_timer_handler();

    vTaskDelay(pdMS_TO_TICKS(2000));

    ESP_LOGI(TAG, "LVGL running (%dx%d)", LCD_H_RES, LCD_V_RES);

    lvgl_gif_open();

    while (1) {
        uint32_t delay_ms = lv_timer_handler();
        if (delay_ms > 300) {
            delay_ms = 300;
        }
        if (delay_ms < 30) {
            delay_ms = 30;
        }
        vTaskDelay(pdMS_TO_TICKS(delay_ms));
    }
}

void lvgl_display_init(esp_lcd_panel_handle_t panel, esp_lcd_touch_handle_t touch)
{

    lvgl_contxt_t *ctx = malloc(sizeof(lvgl_contxt_t));
    ctx->panel = panel;
    ctx->touch = touch;

    const BaseType_t ok = xTaskCreate(lvgl_port_task, "lvgl", 8192, ctx, 5, NULL);
    if (ok != pdPASS) {
        ESP_LOGE(TAG, "xTaskCreate(lvgl) failed");
    }
}
