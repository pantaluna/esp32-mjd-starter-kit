/*
 * Component main file.
 */

// Component header file(s)
#include "mjd.h"
#include "mjd_ssd1306.h"

/*
 * Logging
 */
static const char TAG[] = "mjd_ssd1306";

/*
 * MAIN
 */

/*********************************************************************************
 * _log_config()
 *
 * @doc mjd_ssd1306_config_t
 *
 *********************************************************************************/
static esp_err_t _log_config(const mjd_ssd1306_config_t* param_ptr_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    ESP_LOGD(TAG, "LOG instance of mjd_ssd1306_config_t");

    ESP_LOGD(TAG, "  i2c_slave_addr:        0x%02X (%u)", param_ptr_config->i2c_slave_addr,
            param_ptr_config->i2c_slave_addr);
    ESP_LOGD(TAG, "  i2c_scl_gpio_num:      %u", param_ptr_config->i2c_scl_gpio_num);
    ESP_LOGD(TAG, "  i2c_sda_gpio_num:      %u", param_ptr_config->i2c_sda_gpio_num);

    return f_retval;
}

/*********************************************************************************
 * @doc mjd_ssd1306.h
 *
 * @param
 *
 *********************************************************************************/
esp_err_t mjd_ssd1306_cmd_clear_screen(mjd_ssd1306_config_t* param_ptr_config) {

    esp_err_t f_retval = ESP_OK;

    /*
     * Main
     */
    u8g2_ClearBuffer(&param_ptr_config->u8g2);
    u8g2_SendBuffer(&param_ptr_config->u8g2);

    return f_retval;
}

/*********************************************************************************
 * @doc mjd_ssd1306.h
 *
 * @param
 *
 *********************************************************************************/
esp_err_t mjd_ssd1306_cmd_write_line(mjd_ssd1306_config_t* param_ptr_config, const mjd_ssd1306_line_nr_t param_line_nr,
                                  const char* param_ptr_text) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    /*
     * Validate params
     */
    if (param_line_nr < MJD_SSD1306_LINE_NR_1 || param_line_nr > MJD_SSD1306_LINE_NR_4) {
        f_retval = ESP_ERR_INVALID_ARG;
        ESP_LOGE(TAG, "%s(). ABORT. invalid input param value | err %i (%s)", __FUNCTION__, f_retval,
                esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /*
     * Main
     */
    if (param_line_nr == MJD_SSD1306_LINE_NR_1) {
        u8g2_ClearBuffer(&param_ptr_config->u8g2);
    }
    u8g2_SetFont(&param_ptr_config->u8g2, MJD_SSD1306_FONT_ID);
    u8g2_DrawStr(&param_ptr_config->u8g2, 0, MJD_SSD1306_Y_FIRST_LINE + (param_line_nr-1) * MJD_SSD1306_Y_LINE_SPACING, param_ptr_text);
    u8g2_SendBuffer(&param_ptr_config->u8g2);

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * mjd_ssd1306_init()
 *
 * @important This init includes running
 *     a) The Cmd Stop Continuous Measurement. That might have been active from an earlier run. Disable it to save power.
 *     b) The Cmd SoftReset after installing the ESP32 I2C Driver, else the subsequent I2C actions will sometimes NOT WORK properly.
 *
 *********************************************************************************/
esp_err_t mjd_ssd1306_init(mjd_ssd1306_config_t* param_ptr_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    /*
     * Validate params
     *
     */
    if (param_ptr_config->i2c_scl_gpio_num == -1 || param_ptr_config->i2c_sda_gpio_num == -1) {
        f_retval = ESP_FAIL;
        ESP_LOGE(TAG, "%s(). ABORT. i2c_scl_gpio_num or i2c_sda_gpio_num is not initialized | err %i (%s)", __FUNCTION__,
                f_retval,
                esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /*
     * MAIN
     */
    u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
    u8g2_esp32_hal.manage_i2c_driver = param_ptr_config->manage_i2c_driver; // ***Added new property***
    u8g2_esp32_hal.i2c_port_num = param_ptr_config->i2c_port_num; // ***Added new property***
    u8g2_esp32_hal.scl = param_ptr_config->i2c_scl_gpio_num;
    u8g2_esp32_hal.sda = param_ptr_config->i2c_sda_gpio_num;
    u8g2_esp32_hal_init(u8g2_esp32_hal);

    if (param_ptr_config->oled_dimension == MJD_SSD1306_OLED_DIMENSION_128x32) {
        u8g2_Setup_ssd1306_i2c_128x32_univision_f(
                &param_ptr_config->u8g2,
                U8G2_R0,
                u8g2_esp32_i2c_byte_cb,
                u8g2_esp32_gpio_and_delay_cb);
    } else if (param_ptr_config->oled_dimension == MJD_SSD1306_OLED_DIMENSION_128x64) {
        u8g2_Setup_ssd1306_i2c_128x64_noname_f(
                &param_ptr_config->u8g2,
                U8G2_R0,
                u8g2_esp32_i2c_byte_cb,
                u8g2_esp32_gpio_and_delay_cb);
    }

    u8x8_SetI2CAddress(&param_ptr_config->u8g2.u8x8, (param_ptr_config->i2c_slave_addr << 1) | I2C_MASTER_WRITE); // 0x3C => 0x78
    u8g2_InitDisplay(&param_ptr_config->u8g2); // send init sequence to the display, display is in sleep mode after this
    u8g2_SetPowerSave(&param_ptr_config->u8g2, 0); // wake up display

    /*
     * Logging
     */
    f_retval = _log_config(param_ptr_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). _log_config() err %i %s", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /*
     * Extra props and commands
     */
    f_retval = mjd_ssd1306_cmd_clear_screen(param_ptr_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_ssd1306_cmd_clear_screen() err %i %s", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // DEVTEMP
    /////mjd_rtos_wait_forever();

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * mjd_ssd1306_deinit()
 *
 *********************************************************************************/
esp_err_t mjd_ssd1306_deinit(mjd_ssd1306_config_t* param_ptr_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    /*
     * NOP so far
     */
    return f_retval;
}
