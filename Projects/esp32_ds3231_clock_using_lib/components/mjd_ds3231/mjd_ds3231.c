/*
 * ZS-042 DS1302 Real Time Clock Module | I2C protocol
 *
 */

// Component header file(s)
#include "mjd.h"
#include "mjd_ds3231.h"

/*
 * Logging
 */
static const char TAG[] = "mjd_ds3231";

/*
 * MAIN
 */

/*********************************************************************************
 * PUBLIC.
 * DS3231: I2C initialization
 * @important Custom for the DS3231 sensor!
 *********************************************************************************/
esp_err_t mjd_ds3231_init(const mjd_ds3231_config_t* config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    i2c_cmd_handle_t cmd;

    if (config->manage_i2c_driver == true) {
        // Config
        i2c_config_t i2c_conf =
            { 0 };
        i2c_conf.mode = I2C_MODE_MASTER;
        i2c_conf.scl_io_num = config->scl_io_num;
        i2c_conf.sda_io_num = config->sda_io_num;
        i2c_conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
        i2c_conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
        i2c_conf.master.clk_speed = DS3231_I2C_MASTER_FREQ_HZ;
        if (i2c_param_config(config->i2c_port_num, &i2c_conf) != ESP_OK) {
            ESP_LOGE(TAG, "ABORT. i2c_param_config()");
            return MJD_ERR_ESP_I2C; // EXIT
        }
        if (i2c_driver_install(config->i2c_port_num, I2C_MODE_MASTER, DS3231_I2C_MASTER_RX_BUF_DISABLE,
        DS3231_I2C_MASTER_TX_BUF_DISABLE,
        DS3231_I2C_MASTER_INTR_FLAG_NONE) != ESP_OK) {
            ESP_LOGE(TAG, "ABORT. i2c_driver_install()");
            return MJD_ERR_ESP_I2C; // EXIT
        }
    }

    // Verify that the I2C slave is working properly
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (config->i2c_slave_addr << 1) | I2C_MASTER_WRITE, I2C_MASTER_ACK);
    i2c_master_stop(cmd);
    if (i2c_master_cmd_begin(config->i2c_port_num, cmd, RTOS_DELAY_2SEC) != ESP_OK) {
        ESP_LOGE(TAG, "ABORT. i2c_master_cmd_begin() I2C slave is NOT working properly");
        return MJD_ERR_ESP_I2C; // EXIT
    }
    i2c_cmd_link_delete(cmd);

    return ESP_OK;
}

/*********************************************************************************
 * PUBLIC.
 * DS3231: I2C DE-initialization
 * @important Custom for the DS3231 sensor!
 *********************************************************************************/
esp_err_t mjd_ds3231_deinit(const mjd_ds3231_config_t* config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    if (config->manage_i2c_driver == true) {
        if (i2c_driver_delete(config->i2c_port_num) != ESP_OK) {
            ESP_LOGE(TAG, "ABORT. i2c_driver_delete() FAIL");
            return MJD_ERR_ESP_I2C; // EXIT
        }
    }
    return ESP_OK;
}

/*********************************************************************************
 * PUBLIC.
 * Read the DS3231 data
 *********************************************************************************/
esp_err_t mjd_ds3231_get_datetime(const mjd_ds3231_config_t* config, mjd_ds3231_data_t* data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    i2c_cmd_handle_t cmd;

    // Send request
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (config->i2c_slave_addr << 1) | I2C_MASTER_WRITE, I2C_MASTER_ACK);
    i2c_master_write_byte(cmd, DS3231_REGISTER_SECONDS, I2C_MASTER_ACK);
    i2c_master_stop(cmd);
    if (i2c_master_cmd_begin(config->i2c_port_num, cmd, RTOS_DELAY_2SEC) != ESP_OK) {
        ESP_LOGE(TAG, "ABORT. Send request");
        i2c_cmd_link_delete(cmd); // CLEANUP
        return MJD_ERR_ESP_I2C; // EXIT
    }
    i2c_cmd_link_delete(cmd);

    // Wait for the sensor/module. @important The spec of the DS3231 does not specify how long to wait for a response! So use the arbitrary value 100 millisec.
    vTaskDelay(RTOS_DELAY_100MILLISEC);

    // Receive answer
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (config->i2c_slave_addr << 1) | I2C_MASTER_READ, true);

    i2c_master_read_byte(cmd, &data->seconds, I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, &data->minutes, I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, &data->hours, I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, &data->weekday, I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, &data->day, I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, &data->month, I2C_MASTER_ACK);
    i2c_master_read_byte(cmd, &data->year, I2C_MASTER_NACK); // last read must be NO_ACK
    i2c_master_stop(cmd);
    if (i2c_master_cmd_begin(config->i2c_port_num, cmd, RTOS_DELAY_1SEC) != ESP_OK) { // This function will trigger sending all queued commands.
        ESP_LOGE(TAG, "ABORT. Receive answer");
        i2c_cmd_link_delete(cmd); // CLEANUP
        return MJD_ERR_ESP_I2C; // EXIT
    }
    i2c_cmd_link_delete(cmd);

    // Process response: seconds BCD
    data->seconds &= 0b01111111; // Keep 7 LSB
    data->seconds = mjd_bcd_to_byte(data->seconds);

    // Process response: minutes BCD
    data->minutes &= 0b01111111; // Keep 7 LSB
    data->minutes = mjd_bcd_to_byte(data->minutes);

    // Process response: hours BCD
    data->hours &= 0b00111111; // Keep 6 LSB
    data->hours = mjd_bcd_to_byte(data->hours);

    // Process response: weekday BCD
    data->weekday = mjd_bcd_to_byte(data->weekday);

    // Process response: day BCD
    data->day &= 0b00111111; // Keep 6 LSB
    data->day = mjd_bcd_to_byte(data->day);

    // Process response: month BCD
    data->month &= 0b00011111; // Keep 5 LSB
    data->month = mjd_bcd_to_byte(data->month);

    // Process response: year BCD @important year storage = 20YY
    data->year = mjd_bcd_to_byte(data->year);

    return ESP_OK;
}

/*********************************************************************************
 * PUBLIC.
 * Set the DS3231 data
 *********************************************************************************/
esp_err_t mjd_ds3231_set_datetime(const mjd_ds3231_config_t* config, const mjd_ds3231_data_t* data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    i2c_cmd_handle_t cmd;

    // Extract & transform field values
    int seconds = mjd_byte_to_bcd(data->seconds);
    int minutes = mjd_byte_to_bcd(data->minutes);
    int hours = mjd_byte_to_bcd(data->hours);
    int weekday = mjd_byte_to_bcd(data->weekday);
    int day = mjd_byte_to_bcd(data->day);
    int month = mjd_byte_to_bcd(data->month);
    int year = mjd_byte_to_bcd(data->year); // year storage = 20YY

    // Send write request
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (config->i2c_slave_addr << 1) | I2C_MASTER_WRITE, I2C_MASTER_ACK);
    // Start writing at this register (and increment to next register for each next write)
    i2c_master_write_byte(cmd, DS3231_REGISTER_SECONDS, I2C_MASTER_ACK);

    i2c_master_write_byte(cmd, seconds, I2C_MASTER_ACK);
    i2c_master_write_byte(cmd, minutes, I2C_MASTER_ACK);
    i2c_master_write_byte(cmd, hours, I2C_MASTER_ACK);
    i2c_master_write_byte(cmd, weekday, I2C_MASTER_ACK);
    i2c_master_write_byte(cmd, day, I2C_MASTER_ACK);
    i2c_master_write_byte(cmd, month, I2C_MASTER_ACK);
    i2c_master_write_byte(cmd, year, I2C_MASTER_ACK);

    i2c_master_stop(cmd);
    if (i2c_master_cmd_begin(config->i2c_port_num, cmd, RTOS_DELAY_2SEC) != ESP_OK) {
        ESP_LOGE(TAG, "ABORT. Send write request");
        i2c_cmd_link_delete(cmd); // CLEANUP
        return MJD_ERR_ESP_I2C; // EXIT
    }
    i2c_cmd_link_delete(cmd);

    return ESP_OK;
}
