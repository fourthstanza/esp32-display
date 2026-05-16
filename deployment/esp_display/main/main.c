#include <stdio.h>
#include "esp_system.h"
#include "board.h"
#include "display.h"
#include "lvgl_display.h"
#include "touch_panel.h"
#include "esp_log.h"
#include "esp_sleep.h"

void app_main(void)
{
    printf("Booted, beginning program...\n");

    esp_lcd_panel_handle_t panel_handle = display_startup();
    esp_lcd_touch_handle_t touch_handle = touch_panel_startup();
    lvgl_display_init(panel_handle, touch_handle);
    esp_sleep_enable_ext0_wakeup(PIN_NUM_TOUCH_INT, 0);
    
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
