/*
 * CJMCU-280E GY-BME280 Bosch BME280 meteo sensor: I2C protocol
 *
 */

// Component header file(s)
#include "mjd.h"
#include "mjd_bme280.h"

/*
 * Logging
 */
static const char TAG[] = "mjd_bme280";

/*********************************************************************************
 * Bosch driver interface functions
 */

/*
 * static variables
 */
// SAVE the I2C Port Num for later use in the Bosch read and write interface functions!
//   @doc typedef enum{I2C_NUM_0 = 0, I2C_NUM_1} i2c_port_t;
static uint32_t bme280_current_i2c_port;

/*  \Brief : The delay routine
 *  \param : delay in ms
 *  @important ESP32 The FreeRTOS func does NOT WORK for short delays < 1s: TaskDelay(millisec / portTICK_PERIOD_MS);
 */
void BME280_delay_millisec(uint32_t millisec) {
    ets_delay_us(millisec * 1000);
}

/*
 *   \Brief: The function is used as I2C bus read
 *   \Return : Status of the I2C read
 *   \param dev_id : The device address of the sensor
 *   \param reg_addr : Address of the first register, where data is going to be read
 *   \param reg_data : This is the data read from the sensor, which is held in an array
 *   \param len : The nbr of bytes of data to be read
 *
 *       * Data on the bus should be like
         * |------------+---------------------|
         * | I2C action | Data                |
         * |------------+---------------------|
         * | Start      | -                   |
         * | Write      | (reg_addr)          |
         * | Stop       | -                   |
         * | Start      | -                   |
         * | Read       | (reg_data[0])       |
         * | Read       | (....)              |
         * | Read       | (reg_data[len - 1]) |
         * | Stop       | -                   |
         * |------------+---------------------|
         *
 * Please take the below function as your reference to read the data using I2C communication
 *   "f_retval = I2C_WRITE_READ_STRING(DEV_ID, ARRAY, ARRAY, 1, CNT)"
 *      f_retval is an return value of I2C write function
 *       Assign a valid return value. BME280_OK | BME280_E_COMM_FAIL defined in Bosch driver
 *
 */
int8_t BME280_I2C_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len) {
    int32_t f_retval = BME280_OK;
    esp_err_t esp_retval;

    i2c_cmd_handle_t cmd;

    cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_id << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_id << 1) | I2C_MASTER_READ, true);
    if (len > 1) {
        i2c_master_read(cmd, reg_data, len - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, reg_data + len - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);

    esp_retval = i2c_master_cmd_begin(bme280_current_i2c_port, cmd, RTOS_DELAY_100MILLISEC);
    if (esp_retval == ESP_OK) {
        f_retval = BME280_OK;  // = defined in the Bosch driver
    } else {
        ESP_LOGE(TAG, "ERROR. BME280_I2C_bus_write(): 2nd i2c_master_cmd_begin() err %i", esp_retval);
        f_retval = BME280_E_COMM_FAIL;  // = defined in the Bosch driver
    }
    i2c_cmd_link_delete(cmd);

    //DEVTEMP DEBUG Insert this after the read code
    /*printf("RD: reg 0x%X len %d:", reg_addr, len);
    for(uint16_t idx = 0; idx < len; idx++)
        printf(" 0x%X", reg_data[idx]);
    printf("\n");*/
    //DEVTEMP DEBUG END

    return (int8_t) f_retval;
}

/*  \Brief: The function is used as I2C bus write
 *   \Return : Status of the I2C write
 *   \param dev_id : The device address of the sensor
 *   \param reg_addr : Address of the first register, where data is to be written
 *   \param reg_data : It is a value held in the array, which is written in the register
 *   \param len : The nbr of bytes of data to be written
 *
 *   *
     * Data on the bus should be like
     * |------------+---------------------|
     * | I2C action | Data                |
     * |------------+---------------------|
     * | Start      | -                   |
     * | Write      | (reg_addr)          |
     * | Write      | (reg_data[0])       |
     * | Write      | (....)              |
     * | Write      | (reg_data[len - 1]) |
     * | Stop       | -                   |
     * |------------+---------------------|
     *
 * Please take the below function as your reference for write the data using I2C communication
 *   "f_retval = I2C_WRITE_STRING(DEV_ADDR, ARRAY, CNT+1)"
 *       f_retval is an return value of I2C read function
 *       Assign a valid return value. BME280_OK | BME280_E_COMM_FAIL defined in Bosch driver
 */
int8_t BME280_I2C_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len) {
    int32_t f_retval = BME280_OK;
    esp_err_t esp_retval;

    /*
     * Notes:
     *   This is a full duplex operation.
     *   ***TODO*** The first read data is discarded, for that extra write operation have to be initiated.
     *       Thus cnt+1 operation done in the I2C write string function. => More info in data sheet.
     */
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_id << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_write(cmd, reg_data, len, true);
    i2c_master_stop(cmd);
    esp_retval = i2c_master_cmd_begin(bme280_current_i2c_port, cmd, RTOS_DELAY_100MILLISEC);
    if (esp_retval == ESP_OK) {
        f_retval = BME280_OK;  // = defined in the Bosch driver
    } else {
        ESP_LOGE(TAG, "ERROR. BME280_I2C_bus_write(): i2c_master_cmd_begin() err %i", esp_retval);
        f_retval = BME280_E_COMM_FAIL;  // = defined in the Bosch driver
    }
    i2c_cmd_link_delete(cmd);

    //DEVTEMP DEBUG Insert this before the write code
    /*printf("WR: 0x%X len %d", reg_addr, len);
    for(uint16_t idx = 0; idx < len; idx++)
    printf(" 0x%X", reg_data[idx]);
    printf("\n");*/
    //DEVTEMP DEBUG END

    return (int8_t) f_retval;
}

/*********************************************************************************
 * PUBLIC.
 */

esp_err_t mjd_bme280_init(mjd_bme280_config_t* config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    i2c_cmd_handle_t cmd;
    int32_t com_rslt;

    /*
     * Pointer (reused in this func)
     */
    struct bme280_dev *ptr_bme280_device = &config->bme280_device;

    /*
     * MAIN
     */
    if (config->manage_i2c_driver == true) {
        // Config
        i2c_config_t i2c_conf = { 0 };
        i2c_conf.mode = I2C_MODE_MASTER;
        i2c_conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
        i2c_conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
        i2c_conf.master.clk_speed = BME280_I2C_MASTER_FREQ_HZ;
        i2c_conf.scl_io_num = config->i2c_scl_gpio_num;
        i2c_conf.sda_io_num = config->i2c_sda_gpio_num;

        f_retval = i2c_param_config(config->i2c_port_num, &i2c_conf);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "ABORT. i2c_param_config() error (%i)", f_retval);
            // LABEL
            goto cleanup;
        }

        f_retval = i2c_driver_install(config->i2c_port_num, I2C_MODE_MASTER, BME280_I2C_MASTER_RX_BUF_DISABLE,
        BME280_I2C_MASTER_TX_BUF_DISABLE,
        BME280_I2C_MASTER_INTR_FLAG_NONE);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "ABORT. i2c_driver_install() error (%i)", f_retval);
            // LABEL
            goto cleanup;
        }
    }

    // Verify that the I2C slave is working properly
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (config->i2c_slave_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_stop(cmd);
    f_retval = i2c_master_cmd_begin(config->i2c_port_num, cmd, RTOS_DELAY_1SEC);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "ABORT. i2c_master_cmd_begin() I2C slave NOT working or wrong I2C slave address - error (%i)",
                f_retval);
        // LABEL
        goto cleanup;
    }
    i2c_cmd_link_delete(cmd);

    // @IMPORTANT SAVE the I2C Port Num for later use in the Bosch read and write interface implementation functions!
    bme280_current_i2c_port = config->i2c_port_num;

    // Bosch driver: verify this is the correct sensor device (chip_id)
    ptr_bme280_device->dev_id = config->i2c_slave_addr; // @important Clone the i2c_slave_addrfrom my main config struct!
    ptr_bme280_device->intf = BME280_I2C_INTF;
    ptr_bme280_device->delay_ms = BME280_delay_millisec;
    ptr_bme280_device->read = BME280_I2C_read;
    ptr_bme280_device->write = BME280_I2C_write;

    com_rslt = bme280_init(ptr_bme280_device);
    if (com_rslt != BME280_OK) {
        ESP_LOGE(TAG, "ABORT. bme280_init() failed err %i", com_rslt);
        f_retval = ESP_FAIL;
        // LABEL
        goto cleanup;
    }
    ESP_LOGD(TAG, "  dev_id:  0x%hhx (d %hhu) [EXPECT 0x76 or 0x77 for sensor BME280]", ptr_bme280_device->dev_id,
            ptr_bme280_device->dev_id);
    ESP_LOGD(TAG, "  chip id: 0x%hhx (d %hhu) [EXPECT 0x60 for sensor BME280]", ptr_bme280_device->chip_id,
            ptr_bme280_device->chip_id);

    // LABEL
    cleanup: ;

    return f_retval;
}

esp_err_t mjd_bme280_deinit(mjd_bme280_config_t* ptr_param_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    /*
     * I2C Driver
     */
    if (ptr_param_config->manage_i2c_driver == true) {
        f_retval = i2c_driver_delete(ptr_param_config->i2c_port_num);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "ABORT. i2c_driver_delete() error (%i)", f_retval);
            // LABEL
            goto cleanup;
        }
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

esp_err_t mjd_bme280_read_forced(mjd_bme280_config_t* ptr_param_config, mjd_bme280_data_t* ptr_param_data) {
    // @IMPORTANT The sequence of the Bosch API calls matters.
    // @doc Sleep mode is set by default after sensor power-on or soft-reset. In sleep mode, no measurements are  performed and power consumption is at a minimum.
    // @doc The timing for data readout in forced mode should be done so that the maximum measurement times are respected (see chapter 3.8.1).
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    int32_t com_rslt;
    uint8_t sensor_mode = 255;

    /*
     * Reset receive values
     */
    ptr_param_data->humidity_percent = 0.0;
    ptr_param_data->pressure_hpascal = 0.0;
    ptr_param_data->temperature_celsius = 0.0;

    /*
     * Pointer (reused in this func)
     */
    struct bme280_dev *ptr_bme280_device = &(ptr_param_config->bme280_device);  // @important Use &(p->v)
    struct bme280_settings *ptr_bme280_device_settings = &(ptr_bme280_device->settings);  // @important Use &(p->v)

    /*
     * STEP 1: INIT Bosch driver
     *
     */
    ESP_LOGD(TAG, "do bme280_init()");

    // @IMPORTANT SAVE the I2C Port Num for later use in the Bosch read and write interface functions!
    bme280_current_i2c_port = ptr_param_config->i2c_port_num;

    // Bosch driver: verify this is the correct sensor device (chip_id)
    ptr_bme280_device->dev_id = ptr_param_config->i2c_slave_addr; // @important Clone the i2c_slave_addr from my main config struct!
    ptr_bme280_device->intf = BME280_I2C_INTF;
    ptr_bme280_device->read = BME280_I2C_read;
    ptr_bme280_device->write = BME280_I2C_write;
    ptr_bme280_device->delay_ms = BME280_delay_millisec;

    com_rslt = bme280_init(ptr_bme280_device);
    if (com_rslt != BME280_OK) {
        ESP_LOGE(TAG, "ABORT. bme280_init() failed err %i", com_rslt);
        f_retval = ESP_FAIL;
        // LABEL
        goto cleanup;
    }
    ESP_LOGD(TAG, "  dev_id:  0x%hhx (d %hhu) [EXPECT 0x76 or 0x77 for sensor BME280]", ptr_bme280_device->dev_id,
            ptr_bme280_device->dev_id);
    ESP_LOGD(TAG, "  chip id: 0x%hhx (d %hhu) [EXPECT 0x60 for sensor BME280]", ptr_bme280_device->chip_id,
            ptr_bme280_device->chip_id);

    /*
     * STEP 2: SENSOR SETTINGS (ACCURACY RELATED / OVERSAMPLING RATIO)
     *  a. The measurement period depends on the setting of oversampling setting of humidity, pressure, temperature and standby time:
     *                                    => humidity OSR   pressure OSR    temperature OSR     Filter coefficient
     *                                     --------------   ------------    ---------------     -----------------------
     *      ULTRA_LOW_POWER_MODE                       x1             x1                 x1     BME280_FILTER_COEFF_OFF
     *      LOW_POWER_MODE                              ?             x2                 x1     ?
     *      STANDARD_RESOLUTION_MODE                    ?             x4                 x1     ?
     *      HIGH_RESOLUTION_MODE                        ?             x8                 x2     ?
     *      ULTRA_HIGH_RESOLUTION_MODE                  ?              ?                  ?     ?
     *
     *  b. u8 The value of filter coefficient
     *    value       | Filter coefficient
     * ---------------|-------------------------
     *    0x00        | BME280_FILTER_COEFF_OFF
     *    0x01        | BME280_FILTER_COEFF_2
     *    0x02        | BME280_FILTER_COEFF_4
     *    0x03        | BME280_FILTER_COEFF_8
     *    0x04        | BME280_FILTER_COEFF_16
     *
     */
    ESP_LOGD(TAG, "do bme280_set_sensor_settings()");

    uint8_t settings_selection_mask = BME280_OSR_HUM_SEL | BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_FILTER_SEL;

    ptr_bme280_device_settings->osr_h = BME280_OVERSAMPLING_1X;
    ptr_bme280_device_settings->osr_p = BME280_OVERSAMPLING_1X;
    ptr_bme280_device_settings->osr_t = BME280_OVERSAMPLING_1X;
    ptr_bme280_device_settings->filter = BME280_FILTER_COEFF_OFF;

    com_rslt = bme280_set_sensor_settings(settings_selection_mask, ptr_bme280_device);
    if (com_rslt != BME280_OK) {
        ESP_LOGE(TAG, "ABORT. bme280_set_sensor_settings() failed - err %i", com_rslt);
        f_retval = ESP_FAIL;
        // LABEL
        goto cleanup;
    }

    /*
     * DEBUG Sensor Settings
     *
     */
    ESP_LOGD(TAG, "do bme280_get_sensor_settings()");

    com_rslt = bme280_get_sensor_settings(ptr_bme280_device);
    if (com_rslt != BME280_OK) {
        ESP_LOGE(TAG, "ABORT. bme280_get_sensor_settings() failed err %i", com_rslt);
        f_retval = ESP_FAIL;
        // LABEL
        goto cleanup;
    }

    ESP_LOGD(TAG, "  settings.osr_h:  0x%hhx (d %hhu) [EXPECT 0x01 BME280_OVERSAMPLING_1X]",
            ptr_bme280_device_settings->osr_h, ptr_bme280_device_settings->osr_h);
    ESP_LOGD(TAG, "  settings.osr_p:  0x%hhx (d %hhu) [EXPECT 0x01 BME280_OVERSAMPLING_1X]",
            ptr_bme280_device_settings->osr_p, ptr_bme280_device_settings->osr_p);
    ESP_LOGD(TAG, "  settings.osr_t:  0x%hhx (d %hhu) [EXPECT 0x01 BME280_OVERSAMPLING_1X]",
            ptr_bme280_device_settings->osr_t, ptr_bme280_device_settings->osr_t);
    ESP_LOGD(TAG, "  settings.filter: 0x%hhx (d %hhu) [EXPECT 0x00 BME280_FILTER_COEFF_OFF]",
            ptr_bme280_device_settings->filter, ptr_bme280_device_settings->filter);

    /*
     * STEP 3: SET SENSOR MODE = FORCED
     * After initialization it is required to set the mode of the sensor as FORCED_MODE or NORMAL_MODE. Data acquisition/read/write is possible in this mode.
     *   @doc In forced mode, a single measurement is performed. When the measurement is finished, the sensor returns to sleep mode.
     *
     *   \name Sensor power modes
     *      #define BME280_SLEEP_MODE       UINT8_C(0x00)
     *      #define BME280_FORCED_MODE      UINT8_C(0x01)
     *      #define BME280_NORMAL_MODE      UINT8_C(0x03)
     */
    ESP_LOGD(TAG, "do bme280_set_sensor_mode(BME280_FORCED_MODE)");

    com_rslt = bme280_set_sensor_mode(BME280_FORCED_MODE, ptr_bme280_device);
    if (com_rslt != BME280_OK) {
        ESP_LOGE(TAG, "ABORT. bme280_set_sensor_mode(BME280_FORCED_MODE) failed - err %i", com_rslt);
        f_retval = ESP_FAIL;
        // LABEL
        goto cleanup;
    }

    /*
     * IMPORTANT: Add a delay of at least 100 millisec AFTER setting forced_mode & BEFORE requesting the sensor readings!
     * @doc This depends on the Oversampling settings in struct bme280_settings
     *
     */
    BME280_delay_millisec(100);

    /*
     * DEBUG SENSOR MODE (before reading sensor data).
     *
     */
    /*ESP_LOGD(TAG, "do bme280_get_sensor_mode()");

    com_rslt = bme280_get_sensor_mode(&sensor_mode, ptr_bme280_device);
    if (com_rslt != BME280_OK) {
        ESP_LOGE(TAG, "ABORT. bme280_get_sensor_mode() failed - err %i", com_rslt);
        f_retval = ESP_FAIL;
        // LABEL
        goto cleanup;
    }
    ESP_LOGD(TAG, "  sensor_mode: 0x%hhx (d %hhu) [EXPECT 0x01 BME280_FORCED_MODE]", sensor_mode, sensor_mode);*/

    /*
     * DEBUG Standby settings are NOT relevant in BME280_FORCED_MODE :)
     *
     */

    /*
     * STEP 4: READ SENSOR DATA
     *  @doc The sensor goes automatically into SLEEP MODE after reading the sensor data
     */
    ESP_LOGD(TAG, "do bme280_get_sensor_data()");

    struct bme280_data compensated_data = { 0 }; // @important = { 0 }

    com_rslt = bme280_get_sensor_data(BME280_ALL, &compensated_data, ptr_bme280_device);
    if (com_rslt != BME280_OK) {
        ESP_LOGE(TAG, "ABORT. bme280_get_sensor_data() failed - rr %i", com_rslt);
        f_retval = ESP_FAIL;
        // LABEL
        goto cleanup;
    }

    // transfer received values to my data store
    ptr_param_data->humidity_percent = compensated_data.humidity; // Percent
    ptr_param_data->pressure_hpascal = compensated_data.pressure / 100; // Pascal => HPa
    ptr_param_data->temperature_celsius = compensated_data.temperature; // Celsius

    /*
     * DEBUG SENSOR MODE (after reading sensor data).
     *
     */
    /*ESP_LOGD(TAG, "do bme280_get_sensor_mode()");

    com_rslt = bme280_get_sensor_mode(&sensor_mode, ptr_bme280_device);
    if (com_rslt != BME280_OK) {
        ESP_LOGE(TAG, "ABORT. bme280_get_sensor_mode() failed - err %i", com_rslt);
        f_retval = ESP_FAIL;
        // LABEL
        goto cleanup;
    }
    ESP_LOGD(TAG, "  sensor_mode: 0x%hhx (d %hhu) [EXPECT 0x00 BME280_SLEEP_MODE]", sensor_mode, sensor_mode);*/

    // LABEL
    cleanup: ;

    return f_retval;
}
