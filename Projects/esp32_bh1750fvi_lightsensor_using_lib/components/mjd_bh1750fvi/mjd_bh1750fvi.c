/*
 * GY-032 BH1750FVI Digital Light Intensity Sensor  | I2C protocol | Using the ESP-IDF I2C driver.
 *
 */

// Component header file(s)
#include "mjd.h"
#include "mjd_bh1750fvi.h"

/*
 * Logging
 */
static const char TAG[] = "mjd_bh1750fvi";

/*
 * MAIN
 */

/*********************************************************************************
 * PUBLIC.
 * BH1750FVI: init
 *********************************************************************************/
esp_err_t mjd_bh1750fvi_init(const mjd_bh1750fvi_config_t* config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    i2c_cmd_handle_t cmd;

    if (config->manage_i2c_driver == true) {
        // Config
        i2c_config_t i2c_conf = { 0 };
        i2c_conf.mode = I2C_MODE_MASTER;
        i2c_conf.scl_io_num = config->i2c_scl_gpio_num;
        i2c_conf.sda_io_num = config->i2c_sda_gpio_num;
        i2c_conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
        i2c_conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
        i2c_conf.master.clk_speed = BH1750FVI_I2C_MASTER_FREQ_HZ;

        f_retval = i2c_param_config(config->i2c_port_num, &i2c_conf);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "ABORT. i2c_param_config() error (%i)", f_retval);
            return f_retval; // EXIT
        }

        f_retval = i2c_driver_install(config->i2c_port_num, I2C_MODE_MASTER, BH1750FVI_I2C_MASTER_RX_BUF_DISABLE,
        BH1750FVI_I2C_MASTER_TX_BUF_DISABLE, BH1750FVI_I2C_MASTER_INTR_FLAG_NONE);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "ABORT. i2c_driver_install() error (%i)", f_retval);
            return f_retval; // EXIT
        }
    }

    // Verify that the I2C slave is working properly
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (config->i2c_slave_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_stop(cmd);

    f_retval = i2c_master_cmd_begin(config->i2c_port_num, cmd, RTOS_DELAY_1SEC);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "ABORT. i2c_master_cmd_begin() I2C slave is NOT working properly error (%i)", f_retval);
        return f_retval; // EXIT
    }
    i2c_cmd_link_delete(cmd);

    return f_retval;
}

/*********************************************************************************
 * PUBLIC.
 * BH1750FVI: DE-init
 *********************************************************************************/
esp_err_t mjd_bh1750fvi_deinit(const mjd_bh1750fvi_config_t* config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    /*
     * I2C Driver
     */
    if (config->manage_i2c_driver == true) {
        f_retval = i2c_driver_delete(config->i2c_port_num);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "ABORT. i2c_driver_delete() error (%i)", f_retval);
        }
    }

    return f_retval;
}

/*********************************************************************************
 * PUBLIC.
 * Read the BH1750FVI data
 *********************************************************************************/
esp_err_t mjd_bh1750fvi_read(const mjd_bh1750fvi_config_t* config, mjd_bh1750fvi_data_t* data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    i2c_cmd_handle_t cmd;
    uint8_t delay_ms_before_reading_data = 0;
    uint8_t msb;
    uint8_t lsb;

    /*
     * Compute ratios depending on MODE: delay, divider
     */
    uint8_t extra_margin_for_delay = 100;

    switch (config->bh1750fvi_mode) {
    case BH1750FVI_REGISTER_CONTINUOUS_HIGH_RES_MODE:
        delay_ms_before_reading_data = 120 + extra_margin_for_delay;
        break;
    case BH1750FVI_REGISTER_CONTINUOUS_HIGH_RES_MODE_2:
        delay_ms_before_reading_data = 120 + extra_margin_for_delay;
        break;
    case BH1750FVI_REGISTER_CONTINUOUS_LOW_RES_MODE:
        delay_ms_before_reading_data = 16 + extra_margin_for_delay;
        break;
    case BH1750FVI_REGISTER_ONE_TIME_HIGH_RES_MODE:
        delay_ms_before_reading_data = 120 + extra_margin_for_delay;
        break;
    case BH1750FVI_REGISTER_ONE_TIME_HIGH_RES_MODE_2:
        delay_ms_before_reading_data = 120 + extra_margin_for_delay;
        break;
    case BH1750FVI_REGISTER_ONE_TIME_LOW_RES_MODE:
        delay_ms_before_reading_data = 16 + extra_margin_for_delay;
        break;
    }

    /*
     * Reset receive values
     */
    data->light_intensity_lux = 0.0;

    // Send request
    ESP_LOGD(TAG, "Send request");
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (config->i2c_slave_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, config->bh1750fvi_mode, true);
    i2c_master_stop(cmd);
    f_retval = i2c_master_cmd_begin(config->i2c_port_num, cmd, RTOS_DELAY_1SEC);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "ABORT. Send request i2c_master_cmd_begin() error (%i)", f_retval);
        i2c_cmd_link_delete(cmd); // CLEANUP
        return f_retval; // EXIT
    }
    i2c_cmd_link_delete(cmd);

    // The maximum Measurement Time depends on the resolution mode of the sensor.
    vTaskDelay(delay_ms_before_reading_data / portTICK_PERIOD_MS);

    // Receive response
    ESP_LOGD(TAG, "Receive response");
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (config->i2c_slave_addr << 1) | I2C_MASTER_READ, true);

    i2c_master_read_byte(cmd, &msb, I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, &lsb, I2C_MASTER_NACK); // last read must be ACK_CHECK_DISABLED
    i2c_master_stop(cmd);
    f_retval = i2c_master_cmd_begin(config->i2c_port_num, cmd, RTOS_DELAY_1SEC);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "ABORT.  i2c_master_cmd_begin() error (%i)", f_retval);
        i2c_cmd_link_delete(cmd); // CLEANUP
        return f_retval; // EXIT
    }
    i2c_cmd_link_delete(cmd);

    // Process response
    ESP_LOGD(TAG, "  msb << 8: %hu | msb: %hhu | lsb: %hhu", msb << 8, msb, lsb);
    data->light_intensity_lux = ((msb << 8) | lsb) / 1.2;

    return ESP_OK;
}

