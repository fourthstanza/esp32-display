#include "touch_panel.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_lcd_touch_cst816s.h"
#include "lvgl_display.h"

static volatile bool touch_event_pending = false;

bool touch_event_pending_get(void)
{
    return touch_event_pending;
}

void touch_event_pending_clear(void)
{
    touch_event_pending = false;
}

i2c_master_bus_handle_t i2c_init(void) 
{
    i2c_master_bus_handle_t i2c_handle = NULL;
    const i2c_master_bus_config_t i2c_config = {
        .i2c_port = TOUCH_I2C_NUM,
        .sda_io_num = PIN_NUM_TOUCH_SDA,
        .scl_io_num = PIN_NUM_TOUCH_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_config, &i2c_handle));
    return i2c_handle;
}

esp_lcd_touch_handle_t touch_panel_init(i2c_master_bus_handle_t i2c_handle) 
{
    const esp_lcd_touch_config_t tp_cfg = {
        .x_max = LCD_H_RES,
        .y_max = LCD_V_RES,
        .rst_gpio_num = PIN_NUM_TOUCH_RST,
        .int_gpio_num = PIN_NUM_TOUCH_INT,
        .levels = {
            .reset = 0,
            .interrupt = 0,
        },
        .flags = {
            .swap_xy = 0,
            .mirror_x = 0,
            .mirror_y = 1,
        },
    };

    esp_lcd_touch_handle_t touch_handle;
    esp_lcd_panel_io_handle_t tp_io_handle = NULL;
    esp_lcd_panel_io_i2c_config_t tp_io_config = ESP_LCD_TOUCH_IO_I2C_CST816S_CONFIG();
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i2c(i2c_handle, &tp_io_config, &tp_io_handle));
    ESP_ERROR_CHECK(esp_lcd_touch_new_i2c_cst816s(tp_io_handle, &tp_cfg, &touch_handle));
    return touch_handle;
}

void touch_interrupt_handler(void *arg)
{
    touch_event_pending = true;
}

void int_touch_interrupt_handler(void){
    
    gpio_config_t io_conf = {
        .pin_bit_mask = 1ULL << PIN_NUM_TOUCH_INT,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE,   
    };
    gpio_config(&io_conf);

    gpio_install_isr_service(0);

    gpio_isr_handler_add(PIN_NUM_TOUCH_INT, touch_interrupt_handler, NULL);
}

esp_lcd_touch_handle_t touch_panel_startup(void) 
{
    i2c_master_bus_handle_t i2c_handle = i2c_init();
    int_touch_interrupt_handler();
    return touch_panel_init(i2c_handle);
}

