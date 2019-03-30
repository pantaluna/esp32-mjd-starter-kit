/*
 * Component main file.
 */
#include <math.h>
#include "driver/timer.h"

// Component header file(s)
#include "mjd.h"
#include "mjd_sht3x.h"

/*
 * Logging
 */
static const char TAG[] = "mjd_sht3x";

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
 * @doc mjd_sht3x_config_t
 *
 *********************************************************************************/
static esp_err_t _log_config(const mjd_sht3x_config_t* param_ptr_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    ESP_LOGD(TAG, "LOG instance of mjd_sht3x_config_t");

    ESP_LOGD(TAG, "  manage_i2c_driver:   %u", param_ptr_config->manage_i2c_driver);
    ESP_LOGD(TAG, "  i2c_slave_addr:      0x%X (%u)", param_ptr_config->i2c_slave_addr, param_ptr_config->i2c_slave_addr);
    ESP_LOGD(TAG, "  i2c_timeout:         %i", param_ptr_config->i2c_timeout);
    ESP_LOGD(TAG, "  i2c_port_num:        %u", param_ptr_config->i2c_port_num);
    ESP_LOGD(TAG, "  i2c_scl_gpio_num:    %u", param_ptr_config->i2c_scl_gpio_num);
    ESP_LOGD(TAG, "  i2c_sda_gpio_num:    %u", param_ptr_config->i2c_sda_gpio_num);

    ESP_LOGD(TAG, "  repeatability:       0x%X (%u)", param_ptr_config->repeatability, param_ptr_config->repeatability);

    return f_retval;
}

/*********************************************************************************
 * _send_cmd()
 *
 *   A command with no extra data to be sent and no extra data to be received (the ones which have, have separate methods!)
 *
 *  @param param_...
 *
 * Split the uint16_t param_command in two uint8_t's (MSB LSB)
 * After sending a command to the sensor a minimal waiting time of 1ms is needed before another command can be received by the sensor.
 *
 *********************************************************************************/
static esp_err_t _send_cmd(const mjd_sht3x_config_t* param_ptr_config, mjd_sht3x_command_t param_command) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    i2c_cmd_handle_t handle;

    ESP_LOGD(TAG, "Send Command");
    ESP_LOGD(TAG, "  command: 0x%X (sizeof=%zu %u)", param_command, sizeof(param_command), param_command);

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
    f_retval = i2c_master_write_byte(handle, MJD_HIBYTE(param_command), true); // MSByte
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. Send request i2c_master_write_byte() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        i2c_cmd_link_delete(handle);
        // GOTO
        goto cleanup;
    }
    f_retval = i2c_master_write_byte(handle, MJD_LOBYTE(param_command), true); // LSByte
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

    _delay_millisec(1);

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * _read_register()
 *
 *  @doc To read out the content of one specific register address. *
 *  @param param_data is a pointer to an unsigned int 16bits (not signed!).
 *
 *********************************************************************************/
static esp_err_t _read_register(const mjd_sht3x_config_t* param_ptr_config, mjd_sht3x_reg_t param_reg, uint16_t* param_ptr_data) {
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
    ESP_LOGD(TAG, "%s(). Log REGDATA (uint16_t): 0x%X (%u)", __FUNCTION__, *param_ptr_data, *param_ptr_data);

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * _check_crc()
 *
 * @doc (rx_buf+3, rx_buf[5]);
 *
 * @datasheet 4.12 Checksum Calculation
 *    The 8-bit CRC checksum transmitted after each data word is generated by a CRC algorithm. Its properties are displayed in Table 20.
 *    The CRC covers the contents of the two previously transmitted data bytes. To calculate the checksum only these two previously transmitted data bytes are used.
 *      Property        Value
 *      -------         --------
 *      Name            CRC-8
 *      Width           8 bit
 *      Protected data  read and/or write data
 *      Polynomial      0x31 (x8 + x5 + x4 + 1)
 *      Initialization  0xFF
 *      Reflect input   false
 *      Reflect output  false
 *      Final XOR       0x00
 *
 * Polynomial for CRC is 0x31 and not 0x131 (an 8-bit checksum can't use the 9th bit in the polynomial)
 *
 *  Example CRC (0xBEEF) = 0x92
 *
 *********************************************************************************/
static esp_err_t _check_crc(const uint8_t *param_data, int param_len, uint8_t param_expected_value) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    const uint8_t POLYNOMIAL = 0x31;

    uint8_t crc = 0xFF;

    // calculates 8-Bit checksum with given polynomial
    for (uint8_t idx = 0; idx < param_len; idx++) {
        crc ^= (param_data[idx]);
        for (uint8_t bit = 8; bit > 0; --bit) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ POLYNOMIAL;
            } else {
                crc = (crc << 1);
            }
        }
    }

    if (crc != param_expected_value) {
        f_retval = ESP_FAIL;
    }

    return f_retval;
}

/*********************************************************************************
 * @doc mjd_sht3x_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_sht3x_get_alert_pending_status(const mjd_sht3x_config_t* param_ptr_config, mjd_sht3x_alert_pending_status_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    uint16_t reg_data; // word

    f_retval = _read_register(param_ptr_config, MJD_SHT3X_ALERT_PENDING_STATUS_REG, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Extract parameter value from register data
    *param_ptr_data = (reg_data & MJD_SHT3X_ALERT_PENDING_STATUS_BITMASK) >> MJD_SHT3X_ALERT_PENDING_STATUS_BITSHIFT;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * @doc mjd_sht3x_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_sht3x_get_heater_status(mjd_sht3x_config_t* param_ptr_config, mjd_sht3x_heater_status_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    uint16_t reg_data; // word

    f_retval = _read_register(param_ptr_config, MJD_SHT3X_HEATER_STATUS_REG, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Extract parameter value from register data
    *param_ptr_data = (reg_data & MJD_SHT3X_HEATER_STATUS_BITMASK) >> MJD_SHT3X_HEATER_STATUS_BITSHIFT;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * @doc mjd_sht3x_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_sht3x_get_rh_tracking_alert(mjd_sht3x_config_t* param_ptr_config, mjd_sht3x_rh_tracking_alert_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    uint16_t reg_data; // word

    f_retval = _read_register(param_ptr_config, MJD_SHT3X_RH_TRACKING_ALERT_REG, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Extract parameter value from register data
    *param_ptr_data = (reg_data & MJD_SHT3X_RH_TRACKING_ALERT_BITMASK) >> MJD_SHT3X_RH_TRACKING_ALERT_BITSHIFT;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * @doc mjd_sht3x_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_sht3x_get_t_tracking_alert(const mjd_sht3x_config_t* param_ptr_config, mjd_sht3x_t_tracking_alert_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    uint16_t reg_data; // word

    f_retval = _read_register(param_ptr_config, MJD_SHT3X_T_TRACKING_ALERT_REG, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Extract parameter value from register data
    *param_ptr_data = (reg_data & MJD_SHT3X_T_TRACKING_ALERT_BITMASK) >> MJD_SHT3X_T_TRACKING_ALERT_BITSHIFT;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * @doc mjd_sht3x_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_sht3x_get_system_reset_detected(mjd_sht3x_config_t* param_ptr_config, mjd_sht3x_system_reset_detected_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    uint16_t reg_data; // word

    f_retval = _read_register(param_ptr_config, MJD_SHT3X_SYSTEM_RESET_DETECTED_REG, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Extract parameter value from register data
    *param_ptr_data = (reg_data & MJD_SHT3X_SYSTEM_RESET_DETECTED_BITMASK) >> MJD_SHT3X_SYSTEM_RESET_DETECTED_BITSHIFT;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * @doc mjd_sht3x_defs.h
 *
 * @important Nobody seems to use this feature, so do NOT use it as well to check if Commands have been executed succesfully.
 *
 *********************************************************************************/
esp_err_t mjd_sht3x_get_last_command_status(const mjd_sht3x_config_t* param_ptr_config, mjd_sht3x_last_command_status_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    uint16_t reg_data; // word

    f_retval = _read_register(param_ptr_config, MJD_SHT3X_LAST_COMMAND_STATUS_REG, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Extract parameter value from register data
    *param_ptr_data = (reg_data & MJD_SHT3X_LAST_COMMAND_STATUS_BITMASK) >> MJD_SHT3X_LAST_COMMAND_STATUS_BITSHIFT;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * @doc mjd_sht3x_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_sht3x_get_write_data_checksum_status(const mjd_sht3x_config_t* param_ptr_config, mjd_sht3x_write_data_checksum_status_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    uint16_t reg_data; // word

    f_retval = _read_register(param_ptr_config, MJD_SHT3X_WRITE_DATA_CHECKSUM_STATUS_REG, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Extract parameter value from register data
    *param_ptr_data = (reg_data & MJD_SHT3X_WRITE_DATA_CHECKSUM_STATUS_BITMASK) >> MJD_SHT3X_WRITE_DATA_CHECKSUM_STATUS_BITSHIFT;

    // LABEL
    cleanup: ;

    return f_retval;
}

/**********
 **********
 * SET functions: NONE for this device (The status register is read-only)
 *
 */

/*********************************************************************************
 * mjd_sht3x_log_device_parameters()
 *
 * @doc mjd_sht3x_config_t
 *
 *********************************************************************************/
esp_err_t mjd_sht3x_log_device_parameters(mjd_sht3x_config_t* param_ptr_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    ESP_LOGI(TAG, "SHT3X Log Device Params (*Read again from registers*):");

    // #1
    mjd_sht3x_alert_pending_status_t alert_pending_status;
    f_retval = mjd_sht3x_get_alert_pending_status(param_ptr_config, &alert_pending_status);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_sht3x_get_alert_pending_status() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // #2
    mjd_sht3x_heater_status_t heater_status;
    f_retval = mjd_sht3x_get_heater_status(param_ptr_config, &heater_status);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_sht3x_get_heater_status() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // #3
    mjd_sht3x_rh_tracking_alert_t rh_tracking_alert;
    f_retval = mjd_sht3x_get_rh_tracking_alert(param_ptr_config, &rh_tracking_alert);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_sht3x_get_rh_tracking_alert() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // #4
    mjd_sht3x_t_tracking_alert_t t_tracking_alert;
    f_retval = mjd_sht3x_get_t_tracking_alert(param_ptr_config, &t_tracking_alert);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_sht3x_get_t_tracking_alert() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // #5
    mjd_sht3x_system_reset_detected_t system_reset_detected;
    f_retval = mjd_sht3x_get_system_reset_detected(param_ptr_config, &system_reset_detected);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_sht3x_get_system_reset_detected() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // #6
    mjd_sht3x_last_command_status_t last_command_status;
    f_retval = mjd_sht3x_get_last_command_status(param_ptr_config, &last_command_status);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_sht3x_get_last_command_status() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // #7
    mjd_sht3x_write_data_checksum_status_t write_data_checksum_status;
    f_retval = mjd_sht3x_get_write_data_checksum_status(param_ptr_config, &write_data_checksum_status);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_sht3x_get_write_data_checksum_status() failed | err %i (%s)", __FUNCTION__, f_retval,
                esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // DUMP
    char binary_output_string8[8 + 1] = "12345678";
    mjd_byte_to_binary_string(alert_pending_status, binary_output_string8);
    ESP_LOGI(TAG, "  alert_pending_status:       0x%X 0b%s (%u)", alert_pending_status, binary_output_string8, alert_pending_status);
    mjd_byte_to_binary_string(heater_status, binary_output_string8);
    ESP_LOGI(TAG, "  heater_status:              0x%X 0b%s (%u)", heater_status, binary_output_string8, heater_status);
    mjd_byte_to_binary_string(rh_tracking_alert, binary_output_string8);
    ESP_LOGI(TAG, "  rh_tracking_alert:          0x%X 0b%s (%u)", rh_tracking_alert, binary_output_string8, rh_tracking_alert);
    mjd_byte_to_binary_string(t_tracking_alert, binary_output_string8);
    ESP_LOGI(TAG, "  t_tracking_alert:           0x%X 0b%s (%u)", t_tracking_alert, binary_output_string8, t_tracking_alert);
    mjd_byte_to_binary_string(system_reset_detected, binary_output_string8);
    ESP_LOGI(TAG, "  system_reset_detected:      0x%X 0b%s (%u)", system_reset_detected, binary_output_string8, system_reset_detected);
    mjd_byte_to_binary_string(last_command_status, binary_output_string8);
    ESP_LOGI(TAG, "  last_command_status:        0x%X 0b%s (%u)", last_command_status, binary_output_string8, last_command_status);
    mjd_byte_to_binary_string(write_data_checksum_status, binary_output_string8);
    ESP_LOGI(TAG, "  write_data_checksum_status: 0x%X 0b%s (%u)", write_data_checksum_status, binary_output_string8, write_data_checksum_status);

    // DEVTEMP
    /////mjd_rtos_wait_forever();

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * CMD Break command / Stop Periodic Data Acquisition Mode
 *
 * Upon reception of the break command the sensor will abort the ongoing measurement and enter the single shot mode. This takes ***1ms***.
 *
 * The periodic data acquisition mode can be stopped using the break command shown in Table 13.
 * It is recommended to stop the periodic data acquisition prior to sending another command (except Fetch Data
 * command) using the break command.
 *
 * Not used yet because this version's focus is on single-shot-measurement.
 *********************************************************************************/
esp_err_t mjd_sht3x_cmd_break(const mjd_sht3x_config_t* param_ptr_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    f_retval = _send_cmd(param_ptr_config, MJD_SHT3X_CMD_BREAK);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _send_cmd() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * CMD Clear Status Register
 *
 * All user-readable bitmap flags (bit 15, 11, 10, 4) in the status register will be cleared (set to zero)
 *
 *********************************************************************************/
esp_err_t mjd_sht3x_cmd_clear_status_register(const mjd_sht3x_config_t* param_ptr_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    f_retval = _send_cmd(param_ptr_config, MJD_SHT3X_CMD_CLEAR_STATUS_REGISTER);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _send_cmd() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * CMD Soft Reset
 *
 * @important SHT3x Power-up time: 1 ms | SHT85 Power-up time: 1.5 ms => Use 2 ms
 *
 *********************************************************************************/
esp_err_t mjd_sht3x_cmd_soft_reset(const mjd_sht3x_config_t* param_ptr_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    f_retval = _send_cmd(param_ptr_config, MJD_SHT3X_CMD_SOFT_RESET);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _send_cmd() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    _delay_millisec(1); // 1ms delay already in _send_cmd()

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * CMD Start & Get Single Conversion
 *
 * @dep prop param_ptr_config->repeatability
 *
 * THE DEVICE NEEDS TIME TO POWER-ON FROM SLEEP MODE, DO A CONVERSION AND PROVIDE THE DATA IN THE CONVERSION REGISTER
 *     @datasheet Measurement duration/delay:
 *          [Add +1 ms as a margin.]
 *          High repeatability:   15 ms
 *          Medium repeatability:  6 ms
 *          Low repeatability:     4 ms
 *
 *********************************************************************************/
esp_err_t mjd_sht3x_cmd_get_single_measurement(const mjd_sht3x_config_t* param_ptr_config, mjd_sht3x_data_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    /*****
     * Start single-shot measurement mode
     */
    mjd_sht3x_command_t command;
    if (param_ptr_config->repeatability == MJD_SHT3X_REPEATABILITY_HIGH) {
        command = MJD_SHT3X_CMD_START_SINGLE_SHOT_MODE_REPEATABILITY_HIGH;
    } else if (param_ptr_config->repeatability == MJD_SHT3X_REPEATABILITY_MEDIUM) {
        command = MJD_SHT3X_CMD_START_SINGLE_SHOT_MODE_REPEATABILITY_MEDIUM;
    } else if (param_ptr_config->repeatability == MJD_SHT3X_REPEATABILITY_LOW) {
        command = MJD_SHT3X_CMD_START_SINGLE_SHOT_MODE_REPEATABILITY_LOW;
    } else {
        f_retval = ESP_FAIL;
        ESP_LOGE(TAG, "%s(). ABORT _send_cmd() invalid prop Repeatability | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    f_retval = _send_cmd(param_ptr_config, command);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _send_cmd() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /*****
     * DO DELAY:
     */
    static uint32_t _delay_ms_per_repeatability[MJD_SHT3X_REPEATABILITY_MAX] =
        { 15, 6, 4 };

    uint32_t delay = 1 + _delay_ms_per_repeatability[param_ptr_config->repeatability];
    ESP_LOGD(TAG, "%s(). Computed delay (ms): %u", __FUNCTION__, delay);
    _delay_millisec(delay);

    /*****
     * READ DATA
     *
     * After the sensor has completed the measurement (takes time), the master can read the measurement results (pair of RH& T)
     * by sending a START condition followed by an I2C read header.
     * The sensor will acknowledge the reception of the read header and send two bytes of data (temperature) followed by one byte CRC checksum
     * and another two bytes of data (relative humidity) followed by one byte CRC checksum.
     *
     */
    i2c_cmd_handle_t handle;
    uint8_t rx_buf[6];

    handle = i2c_cmd_link_create();

    f_retval = i2c_master_start(handle);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. Send request i2c_master_start() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        i2c_cmd_link_delete(handle);
        // GOTO
        goto cleanup;
    }
    f_retval = i2c_master_write_byte(handle, (param_ptr_config->i2c_slave_addr << 1) | I2C_MASTER_READ, true);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. Send request i2c_master_write_byte() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        i2c_cmd_link_delete(handle);
        // GOTO
        goto cleanup;
    }

    // ---@doc i2c_master_read() param4=I2C_MASTER_LAST_NACK: do ACK for all reads except do NACK for the last read (handy func!)
    f_retval = i2c_master_read(handle, rx_buf, ARRAY_SIZE(rx_buf), I2C_MASTER_LAST_NACK);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. Receive response i2c_master_read() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        i2c_cmd_link_delete(handle);
        // GOTO
        goto cleanup;
    }
    // ---

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

    for (uint32_t j = 0; j < ARRAY_SIZE(rx_buf); j++) {
        ESP_LOGD(TAG, "%s(). rx_buf[%u]: %u", __FUNCTION__, j, rx_buf[j]);
    }

    /***
     * Check CRC
     */
    f_retval = _check_crc(rx_buf, 2, rx_buf[2]);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. CRC check for temperature failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    f_retval = _check_crc(rx_buf + 3, 2, rx_buf[5]);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. CRC check for relative_humidity failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /**
     * Conversions & Calculations
     *
     * @datasheet Relative Humidity, Temperature Celsius & Fahrenheit.
     * @pdf "Sensirion_Humidity_Sensors_at_a_Glance.pdf" Dew point formula.
     *   Inputs:
     *      td: dew point temperature in °C
     *      t:  actual temperature in °C
     *      RH: actual relative humidity in %
     *      m:  17.62
     *      Tn  243.12 °C
     *   Formula:
     *      H = (log10(RH)-2.0)/0.4343+(17.62*t)/(243.12+t);
     *      td = 243.12*H/(17.62-H);
     */
    param_ptr_data->repeatability = param_ptr_config->repeatability;

    param_ptr_data->temperature_celsius = -45.0 + 175.0 * (rx_buf[0] * 256.0 + rx_buf[1]) / 65535.0;
    param_ptr_data->temperature_fahrenheit = -49.0 + 315.0 * (rx_buf[0] * 256.0 + rx_buf[1]) / 65535.0;

    param_ptr_data->relative_humidity = 100.0 * (rx_buf[3] * 256.0 + rx_buf[4]) / 65535.0;

    float helper_dew_point_celsius = (log10(param_ptr_data->relative_humidity) - 2.0) / 0.4343
            + (17.62 * param_ptr_data->temperature_celsius) / (243.12 + param_ptr_data->temperature_celsius);
    param_ptr_data->dew_point_celsius = 243.12 * helper_dew_point_celsius / (17.62 - helper_dew_point_celsius);

    float helper_dew_point_fahrenheit = (log10(param_ptr_data->relative_humidity) - 2.0) / 0.4343
            + (17.62 * param_ptr_data->temperature_fahrenheit) / (243.12 + param_ptr_data->temperature_fahrenheit);
    param_ptr_data->dew_point_fahrenheit = 243.12 * helper_dew_point_fahrenheit / (17.62 - helper_dew_point_fahrenheit);

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * mjd_sht3x_init()
 *
 * @important SHT3x Power-up time: 1 ms | SHT85 Power-up time: 1.5 ms => Use 2 ms.
 * @important Always soft-reset after installing the ESP32 I2C Driver (else the subsequent I2C actions will NOT WORK!
 *
 *********************************************************************************/
esp_err_t mjd_sht3x_init(mjd_sht3x_config_t* param_ptr_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    /*
     * Validate params
     *
     */
    if (param_ptr_config->i2c_scl_gpio_num == -1 || param_ptr_config->i2c_sda_gpio_num == -1) {
        f_retval = ESP_FAIL;
        ESP_LOGE(TAG, "%s(). ABORT. i2c_scl_gpio_num or i2c_sda_gpio_num is not initialized | err %i (%s)", __FUNCTION__, f_retval,
                esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    if (param_ptr_config->repeatability != MJD_SHT3X_REPEATABILITY_HIGH && param_ptr_config->repeatability != MJD_SHT3X_REPEATABILITY_MEDIUM
            && param_ptr_config->repeatability != MJD_SHT3X_REPEATABILITY_LOW) {
        f_retval = ESP_FAIL;
        ESP_LOGE(TAG, "%s(). ABORT. repeatability invalid value | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    /*
     * I2C
     *
     * @important The SHT3X breakout board already contains a 10K pullup for the SCL & SDA pins. So the MCU's internal pullups for these pins can be disabled.
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
        i2c_conf.master.clk_speed = MJD_SHT3X_I2C_MASTER_FREQ_HZ;

        f_retval = i2c_param_config(param_ptr_config->i2c_port_num, &i2c_conf);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "%s(). ABORT. i2c_param_config() | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
            // GOTO
            goto cleanup;
        }

        f_retval = i2c_driver_install(param_ptr_config->i2c_port_num, I2C_MODE_MASTER, MJD_SHT3X_I2C_MASTER_RX_BUF_DISABLE,
        MJD_SHT3X_I2C_MASTER_TX_BUF_DISABLE, MJD_SHT3X_I2C_MASTER_INTR_FLAG_NONE);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "%s(). ABORT. i2c_driver_install() | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
            // GOTO
            goto cleanup;
        }
    }

    _delay_millisec(2);

    f_retval = mjd_sht3x_cmd_soft_reset(param_ptr_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_sht3x_cmd_soft_reset() err %i %s", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /*
     * AFTER ACTIONs: logging
     */
    _log_config(param_ptr_config);

    // DEVTEMP
    /////mjd_rtos_wait_forever();

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * mjd_sht3x_deinit()
 *
 *********************************************************************************/
esp_err_t mjd_sht3x_deinit(const mjd_sht3x_config_t* param_ptr_config) {
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
