idf_component_register(SRCS "lvgl_display.c" "display.c" "main.c" "gif/lusctalk_one_to_one.c" "gif/array.c" "touch_panel.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES spi_flash driver esp_lcd esp_lcd_gc9a01 lvgl esp_timer esp_driver_ledc)
