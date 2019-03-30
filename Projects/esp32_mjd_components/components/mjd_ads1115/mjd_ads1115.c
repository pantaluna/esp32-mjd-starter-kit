/*
 * Component main file.
 */
#include "driver/timer.h"

// Component header file(s)
#include "mjd.h"
#include "mjd_ads1115.h"

/*
 * Logging
 */
static const char TAG[] = "mjd_ads1115";

/*
 * MAIN
 */

/*********************************************************************************
 * _delay_millisec()
 *  @brief: The delay routine
 *
 *  @param millisec delay in ms
 *
 *  @important The RTOS func TaskDelay() is not suited; its lowest delay is 10 milliseconds and it does not work with high accuracy for low values.
 */
static void _delay_millisec(uint32_t millisec) {
    if (millisec > 0) {
        ets_delay_us(millisec * 1000);
    }
}

/*********************************************************************************
 * _log_config()
 *
 * @doc mjd_ads1115_config_t
 *
 *********************************************************************************/
static esp_err_t _log_config(const mjd_ads1115_config_t* param_ptr_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    ESP_LOGD(TAG, "LOG instance of mjd_ads1115_config_t");

    ESP_LOGD(TAG, "  manage_i2c_driver:   %u", param_ptr_config->manage_i2c_driver);
    ESP_LOGD(TAG, "  i2c_slave_addr:      0x%X (%u)", param_ptr_config->i2c_slave_addr, param_ptr_config->i2c_slave_addr);
    ESP_LOGD(TAG, "  i2c_timeout:         %i", param_ptr_config->i2c_timeout);
    ESP_LOGD(TAG, "  i2c_port_num:        %u", param_ptr_config->i2c_port_num);
    ESP_LOGD(TAG, "  i2c_scl_gpio_num:    %u", param_ptr_config->i2c_scl_gpio_num);
    ESP_LOGD(TAG, "  i2c_sda_gpio_num:    %u", param_ptr_config->i2c_sda_gpio_num);

    ESP_LOGD(TAG, "  alert_ready_gpio_num: %u", param_ptr_config->alert_ready_gpio_num);

    ESP_LOGD(TAG, "  mux:        0x%X (%u)", param_ptr_config->mux, param_ptr_config->mux);
    ESP_LOGD(TAG, "  pga:        0x%X (%u)", param_ptr_config->pga, param_ptr_config->pga);
    ESP_LOGD(TAG, "  data_rate:  0x%X (%u)", param_ptr_config->data_rate, param_ptr_config->data_rate);

    return f_retval;
}

/*********************************************************************************
 * _log_int_pin_value()
 *
 *********************************************************************************/
static esp_err_t _log_alert_ready_pin_value(const mjd_ads1115_config_t* param_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    ESP_LOGD(TAG, "ALERT READY pin value (GPIO#%i): %u", param_config->alert_ready_gpio_num, gpio_get_level(param_config->alert_ready_gpio_num));

    return f_retval;
}

/*********************************************************************************
 * _read_register()
 *
 *  @doc To read out the content of one specific register address. *
 *  @param param_data is a pointer to an unsigned int 16bits (not signed!).
 *
 *********************************************************************************/
static esp_err_t _read_register(const mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_reg_t param_reg, uint16_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    i2c_cmd_handle_t handle;

    uint8_t msbyte = 0, lsbyte = 0;

    // Log params
    ESP_LOGD(TAG, "  param_reg: 0x%X (%u)", param_reg, param_reg);

    handle = i2c_cmd_link_create();

    f_retval = i2c_master_start(handle);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. Send request i2c_master_start() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        i2c_cmd_link_delete(handle);
        // GOTO
        goto cleanup;
    }
    f_retval = i2c_master_write_byte(handle, (param_ptr_config->i2c_slave_addr << 1) | I2C_MASTER_WRITE, true);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. Send request i2c_master_write_byte() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        i2c_cmd_link_delete(handle);
        // GOTO
        goto cleanup;
    }
    f_retval = i2c_master_write_byte(handle, param_reg, true);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. Send request i2c_master_write_byte() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        i2c_cmd_link_delete(handle);
        // GOTO
        goto cleanup;
    }

    f_retval = i2c_master_start(handle);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. Receive response i2c_master_start() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        i2c_cmd_link_delete(handle);
        // GOTO
        goto cleanup;
    }
    f_retval = i2c_master_write_byte(handle, (param_ptr_config->i2c_slave_addr << 1) | I2C_MASTER_READ, true);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. Receive response i2c_master_write_byte() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        i2c_cmd_link_delete(handle);
        // GOTO
        goto cleanup;
    }
    f_retval = i2c_master_read_byte(handle, &msbyte, I2C_MASTER_ACK);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. Receive response i2c_master_read_byte() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        i2c_cmd_link_delete(handle);
        // GOTO
        goto cleanup;
    }
    f_retval = i2c_master_read_byte(handle, &lsbyte, I2C_MASTER_NACK); // last read must be I2C_MASTER_NACK
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. Receive response i2c_master_read_byte() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        i2c_cmd_link_delete(handle);
        // GOTO
        goto cleanup;
    }
    f_retval = i2c_master_stop(handle);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. Receive response i2c_master_stop() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        i2c_cmd_link_delete(handle);
        // GOTO
        goto cleanup;
    }

    f_retval = i2c_master_cmd_begin(param_ptr_config->i2c_port_num, handle, param_ptr_config->i2c_timeout);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. Receive response i2c_master_cmd_begin() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        i2c_cmd_link_delete(handle);
        // GOTO
        goto cleanup;
    }

    i2c_cmd_link_delete(handle);

    // Process REGDATA
    *param_ptr_data = (((uint16_t) msbyte << 8) | (uint16_t) lsbyte);
    ESP_LOGD(TAG, "%s(). Log REGDATA(uint16_t): 0x%X (%u)", __FUNCTION__, *param_ptr_data, *param_ptr_data);

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * _write_register()
 *
 * @doc To write data to one specific register address.
 * @param param_data is a pointer to an unsigned int 16bits (not signed!).
 *
 *********************************************************************************/
static esp_err_t _write_register(const mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_reg_t param_reg, uint16_t param_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    i2c_cmd_handle_t handle;

    // Log params
    ESP_LOGD(TAG, "  param_reg:  0x%X (%u)", param_reg, param_reg);
    ESP_LOGD(TAG, "  param_data: 0x%X (%u)", param_data, param_data);

    handle = i2c_cmd_link_create();

    f_retval = i2c_master_start(handle);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. Send request i2c_master_start() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        i2c_cmd_link_delete(handle);
        // GOTO
        goto cleanup;
    }
    f_retval = i2c_master_write_byte(handle, (param_ptr_config->i2c_slave_addr << 1) | I2C_MASTER_WRITE, true);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. Send request i2c_master_write_byte() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        i2c_cmd_link_delete(handle);
        // GOTO
        goto cleanup;
    }
    f_retval = i2c_master_write_byte(handle, param_reg, true);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. Send request i2c_master_write_byte() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        i2c_cmd_link_delete(handle);
        // GOTO
        goto cleanup;
    }
    f_retval = i2c_master_write_byte(handle, param_data >> 8, true); // MSByte
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. Send request i2c_master_write_byte() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        i2c_cmd_link_delete(handle);
        // GOTO
        goto cleanup;
    }
    f_retval = i2c_master_write_byte(handle, param_data & 0x00ff, true); // LSByte
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. Send request i2c_master_write_byte() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        i2c_cmd_link_delete(handle);
        // GOTO
        goto cleanup;
    }
    f_retval = i2c_master_stop(handle);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. Receive response i2c_master_stop() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        i2c_cmd_link_delete(handle);
        // GOTO
        goto cleanup;
    }

    f_retval = i2c_master_cmd_begin(param_ptr_config->i2c_port_num, handle, param_ptr_config->i2c_timeout);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. Receive response i2c_master_cmd_begin() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        i2c_cmd_link_delete(handle);
        // GOTO
        goto cleanup;
    }

    i2c_cmd_link_delete(handle);

    // (no data coming back)

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * @doc mjd_ads1115_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_ads1115_get_operational_status(const mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_operational_status_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    uint16_t reg_data; // word

    f_retval = _read_register(param_ptr_config, MJD_ADS1115_OPSTATUS_REG, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Extract parameter value from register data
    *param_ptr_data = (reg_data & MJD_ADS1115_OPSTATUS_BITMASK) >> MJD_ADS1115_OPSTATUS_BITSHIFT;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * @doc mjd_ads1115_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_ads1115_get_mux(mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_mux_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    uint16_t reg_data; // word

    f_retval = _read_register(param_ptr_config, MJD_ADS1115_MUX_REG, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Extract parameter value from register data
    *param_ptr_data = (reg_data & MJD_ADS1115_MUX_BITMASK) >> MJD_ADS1115_MUX_BITSHIFT;

    // SAVE IN CONFIG STRUCT
    param_ptr_config->mux = *param_ptr_data;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * @doc mjd_ads1115_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_ads1115_get_pga(mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_pga_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    uint16_t reg_data; // word

    f_retval = _read_register(param_ptr_config, MJD_ADS1115_PGA_REG, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Extract parameter value from register data
    *param_ptr_data = (reg_data & MJD_ADS1115_PGA_BITMASK) >> MJD_ADS1115_PGA_BITSHIFT;

    // SAVE IN CONFIG STRUCT
    param_ptr_config->pga = *param_ptr_data;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * @doc mjd_ads1115_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_ads1115_get_operating_mode(const mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_operating_mode_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    uint16_t reg_data; // word

    f_retval = _read_register(param_ptr_config, MJD_ADS1115_OPMODE_REG, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Extract parameter value from register data
    *param_ptr_data = (reg_data & MJD_ADS1115_OPMODE_BITMASK) >> MJD_ADS1115_OPMODE_BITSHIFT;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * @doc mjd_ads1115_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_ads1115_get_data_rate(mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_data_rate_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    uint16_t reg_data; // word

    f_retval = _read_register(param_ptr_config, MJD_ADS1115_DATARATE_REG, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Extract parameter value from register data
    *param_ptr_data = (reg_data & MJD_ADS1115_DATARATE_BITMASK) >> MJD_ADS1115_DATARATE_BITSHIFT;

    // SAVE IN CONFIG STRUCT
    param_ptr_config->data_rate = *param_ptr_data;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * @doc mjd_ads1115_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_ads1115_get_comparator_mode(const mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_comparator_mode_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    uint16_t reg_data; // word

    f_retval = _read_register(param_ptr_config, MJD_ADS1115_COMPARATORMODE_REG, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Extract parameter value from register data
    *param_ptr_data = (reg_data & MJD_ADS1115_COMPARATORMODE_BITMASK) >> MJD_ADS1115_COMPARATORMODE_BITSHIFT;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * @doc mjd_ads1115_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_ads1115_get_comparator_polarity(const mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_comparator_polarity_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    uint16_t reg_data; // word

    f_retval = _read_register(param_ptr_config, MJD_ADS1115_COMPARATORPOLARITY_REG, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Extract parameter value from register data
    *param_ptr_data = (reg_data & MJD_ADS1115_COMPARATORPOLARITY_BITMASK) >> MJD_ADS1115_COMPARATORPOLARITY_BITSHIFT;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * @doc mjd_ads1115_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_ads1115_get_latching_comparator(const mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_latching_comparator_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    uint16_t reg_data; // word

    f_retval = _read_register(param_ptr_config, MJD_ADS1115_LATCHINGCOMPARATOR_REG, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Extract parameter value from register data
    *param_ptr_data = (reg_data & MJD_ADS1115_LATCHINGCOMPARATOR_BITMASK) >> MJD_ADS1115_LATCHINGCOMPARATOR_BITSHIFT;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * @doc mjd_ads1115_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_ads1115_get_comparator_queue(const mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_comparator_queue_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    uint16_t reg_data; // word

    f_retval = _read_register(param_ptr_config, MJD_ADS1115_COMPARATORQUEUE_REG, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Extract parameter value from register data
    *param_ptr_data = (reg_data & MJD_ADS1115_COMPARATORQUEUE_BITMASK) >> MJD_ADS1115_COMPARATORQUEUE_BITSHIFT;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * @doc mjd_ads1115_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_ads1115_get_low_threshold(const mjd_ads1115_config_t* param_ptr_config, uint16_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    uint16_t reg_data; // word

    f_retval = _read_register(param_ptr_config, MJD_ADS1115_LOWTHRESHOLD_REG, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Extract parameter value from register data
    *param_ptr_data = (reg_data & MJD_ADS1115_LOWTHRESHOLD_BITMASK) >> MJD_ADS1115_LOWTHRESHOLD_BITSHIFT;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * @doc mjd_ads1115_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_ads1115_get_high_threshold(const mjd_ads1115_config_t* param_ptr_config, uint16_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    uint16_t reg_data; // word

    f_retval = _read_register(param_ptr_config, MJD_ADS1115_HIGHTHRESHOLD_REG, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Extract parameter value from register data
    *param_ptr_data = (reg_data & MJD_ADS1115_HIGHTHRESHOLD_BITMASK) >> MJD_ADS1115_HIGHTHRESHOLD_BITSHIFT;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * @doc mjd_ads1115_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_ads1115_get_conversion_ready_pin_in_low_reg(const mjd_ads1115_config_t* param_ptr_config,
                                                          mjd_ads1115_conversion_ready_pin_in_low_reg_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    uint16_t reg_data; // word

    f_retval = _read_register(param_ptr_config, MJD_ADS1115_CONVERSIONREADYPININLOWREG_REG, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Extract parameter value from register data
    *param_ptr_data = (reg_data & MJD_ADS1115_CONVERSIONREADYPININLOWREG_BITMASK) >> MJD_ADS1115_CONVERSIONREADYPININLOWREG_BITSHIFT;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * @doc mjd_ads1115_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_ads1115_get_conversion_ready_pin_in_high_reg(const mjd_ads1115_config_t* param_ptr_config,
                                                           mjd_ads1115_conversion_ready_pin_in_high_reg_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    uint16_t reg_data; // word

    f_retval = _read_register(param_ptr_config, MJD_ADS1115_CONVERSIONREADYPININHIGHREG_REG, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Extract parameter value from register data
    *param_ptr_data = (reg_data & MJD_ADS1115_CONVERSIONREADYPININLOWREG_BITMASK) >> MJD_ADS1115_CONVERSIONREADYPININLOWREG_BITSHIFT;

    // LABEL
    cleanup: ;

    return f_retval;
}

/**********
 **********
 * SET functions
 *
 */

/*********************************************************************************
 * @doc mjd_ads1115_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_ads1115_set_operational_status(const mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_operational_status_t param_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    uint16_t reg_data; // word

    // READ
    f_retval = _read_register(param_ptr_config, MJD_ADS1115_OPSTATUS_REG, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Inject new data
    ESP_LOGD(TAG, "%s(). CUR REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);
    reg_data = (reg_data & ~MJD_ADS1115_OPSTATUS_BITMASK) | ((param_data << MJD_ADS1115_OPSTATUS_BITSHIFT) & MJD_ADS1115_OPSTATUS_BITMASK);
    ESP_LOGD(TAG, "%s(). NEW REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);

    // WRITE
    f_retval = _write_register(param_ptr_config, MJD_ADS1115_OPSTATUS_REG, reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _write_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * @doc mjd_ads1115_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_ads1115_set_mux(mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_mux_t param_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    uint16_t reg_data; // word

    // READ
    f_retval = _read_register(param_ptr_config, MJD_ADS1115_MUX_REG, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Inject new data
    ESP_LOGD(TAG, "%s(). CUR REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);
    reg_data = (reg_data & ~MJD_ADS1115_MUX_BITMASK) | ((param_data << MJD_ADS1115_MUX_BITSHIFT) & MJD_ADS1115_MUX_BITMASK);
    ESP_LOGD(TAG, "%s(). NEW REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);

    // WRITE
    f_retval = _write_register(param_ptr_config, MJD_ADS1115_MUX_REG, reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _write_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // SAVE IN CONFIG STRUCT
    param_ptr_config->mux = param_data;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * @doc mjd_ads1115_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_ads1115_set_pga(mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_pga_t param_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    uint16_t reg_data; // word

    // READ
    f_retval = _read_register(param_ptr_config, MJD_ADS1115_PGA_REG, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Inject new data
    ESP_LOGD(TAG, "%s(). CUR REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);
    reg_data = (reg_data & ~MJD_ADS1115_PGA_BITMASK) | ((param_data << MJD_ADS1115_PGA_BITSHIFT) & MJD_ADS1115_PGA_BITMASK);
    ESP_LOGD(TAG, "%s(). NEW REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);

    // WRITE
    f_retval = _write_register(param_ptr_config, MJD_ADS1115_PGA_REG, reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _write_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // SAVE IN CONFIG STRUCT
    param_ptr_config->pga = param_data;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * @doc mjd_ads1115_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_ads1115_set_operating_mode(const mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_operating_mode_t param_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    uint16_t reg_data; // word

    // READ
    f_retval = _read_register(param_ptr_config, MJD_ADS1115_OPMODE_REG, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Inject new data
    ESP_LOGD(TAG, "%s(). CUR REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);
    reg_data = (reg_data & ~MJD_ADS1115_OPMODE_BITMASK) | ((param_data << MJD_ADS1115_OPMODE_BITSHIFT) & MJD_ADS1115_OPMODE_BITMASK);
    ESP_LOGD(TAG, "%s(). NEW REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);

    // WRITE
    f_retval = _write_register(param_ptr_config, MJD_ADS1115_OPMODE_REG, reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _write_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * @doc mjd_ads1115_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_ads1115_set_data_rate(mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_data_rate_t param_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    uint16_t reg_data; // word

    // READ
    f_retval = _read_register(param_ptr_config, MJD_ADS1115_DATARATE_REG, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Inject new data
    ESP_LOGD(TAG, "%s(). CUR REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);
    reg_data = (reg_data & ~MJD_ADS1115_DATARATE_BITMASK) | ((param_data << MJD_ADS1115_DATARATE_BITSHIFT) & MJD_ADS1115_DATARATE_BITMASK);
    ESP_LOGD(TAG, "%s(). NEW REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);

    // WRITE
    f_retval = _write_register(param_ptr_config, MJD_ADS1115_DATARATE_REG, reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _write_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // SAVE IN CONFIG STRUCT
    param_ptr_config->data_rate = param_data;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * @doc mjd_ads1115_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_ads1115_set_comparator_mode(const mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_comparator_mode_t param_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    uint16_t reg_data; // word

    // READ
    f_retval = _read_register(param_ptr_config, MJD_ADS1115_COMPARATORMODE_REG, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Inject new data
    ESP_LOGD(TAG, "%s(). CUR REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);
    reg_data = (reg_data & ~MJD_ADS1115_COMPARATORMODE_BITMASK)
            | ((param_data << MJD_ADS1115_COMPARATORMODE_BITSHIFT) & MJD_ADS1115_COMPARATORMODE_BITMASK);
    ESP_LOGD(TAG, "%s(). NEW REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);

    // WRITE
    f_retval = _write_register(param_ptr_config, MJD_ADS1115_COMPARATORMODE_REG, reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _write_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * @doc mjd_ads1115_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_ads1115_set_comparator_polarity(const mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_comparator_polarity_t param_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    uint16_t reg_data; // word

    // READ
    f_retval = _read_register(param_ptr_config, MJD_ADS1115_COMPARATORPOLARITY_REG, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Inject new data
    ESP_LOGD(TAG, "%s(). CUR REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);
    reg_data = (reg_data & ~MJD_ADS1115_COMPARATORPOLARITY_BITMASK)
            | ((param_data << MJD_ADS1115_COMPARATORPOLARITY_BITSHIFT) & MJD_ADS1115_COMPARATORPOLARITY_BITMASK);
    ESP_LOGD(TAG, "%s(). NEW REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);

    // WRITE
    f_retval = _write_register(param_ptr_config, MJD_ADS1115_COMPARATORPOLARITY_REG, reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _write_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * @doc mjd_ads1115_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_ads1115_set_latching_comparator(const mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_latching_comparator_t param_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    uint16_t reg_data; // word

    // READ
    f_retval = _read_register(param_ptr_config, MJD_ADS1115_LATCHINGCOMPARATOR_REG, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Inject new data
    ESP_LOGD(TAG, "%s(). CUR REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);
    reg_data = (reg_data & ~MJD_ADS1115_LATCHINGCOMPARATOR_BITMASK)
            | ((param_data << MJD_ADS1115_LATCHINGCOMPARATOR_BITSHIFT) & MJD_ADS1115_LATCHINGCOMPARATOR_BITMASK);
    ESP_LOGD(TAG, "%s(). NEW REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);

    // WRITE
    f_retval = _write_register(param_ptr_config, MJD_ADS1115_LATCHINGCOMPARATOR_REG, reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _write_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * @doc mjd_ads1115_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_ads1115_set_comparator_queue(const mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_comparator_queue_t param_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    uint16_t reg_data; // word

    // READ
    f_retval = _read_register(param_ptr_config, MJD_ADS1115_COMPARATORQUEUE_REG, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Inject new data
    ESP_LOGD(TAG, "%s(). CUR REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);
    reg_data = (reg_data & ~MJD_ADS1115_COMPARATORQUEUE_BITMASK)
            | ((param_data << MJD_ADS1115_COMPARATORQUEUE_BITSHIFT) & MJD_ADS1115_COMPARATORQUEUE_BITMASK);
    ESP_LOGD(TAG, "%s(). NEW REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);

    // WRITE
    f_retval = _write_register(param_ptr_config, MJD_ADS1115_COMPARATORQUEUE_REG, reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _write_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * @doc mjd_ads1115_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_ads1115_set_low_threshold(const mjd_ads1115_config_t* param_ptr_config, uint16_t param_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    uint16_t reg_data; // word

    // READ
    f_retval = _read_register(param_ptr_config, MJD_ADS1115_LOWTHRESHOLD_REG, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Inject new data
    ESP_LOGD(TAG, "%s(). CUR REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);
    reg_data = (reg_data & ~MJD_ADS1115_LOWTHRESHOLD_BITMASK)
            | ((param_data << MJD_ADS1115_LOWTHRESHOLD_BITSHIFT) & MJD_ADS1115_LOWTHRESHOLD_BITMASK);
    ESP_LOGD(TAG, "%s(). NEW REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);

    // WRITE
    f_retval = _write_register(param_ptr_config, MJD_ADS1115_LOWTHRESHOLD_REG, reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _write_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * @doc mjd_ads1115_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_ads1115_set_high_threshold(const mjd_ads1115_config_t* param_ptr_config, uint16_t param_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    uint16_t reg_data; // word

    // READ
    f_retval = _read_register(param_ptr_config, MJD_ADS1115_HIGHTHRESHOLD_REG, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Inject new data
    ESP_LOGD(TAG, "%s(). CUR REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);
    reg_data = (reg_data & ~MJD_ADS1115_HIGHTHRESHOLD_BITMASK)
            | ((param_data << MJD_ADS1115_HIGHTHRESHOLD_BITSHIFT) & MJD_ADS1115_HIGHTHRESHOLD_BITMASK);
    ESP_LOGD(TAG, "%s(). NEW REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);

    // WRITE
    f_retval = _write_register(param_ptr_config, MJD_ADS1115_HIGHTHRESHOLD_REG, reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _write_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * @doc mjd_ads1115_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_ads1115_set_conversion_ready_pin_in_low_reg(const mjd_ads1115_config_t* param_ptr_config,
                                                          mjd_ads1115_conversion_ready_pin_in_low_reg_t param_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    uint16_t reg_data; // word

    // READ
    f_retval = _read_register(param_ptr_config, MJD_ADS1115_CONVERSIONREADYPININLOWREG_REG, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Inject new data
    ESP_LOGD(TAG, "%s(). CUR REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);
    reg_data = (reg_data & ~MJD_ADS1115_CONVERSIONREADYPININLOWREG_BITMASK)
            | ((param_data << MJD_ADS1115_CONVERSIONREADYPININLOWREG_BITSHIFT) & MJD_ADS1115_CONVERSIONREADYPININLOWREG_BITMASK);
    ESP_LOGD(TAG, "%s(). NEW REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);

    // WRITE
    f_retval = _write_register(param_ptr_config, MJD_ADS1115_CONVERSIONREADYPININLOWREG_REG, reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _write_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * @doc mjd_ads1115_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_ads1115_set_conversion_ready_pin_in_high_reg(const mjd_ads1115_config_t* param_ptr_config,
                                                           mjd_ads1115_conversion_ready_pin_in_high_reg_t param_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    uint16_t reg_data; // word

    // READ
    f_retval = _read_register(param_ptr_config, MJD_ADS1115_CONVERSIONREADYPININHIGHREG_REG, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Inject new data
    ESP_LOGD(TAG, "%s(). CUR REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);
    reg_data = (reg_data & ~MJD_ADS1115_CONVERSIONREADYPININHIGHREG_BITMASK)
            | ((param_data << MJD_ADS1115_CONVERSIONREADYPININHIGHREG_BITSHIFT) & MJD_ADS1115_CONVERSIONREADYPININHIGHREG_BITMASK);
    ESP_LOGD(TAG, "%s(). NEW REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);

    // WRITE
    f_retval = _write_register(param_ptr_config, MJD_ADS1115_CONVERSIONREADYPININHIGHREG_REG, reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _write_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * mjd_ads1115_log_device_parameters()
 *
 * @doc mjd_ads1115_config_t
 *
 *********************************************************************************/
esp_err_t mjd_ads1115_log_device_parameters(mjd_ads1115_config_t* param_ptr_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    ESP_LOGI(TAG, "ADS1115 Log Device Params (*Read again from registers*):");

    // #1
    mjd_ads1115_operational_status_t operational_status;
    f_retval = mjd_ads1115_get_operational_status(param_ptr_config, &operational_status);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_ads1115_get_operational_status() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // #2
    mjd_ads1115_mux_t mux;
    f_retval = mjd_ads1115_get_mux(param_ptr_config, &mux);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_ads1115_get_mux() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // #3
    mjd_ads1115_pga_t pga;
    f_retval = mjd_ads1115_get_pga(param_ptr_config, &pga);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_ads1115_get_pga() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // #4
    mjd_ads1115_operating_mode_t operating_mode;
    f_retval = mjd_ads1115_get_operating_mode(param_ptr_config, &operating_mode);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_ads1115_get_operating_mode() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // #5
    mjd_ads1115_data_rate_t data_rate;
    f_retval = mjd_ads1115_get_data_rate(param_ptr_config, &data_rate);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_ads1115_get_data_rate() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // #6
    mjd_ads1115_comparator_mode_t comparator_mode;
    f_retval = mjd_ads1115_get_comparator_mode(param_ptr_config, &comparator_mode);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_ads1115_get_comparator_mode() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // #7
    mjd_ads1115_comparator_polarity_t comparator_polarity;
    f_retval = mjd_ads1115_get_comparator_polarity(param_ptr_config, &comparator_polarity);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_ads1115_get_comparator_polarity() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // #8
    mjd_ads1115_latching_comparator_t latching_comparator;
    f_retval = mjd_ads1115_get_latching_comparator(param_ptr_config, &latching_comparator);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_ads1115_get_latching_comparator() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // #9
    mjd_ads1115_comparator_queue_t comparator_queue;
    f_retval = mjd_ads1115_get_comparator_queue(param_ptr_config, &comparator_queue);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_ads1115_get_comparator_queue() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // #10
    uint16_t low_threshold;
    f_retval = mjd_ads1115_get_low_threshold(param_ptr_config, &low_threshold);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_ads1115_get_low_threshold() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // #11
    uint16_t high_threshold;
    f_retval = mjd_ads1115_get_high_threshold(param_ptr_config, &high_threshold);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_ads1115_get_high_threshold() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // #12
    mjd_ads1115_conversion_ready_pin_in_low_reg_t conversion_ready_pin_in_low_reg;
    f_retval = mjd_ads1115_get_conversion_ready_pin_in_low_reg(param_ptr_config, &conversion_ready_pin_in_low_reg);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_ads1115_get_conversion_ready_pin_in_low_reg() failed | err %i (%s)", __FUNCTION__, f_retval,
                esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // #13
    mjd_ads1115_conversion_ready_pin_in_high_reg_t conversion_ready_pin_in_high_reg;
    f_retval = mjd_ads1115_get_conversion_ready_pin_in_high_reg(param_ptr_config, &conversion_ready_pin_in_high_reg);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_ads1115_get_conversion_ready_pin_in_high_reg() failed | err %i (%s)", __FUNCTION__, f_retval,
                esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // DUMP
    char binary_output_string8[8 + 1] = "12345678";
    char binary_output_string16[16 + 1] = "1234567890123456";
    mjd_byte_to_binary_string(operational_status, binary_output_string8);
    ESP_LOGI(TAG, "  OPSTATUS:    0x%X 0b%s (%u)", operational_status, binary_output_string8, operational_status);
    mjd_byte_to_binary_string(mux, binary_output_string8);
    ESP_LOGI(TAG, "  MUX:         0x%X 0b%s (%u)", mux, binary_output_string8, mux);
    mjd_byte_to_binary_string(pga, binary_output_string8);
    ESP_LOGI(TAG, "  PGA:         0x%X 0b%s (%u)", pga, binary_output_string8, pga);
    mjd_byte_to_binary_string(operating_mode, binary_output_string8);
    ESP_LOGI(TAG, "  OPMODE:      0x%X 0b%s (%u)", operating_mode, binary_output_string8, operating_mode);
    mjd_byte_to_binary_string(data_rate, binary_output_string8);
    ESP_LOGI(TAG, "  DATARATE:    0x%X 0b%s (%u)", data_rate, binary_output_string8, data_rate);
    mjd_byte_to_binary_string(comparator_mode, binary_output_string8);
    ESP_LOGI(TAG, "  COMPARATORMODE:     0x%X 0b%s (%u)", comparator_mode, binary_output_string8, comparator_mode);
    mjd_byte_to_binary_string(comparator_polarity, binary_output_string8);
    ESP_LOGI(TAG, "  COMPARATORPOLARITY: 0x%X 0b%s (%u)", comparator_polarity, binary_output_string8, comparator_polarity);
    mjd_byte_to_binary_string(latching_comparator, binary_output_string8);
    ESP_LOGI(TAG, "  LATCHINGCOMPARATOR: 0x%X 0b%s (%u)", latching_comparator, binary_output_string8, latching_comparator);
    mjd_byte_to_binary_string(comparator_queue, binary_output_string8);
    ESP_LOGI(TAG, "  COMPARATORQUEUE:    0x%X 0b%s (%u)", comparator_queue, binary_output_string8, comparator_queue);
    mjd_word_to_binary_string(low_threshold, binary_output_string16);
    ESP_LOGI(TAG, "  LOWTHRESHOLD:  0x%X 0b%s (%u)", low_threshold, binary_output_string16, low_threshold);
    mjd_word_to_binary_string(high_threshold, binary_output_string16);
    ESP_LOGI(TAG, "  HIGHTHRESHOLD: 0x%X 0b%s (%u)", high_threshold, binary_output_string16, high_threshold);
    mjd_byte_to_binary_string(conversion_ready_pin_in_low_reg, binary_output_string8);
    ESP_LOGI(TAG, "  CONVERSIONREADYPININLOWREG: 0x%X 0b%s (%u)", conversion_ready_pin_in_low_reg, binary_output_string8,
            conversion_ready_pin_in_low_reg);
    mjd_byte_to_binary_string(conversion_ready_pin_in_high_reg, binary_output_string8);
    ESP_LOGI(TAG, "  CONVERSIONREADYPININLOWREG: 0x%X 0b%s (%u)", conversion_ready_pin_in_high_reg, binary_output_string8,
            conversion_ready_pin_in_high_reg);

    // DEVTEMP
    /////mjd_rtos_wait_forever();

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * mjd_ads1115_init()
 *
 *********************************************************************************/
esp_err_t mjd_ads1115_init(mjd_ads1115_config_t* param_ptr_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    /*
     * Validate params
     *
     *
     */
    if (param_ptr_config->i2c_scl_gpio_num == -1 || param_ptr_config->i2c_sda_gpio_num == -1) {
        f_retval = ESP_FAIL;
        ESP_LOGE(TAG, "%s(). ABORT. i2c_scl_gpio_num or i2c_sda_gpio_num is not initialized | err %i (%s)", __FUNCTION__, f_retval,
                esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /*
     * I2C
     *
     * @important The ADS1115 breakout board already contains a 10K pullup for the SCL & SDA pins. So the MCU's internal pullups for these pins can be disabled.
     */
    if (param_ptr_config->manage_i2c_driver == true) {
        // Config
        i2c_config_t i2c_conf =
            { 0 };
        i2c_conf.mode = I2C_MODE_MASTER;
        i2c_conf.scl_io_num = param_ptr_config->i2c_scl_gpio_num;
        i2c_conf.sda_io_num = param_ptr_config->i2c_sda_gpio_num;
        i2c_conf.sda_pullup_en = GPIO_PULLUP_DISABLE; //
        i2c_conf.scl_pullup_en = GPIO_PULLUP_DISABLE; //
        i2c_conf.master.clk_speed = MJD_ADS1115_I2C_MASTER_FREQ_HZ;

        f_retval = i2c_param_config(param_ptr_config->i2c_port_num, &i2c_conf);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "%s(). ABORT. i2c_param_config() | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
            // GOTO
            goto cleanup;
        }

        f_retval = i2c_driver_install(param_ptr_config->i2c_port_num, I2C_MODE_MASTER, MJD_ADS1115_I2C_MASTER_RX_BUF_DISABLE,
        MJD_ADS1115_I2C_MASTER_TX_BUF_DISABLE, MJD_ADS1115_I2C_MASTER_INTR_FLAG_NONE);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "%s(). ABORT. i2c_driver_install() | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
            // GOTO
            goto cleanup;
        }

        /*
         * FYI Temp code for resolving ESP_ERR_TIMEOUT when using long wires (5 meters) #DOESNOTWORK#
         * @conclusion An I2C Range Extended must be used in ase of long wires (>25cm).
         * @doc esp_err_t i2c_set_timeout(i2c_port_t i2c_num , int);
         * @doc #define I2C_SLAVE_TIMEOUT_DEFAULT (32000)     // 0x7D00 I2C slave timeout value, APB clock cycle number
         * @doc #define I2C_TIME_OUT_REG_V  0xFFFF
         f_retval = i2c_set_timeout(param_ptr_config->i2c_port_num, 0xFFFE);
         if (f_retval != ESP_OK) {
             ESP_LOGE(TAG, "%s(). ABORT. i2c_set_timeout() | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
             // GOTO
             goto cleanup;
         }
         */
    }

    /*
     * Setting ADS1115 good config params
     */

    // MUX
    ESP_LOGD(TAG, "%s(). Set MUX: param_ptr_config->mux 0x%X (%u)", __FUNCTION__, param_ptr_config->mux, param_ptr_config->mux);
    f_retval = mjd_ads1115_set_mux(param_ptr_config, param_ptr_config->mux);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_ads1115_set_mux() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // PGA
    ESP_LOGD(TAG, "%s(). Set PGA: param_ptr_config->pga 0x%X (%u)", __FUNCTION__, param_ptr_config->pga, param_ptr_config->pga);
    f_retval = mjd_ads1115_set_pga(param_ptr_config, param_ptr_config->pga);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_ads1115_set_pga() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // DATARATE
    ESP_LOGD(TAG, "%s(). Set DATARATE: param_ptr_config->data_rate 0x%X (%u)", __FUNCTION__, param_ptr_config->data_rate,
             param_ptr_config->data_rate);
    f_retval = mjd_ads1115_set_data_rate(param_ptr_config, param_ptr_config->data_rate);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_ads1115_set_data_rate() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /*
     * ALERT READY pin: REGISTERS & GPIO & TIMER
     *   @rule -1 means not used to detect that a measurement is ready to be read.
     *   @doc IN single-shot mode, the ALERT/RDY pin asserts LOW at the end of a conversion by default.
     */
    if (param_ptr_config->alert_ready_gpio_num != -1) {
        // 1. ADS1115 register setting to enable the ALERT READY Pin (3 settings!)
        f_retval = mjd_ads1115_set_comparator_queue(param_ptr_config, MJD_ADS1115_COMPARATORQUEUE_ASSERT_AFTER_FOUR_CONVERSIONS);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "%s(). ABORT. mjd_ads1115_set_comparator_queue() failed | err %i (%s)", __FUNCTION__, f_retval,
                    esp_err_to_name(f_retval));
            // GOTO
            goto cleanup;
        }
        f_retval = mjd_ads1115_set_conversion_ready_pin_in_low_reg(param_ptr_config, MJD_ADS1115_CONVERSIONREADYPININLOWREG_ENABLED);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "%s(). ABORT. mjd_ads1115_set_conversion_ready_pin_in_low_reg() failed | err %i (%s)", __FUNCTION__, f_retval,
                    esp_err_to_name(f_retval));
            // GOTO
            goto cleanup;
        }
        f_retval = mjd_ads1115_set_conversion_ready_pin_in_high_reg(param_ptr_config, MJD_ADS1115_CONVERSIONREADYPININHIGHREG_ENABLED);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "%s(). ABORT. mjd_ads1115_set_conversion_ready_pin_in_high_reg() failed | err %i (%s)", __FUNCTION__, f_retval,
                    esp_err_to_name(f_retval));
            // GOTO
            goto cleanup;
        }

        // 2. GPIO
        gpio_config_t io_conf =
            { 0 };
        io_conf.pin_bit_mask = (1ULL << param_ptr_config->alert_ready_gpio_num);
        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pull_up_en = GPIO_PULLUP_ENABLE;  // @important When configured as a conversion ready pin, ALERT/RDY requires a pullup resistor.
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        io_conf.intr_type = GPIO_INTR_DISABLE;
        f_retval = gpio_config(&io_conf);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "%s(). ABORT. gpio_config() | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
            // GOTO
            goto cleanup;
        }
        _log_alert_ready_pin_value(param_ptr_config);

        // 3. TIMER
        timer_config_t tconfig = {};
        tconfig.divider = 64000; // Let the timer tick on a relative slow pace. 1.25 Khz: esp_clk_apb_freq() / 64000 = 1250 ticks/second
        tconfig.counter_dir = TIMER_COUNT_UP;
        tconfig.counter_en = TIMER_PAUSE; // Pause when configured (do not start right now)
        tconfig.alarm_en = TIMER_ALARM_DIS;
        tconfig.auto_reload = false;
        f_retval = timer_init(MJD_ADS1115_TIMER_GROUP_ID, MJD_ADS1115_TIMER_ID, &tconfig);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "%s(). timer_init() | err %d %s", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
            // GOTO
            goto cleanup;
        }

    } else {
        ESP_LOGI(TAG, "ADS1115 ALERT READY pin disabled in param_ptr_config");
    }

    /*
     * AFTER ACTION
     */
    _log_config(param_ptr_config);

    // DEVTEMP
    /////mjd_rtos_wait_forever();

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * mjd_ads1115_deinit()
 *
 *********************************************************************************/
esp_err_t mjd_ads1115_deinit(const mjd_ads1115_config_t* param_ptr_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    _log_config(param_ptr_config);

    /*
     * I2C Driver
     */
    if (param_ptr_config->manage_i2c_driver == true) {
        f_retval = i2c_driver_delete(param_ptr_config->i2c_port_num);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "%s(). ABORT. i2c_driver_delete() | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
            // GOTO
            goto cleanup;
        }
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * CMD Start & Get Single Conversion
 *
 *
 *********************************************************************************/
esp_err_t mjd_ads1115_cmd_get_single_conversion(mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_data_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    _log_alert_ready_pin_value(param_ptr_config);

    /*****
     * Start single-shot conversion
     */
    f_retval = mjd_ads1115_set_operational_status(param_ptr_config, MJD_ADS1115_OPSTATUS_START_SINGLE_CONVERSION);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT mjd_ads1115_set_operational_status() | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /*****
     * THE DEVICE NEEDS TIME TO POWER-ON FROM SLEEP MODE, DO A CONVERSION AND PROVIDE THE DATA IN THE CONVERSION REGISTER
     * @doc Wait. Either a computed waiting time (based on data rate SPS) XOR using the ALERT READY Pin (going from HIGH to LOW).
     * @rule alert_ready_gpio_num = -1 means the pin is not used to detect that a measurement is ready to be read.
     *
     * @rule Wait formula when not using the pin: 1millisec [reserve] + 1millisec [power up time 25microsec rounded up] + (1000 * 1 / data_rate_SPS)]
     *
     * @doc The ADS111x remain in this power-down state until a 1 is written to the operational status (OS) bit in the Config register.
     *      When the OS bit is asserted, the device powers up in approximately 25µs, resets the OS bit to 0, and starts a single conversion.
     *
     */
    /*
     * LOOKUP TABLE: Data Rate, Samples Per Second
     *      These bits control the data rate setting.
     *          000 : 8 SPS
     *          001 : 16 SPS
     *          010 : 32 SPS
     *          011 : 64 SPS
     *          100 : 128 SPS (default)
     *          101 : 250 SPS
     *          110 : 475 SPS
     *          111 : 860 SPS
     *
     */
    static uint32_t _data_rate_values[MJD_ADS1115_DATARATE_MAX] =
        { 8, 16, 32, 64, 128, 250, 475, 860 };

    if (param_ptr_config->alert_ready_gpio_num == -1) {
        // @ref Wait formula if pin is not used.
        uint32_t delay = 1 + 1 + (1000 * 1 / _data_rate_values[param_ptr_config->data_rate]);
        ESP_LOGD(TAG, "%s(). Computed wait delay (ms): %u", __FUNCTION__, delay);
        _delay_millisec(delay);
    } else {
        // WAIT for the ALERT READY Pin value go LOW -XOR- timeout from esptimer
        bool has_timed_out = false;
        const double TIMEOUT_SECONDS = 2; // FAIL when pin is not LOW after 2 seconds
        double timer_counter_value_seconds = 0;

        timer_set_counter_value(MJD_ADS1115_TIMER_GROUP_ID, MJD_ADS1115_TIMER_ID, 00000000ULL);
        timer_start(MJD_ADS1115_TIMER_GROUP_ID, MJD_ADS1115_TIMER_ID);

        while (gpio_get_level(param_ptr_config->alert_ready_gpio_num) != 0) {
            timer_get_counter_time_sec(MJD_ADS1115_TIMER_GROUP_ID, MJD_ADS1115_TIMER_ID, &timer_counter_value_seconds);
            if (timer_counter_value_seconds > TIMEOUT_SECONDS) {
                has_timed_out = true;
                break; // BREAK WHILE
            }
            vTaskDelay(RTOS_DELAY_10MILLISEC); // Wait in increments of 10 milliseonds (the lowest possible value for this RTOS func)
        }

        // pause timer (stop = n.a.)
        timer_pause(TIMER_GROUP_0, TIMER_0);

        if (has_timed_out == false) {
            ESP_LOGD(TAG, "OK. The ALERT READY Pin value did go to LOW before the timeout (after +-%5f seconds)", timer_counter_value_seconds);
        } else {
            mjd_ads1115_log_device_parameters(param_ptr_config);
            _log_alert_ready_pin_value(param_ptr_config);
            f_retval = ESP_FAIL;
            ESP_LOGE(TAG, "%s(). The ALERT READY Pin value did not go LOW before the timeout (%5f seconds) | err %i (%s)", __FUNCTION__,
                    timer_counter_value_seconds, f_retval, esp_err_to_name(f_retval));
            // GOTO
            goto cleanup;
        }
    }

    _log_alert_ready_pin_value(param_ptr_config);

    /*****
     * READ CONVERSION REGISTER
     *
     * @rule Strip bit#15 from the register value to get an unsigned integer with 15 bits in total.
     */
    /*****
     * LOOKUP TABLE: PGA Voltage Levels.
     *  These bits set the FSR of the programmable gain amplifier. These bits serve no function on the ADS1113.
     *      000 : FSR = ±6.144 V
     *      001 : FSR = ±4.096 V
     *      010 : FSR = ±2.048 V (default)
     *      011 : FSR = ±1.024 V
     *      100 : FSR = ±0.512 V
     *      101 : FSR = ±0.256 V
     */
    float _pga_voltage_values[MJD_ADS1115_PGA_MAX] =
        { 6.144, 4.096, 2.048, 1.024, 0.512, 0.256 };

    uint16_t reg_data_uint16; // Read as unsigned UINT16 & Interpreted as signed INT16!
    f_retval = _read_register(param_ptr_config, MJD_ADS1115_REG_CONVERSION, &reg_data_uint16);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    ESP_LOGD(TAG, "%s(). reg_data_uint16: %u", __FUNCTION__, reg_data_uint16);

    param_ptr_data->pga = param_ptr_config->pga;
    param_ptr_data->raw_value = (int16_t)(reg_data_uint16); // Read as unsigned UINT16 & Interpreted as signed INT16!
    param_ptr_data->volt_value = (float) (param_ptr_data->raw_value) / (float) (0x7FFF) * _pga_voltage_values[param_ptr_data->pga];

    // LABEL
    cleanup: ;

    return f_retval;
}
