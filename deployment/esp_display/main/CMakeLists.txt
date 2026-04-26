idf_component_register(SRCS "lvgl_display.c" "display.c" "main.c"
                       INCLUDE_DIRS "."
                       PRIV_REQUIRES spi_flash driver esp_lcd esp_lcd_gc9a01 lvgl esp_timer)
