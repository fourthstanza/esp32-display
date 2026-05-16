#define LCD_HOST               SPI2_HOST
#define LCD_H_RES              (240)
#define LCD_V_RES              (240)
#define LCD_BIT_PER_PIXEL      (16)

#define PIN_NUM_LCD_CS         (GPIO_NUM_9)
#define PIN_NUM_LCD_PCLK       (GPIO_NUM_10)
#define PIN_NUM_LCD_MOSI       (GPIO_NUM_11) 
#define PIN_NUM_LCD_RST        (GPIO_NUM_14)
#define PIN_NUM_LCD_DC         (GPIO_NUM_8)
#define PIN_NUM_LCD_BL         (GPIO_NUM_2)

#define PIN_NUM_TOUCH_SCL       (GPIO_NUM_7)
#define PIN_NUM_TOUCH_SDA       (GPIO_NUM_6)
#define PIN_NUM_TOUCH_INT       (GPIO_NUM_5)
#define PIN_NUM_TOUCH_RST       (GPIO_NUM_13)
#define TOUCH_I2C_NUM           (0)

#define RGB_ORDER              LCD_RGB_ELEMENT_ORDER_BGR