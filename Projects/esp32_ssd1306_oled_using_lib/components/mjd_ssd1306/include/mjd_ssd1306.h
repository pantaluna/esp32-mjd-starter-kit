/*
 * Component header file.
 */
#ifndef __MJD_SSD1306_H__
#define __MJD_SSD1306_H__

#ifdef __cplusplus
extern "C" {
#endif

// Includes
#include "u8g2_esp32_hal.h"
#include "u8g2.h"
#include "mjd.h"

/**
 * Device specifics (registers, commands, etc.)
 */

/**
 * Component Settings
 *
 */
#define MJD_SSD1306_I2C_ADDRESS_DEFAULT     (0x3C)       /*!< */
#define MJD_SSD1306_I2C_MASTER_NUM_DEFAULT  (I2C_NUM_0)  /*!< */
#define MJD_SSD1306_OLED_DIMENSION_DEFAULT  (MJD_SSD1306_OLED_DIMENSION_128x32)  /*!< */

#define MJD_SSD1306_FONT_ID        (u8g2_font_courR12_tf) /*!< u8g2_font_courR10_tf u8g2_font_courR12_tf Font and Line Height are correlated. */
#define MJD_SSD1306_Y_FIRST_LINE   (11) /*!< Y coordinate: top->down. Correlated to Font. 11 | 11 */
#define MJD_SSD1306_Y_LINE_SPACING (17) /*!< Correlated to Font. 18 |17 */

/**
 * Data structs
 *
 */
/*****
 * Classification: OLED Dimension
 *
 */
typedef enum {
    MJD_SSD1306_OLED_DIMENSION_128x32 = 0,
    MJD_SSD1306_OLED_DIMENSION_128x64 = 1,
} mjd_ssd1306_oled_dimension_t;

/*****
 * Classification: Line Nr
 *
 */
typedef enum {
    MJD_SSD1306_LINE_NR_1 = 1,
    MJD_SSD1306_LINE_NR_2 = 2,
    MJD_SSD1306_LINE_NR_3 = 3,
    MJD_SSD1306_LINE_NR_4 = 4,
} mjd_ssd1306_line_nr_t;

/*****
 * mjd_ssd1306_config_t
 *
 */
typedef struct {
    bool manage_i2c_driver;
        uint8_t i2c_slave_addr;
        i2c_port_t i2c_port_num;
        gpio_num_t i2c_scl_gpio_num;
        gpio_num_t i2c_sda_gpio_num;

        mjd_ssd1306_oled_dimension_t oled_dimension;
        uint8_t oled_flip_mode; /*!< 0: default, the screen is at the right of the pin row. 1: flip it (if you mounted the oled board the other way around). */

        u8g2_t _u8g2; /*!< Instance of the U8G2 component */
        uint8_t _y_first_line;   /*!< pixels, Y coordinate top->down. Depends on selected font */
        uint8_t _y_line_spacing; /*!< pixels, Y coordinate top->down. Depends on selected font */
} mjd_ssd1306_config_t;

#define MJD_SSD1306_CONFIG_DEFAULT() { \
    .manage_i2c_driver = true, \
    .i2c_slave_addr = MJD_SSD1306_I2C_ADDRESS_DEFAULT, \
    .i2c_port_num = MJD_SSD1306_I2C_MASTER_NUM_DEFAULT, \
    .i2c_scl_gpio_num = -1, \
    .i2c_sda_gpio_num = -1, \
    .oled_dimension = MJD_SSD1306_OLED_DIMENSION_DEFAULT, \
    .oled_flip_mode = 0, \
    ._y_first_line = 0, \
    ._y_line_spacing = 0, \
};

/*****
 * Function declarations
 */
esp_err_t mjd_ssd1306_cmd_clear_screen(mjd_ssd1306_config_t* param_ptr_config);
esp_err_t mjd_ssd1306_cmd_write_line(mjd_ssd1306_config_t* param_ptr_config, const mjd_ssd1306_line_nr_t param_line_nr, const char* param_ptr_text);
esp_err_t mjd_ssd1306_init(mjd_ssd1306_config_t* param_ptr_config);
esp_err_t mjd_ssd1306_deinit(mjd_ssd1306_config_t* param_ptr_config);

#ifdef __cplusplus
}
#endif

#endif /* __MJD_SSD1306_H__ */
