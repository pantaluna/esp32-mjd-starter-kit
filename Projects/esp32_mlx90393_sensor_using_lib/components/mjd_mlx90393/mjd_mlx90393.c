/*
 * Component main file.
 */
#include "driver/timer.h"

// Component header file(s)
#include "mjd.h"
#include "mjd_mlx90393.h"

/*
 * Logging
 */
static const char TAG[] = "mjd_mlx90393";

/*
 * LOOKUP TABLES
 *
 * - Sensitivity lookup tables XY and Z
 *      Data sheet pg27 Table 14: Sensitivity table for given gain and resolution selection for HALLCONF=0xC.
 *      It is a function of GAIN_SEL and RES_XY/RES_Z.
 *      The gains are different for XY and Z.
 */
static float _sensitivity_xy[MJD_MLX90393_GAIN_SEL_MAX][MJD_MLX90393_RES_XYZ_MAX] =
    {
        { 0.751, 1.502, 3.004, 6.009 },
        { 0.601, 1.202, 2.403, 4.840 },
        { 0.451, 0.901, 1.803, 3.605 },
        { 0.376, 0.751, 1.502, 3.004 },
        { 0.300, 0.601, 1.202, 2.403 },
        { 0.250, 0.501, 1.001, 2.003 },
        { 0.200, 0.401, 0.801, 1.602 },
        { 0.150, 0.300, 0.601, 1.202 }, };
static float _sensitivity_z[MJD_MLX90393_GAIN_SEL_MAX][MJD_MLX90393_RES_XYZ_MAX] =
    {
        { 1.210, 2.420, 4.840, 9.680 },
        { 0.968, 1.936, 3.872, 7.744 },
        { 0.726, 1.452, 2.904, 5.808 },
        { 0.605, 1.210, 2.420, 4.840 },
        { 0.484, 0.968, 1.936, 3.872 },
        { 0.403, 0.807, 1.613, 3.227 },
        { 0.323, 0.645, 1.291, 2.581 },
        { 0.242, 0.484, 0.968, 1.936 }, };

/*
 * MAIN
 */

/*********************************************************************************
 * _delay_millisec()
 *  @brief: The delay routine
 *
 *  @param : delay in ms
 *
 *  @important The RTOS func TaskDelay() is not suited. Its lowest delay is 10 milliseconds and it does not work with high accuracy for low values.
 */
static void _delay_millisec(uint32_t millisec) {
    if (millisec > 0) {
        ets_delay_us(millisec * 1000);
    }
}

/*********************************************************************************
 * _log_config()
 *
 * @doc mjd_mlx90393_config_t
 *
 *********************************************************************************/
static esp_err_t _log_config(const mjd_mlx90393_config_t* param_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    ESP_LOGD(TAG, "LOG instance of mjd_mlx90393_config_t");

    ESP_LOGD(TAG, "  manage_i2c_driver:   %u", param_config->manage_i2c_driver);
    ESP_LOGD(TAG, "  i2c_port_num:        %u", param_config->i2c_port_num);
    ESP_LOGD(TAG, "  i2c_slave_addr:      0x%X (%u)", param_config->i2c_slave_addr, param_config->i2c_slave_addr);
    ESP_LOGD(TAG, "  i2c_scl_gpio_num:    %u", param_config->i2c_scl_gpio_num);
    ESP_LOGD(TAG, "  i2c_sda_gpio_num:    %u", param_config->i2c_sda_gpio_num);
    ESP_LOGD(TAG, "  int_gpio_num:        %u", param_config->int_gpio_num);

    ESP_LOGD(TAG, "  mlx_metrics_selector:");
    ESP_LOGD(TAG, "    x_axis (bool):      %u", param_config->mlx_metrics_selector.x_axis);
    ESP_LOGD(TAG, "    y_axis (bool):      %u", param_config->mlx_metrics_selector.y_axis);
    ESP_LOGD(TAG, "    z_axis (bool):      %u", param_config->mlx_metrics_selector.z_axis);
    ESP_LOGD(TAG, "    temperature (bool): %u", param_config->mlx_metrics_selector.temperature);

    ESP_LOGD(TAG, "  mlx_sens_tc_lt (uint16_t): 0x%" PRIX16 " (%" PRIu16")", param_config->mlx_sens_tc_lt, param_config->mlx_sens_tc_lt);
    ESP_LOGD(TAG, "  mlx_sens_tc_ht (uint16_t): 0x%" PRIX16 " (%" PRIu16")", param_config->mlx_sens_tc_ht, param_config->mlx_sens_tc_ht);
    ESP_LOGD(TAG, "  mlx_tref (uint16_t):       0x%" PRIX16 " (%" PRIu16")", param_config->mlx_tref, param_config->mlx_tref);

    ESP_LOGD(TAG, "  mlx_comm_mode:       0x%X (%u)", param_config->mlx_comm_mode, param_config->mlx_comm_mode);
    ESP_LOGD(TAG, "  mlx_tcmp_en:         0x%X (%u)", param_config->mlx_tcmp_en, param_config->mlx_tcmp_en);
    ESP_LOGD(TAG, "  mlx_hallconf:        0x%X (%u)", param_config->mlx_hallconf, param_config->mlx_hallconf);
    ESP_LOGD(TAG, "  mlx_gain_sel:        0x%X (%u)", param_config->mlx_gain_sel, param_config->mlx_gain_sel);
    ESP_LOGD(TAG, "  mlx_osr:             0x%X (%u)", param_config->mlx_osr, param_config->mlx_osr);
    ESP_LOGD(TAG, "  mlx_dig_filt:        0x%X (%u)", param_config->mlx_dig_filt, param_config->mlx_dig_filt);
    ESP_LOGD(TAG, "  mlx_res_x:           0x%X (%u)", param_config->mlx_res_x, param_config->mlx_res_x);
    ESP_LOGD(TAG, "  mlx_res_y:           0x%X (%u)", param_config->mlx_res_y, param_config->mlx_res_y);
    ESP_LOGD(TAG, "  mlx_res_z            0x%X (%u)", param_config->mlx_res_z, param_config->mlx_res_z);
    ESP_LOGD(TAG, "  mlx_offset_x (uint16_t): 0x%" PRIX16 " (%" PRIu16")", param_config->mlx_offset_x, param_config->mlx_offset_x);
    ESP_LOGD(TAG, "  mlx_offset_y (uint16_t): 0x%" PRIX16 " (%" PRIu16")", param_config->mlx_offset_y, param_config->mlx_offset_y);
    ESP_LOGD(TAG, "  mlx_offset_z (uint16_t): 0x%" PRIX16 " (%" PRIu16")", param_config->mlx_offset_z, param_config->mlx_offset_z);

    return f_retval;
}

/*********************************************************************************
 * _log_int_pin_value()
 *
 *********************************************************************************/
static esp_err_t _log_int_pin_value(const mjd_mlx90393_config_t* param_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    ESP_LOGD(TAG, "The INT DRDY Data Ready pin value (GPIO#%u): %u", param_config->int_gpio_num, gpio_get_level(param_config->int_gpio_num));

    return f_retval;
}

/*********************************************************************************
 * _log_status_byte()
 *
 * @doc mjd_mlx90393_status_bit_t;
 *
 *********************************************************************************/
static esp_err_t _log_status_byte(uint8_t param_status) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    char status_binary_string[8 + 1] = "12345678";
    mjd_byte_to_binary_string(param_status, status_binary_string);
    ESP_LOGD(TAG, "STATUS BYTE: 0x%X (%hu) bin 0b%s", param_status, param_status, status_binary_string);

    ESP_LOGD(TAG, "  BURST_MODE_BIT?:              %u", (param_status & MJD_MLX90393_STATUS_BURST_MODE_BITMASK) != 0);
    ESP_LOGD(TAG, "  WAKE_ON_CHANGE_BIT?:          %u", (param_status & MJD_MLX90393_STATUS_WAKE_ON_CHANGE_MODE_BITMASK) != 0);
    ESP_LOGD(TAG, "  SINGLE_MEASUREMENT_MODE_BIT?: %u", (param_status & MJD_MLX90393_STATUS_SINGLE_MEASUREMENT_MODE_BITMASK) != 0);
    ESP_LOGD(TAG, "  ERROR_BIT?:                   %u", (param_status & MJD_MLX90393_STATUS_ERROR_BITMASK) != 0);
    ESP_LOGD(TAG, "  SED_BIT?:                     %u", (param_status & MJD_MLX90393_STATUS_SED_BITMASK) != 0);
    ESP_LOGD(TAG, "  RESET_BIT?:                   %u", (param_status & MJD_MLX90393_STATUS_RESET_BITMASK) != 0);
    ESP_LOGD(TAG, "  D1_BIT?:                      %u", (param_status & MJD_MLX90393_STATUS_D1_BITMASK) != 0);
    ESP_LOGD(TAG, "  D0_BIT?:                      %u", (param_status & MJD_MLX90393_STATUS_D0_BITMASK) != 0);

    return f_retval;
}

/*********************************************************************************
 * _log_data_raw()
 *
 * @doc mjd_mlx90393_data_raw_t;
 *
 *********************************************************************************/
static esp_err_t _log_data_raw(const mjd_mlx90393_data_raw_t* param_data_raw) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    ESP_LOGD(TAG, "data_raw X: 0x%X (%10u) | Y: 0x%X (%10u) | Z: 0x%X (%10u) | T: 0x%X (%10u)", param_data_raw->x, param_data_raw->x,
            param_data_raw->y, param_data_raw->y, param_data_raw->z, param_data_raw->z, param_data_raw->t, param_data_raw->t);

    return f_retval;
}

/*********************************************************************************
 * _send_cmd()
 *
 *  A command with no extra data to be sent and no extra data to be received (the ones which have, have separate methods!)
 *
 *********************************************************************************/
static esp_err_t _send_cmd(const mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_command_t param_command,
                           mjd_mlx90393_status_byte_t* param_ptr_status_byte) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    i2c_cmd_handle_t handle;

    // Send request & Receive response (STATUS BYTE)
    char command_binary_string[8 + 1] = "12345678";
    mjd_byte_to_binary_string(param_command, command_binary_string);
    ESP_LOGD(TAG, "Send request & Receive response (STATUS BYTE)");
    ESP_LOGD(TAG, "  command: 0x%X (%hu) bin 0b%s", param_command, param_command, command_binary_string);

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
    f_retval = i2c_master_write_byte(handle, param_command, true);
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
    f_retval = i2c_master_read_byte(handle, param_ptr_status_byte, I2C_MASTER_NACK); // last read must be I2C_MASTER_NACK
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

    f_retval = i2c_master_cmd_begin(param_ptr_config->i2c_port_num, handle, MJD_MLX90393_I2C_TIMEOUT_DEFAULT);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. Receive response i2c_master_cmd_begin() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        i2c_cmd_link_delete(handle);
        // GOTO
        goto cleanup;
    }

    i2c_cmd_link_delete(handle);

    // Process STATUS BYTE
    _log_status_byte(*param_ptr_status_byte);

    // Check status byte: error bit
    if ((*param_ptr_status_byte & MJD_MLX90393_STATUS_ERROR_BITMASK) != 0) {
        f_retval = ESP_FAIL;
        ESP_LOGE(TAG, "%s(). ABORT. The STATUS BYTE's ERROR BIT is set | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * _read_register()
 *
 *  @doc The RR command is used to read out the content of one specific address of the volatile RAM. The address ranges from 0 to 63.
 *       Remind that the last 32 addresses belong to the MLX area which cannot be changed by the user.
 *       Note that in the command, the address will be sent as if it is multiplied by 4 (<< 2).
 *
 *  @doc This func validates the status byte's error bit is not set.
 *
 *********************************************************************************/
static esp_err_t _read_register(const mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_reg_t param_reg,
                                mjd_mlx90393_status_byte_t* param_ptr_status_byte, uint16_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    i2c_cmd_handle_t handle;

    uint8_t msb = 0, lsb = 0;

    // Send request & Receive response (STATUS BYTE + REGDATA)
    char command_binary_string[8 + 1] = "12345678";
    mjd_byte_to_binary_string(MJD_MLX90393_CMD_READ_REGISTER, command_binary_string);
    ESP_LOGD(TAG, "Send request & Receive response (STATUS BYTE + REGDATA)");
    ESP_LOGD(TAG, "  command:  0x%X (%hu) bin 0b%s", MJD_MLX90393_CMD_READ_REGISTER, MJD_MLX90393_CMD_READ_REGISTER, command_binary_string);
    ESP_LOGD(TAG, "  register: 0x%X (%u)", param_reg, param_reg);

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
    f_retval = i2c_master_write_byte(handle, MJD_MLX90393_CMD_READ_REGISTER, true);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. Send request i2c_master_write_byte() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        i2c_cmd_link_delete(handle);
        // GOTO
        goto cleanup;
    }
    f_retval = i2c_master_write_byte(handle, param_reg << 2, true);
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
    f_retval = i2c_master_read_byte(handle, param_ptr_status_byte, I2C_MASTER_ACK);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. Receive response i2c_master_read_byte() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        i2c_cmd_link_delete(handle);
        // GOTO
        goto cleanup;
    }
    f_retval = i2c_master_read_byte(handle, &msb, I2C_MASTER_ACK);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. Receive response i2c_master_read_byte() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        i2c_cmd_link_delete(handle);
        // GOTO
        goto cleanup;
    }
    f_retval = i2c_master_read_byte(handle, &lsb, I2C_MASTER_NACK); // last read must be I2C_MASTER_NACK
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

    f_retval = i2c_master_cmd_begin(param_ptr_config->i2c_port_num, handle, MJD_MLX90393_I2C_TIMEOUT_DEFAULT);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. Receive response i2c_master_cmd_begin() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        i2c_cmd_link_delete(handle);
        // GOTO
        goto cleanup;
    }

    i2c_cmd_link_delete(handle);

    // Process STATUS BYTE
    _log_status_byte(*param_ptr_status_byte);

    // Check status byte: error bit
    if ((*param_ptr_status_byte & MJD_MLX90393_STATUS_ERROR_BITMASK) != 0) {
        f_retval = ESP_FAIL;
        ESP_LOGE(TAG, "%s(). ABORT. The STATUS BYTE's ERROR BIT is set | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Process REGDATA
    *param_ptr_data = (((uint16_t) msb << 8) | (uint16_t) lsb);
    ESP_LOGD(TAG, "%s(). Log REGDATA: 0x%X (%u)", __FUNCTION__, *param_ptr_data, *param_ptr_data);

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * _write_register()
 *
 *  @doc The WR command is used to write directly in the volatile RAM.
 *       The address ranges from 0 to 63. Remind that the last 32 addresses are MLX area which cannot be changed by the user.
 *       The reg address will be sent as if it is multiplied by 4 (<< 2).
 *       Always full words will be written. Words 0x00 to 0x09 are used to store operating parameters.
 *       Words 0x0A to 0x1F are free and usable to store any other data.
 *
 * @param param_data is an unsigned int (not signed!).
 *
 *  @doc This func validates the status byte's error bit (not set).
 *
 *********************************************************************************/
static esp_err_t _write_register(const mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_reg_t param_reg, uint16_t param_data,
                                 mjd_mlx90393_status_byte_t* param_ptr_status_byte) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    i2c_cmd_handle_t handle;

    // Send request & Receive response (STATUS BYTE)
    char command_binary_string[8 + 1] = "12345678";
    mjd_byte_to_binary_string(MJD_MLX90393_CMD_WRITE_REGISTER, command_binary_string);
    ESP_LOGD(TAG, "Send request & Receive response (STATUS BYTE)");
    ESP_LOGD(TAG, "  command:  0x%X (%hu) bin 0b%s", MJD_MLX90393_CMD_WRITE_REGISTER, MJD_MLX90393_CMD_WRITE_REGISTER, command_binary_string);
    ESP_LOGD(TAG, "  register: 0x%X (%u)", param_reg, param_reg);
    ESP_LOGD(TAG, "  data:     0x%X (%u)", param_data, param_data);

    /*
     ESP_LOGD(TAG, "  Storage size (bytes):");
     ESP_LOGD(TAG, "    command: %d | param_reg: %d | param_data: %d", sizeof(MJD_MLX90393_CMD_WRITE_REGISTER), sizeof(param_reg), sizeof(param_data));
     */

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
    f_retval = i2c_master_write_byte(handle, MJD_MLX90393_CMD_WRITE_REGISTER, true);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. Send request i2c_master_write_byte() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        i2c_cmd_link_delete(handle);
        // GOTO
        goto cleanup;
    }
    f_retval = i2c_master_write_byte(handle, param_data >> 8, true); // MSB
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. Send request i2c_master_write_byte() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        i2c_cmd_link_delete(handle);
        // GOTO
        goto cleanup;
    }
    f_retval = i2c_master_write_byte(handle, param_data & 0x00ff, true); // LSB
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. Send request i2c_master_write_byte() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        i2c_cmd_link_delete(handle);
        // GOTO
        goto cleanup;
    }
    f_retval = i2c_master_write_byte(handle, param_reg << 2, true); // LEFTSHFT 2
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
    f_retval = i2c_master_read_byte(handle, param_ptr_status_byte, I2C_MASTER_NACK); // last read must be I2C_MASTER_NACK
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

    f_retval = i2c_master_cmd_begin(param_ptr_config->i2c_port_num, handle, MJD_MLX90393_I2C_TIMEOUT_DEFAULT);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. Receive response i2c_master_cmd_begin() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        i2c_cmd_link_delete(handle);
        // GOTO
        goto cleanup;
    }

    i2c_cmd_link_delete(handle);

    // Process STATUS BYTE
    _log_status_byte(*param_ptr_status_byte);

    // Check status byte: error bit
    if ((*param_ptr_status_byte & MJD_MLX90393_STATUS_ERROR_BITMASK) != 0) {
        f_retval = ESP_FAIL;
        ESP_LOGE(TAG, "%s(). ABORT. The STATUS BYTE's ERROR BIT is set | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // (no data coming back)

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * mjd_mlx90393_init()
 *
 * @datasheet Suggested EEPROM Configuration (pg20)
 *      For rotary applications three EEPROM addresses may need to be modified. They control the gain, filter, and communication mode among others.
 *      Suggested EEPROM values are given below. These settings allow a sampling rate of 100Hz with minimal noise and a nominal airgap of 6mm.
 *
 *      Parameter           Value
 *      ----------------    -----
 *      ANA_RESERVED_LOW    0x0
 *      BIST                0x0
 *      Z_SERIES            0x0
 *      GAIN_SEL            0x5
 *      HALL_CONF           0xC
 *      TRIG_INT            0x0
 *      COMM_MODE           0x0
 *      WOC_DIFF            0x0
 *      EXT_TRIG            0x0
 *      TCMP_EN             0x0
 *      BURST_SEL           0x0
 *      BURST_DATA_RATE     0x0
 *      OSR2                0x0
 *      RES_XYZ             0x1
 *      DIG_FILT            0x5
 *      OSR                 0x0
 *
 * @doc Set these params to the following values:
 *      COMM_MODE: 0x0 => 0x3
 *      TCMP_EN:   0x0 => OK
 *      HALLCONF:  0xC => OK
 *      GAIN_SEL:  0x7 => OK
 *      RES_XYZ:   X= 0x0 (0) | Y= 0x0 (0) | Z= 0x0 (0) => OK
 *      DIG_FILT:  0x0 (0) => 0x7
 *      OSR:       0x0 (0) => 0x3
 *
 *********************************************************************************/
esp_err_t mjd_mlx90393_init(mjd_mlx90393_config_t* param_ptr_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    /*
     * BEFORE ACTION
     */
    _log_config(param_ptr_config);

    /*
     * I2C
     *
     * @important Can disable the MCU internal pullups for the SCL and SDA pins because the MLX90393 breakout board already contains a 10K pullup for each.
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
        i2c_conf.master.clk_speed = MJD_MLX90393_I2C_MASTER_FREQ_HZ;

        f_retval = i2c_param_config(param_ptr_config->i2c_port_num, &i2c_conf);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "%s(). ABORT. i2c_param_config() | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
            return f_retval; // EXIT
        }

        f_retval = i2c_driver_install(param_ptr_config->i2c_port_num, I2C_MODE_MASTER, MJD_MLX90393_I2C_MASTER_RX_BUF_DISABLE,
        MJD_MLX90393_I2C_MASTER_TX_BUF_DISABLE, MJD_MLX90393_I2C_MASTER_INTR_FLAG_NONE);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "%s(). ABORT. i2c_driver_install() | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
            return f_retval; // EXIT
        }
    }

    /*
     * INT DRDY Data Ready pin: GPIO & TIMER
     *   @rule -1 means not used to detect that a measurement is ready to be read.
     */
    if (param_ptr_config->int_gpio_num != -1) {
        // GPIO
        gpio_config_t io_conf =
            { 0 };
        io_conf.pin_bit_mask = (1ULL << param_ptr_config->int_gpio_num);
        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE; // @important
        io_conf.intr_type = GPIO_INTR_DISABLE;
        f_retval = gpio_config(&io_conf);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "%s(). ABORT. gpio_config() | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
            // GOTO
            goto cleanup;
        }
        ESP_LOGI(TAG, "%s(). I2C slave addr 0x%X (%u)", __FUNCTION__, param_ptr_config->int_gpio_num, param_ptr_config->int_gpio_num);
        _log_int_pin_value(param_ptr_config);

        // TIMER
        timer_config_t tconfig = {};
        tconfig.divider = 64000; // Let the timer tick on a relative slow pace. 1.25 Khz: esp_clk_apb_freq() / 64000 = 1250 ticks/second
        tconfig.counter_dir = TIMER_COUNT_UP;
        tconfig.counter_en = TIMER_PAUSE; // Pause when configured (do not start)
        tconfig.alarm_en = TIMER_ALARM_DIS;
        tconfig.auto_reload = false;
        f_retval = timer_init(MJD_MLX90393_TIMER_GROUP_ID, MJD_MLX90393_TIMER_ID, &tconfig);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "%s(). timer_init() err %d %s", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
            // GOTO
            goto cleanup;
        }
    } else {
        ESP_LOGI(TAG, "Melexis INT pin disabled by param_ptr_config");
    }

    /*
     * Commands
     */

    // @doc The exit command is used to force the IC into idle mode.
    // @important IF this command is not executed as the FIRST I2C command
    //            THEN the board must be hardware-reset (power-off-on) after every run ELSE the board does not respond anymore!
    f_retval = mjd_mlx90393_cmd_exit(param_ptr_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_mlx90393_cmd_exit() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Reset the device (this implicitly verifies that the I2C slave device is working properly)
    f_retval = mjd_mlx90393_cmd_reset(param_ptr_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_mlx90393_cmd_reset() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // DEVTEMP
    /////mjd_rtos_wait_forever();

    /*
     * Getting MLX NVRAM param values (some are read-only)
     */
    // SENS_TC_LT
    uint8_t sens_tc_lt;
    f_retval = mjd_mlx90393_get_sens_tc_lt(param_ptr_config, &sens_tc_lt);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_mlx90393_get_sens_tc_lt() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    ESP_LOGI(TAG, "%s(). Save in myconfig: get SENS_TC_LT: 0x%X (%u)", __FUNCTION__, sens_tc_lt, sens_tc_lt);
    param_ptr_config->mlx_sens_tc_lt = sens_tc_lt;

    // SENS_TC_HT
    uint8_t sens_tc_ht;
    f_retval = mjd_mlx90393_get_sens_tc_ht(param_ptr_config, &sens_tc_ht);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_mlx90393_get_sens_tc_ht() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    ESP_LOGI(TAG, "%s(). Save in myconfig: get SENS_TC_HT: 0x%X (%u)", __FUNCTION__, sens_tc_ht, sens_tc_ht);
    param_ptr_config->mlx_sens_tc_ht = sens_tc_ht;

    // TREF (16bit, storing this readonly register value for converting raw metrics to functional metrics
    uint16_t tref;
    f_retval = mjd_mlx90393_get_tref(param_ptr_config, &tref);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_mlx90393_get_tref() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    ESP_LOGI(TAG, "%s(). Save in myconfig: get TREF (uint16_t): 0x%" PRIX16 " (%" PRIu16")", __FUNCTION__, tref, tref);
    param_ptr_config->mlx_tref = tref;

    /*
     * Setting good parameter values
     */
    // COMM_MODE
    ESP_LOGI(TAG, "%s(). Set COMM_MODE: MJD_MLX90393_COMM_MODE_DEFAULT 0x%X (%u)", __FUNCTION__, MJD_MLX90393_COMM_MODE_DEFAULT,
            MJD_MLX90393_COMM_MODE_DEFAULT);
    f_retval = mjd_mlx90393_set_comm_mode(param_ptr_config, MJD_MLX90393_COMM_MODE_DEFAULT);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_mlx90393_set_comm_mode() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // TCMP_EN
    ESP_LOGI(TAG, "%s(). Set TCMP_EN: MJD_MLX90393_TCMP_EN_DEFAULT 0x%X (%u)", __FUNCTION__, MJD_MLX90393_TCMP_EN_DEFAULT,
            MJD_MLX90393_TCMP_EN_DEFAULT);
    f_retval = mjd_mlx90393_set_tcmp_en(param_ptr_config, MJD_MLX90393_TCMP_EN_DEFAULT);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_mlx90393_set_tcmp_en() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // HALLCONF
    ESP_LOGI(TAG, "%s(). Set HALLCONF: MJD_MLX90393_HALLCONF_DEFAULT 0x%X (%u)", __FUNCTION__, MJD_MLX90393_HALLCONF_DEFAULT,
            MJD_MLX90393_HALLCONF_DEFAULT);
    f_retval = mjd_mlx90393_set_hallconf(param_ptr_config, MJD_MLX90393_HALLCONF_DEFAULT);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_mlx90393_set_hallconf() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // GAIN_SEL
    ESP_LOGI(TAG, "%s(). Set GAIN_SEL: MJD_MLX90393_GAIN_SEL_DEFAULT 0x%X (%u)", __FUNCTION__, MJD_MLX90393_GAIN_SEL_DEFAULT,
            MJD_MLX90393_GAIN_SEL_DEFAULT);
    f_retval = mjd_mlx90393_set_gain_sel(param_ptr_config, MJD_MLX90393_GAIN_SEL_DEFAULT);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_mlx90393_set_gain_sel() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // OSR
    ESP_LOGI(TAG, "%s(). Set OSR: MJD_MLX90393_OSR_DEFAULT 0x%X (%u)", __FUNCTION__, MJD_MLX90393_OSR_DEFAULT, MJD_MLX90393_OSR_DEFAULT);
    f_retval = mjd_mlx90393_set_osr(param_ptr_config, MJD_MLX90393_OSR_DEFAULT);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_mlx90393_set_osr() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // DIG_FILT
    ESP_LOGI(TAG, "%s(). Set DIG_FILT: MJD_MLX90393_DIG_FILT_DEFAULT 0x%X (%u)", __FUNCTION__, MJD_MLX90393_DIG_FILT_DEFAULT,
            MJD_MLX90393_DIG_FILT_DEFAULT);
    f_retval = mjd_mlx90393_set_dig_filt(param_ptr_config, MJD_MLX90393_DIG_FILT_DEFAULT);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_mlx90393_set_dig_filt() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // RES_XYZ
    ESP_LOGI(TAG,
            "%s(). Set RES_XYZ: X= MJD_MLX90393_RES_XYZ_DEFAULT 0x%X (%u) | Y= MJD_MLX90393_RES_XYZ_DEFAULT 0x%X (%u) | Z= MJD_MLX90393_RES_XYZ_DEFAULT 0x%X (%u)",
            __FUNCTION__, MJD_MLX90393_RES_XYZ_DEFAULT, MJD_MLX90393_RES_XYZ_DEFAULT, MJD_MLX90393_RES_XYZ_DEFAULT, MJD_MLX90393_RES_XYZ_DEFAULT,
            MJD_MLX90393_RES_XYZ_DEFAULT, MJD_MLX90393_RES_XYZ_DEFAULT);
    f_retval = mjd_mlx90393_set_res_xyz(param_ptr_config, MJD_MLX90393_RES_XYZ_DEFAULT, MJD_MLX90393_RES_XYZ_DEFAULT, MJD_MLX90393_RES_XYZ_DEFAULT);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_mlx90393_set_res_xyz() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // OFFSET_X OFFSET_Y OFFSET_Z
    ESP_LOGI(TAG,
            "%s(). Set OFFSET_*: X= MJD_MLX90393_OFFSET_X_DEFAULT 0x%X (%u) | Y= MJD_MLX90393_OFFSET_Y_DEFAULT 0x%X (%u) | Z= MJD_MLX90393_OFFSET_Z_DEFAULT 0x%X (%u)",
            __FUNCTION__, MJD_MLX90393_OFFSET_X_DEFAULT, MJD_MLX90393_OFFSET_X_DEFAULT, MJD_MLX90393_OFFSET_Y_DEFAULT, MJD_MLX90393_OFFSET_Y_DEFAULT,
            MJD_MLX90393_OFFSET_Z_DEFAULT, MJD_MLX90393_OFFSET_Z_DEFAULT);
    f_retval = mjd_mlx90393_set_offset_xyz(param_ptr_config, MJD_MLX90393_OFFSET_X_DEFAULT, MJD_MLX90393_OFFSET_Y_DEFAULT,
            MJD_MLX90393_OFFSET_Z_DEFAULT);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_mlx90393_set_res_xyz() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
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
 * mjd_mlx90393_deinit()
 *
 *********************************************************************************/
esp_err_t mjd_mlx90393_deinit(const mjd_mlx90393_config_t* param_ptr_config) {
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
 * mjd_mlx90393_log_device_parameters()
 *
 * @doc mjd_mlx90393_config_t
 *
 *********************************************************************************/
esp_err_t mjd_mlx90393_log_device_parameters(const mjd_mlx90393_config_t* param_ptr_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    ESP_LOGI(TAG, "MLX90393 LOG DEVICE PARAMETERS (*READ AGAIN FROM REGISTERS*):");

    // COMM_MODE
    mjd_mlx90393_comm_mode_t comm_mode;
    f_retval = mjd_mlx90393_get_comm_mode(param_ptr_config, &comm_mode);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_mlx90393_get_comm_mode() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    ESP_LOGI(TAG, "  COMM_MODE: 0x%X (%u)", comm_mode, comm_mode);

    // TCMP_EN
    mjd_mlx90393_tcmp_en_t tcmp_en;
    f_retval = mjd_mlx90393_get_tcmp_en(param_ptr_config, &tcmp_en);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_mlx90393_get_tcmp_en() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    ESP_LOGI(TAG, "  TCMP_EN: 0x%X (%u)", tcmp_en, tcmp_en);

    // HALLCONF
    mjd_mlx90393_hallconf_t hallconf;
    f_retval = mjd_mlx90393_get_hallconf(param_ptr_config, &hallconf);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_mlx90393_get_hallconf() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    ESP_LOGI(TAG, "  HALLCONF: 0x%X (%u)", hallconf, hallconf);

    // GAIN_SEL
    mjd_mlx90393_gain_sel_t gain_sel;
    f_retval = mjd_mlx90393_get_gain_sel(param_ptr_config, &gain_sel);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_mlx90393_get_gain_sel() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    ESP_LOGI(TAG, "  GAIN_SEL: 0x%X (%u)", gain_sel, gain_sel);

    // Z_SERIES Only get()
    mjd_mlx90393_z_series_t z_series;
    f_retval = mjd_mlx90393_get_z_series(param_ptr_config, &z_series);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_mlx90393_get_z_series() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    ESP_LOGI(TAG, "  Z_SERIES: 0x%X (%u)", z_series, z_series);

    // BIST Only get()
    mjd_mlx90393_bist_t bist;
    f_retval = mjd_mlx90393_get_bist(param_ptr_config, &bist);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_mlx90393_get_bist() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    ESP_LOGI(TAG, "  BIST: 0x%X (%u)", bist, bist);

    // EXT_TRIG Only get()
    mjd_mlx90393_ext_trig_t ext_trig;
    f_retval = mjd_mlx90393_get_ext_trig(param_ptr_config, &ext_trig);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_mlx90393_get_ext_trig() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    ESP_LOGI(TAG, "  EXT_TRIG: 0x%X (%u)", ext_trig, ext_trig);

    // TRIG_INT_SEL Only get()
    mjd_mlx90393_trig_int_sel_t trig_int_sel;
    f_retval = mjd_mlx90393_get_trig_int_sel(param_ptr_config, &trig_int_sel);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_mlx90393_get_trig_int_sel() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    ESP_LOGI(TAG, "  TRIG_INT_SEL: 0x%X (%u)", trig_int_sel, trig_int_sel);

    // OSR
    mjd_mlx90393_osr_t osr;
    f_retval = mjd_mlx90393_get_osr(param_ptr_config, &osr);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_mlx90393_get_osr() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    ESP_LOGI(TAG, "  OSR: 0x%X (%u)", osr, osr);

    // DIG_FILT
    mjd_mlx90393_dig_filt_t dig_filt;
    f_retval = mjd_mlx90393_get_dig_filt(param_ptr_config, &dig_filt);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_mlx90393_get_dig_filt() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    ESP_LOGI(TAG, "  DIG_FILT: 0x%X (%u)", dig_filt, dig_filt);

    // RES_XYZ
    mjd_mlx90393_res_xyz_t res_x, res_y, res_z;
    f_retval = mjd_mlx90393_get_res_xyz(param_ptr_config, &res_x, &res_y, &res_z);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_mlx90393_get_res_xyz() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    ESP_LOGI(TAG, "  RES_XYZ: X= 0x%X (%u) | Y= 0x%X (%u) | Z= 0x%X (%u)", res_x, res_x, res_y, res_y, res_z, res_z);

    // SENS_TC_LT Only get()
    uint8_t sens_tc_lt;
    f_retval = mjd_mlx90393_get_sens_tc_lt(param_ptr_config, &sens_tc_lt);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_mlx90393_get_sens_tc_lt() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    ESP_LOGI(TAG, "  SENS_TC_LT: 0x%X (%u)", sens_tc_lt, sens_tc_lt);

    // SENS_TC_HT Only get()
    uint8_t sens_tc_ht;
    f_retval = mjd_mlx90393_get_sens_tc_ht(param_ptr_config, &sens_tc_ht);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_mlx90393_get_sens_tc_ht() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    ESP_LOGI(TAG, "  SENS_TC_HT: 0x%X (%u)", sens_tc_ht, sens_tc_ht);

    // OFFSET_X OFFSET_Y OFFSET_Z Only get()
    uint16_t offset_x, offset_y, offset_z;
    f_retval = mjd_mlx90393_get_offset_xyz(param_ptr_config, &offset_x, &offset_y, &offset_z);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_mlx90393_get_offset_xyz() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    ESP_LOGI(TAG, "  OFFSET_*: X= 0x%X (%u) | Y= 0x%X (%u) | Z= 0x%X (%u)", offset_x, offset_x, offset_y, offset_y, offset_z, offset_z);

    // TREF
    uint16_t tref;
    f_retval = mjd_mlx90393_get_tref(param_ptr_config, &tref);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. mjd_mlx90393_get_tref() failed | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    ESP_LOGI(TAG, "  TREF: 0x%X (%u)", tref, tref);

    // DEVTEMP
    /////mjd_rtos_wait_forever();

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * CMD reset.
 *
 * @doc This command is used to reset the IC. On reset, the idle mode will be entered again.
 *      The status byte will reflect that the reset has been successful.
 *
 *********************************************************************************/
esp_err_t mjd_mlx90393_cmd_reset(const mjd_mlx90393_config_t* param_ptr_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    mjd_mlx90393_status_byte_t status_byte = 0;

    f_retval = _send_cmd(param_ptr_config, MJD_MLX90393_CMD_RESET, &status_byte);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // SPECIFIC: Check status byte: reset bit must be set
    if ((status_byte & MJD_MLX90393_STATUS_RESET_BITMASK) == 0) {
        f_retval = ESP_FAIL;
        ESP_LOGE(TAG, "%s(). ABORT. The STATUS BYTE's RESET BIT is not set | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // @important Delay. Data sheet "TPOR Power-on-reset completion time = 1.5 millisec"
    _delay_millisec(5);

    // INFORM
    ESP_LOGI(TAG, "%s(). The device has been reset :)", __FUNCTION__);
    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * CMD exit.
 *
 * @doc The exit command is used to force the IC into idle mode. The command shall be used to exit the burst mode and the wake-up-on-change mode.
 *
 * @doc During a single measurement in polling mode, the command is REJECTED. The status byte will then show an error and indicate that the IC is in polling mode (SM_mode).
 *
 *********************************************************************************/
esp_err_t mjd_mlx90393_cmd_exit(const mjd_mlx90393_config_t* param_ptr_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    mjd_mlx90393_status_byte_t status_byte = 0;

    f_retval = _send_cmd(param_ptr_config, MJD_MLX90393_CMD_EXIT_MODE, &status_byte);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _send_cmd() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // SPECIFIC: Check status byte: *NO checks needed, however... the error bit is always checked in the _send_cmd()

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * Get COMM_MODE
 *
 * @doc mjd_mlx90393_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_mlx90393_get_comm_mode(const mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_comm_mode_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    mjd_mlx90393_status_byte_t status;
    uint16_t reg_data; // word

    f_retval = _read_register(param_ptr_config, MJD_MLX90393_COMM_MODE_REG, &status, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Extract parameter value from register data
    *param_ptr_data = (reg_data & MJD_MLX90393_COMM_MODE_BITMASK) >> MJD_MLX90393_COMM_MODE_BITSHIFT;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * Get TCMP_EN
 *
 * @doc mjd_mlx90393_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_mlx90393_get_tcmp_en(const mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_tcmp_en_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    mjd_mlx90393_status_byte_t status;
    uint16_t reg_data; // word

    f_retval = _read_register(param_ptr_config, MJD_MLX90393_TCMP_EN_REG, &status, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Extract parameter value from register data
    *param_ptr_data = (reg_data & MJD_MLX90393_TCMP_EN_BITMASK) >> MJD_MLX90393_TCMP_EN_BITSHIFT;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * Get HALLCONF
 *
 * @doc mjd_mlx90393_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_mlx90393_get_hallconf(const mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_hallconf_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    mjd_mlx90393_status_byte_t status;
    uint16_t reg_data; // word

    f_retval = _read_register(param_ptr_config, MJD_MLX90393_HALLCONF_REG, &status, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Extract parameter value from register data
    *param_ptr_data = (reg_data & MJD_MLX90393_HALLCONF_BITMASK) >> MJD_MLX90393_HALLCONF_BITSHIFT;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * Get GAIN_SEL
 *
 * @doc mjd_mlx90393_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_mlx90393_get_gain_sel(const mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_gain_sel_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    mjd_mlx90393_status_byte_t status;
    uint16_t reg_data; // word

    f_retval = _read_register(param_ptr_config, MJD_MLX90393_GAIN_SEL_REG, &status, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Extract parameter value from register data
    *param_ptr_data = (reg_data & MJD_MLX90393_GAIN_SEL_BITMASK) >> MJD_MLX90393_GAIN_SEL_BITSHIFT;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * Get Z_SERIES
 *
 * @doc mjd_mlx90393_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_mlx90393_get_z_series(const mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_z_series_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    mjd_mlx90393_status_byte_t status;
    uint16_t reg_data; // word

    f_retval = _read_register(param_ptr_config, MJD_MLX90393_Z_SERIES_REG, &status, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Extract parameter value from register data
    *param_ptr_data = (reg_data & MJD_MLX90393_Z_SERIES_BITMASK) >> MJD_MLX90393_Z_SERIES_BITSHIFT;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * Get BIST
 *
 * @doc mjd_mlx90393_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_mlx90393_get_bist(const mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_bist_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    mjd_mlx90393_status_byte_t status;
    uint16_t reg_data; // word

    f_retval = _read_register(param_ptr_config, MJD_MLX90393_BIST_REG, &status, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Extract parameter value from register data
    *param_ptr_data = (reg_data & MJD_MLX90393_BIST_BITMASK) >> MJD_MLX90393_BIST_BITSHIFT;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * Get EXT_TRIG
 *
 * @doc mjd_mlx90393_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_mlx90393_get_ext_trig(const mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_ext_trig_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    mjd_mlx90393_status_byte_t status;
    uint16_t reg_data; // word

    f_retval = _read_register(param_ptr_config, MJD_MLX90393_EXT_TRIG_REG, &status, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Extract parameter value from register data
    *param_ptr_data = (reg_data & MJD_MLX90393_EXT_TRIG_BITMASK) >> MJD_MLX90393_EXT_TRIG_BITSHIFT;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * Get TRIG_INT_SEL
 *
 * @doc mjd_mlx90393_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_mlx90393_get_trig_int_sel(const mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_trig_int_sel_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    mjd_mlx90393_status_byte_t status;
    uint16_t reg_data; // word

    f_retval = _read_register(param_ptr_config, MJD_MLX90393_TRIG_INT_SEL_REG, &status, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Extract parameter value from register data
    *param_ptr_data = (reg_data & MJD_MLX90393_TRIG_INT_SEL_BITMASK) >> MJD_MLX90393_TRIG_INT_SEL_BITSHIFT;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * Get OSR
 *
 * @doc mjd_mlx90393_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_mlx90393_get_osr(const mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_osr_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    mjd_mlx90393_status_byte_t status;
    uint16_t reg_data; // word

    f_retval = _read_register(param_ptr_config, MJD_MLX90393_OSR_REG, &status, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Extract parameter value from register data
    *param_ptr_data = (reg_data & MJD_MLX90393_OSR_BITMASK) >> MJD_MLX90393_OSR_BITSHIFT;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * Get DIG_FILT
 *
 * @doc mjd_mlx90393_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_mlx90393_get_dig_filt(const mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_dig_filt_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    mjd_mlx90393_status_byte_t status;
    uint16_t reg_data; // word

    f_retval = _read_register(param_ptr_config, MJD_MLX90393_DIG_FILT_REG, &status, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Extract parameter value from register data
    *param_ptr_data = (reg_data & MJD_MLX90393_DIG_FILT_BITMASK) >> MJD_MLX90393_DIG_FILT_BITSHIFT;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * Get RES_XYZ
 *
 * @doc mjd_mlx90393_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_mlx90393_get_res_xyz(const mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_res_xyz_t* param_x, mjd_mlx90393_res_xyz_t* param_y,
                                   mjd_mlx90393_res_xyz_t* param_z) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    mjd_mlx90393_status_byte_t status;
    uint16_t reg_data; // word

    f_retval = _read_register(param_ptr_config, MJD_MLX90393_RES_XYZ_REG, &status, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Extract parameter value from register data
    uint8_t res_zyx = (reg_data & MJD_MLX90393_RES_XYZ_BITMASK) >> MJD_MLX90393_RES_XYZ_BITSHIFT;

    // Extract X Y Z 2-bit values from parameter value
    *param_x = (res_zyx >> 0) & 0x3;
    *param_y = (res_zyx >> 2) & 0x3;
    *param_z = (res_zyx >> 4) & 0x3;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * Get SENS_TC_LT
 *
 * @doc mjd_mlx90393_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_mlx90393_get_sens_tc_lt(const mjd_mlx90393_config_t* param_ptr_config, uint8_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    mjd_mlx90393_status_byte_t status;
    uint16_t reg_data; // word

    f_retval = _read_register(param_ptr_config, MJD_MLX90393_SENS_TC_LT_REG, &status, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Extract parameter value from register data
    *param_ptr_data = (reg_data & MJD_MLX90393_SENS_TC_LT_BITMASK) >> MJD_MLX90393_SENS_TC_LT_BITSHIFT;

    // Adjust with extra drift due to the QFN packaging (data sheet)
    *param_ptr_data -= 6;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * Get SENS_TC_HT
 *
 * @doc mjd_mlx90393_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_mlx90393_get_sens_tc_ht(const mjd_mlx90393_config_t* param_ptr_config, uint8_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    mjd_mlx90393_status_byte_t status;
    uint16_t reg_data; // word

    f_retval = _read_register(param_ptr_config, MJD_MLX90393_SENS_TC_HT_REG, &status, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Extract parameter value from register data
    *param_ptr_data = (reg_data & MJD_MLX90393_SENS_TC_HT_BITMASK) >> MJD_MLX90393_SENS_TC_HT_BITSHIFT;

    // Adjust with extra drift due to the QFN packaging (data sheet)
    *param_ptr_data -= 15;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * Get OFFSET_X OFFSET_Y OFFSET_Z
 *
 * @doc mjd_mlx90393_defs.h
 *
 *********************************************************************************/
esp_err_t mjd_mlx90393_get_offset_xyz(const mjd_mlx90393_config_t* param_ptr_config, uint16_t* param_x, uint16_t* param_y, uint16_t* param_z) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    mjd_mlx90393_status_byte_t status;
    uint16_t reg_data; // word

    // X
    f_retval = _read_register(param_ptr_config, MJD_MLX90393_OFFSET_X_REG, &status, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    *param_x = (reg_data & MJD_MLX90393_OFFSET_X_BITMASK) >> MJD_MLX90393_OFFSET_X_BITSHIFT;

    // Y
    f_retval = _read_register(param_ptr_config, MJD_MLX90393_OFFSET_Y_REG, &status, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    *param_y = (reg_data & MJD_MLX90393_OFFSET_Y_BITMASK) >> MJD_MLX90393_OFFSET_Y_BITSHIFT;

    // Z
    f_retval = _read_register(param_ptr_config, MJD_MLX90393_OFFSET_Z_REG, &status, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    *param_z = (reg_data & MJD_MLX90393_OFFSET_Z_BITMASK) >> MJD_MLX90393_OFFSET_Z_BITSHIFT;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * Get TREF
 *
 * @doc mjd_mlx90393_defs.h
 *
 */
esp_err_t mjd_mlx90393_get_tref(const mjd_mlx90393_config_t* param_ptr_config, uint16_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    mjd_mlx90393_status_byte_t status;
    uint16_t reg_data; // word

    f_retval = _read_register(param_ptr_config, MJD_MLX90393_TREF_THRESHOLD_REG, &status, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Extract parameter value from register data
    *param_ptr_data = (reg_data & MJD_MLX90393_TREF_THRESHOLD_BITMASK) >> MJD_MLX90393_TREF_THRESHOLD_BITSHIFT;

    // LABEL
    cleanup: ;

    return f_retval;
}

/**********
 * SET functions
 */

/*********************************************************************************
 * Set COMM_MODE
 *
 *********************************************************************************/
esp_err_t mjd_mlx90393_set_comm_mode(mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_comm_mode_t param_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    mjd_mlx90393_status_byte_t status;
    uint16_t reg_data; // word

    // READ
    f_retval = _read_register(param_ptr_config, MJD_MLX90393_COMM_MODE_REG, &status, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Inject new data
    ESP_LOGD(TAG, "%s(). CUR REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);
    reg_data = (reg_data & ~MJD_MLX90393_COMM_MODE_BITMASK) | ((param_data << MJD_MLX90393_COMM_MODE_BITSHIFT) & MJD_MLX90393_COMM_MODE_BITMASK);
    ESP_LOGD(TAG, "%s(). NEW REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);

    // WRITE
    f_retval = _write_register(param_ptr_config, MJD_MLX90393_COMM_MODE_REG, reg_data, &status);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _write_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // SAVE IN CONFIG STRUCT
    param_ptr_config->mlx_comm_mode = param_data;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * Set TCMP_EN
 *
 *********************************************************************************/
esp_err_t mjd_mlx90393_set_tcmp_en(mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_tcmp_en_t param_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    mjd_mlx90393_status_byte_t status;
    uint16_t reg_data; // word

    // Input params
    if (param_data == MJD_MLX90393_TCMP_EN_ENABLED) {
        f_retval = ESP_ERR_INVALID_ARG;
        ESP_LOGE(TAG, "%s(). ABORT The setting TCMP_EN=1 (temperature compensation enabled) is not supported | err %i (%s)", __FUNCTION__, f_retval,
                esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // READ
    f_retval = _read_register(param_ptr_config, MJD_MLX90393_TCMP_EN_REG, &status, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Inject new data
    ESP_LOGD(TAG, "%s(). CUR REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);
    reg_data = (reg_data & ~MJD_MLX90393_TCMP_EN_BITMASK) | ((param_data << MJD_MLX90393_TCMP_EN_BITSHIFT) & MJD_MLX90393_TCMP_EN_BITMASK);
    ESP_LOGD(TAG, "%s(). NEW REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);

    // WRITE
    f_retval = _write_register(param_ptr_config, MJD_MLX90393_TCMP_EN_REG, reg_data, &status);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _write_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // SAVE IN CONFIG STRUCT
    param_ptr_config->mlx_tcmp_en = param_data;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * Set HALLCONF
 *
 *********************************************************************************/
esp_err_t mjd_mlx90393_set_hallconf(mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_hallconf_t param_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    mjd_mlx90393_status_byte_t status;
    uint16_t reg_data; // word

    // Input params
    if (param_data == MJD_MLX90393_HALLCONF_0) {
        f_retval = ESP_ERR_INVALID_ARG;
        ESP_LOGE(TAG,
                "%s(). ABORT The setting MJD_MLX90393_HALLCONF=0x0 is not supported (it is never used in reality). Always use HallConf=0xC | err %i (%s)",
                __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // READ
    f_retval = _read_register(param_ptr_config, MJD_MLX90393_HALLCONF_REG, &status, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Inject new data
    ESP_LOGD(TAG, "%s(). CUR REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);
    reg_data = (reg_data & ~MJD_MLX90393_HALLCONF_BITMASK) | ((param_data << MJD_MLX90393_HALLCONF_BITSHIFT) & MJD_MLX90393_HALLCONF_BITMASK);
    ESP_LOGD(TAG, "%s(). NEW REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);

    // WRITE
    f_retval = _write_register(param_ptr_config, MJD_MLX90393_HALLCONF_REG, reg_data, &status);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _write_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // SAVE IN CONFIG STRUCT
    param_ptr_config->mlx_hallconf = param_data;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * Set GAIN_SEL
 *
 *********************************************************************************/
esp_err_t mjd_mlx90393_set_gain_sel(mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_gain_sel_t param_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    mjd_mlx90393_status_byte_t status;
    uint16_t reg_data; // word

    // READ
    f_retval = _read_register(param_ptr_config, MJD_MLX90393_GAIN_SEL_REG, &status, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Inject new data
    ESP_LOGD(TAG, "%s(). CUR REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);
    reg_data = (reg_data & ~MJD_MLX90393_GAIN_SEL_BITMASK) | ((param_data << MJD_MLX90393_GAIN_SEL_BITSHIFT) & MJD_MLX90393_GAIN_SEL_BITMASK);
    ESP_LOGD(TAG, "%s(). NEW REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);

    // WRITE
    f_retval = _write_register(param_ptr_config, MJD_MLX90393_GAIN_SEL_REG, reg_data, &status);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _write_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // SAVE IN CONFIG STRUCT
    param_ptr_config->mlx_gain_sel = param_data;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * Set OSR
 *
 *********************************************************************************/
esp_err_t mjd_mlx90393_set_osr(mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_osr_t param_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    mjd_mlx90393_status_byte_t status;
    uint16_t reg_data; // word

    // READ
    f_retval = _read_register(param_ptr_config, MJD_MLX90393_OSR_REG, &status, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Inject new data
    ESP_LOGD(TAG, "%s(). CUR REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);
    reg_data = (reg_data & ~MJD_MLX90393_OSR_BITMASK) | ((param_data << MJD_MLX90393_OSR_BITSHIFT) & MJD_MLX90393_OSR_BITMASK);
    ESP_LOGD(TAG, "%s(). NEW REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);

    // WRITE
    f_retval = _write_register(param_ptr_config, MJD_MLX90393_OSR_REG, reg_data, &status);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _write_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // SAVE IN CONFIG STRUCT
    param_ptr_config->mlx_osr = param_data;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * Set DIG_FILT
 *
 *********************************************************************************/
esp_err_t mjd_mlx90393_set_dig_filt(mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_dig_filt_t param_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    mjd_mlx90393_status_byte_t status;
    uint16_t reg_data; // word

    // READ
    f_retval = _read_register(param_ptr_config, MJD_MLX90393_DIG_FILT_REG, &status, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Inject new data
    ESP_LOGD(TAG, "%s(). CUR REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);
    reg_data = (reg_data & ~MJD_MLX90393_DIG_FILT_BITMASK) | ((param_data << MJD_MLX90393_DIG_FILT_BITSHIFT) & MJD_MLX90393_DIG_FILT_BITMASK);
    ESP_LOGD(TAG, "%s(). NEW REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);

    // WRITE
    f_retval = _write_register(param_ptr_config, MJD_MLX90393_DIG_FILT_REG, reg_data, &status);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _write_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // SAVE IN CONFIG STRUCT
    param_ptr_config->mlx_dig_filt = param_data;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * Set RES_XYZ
 *
 *********************************************************************************/
esp_err_t mjd_mlx90393_set_res_xyz(mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_res_xyz_t param_res_x, mjd_mlx90393_res_xyz_t param_res_y,
                                   mjd_mlx90393_res_xyz_t param_res_z) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    mjd_mlx90393_status_byte_t status;
    uint16_t reg_data, zyx_data; // word

    // READ
    f_retval = _read_register(param_ptr_config, MJD_MLX90393_RES_XYZ_REG, &status, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Combine the Z Y X 2-bit values from the xyz input params
    zyx_data = ((param_res_z & 0x3) << 4) | ((param_res_y & 0x3) << 2) | ((param_res_x & 0x3) << 0);

    // Inject new data
    ESP_LOGD(TAG, "%s(). CUR REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);
    reg_data = (reg_data & ~MJD_MLX90393_RES_XYZ_BITMASK) | ((zyx_data << MJD_MLX90393_RES_XYZ_BITSHIFT) & MJD_MLX90393_RES_XYZ_BITMASK);
    ESP_LOGD(TAG, "%s(). NEW REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);

    // WRITE
    f_retval = _write_register(param_ptr_config, MJD_MLX90393_RES_XYZ_REG, reg_data, &status);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _write_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // SAVE IN CONFIG STRUCT
    param_ptr_config->mlx_res_x = param_res_x;
    param_ptr_config->mlx_res_y = param_res_y;
    param_ptr_config->mlx_res_z = param_res_z;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * Set OFFSET_X OFFSET_Y OFFSET_Z
 *
 *********************************************************************************/
esp_err_t mjd_mlx90393_set_offset_xyz(mjd_mlx90393_config_t* param_ptr_config, uint16_t param_offset_x, uint16_t param_offset_y,
                                      uint16_t param_offset_z) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    mjd_mlx90393_status_byte_t status;
    uint16_t reg_data; // word

    /*
     * OFFSET_X
     */
    // READ
    f_retval = _read_register(param_ptr_config, MJD_MLX90393_OFFSET_X_REG, &status, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Inject new data
    ESP_LOGD(TAG, "%s(). CUR REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);
    reg_data = (reg_data & ~MJD_MLX90393_OFFSET_X_BITMASK) | ((param_offset_x << MJD_MLX90393_OFFSET_X_BITSHIFT) & MJD_MLX90393_OFFSET_X_BITMASK);
    ESP_LOGD(TAG, "%s(). NEW REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);

    // WRITE
    f_retval = _write_register(param_ptr_config, MJD_MLX90393_OFFSET_X_REG, reg_data, &status);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _write_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /*
     * OFFSET_Y
     */
    // READ
    f_retval = _read_register(param_ptr_config, MJD_MLX90393_OFFSET_Y_REG, &status, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Inject new data
    ESP_LOGD(TAG, "%s(). CUR REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);
    reg_data = (reg_data & ~MJD_MLX90393_OFFSET_Y_BITMASK) | ((param_offset_y << MJD_MLX90393_OFFSET_Y_BITSHIFT) & MJD_MLX90393_OFFSET_Y_BITMASK);
    ESP_LOGD(TAG, "%s(). NEW REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);

    // WRITE
    f_retval = _write_register(param_ptr_config, MJD_MLX90393_OFFSET_Y_REG, reg_data, &status);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _write_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /*
     * OFFSET_Z
     */
    // READ
    f_retval = _read_register(param_ptr_config, MJD_MLX90393_OFFSET_Z_REG, &status, &reg_data);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _read_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Inject new data
    ESP_LOGD(TAG, "%s(). CUR REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);
    reg_data = (reg_data & ~MJD_MLX90393_OFFSET_Z_BITMASK) | ((param_offset_z << MJD_MLX90393_OFFSET_Z_BITSHIFT) & MJD_MLX90393_OFFSET_Z_BITMASK);
    ESP_LOGD(TAG, "%s(). NEW REGDATA: 0x%X (%u)", __FUNCTION__, reg_data, reg_data);

    // WRITE
    f_retval = _write_register(param_ptr_config, MJD_MLX90393_OFFSET_Z_REG, reg_data, &status);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _write_register() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /*
     * SAVE IN CONFIG STRUCT
     */
    param_ptr_config->mlx_offset_x = param_offset_x;
    param_ptr_config->mlx_offset_y = param_offset_y;
    param_ptr_config->mlx_offset_z = param_offset_z;

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * CMD Start Measurement.
 * @doc The single measurement command is used to instruct the MLX90393 to perform an acquisition cycle.
 *      The low nibble contains flags for each metric: z, y, x, and t.
 *      These four bits determine which axes will be converted whenever they are set to a 1.
 *
 *********************************************************************************/
esp_err_t mjd_mlx90393_cmd_start_measurement(const mjd_mlx90393_config_t* param_ptr_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    mjd_mlx90393_status_byte_t status;

    // Log INT pin value
    _log_int_pin_value(param_ptr_config);

    // Convert config metrics flags to the command's LSNibble syntax
    uint8_t zyxt_nibble = 0;
    if (param_ptr_config->mlx_metrics_selector.x_axis == true) {
        zyxt_nibble |= MJD_MLX90393_METRIC_X_AXIS_BITMASK;
    }
    if (param_ptr_config->mlx_metrics_selector.y_axis == true) {
        zyxt_nibble |= MJD_MLX90393_METRIC_Y_AXIS_BITMASK;
    }
    if (param_ptr_config->mlx_metrics_selector.z_axis == true) {
        zyxt_nibble |= MJD_MLX90393_METRIC_Z_AXIS_BITMASK;
    }
    if (param_ptr_config->mlx_metrics_selector.temperature == true) {
        zyxt_nibble |= MJD_MLX90393_METRIC_TEMPERATURE_BITMASK;
    }

    f_retval = _send_cmd(param_ptr_config, MJD_MLX90393_CMD_START_SINGLE_MEASUREMENT_MODE | zyxt_nibble, &status);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT _send_cmd() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // SPECIFIC: Check status byte: the special SM bit
    if ((status & MJD_MLX90393_STATUS_SINGLE_MEASUREMENT_MODE_BITMASK) == 0) {
        f_retval = ESP_FAIL;
        ESP_LOGE(TAG, "%s(). ABORT. The STATUS BYTE's SINGLE_MEASUREMENT_MODE_BITMASK is not set | err %i (%s)", __FUNCTION__, f_retval,
                esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /*
     * THE DEVICE NEEDS TIME TO COLLECT AND SET READY ITS MEASUREMENTS
     * Wait for measurement ready. Either using the INT DRDY Data Ready pin or a long fixed waiting time
     * @rule int_gpio_num = -1 means the pin is not used to detect that a measurement is ready to be read.
     *
     * When not using the INT DRDY Data Ready pin: 'worst case', when all parameters are max and ZYXT equals 0b1111, the conversion time is 198.5ms.
     *
     */
    if (param_ptr_config->int_gpio_num == -1) {
        // @important A LONG SPECIFIC FIXED DELAY.
        _delay_millisec(250);
    } else {
        // WAIT for the INT DRDY Data Ready pin value go to 1 XOR timeout from esptimer
        bool has_timed_out = false;
        const double MLX_TIMEOUT_SECONDS = 2; // FAIL when pin is not high after 2 seconds
        double timer_counter_value_seconds = 0;

        timer_set_counter_value(MJD_MLX90393_TIMER_GROUP_ID, MJD_MLX90393_TIMER_ID, 00000000ULL);
        timer_start(MJD_MLX90393_TIMER_GROUP_ID, MJD_MLX90393_TIMER_ID);

        while (gpio_get_level(param_ptr_config->int_gpio_num) != 1) {
            timer_get_counter_time_sec(MJD_MLX90393_TIMER_GROUP_ID, MJD_MLX90393_TIMER_ID, &timer_counter_value_seconds);
            if (timer_counter_value_seconds > MLX_TIMEOUT_SECONDS) {
                has_timed_out = true;
                break; // BREAK WHILE
            }
            vTaskDelay(RTOS_DELAY_10MILLISEC); // Wait in increments of 10 milliseonds (the lowest possible value for this RTOS func)
        }

        // pause timer (stop = n.a.)
        timer_pause(TIMER_GROUP_0, TIMER_0);

        if (has_timed_out == false) {
            ESP_LOGD(TAG, "OK. The INT DRDY Data Ready pin value did go to 1");
        } else {
            f_retval = ESP_FAIL;
            ESP_LOGE(TAG, "%s(). The INT DRDY Data Ready pin value did not go to 1 after the time out (%5f seconds) | err %i (%s)", __FUNCTION__,
                    timer_counter_value_seconds, f_retval, esp_err_to_name(f_retval));
            // GOTO
            goto cleanup;
        }
    }

    // --Log INT pin value
    _log_int_pin_value(param_ptr_config);

    // LABEL
    cleanup: ;

    return f_retval;
}

/*********************************************************************************
 * CMD Read Measurement.
 *
 * @doc [I do not use that info] The status byte received from the MLX90393 will indicate the number of data bytes waiting to be read out in the D1-D0 bits.
 *
 * @doc D[1:0] D1-D0 bits:
 *          Indicates the number of bytes to follow the status byte after a read measurement or a read register command has been sent.
 *          The number of response bytes correspond to 2 + (2 * D[1:0]), so the expected byte counts are either 2, 4, 6 or 8.
 *          These bits only have a meaning after the RR and RM commands, when extra data is expected as a response from the MLX90393.
 *          For commands where no response is expected, the content of D[1:0] should be ignored.
 *
 * @doc In the case where all axes and temp are converted the number of bytes will be 8. The data is output in the following order:
 *          T (MSB), T (LSB), X (MSB), X (LSB), Y (MSB), Y (LSB), Z (MSB), Z (LSB)
 *      If an axis wasn’t selected to be read or converted then that value will be skipped in the transmission.
 *
 *********************************************************************************/
esp_err_t mjd_mlx90393_cmd_read_measurement(const mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_data_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    uint8_t metrics_nbr_of_bytes_to_read = 0;
    uint8_t command;
    mjd_mlx90393_status_byte_t status;
    uint8_t buf_rx[9] =
        { 0 };  // @doc 9 = the max nbr of bytes that any MLX command will return via I2C

    // Input params: convert config metrics flags to the command's LSNibble syntax
    uint8_t zyxt_nibble = 0;
    if (param_ptr_config->mlx_metrics_selector.temperature == true) {
        zyxt_nibble |= MJD_MLX90393_METRIC_TEMPERATURE_BITMASK;
        ++metrics_nbr_of_bytes_to_read;
    }
    if (param_ptr_config->mlx_metrics_selector.x_axis == true) {
        zyxt_nibble |= MJD_MLX90393_METRIC_X_AXIS_BITMASK;
        ++metrics_nbr_of_bytes_to_read;
    }
    if (param_ptr_config->mlx_metrics_selector.y_axis == true) {
        zyxt_nibble |= MJD_MLX90393_METRIC_Y_AXIS_BITMASK;
        ++metrics_nbr_of_bytes_to_read;
    }
    if (param_ptr_config->mlx_metrics_selector.z_axis == true) {
        zyxt_nibble |= MJD_MLX90393_METRIC_Z_AXIS_BITMASK;
        ++metrics_nbr_of_bytes_to_read;
    }

    // Input param .mlx_metrics_selector: compute the nbr of BYTES to read next via I2C
    // @rule Each metric is 1 word (2 bytes via I2C)
    // @rule At least one metric must be selected for read
    metrics_nbr_of_bytes_to_read *= 2; // #words => #bytes
    ESP_LOGD(TAG, "%s(). Extra DATA BYTES to read (based on .mlx_metrics_selector flags): metrics_nbr_of_bytes_to_read %u", __FUNCTION__,
            metrics_nbr_of_bytes_to_read);
    if (metrics_nbr_of_bytes_to_read < 2) {
        f_retval = ESP_ERR_INVALID_ARG;
        ESP_LOGE(TAG, "%s(). ABORT. At least one metric must be selected for read | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Log INT pin value
    _log_int_pin_value(param_ptr_config);

    // Send request & Receive response: STATUS BYTE + METRICS DATA (variable nbr of bytes)
    command = MJD_MLX90393_CMD_READ_MEASUREMENT | zyxt_nibble;
    char command_binary_string[8 + 1] = "12345678";
    mjd_byte_to_binary_string(command, command_binary_string);
    ESP_LOGD(TAG, "Send request & Receive response: STATUS BYTE + METRICS DATA (variable nbr of bytes)");
    ESP_LOGD(TAG, "  command:  0x%X (%hu) bin 0b%s", command, command, command_binary_string);

    i2c_cmd_handle_t handle = i2c_cmd_link_create();

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
    f_retval = i2c_master_write_byte(handle, command, true);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. Send request i2c_master_write_byte() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        i2c_cmd_link_delete(handle);
        // GOTO
        goto cleanup;
    }

    f_retval = i2c_master_start(handle);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. Send request i2c_master_start() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
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
    f_retval = i2c_master_read_byte(handle, &status, I2C_MASTER_ACK);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. Receive response i2c_master_read_byte() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        i2c_cmd_link_delete(handle);
        // GOTO
        goto cleanup;
    }
    // ---variable nbr of bytes to read (2 bytes for each selected metric)
    // @doc i2c_master_read() param4 I2C_MASTER_LAST_NACK = do ACK for all reads except do NACk for the last read (handy feature!)
    f_retval = i2c_master_read(handle, buf_rx, metrics_nbr_of_bytes_to_read, I2C_MASTER_LAST_NACK);
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

    f_retval = i2c_master_cmd_begin(param_ptr_config->i2c_port_num, handle, MJD_MLX90393_I2C_TIMEOUT_DEFAULT);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. Receive response i2c_master_cmd_begin() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        i2c_cmd_link_delete(handle);
        // GOTO
        goto cleanup;
    }

    i2c_cmd_link_delete(handle);

    // Process STATUS BYTE: check error bit
    //   @problem the error bit is always set, WHY?
    _log_status_byte(status);
    if ((status & MJD_MLX90393_STATUS_ERROR_BITMASK) != 0) {
        f_retval = ESP_FAIL;
        ESP_LOGE(TAG, "%s(). ABORT. The STATUS BYTE's ERROR BIT is set | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Process METRICS DATA (raw)
    // @important The order of the if-then tree matters
    // @doc The device data is output in the following order: T (MSB), T (LSB), X (MSB), X (LSB), Y (MSB), Y (LSB), Z (MSB), Z (LSB)
    ESP_LOG_BUFFER_HEXDUMP(TAG, buf_rx, ARRAY_SIZE(buf_rx), ESP_LOG_DEBUG);

    mjd_mlx90393_data_raw_t data_raw =
        { 0 };
    uint8_t idx = 0;

    if (param_ptr_config->mlx_metrics_selector.temperature == true) {
        data_raw.t = (((uint16_t) buf_rx[idx] << 8) | (uint16_t) buf_rx[idx + 1]);
        idx += 2;
    } else {
        data_raw.t = 0;
    }
    if (param_ptr_config->mlx_metrics_selector.x_axis == true) {
        data_raw.x = (((uint16_t) buf_rx[idx] << 8) | (uint16_t) buf_rx[idx + 1]);
        idx += 2;
    } else {
        data_raw.x = 0;
    }
    if (param_ptr_config->mlx_metrics_selector.y_axis == true) {
        data_raw.y = (((uint16_t) buf_rx[idx] << 8) | (uint16_t) buf_rx[idx + 1]);
        idx += 2;
    } else {
        data_raw.y = 0;
    }
    if (param_ptr_config->mlx_metrics_selector.z_axis == true) {
        data_raw.z = (((uint16_t) buf_rx[idx] << 8) | (uint16_t) buf_rx[idx + 1]);
    } else {
        data_raw.z = 0;
    }

    _log_data_raw(&data_raw);

    // Save raw data in the final data structure (nice for comparing, and allowing the user to do its own calculations with the raw data)
    param_ptr_data->t_raw = data_raw.t;
    param_ptr_data->x_raw = data_raw.x;
    param_ptr_data->y_raw = data_raw.y;
    param_ptr_data->z_raw = data_raw.z;

    //
    // ***Transform metrics data (raw -> functional depending on system settings)***
    //   @doc https://math.stackexchange.com/questions/285459/how-can-i-convert-2s-complement-to-decimal
    //   @doc https://en.wikipedia.org/wiki/Bitwise_operations_in_C

    // => T
    param_ptr_data->t = 35 + (data_raw.t - param_ptr_config->mlx_tref) / 45.2f; // OK

    // => XYZ
    switch (param_ptr_config->mlx_res_x) {
    case MJD_MLX90393_RES_XYZ_0:
    case MJD_MLX90393_RES_XYZ_1:
        data_raw.x = (~data_raw.x + 1) * -1; // @doc Convert 2's complement to decimal
        param_ptr_data->x = data_raw.x * _sensitivity_xy[param_ptr_config->mlx_gain_sel][param_ptr_config->mlx_res_x]
                * (1 << param_ptr_config->mlx_res_x);
        break;
    case MJD_MLX90393_RES_XYZ_2:
        param_ptr_data->x = (data_raw.x - 0x8000) * _sensitivity_xy[param_ptr_config->mlx_gain_sel][param_ptr_config->mlx_res_x]
                * (1 << param_ptr_config->mlx_res_x);
        break;
    case MJD_MLX90393_RES_XYZ_3:
        param_ptr_data->x = (data_raw.x - 0x4000) * _sensitivity_xy[param_ptr_config->mlx_gain_sel][param_ptr_config->mlx_res_x]
                * (1 << param_ptr_config->mlx_res_x);
        break;
    default:
        break;
    }

    switch (param_ptr_config->mlx_res_y) {
    case MJD_MLX90393_RES_XYZ_0:
    case MJD_MLX90393_RES_XYZ_1:
        data_raw.y = (~data_raw.y + 1) * -1; // @doc Convert 2's complement to decimal
        param_ptr_data->y = data_raw.y * _sensitivity_xy[param_ptr_config->mlx_gain_sel][param_ptr_config->mlx_res_y]
                * (1 << param_ptr_config->mlx_res_y);
        break;
    case MJD_MLX90393_RES_XYZ_2:
        param_ptr_data->y = (data_raw.y - 0x8000) * _sensitivity_xy[param_ptr_config->mlx_gain_sel][param_ptr_config->mlx_res_y]
                * (1 << param_ptr_config->mlx_res_y);
        break;
    case MJD_MLX90393_RES_XYZ_3:
        param_ptr_data->y = (data_raw.y - 0x4000) * _sensitivity_z[param_ptr_config->mlx_gain_sel][param_ptr_config->mlx_res_y]
                * (1 << param_ptr_config->mlx_res_y);
        break;
    default:
        break;
    }

    switch (param_ptr_config->mlx_res_z) {
    case MJD_MLX90393_RES_XYZ_0:
    case MJD_MLX90393_RES_XYZ_1:
        data_raw.z = (~data_raw.z + 1) * -1; // @doc Convert 2's complement to decimal
        param_ptr_data->z = data_raw.z * _sensitivity_z[param_ptr_config->mlx_gain_sel][param_ptr_config->mlx_res_z]
                * (1 << param_ptr_config->mlx_res_z);
        break;
    case MJD_MLX90393_RES_XYZ_2:
        param_ptr_data->z = (data_raw.z - 0x8000) * _sensitivity_z[param_ptr_config->mlx_gain_sel][param_ptr_config->mlx_res_z]
                * (1 << param_ptr_config->mlx_res_z);
        break;
    case MJD_MLX90393_RES_XYZ_3:
        param_ptr_data->z = (data_raw.z - 0x4000) * _sensitivity_z[param_ptr_config->mlx_gain_sel][param_ptr_config->mlx_res_z]
                * (1 << param_ptr_config->mlx_res_z);
        break;
    default:
        break;
    }

    // Log INT pin value
    _log_int_pin_value(param_ptr_config);

    // LABEL
    cleanup: ;

    return f_retval;
}

/***
 * NEXT
 */
