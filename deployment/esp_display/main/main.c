#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "board.h"
#include "display.h"
#include "lvgl_display.h"

void app_main(void)
{
    printf("LVGL display demo starting...\n");

    esp_lcd_panel_handle_t panel_handle = display_startup();
    lvgl_display_init(panel_handle);
    
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
