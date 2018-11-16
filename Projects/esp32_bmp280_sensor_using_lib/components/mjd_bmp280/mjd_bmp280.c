/*
 * GY-BMP280-3.3 Bosch BMP280 Bosch meteo sensor: I2C protocol
 *
 */

// Component header file(s)
#include "mjd.h"
#include "mjd_bmp280.h"

/*
 * Logging
 */
static const char TAG[] = "mjd_bmp280";

/*********************************************************************************
 * Bosch driver interface functions
 */

/*
 * static variables
 */

// @doc typedef enum{I2C_NUM_0 = 0, I2C_NUM_1} i2c_port_t;
static uint32_t bmp280_current_i2c_port;

/*  \Brief : The delay routine
 *  \param : delay in ms
 *  @important ESP32 This does NOT WORK!: TaskDelay(millisec / portTICK_PERIOD_MS);
 */
void BMP280_delay_millisec(u32 millisec) {
    ets_delay_us(millisec * 1000);
}

/*  \Brief: The function is used as I2C bus write
 *   \Return : Status of the I2C write
 *   \param dev_addr : The device address of the sensor
 *   \param reg_addr : Address of the first register, where data is to be written
 *   \param reg_data : It is a value held in the array, which is written in the register
 *   \param cnt : The nbr of bytes of data to be written
 */
s8 BMP280_I2C_bus_write(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt) {
    s32 f_retval = BMP280_INIT_VALUE;
    esp_err_t esp_retval;

    /*
     * Please take the below function as your reference for write the data using I2C communication
     *   "f_retval = I2C_WRITE_STRING(DEV_ADDR, ARRAY, CNT+1)"
     *       f_retval is an return value of I2C read function
     *       Assign a valid return value. SUCCESS 0 | ERROR -1 defined in Bosch driver
     * Notes:
     *   This is a full duplex operation.
     *   ***TODO*** The first read data is discarded, for that extra write operation have to be initiated.
     *       Thus cnt+1 operation done in the I2C write string function. => More info in data sheet.
     */
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);
    i2c_master_write(cmd, reg_data, cnt, true);
    i2c_master_stop(cmd);
    esp_retval = i2c_master_cmd_begin(bmp280_current_i2c_port, cmd, RTOS_DELAY_100MILLISEC);
    if (esp_retval == ESP_OK) {
        f_retval = SUCCESS;  // SUCCES 0 = defined in the Bosch driver
    } else {
        ESP_LOGE(TAG, "ERROR. BMP280_I2C_bus_write(): i2c_master_cmd_begin() err %i", esp_retval);
        f_retval = ERROR;  // ERROR -1 = defined in the Bosch driver
    }
    i2c_cmd_link_delete(cmd);

    //DEVTEMP DEBUG Insert this before the write code
    /*printf("WR: 0x%X len %d", reg_addr, cnt);
    for(uint16_t idx = 0; idx < cnt; idx++)
    printf(" 0x%X", reg_data[idx]);
    printf("\n");*/
    //DEVTEMP DEBUG END

    return (s8) f_retval;
}

/*  \Brief: The function is used as I2C bus read
 *   \Return : Status of the I2C read
 *   \param dev_addr : The device address of the sensor
 *   \param reg_addr : Address of the first register, where data is going to be read
 *   \param reg_data : This is the data read from the sensor, which is held in an array
 *   \param cnt : The nbr of bytes of data to be read
 */
s8 BMP280_I2C_bus_read(u8 dev_addr, u8 reg_addr, u8 *reg_data, u8 cnt) {
    s32 f_retval = BMP280_INIT_VALUE;
    esp_err_t esp_retval;

    /* Please take the below function as your reference to read the data using I2C communication
     *   "f_retval = I2C_WRITE_READ_STRING(DEV_ADDR, ARRAY, ARRAY, 1, CNT)"
     *      f_retval is an return value of I2C write function
     *       Assign a valid return value. SUCCESS 0 | ERROR -1 defined in Bosch driver
     */
    i2c_cmd_handle_t cmd;

    cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg_addr, true);

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev_addr << 1) | I2C_MASTER_READ, true);
    if (cnt > 1) {
        i2c_master_read(cmd, reg_data, cnt - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, reg_data + cnt - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);

    esp_retval = i2c_master_cmd_begin(bmp280_current_i2c_port, cmd, RTOS_DELAY_100MILLISEC);
    if (esp_retval == ESP_OK) {
        f_retval = SUCCESS;  // SUCCES = defined in the Bosch driver
    } else {
        ESP_LOGE(TAG, "ERROR. BMP280_I2C_bus_write(): 2nd i2c_master_cmd_begin() err %i", esp_retval);
        f_retval = ERROR;  // ERROR -1 = defined in the Bosch driver
    }
    i2c_cmd_link_delete(cmd);

    //DEVTEMP DEBUG Insert this after the read code
    /*printf("RD: reg 0x%X len %d:", reg_addr, cnt);
    for(uint16_t idx = 0; idx < cnt; idx++)
        printf(" 0x%X", reg_data[idx]);
    printf("\n");*/
    //DEVTEMP DEBUG END

    return (s8) f_retval;
}

/*********************************************************************************
 * PUBLIC.
 */

esp_err_t mjd_bmp280_init(const mjd_bmp280_config_t* config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    i2c_cmd_handle_t cmd;
    s32 com_rslt;

    if (config->manage_i2c_driver == true) {
        // Config
        i2c_config_t i2c_conf = { 0 };
        i2c_conf.mode = I2C_MODE_MASTER;
        i2c_conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
        i2c_conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
        i2c_conf.master.clk_speed = BMP280_I2C_MASTER_FREQ_HZ;
        i2c_conf.scl_io_num = config->i2c_scl_gpio_num;
        i2c_conf.sda_io_num = config->i2c_sda_gpio_num;

        f_retval = i2c_param_config(config->i2c_port_num, &i2c_conf);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "ABORT. i2c_param_config() error (%i)", f_retval);
            // LABEL
            goto cleanup;
        }

        f_retval = i2c_driver_install(config->i2c_port_num, I2C_MODE_MASTER, BMP280_I2C_MASTER_RX_BUF_DISABLE,
        BMP280_I2C_MASTER_TX_BUF_DISABLE,
        BMP280_I2C_MASTER_INTR_FLAG_NONE);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "ABORT. i2c_driver_install() error (%i)", f_retval);
            // LABEL
            goto cleanup;
        }
    }

    // SAVE the I2C Port Num for later use in the Bosch read and write interface functions!
    bmp280_current_i2c_port = config->i2c_port_num;

    // Verify that the I2C slave is working properly
    cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (config->i2c_slave_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_stop(cmd);
    f_retval = i2c_master_cmd_begin(config->i2c_port_num, cmd, RTOS_DELAY_1SEC);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "ABORT. i2c_master_cmd_begin() I2C slave NOT working or wrong I2C slave address - error (%i)", f_retval);
        // LABEL
        goto cleanup;
    }
    i2c_cmd_link_delete(cmd);

    // Bosch driver: verify the sensor chip_id is correct, e.g. it is the correct sensor device
    struct bmp280_t bmp280 = { .bus_write = BMP280_I2C_bus_write, .bus_read = BMP280_I2C_bus_read, .delay_msec =
            BMP280_delay_millisec, .dev_addr = config->i2c_slave_addr };
    com_rslt = bmp280_init(&bmp280);
    ESP_LOGD(TAG, "  chip id: 0x%hhx (d %hhu) [EXPECT 0x56 or 0x57 or 0x58 for sensor BMP280]", bmp280.chip_id, bmp280.chip_id);
    if (com_rslt != SUCCESS) {
        ESP_LOGE(TAG, "ABORT. bmp280_init() failed err %i", com_rslt);
        f_retval = ESP_FAIL;
        // LABEL
        goto cleanup;
    }

    // LABEL
    cleanup:;

    return f_retval;
}

esp_err_t mjd_bmp280_deinit(const mjd_bmp280_config_t* config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    /*
     * I2C Driver
     */
    if (config->manage_i2c_driver == true) {
        f_retval = i2c_driver_delete(config->i2c_port_num);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "ABORT. i2c_driver_delete() error (%i)", f_retval);
            // LABEL
            goto cleanup;
        }
    }

    // LABEL
    cleanup:;

    return f_retval;
}

esp_err_t mjd_bmp280_read_forced(const mjd_bmp280_config_t* config, mjd_bmp280_data_t* data) {
    // @IMPORTANT The sequence of the Bosch API calls matters: init, set work mode, set filter coefficient, set power mode.
    // @doc Sleep mode is set by default after power on reset. In sleep mode, no measurements are  performed and power consumption (IDDSM) is at a minimum.
    // @doc The timing for data readout in forced mode should be done so that the maximum measurement times (see chapter 3.8.1) are respected.
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    s32 com_rslt;
    u8 filter_coeff = 255;
    u8 power_mode = 255;

    /*
     * Reset receive values
     */
    data->pressure_hpascal = 0.0;
    data->temperature_celsius = 0.0;

    /* STEP 1: INIT.
     * Init Bosch driver
     */
    ESP_LOGD(TAG, "do bmp280_init()");

    struct bmp280_t bmp280 = { .bus_write = BMP280_I2C_bus_write, .bus_read = BMP280_I2C_bus_read, .delay_msec =
            BMP280_delay_millisec, .dev_addr = config->i2c_slave_addr };
    com_rslt = bmp280_init(&bmp280);
    ESP_LOGD(TAG, "  chip id: 0x%hhx (d %hhu) [EXPECT 0x56 or 0x57 or 0x58 for sensor BMP280]", bmp280.chip_id, bmp280.chip_id);
    if (com_rslt != SUCCESS) {
        ESP_LOGE(TAG, "ABORT. bmp280_init() failed err %i", com_rslt);
        f_retval = ESP_FAIL;
        // LABEL
        goto cleanup;
    }

    /*  DEBUG Power mode.
     *  @brief This API used to get the Operational Mode from the sensor in the register 0xF4 bit 0 and 1
     *  @param v_power_mode_u8 : The value of power mode value
     *  value            |   Power mode
     * ------------------|------------------
     *  0x00             | BMP280_SLEEP_MODE
     *  0x01 and 0x02    | BMP280_FORCED_MODE
     *  0x03             | BMP280_NORMAL_MODE
     *
     *  @return results of bus communication function
     *  @retval 0 -> Success
     *  @retval -1 -> Error
     */
    ESP_LOGD(TAG, "do bmp280_get_power_mode()");

    com_rslt = bmp280_get_power_mode(&power_mode);
    ESP_LOGD(TAG, "  power_mode: 0x%hhx (d %hhu) [EXPECT 0x00 BMP280_SLEEP_MODE]", power_mode, power_mode);
    if (com_rslt != SUCCESS) {
        ESP_LOGE(TAG, "ABORT. bmp280_get_power_mode() failed err %i", com_rslt);
        f_retval = ESP_FAIL;
        // LABEL
        goto cleanup;
    }

    /* STEP 2: SET WORK MODE.
     * For reading the pressure and temperature data, it is required to set the work mode.
     * @param v_work_mode_u8 : The value of work mode
     *   value      |  mode
     * -------------|-------------
     *    0x00         | BMP280_ULTRA_LOW_POWER_MODE
     *    0x01         | BMP280_LOW_POWER_MODE
     *    0x02         | BMP280_STANDARD_RESOLUTION_MODE
     *    0x03         | BMP280_HIGH_RESOLUTION_MODE
     *    0x04         | BMP280_ULTRA_HIGH_RESOLUTION_MODE
     *
     *  The measurement period depends on the setting of over samplingsetting of pressure, temperature and standby time:
     *      #define                             OSS                   => pressure OSS  temperature OSS
     *      BMP280_ULTRA_LOW_POWER_MODE         ultra low power                    x1               x1
     *      BMP280_LOW_POWER_MODE               low power                          x2               x1
     *      BMP280_STANDARD_RESOLUTION_MODE     standard resolution                x4               x1
     *      BMP280_HIGH_RESOLUTION_MODE         high resolution                    x8               x2
     *      BMP280_ULTRA_HIGH_RESOLUTION_MODE   ultra high resolution             x16               x2
     */
    ESP_LOGD(TAG, "do bmp280_set_work_mode() [config->bmp280_work_mode = 0x%x (d %u)]", config->bmp280_work_mode,
            config->bmp280_work_mode);

    com_rslt = bmp280_set_work_mode(config->bmp280_work_mode);
    if (com_rslt != SUCCESS) {
        ESP_LOGE(TAG, "ABORT. bmp280_set_work_mode() [config->bmp280_work_mode = 0x%x (d %u)] failed err %i",
                config->bmp280_work_mode, config->bmp280_work_mode, com_rslt);
        f_retval = ESP_FAIL;
        // LABEL
        goto cleanup;
    }

    /*  STEP 3: SET FILTER COEFFICIENT.
     *
     *  @brief This API is used to write filter setting in the register 0xF5 bit 3 and 4.
     *  @param v_value_u8 : The value of filter coefficient
     *    value       | Filter coefficient
     * ---------------|-------------------------
     *    0x00        | BMP280_FILTER_COEFF_OFF
     *    0x01        | BMP280_FILTER_COEFF_2
     *    0x02        | BMP280_FILTER_COEFF_4
     *    0x03        | BMP280_FILTER_COEFF_8
     *    0x04        | BMP280_FILTER_COEFF_16
     *
     *  @return results of bus communication function
     *  @retval 0 -> Success
     *  @retval -1 -> Error
     */
    ESP_LOGD(TAG, "do bmp280_set_filter() [config->bmp280_filter_coefficient = 0x%x (d %u)]",
            config->bmp280_filter_coefficient, config->bmp280_filter_coefficient);

    com_rslt = bmp280_set_filter(config->bmp280_filter_coefficient);
    if (com_rslt != SUCCESS) {
        ESP_LOGE(TAG, "ABORT. bmp280_set_filter() [config->bmp280_filter_coefficient = 0x%x (d %u)] failed err %i",
                config->bmp280_filter_coefficient, config->bmp280_filter_coefficient, com_rslt);
        f_retval = ESP_FAIL;
        // LABEL
        goto cleanup;
    }

    /* STEP 4: SET POWER MODE
     * After initialization it is required to set the mode of the sensor as FORCED_MODE or NORMAL_MODE. Data acquisition/read/write is possible in this mode.
     *   @doc In forced mode, a single measurement is performed. When the measurement is finished, the sensor returns to sleep mode.
     */
    ESP_LOGD(TAG, "do bmp280_set_power_mode(BMP280_FORCED_MODE)");

    com_rslt = bmp280_set_power_mode(BMP280_FORCED_MODE);
    if (com_rslt != SUCCESS) {
        ESP_LOGE(TAG, "ABORT. bmp280_set_power_mode(BMP280_FORCED_MODE) failed err %i", com_rslt);
        f_retval = ESP_FAIL;
        // LABEL
        goto cleanup;
    }

    /*  DEBUG Work mode: N.A.
     *  @problem A get method to retrieve the current work mode does not exist in the Bosch driver :(
     *  TODO Retrieve the oversampling data for temperature and pressure from the sensor registers and display those.
     */

    /*  DEBUG Filter Coefficient. RESULT: BMP280_FILTER_COEFF_OFF for BMP280_ULTRA_LOW_POWER_MODE :)
     *  @brief This API is used to reads filter setting in the register 0xF5 bit 3 and 4
     *  @param v_value_u8 : The value of filter coefficient
     *  value       |   Filter coefficient
     * -------------|-------------------------
     *  0x00        | BMP280_FILTER_COEFF_OFF
     *  0x01        | BMP280_FILTER_COEFF_2
     *  0x02        | BMP280_FILTER_COEFF_4
     *  0x03        | BMP280_FILTER_COEFF_8
     *  0x04        | BMP280_FILTER_COEFF_16
     *
     *  @return results of bus communication function
     *  @retval 0 -> Success
     *  @retval -1 -> Error
     */
    ESP_LOGD(TAG, "do bmp280_get_filter()");

    com_rslt = bmp280_get_filter(&filter_coeff);
    ESP_LOGD(TAG,
            "  filter_coeff: 0x%hhx (d %hhu) [EXPECT 0x00 BMP280_FILTER_COEFF_OFF for work mode BMP280_ULTRA_LOW_POWER_MODE]",
            filter_coeff, filter_coeff);
    if (com_rslt != SUCCESS) {
        ESP_LOGE(TAG, "ABORT. bmp280_get_filter() failed err %i", com_rslt);
        f_retval = ESP_FAIL;
        // LABEL
        goto cleanup;
    }

    /*  DEBUG Power mode.
     *  @brief This API used to get the Operational Mode from the sensor in the register 0xF4 bit 0 and 1
     *  @param v_power_mode_u8 : The value of power mode value
     *  value            |   Power mode
     * ------------------|------------------
     *  0x00             | BMP280_SLEEP_MODE
     *  0x01 and 0x02    | BMP280_FORCED_MODE
     *  0x03             | BMP280_NORMAL_MODE
     *
     *  @return results of bus communication function
     *  @retval 0 -> Success
     *  @retval -1 -> Error
     */
    ESP_LOGD(TAG, "do bmp280_get_power_mode()");

    com_rslt = bmp280_get_power_mode(&power_mode);
    ESP_LOGD(TAG, "  power_mode: 0x%hhx (d %hhu) [EXPECT 0x01 or 0x02 BMP280_FORCED_MODE]", power_mode, power_mode);
    if (com_rslt != SUCCESS) {
        ESP_LOGE(TAG, "ABORT. bmp280_get_power_mode() failed err %i", com_rslt);
        f_retval = ESP_FAIL;
        // LABEL
        goto cleanup;
    }

    /*
     * Standby settings are not relevant for BMP280_FORCED_MODE
     */

    /*
     * Add a delay of minimum 35millisec before running the first reading!
     */
    vTaskDelay(RTOS_DELAY_50MILLISEC);

    /*
     * read sensor data
     */
    ESP_LOGD(TAG, "do bmp280_get_forced_uncomp_pressure_temperature()");

    s32 v_uncomp_pressure_s32;
    s32 v_uncomp_temperature_s32;

    com_rslt = bmp280_get_forced_uncomp_pressure_temperature(&v_uncomp_pressure_s32, &v_uncomp_temperature_s32);
    if (com_rslt != SUCCESS) {
        ESP_LOGE(TAG, "ABORT. bmp280_get_forced_uncomp_pressure_temperature() failed err %i", com_rslt);
        f_retval = ESP_FAIL;
        // LABEL
        goto cleanup;
    }

    // compensate sensor values & transfer values to my data store
    data->pressure_hpascal = bmp280_compensate_pressure_double(v_uncomp_pressure_s32) / 100; // Pascal => HPa
    data->temperature_celsius = bmp280_compensate_temperature_double(v_uncomp_temperature_s32);

    /* STEP *LAST: go to sleep
     * Deinit Bosch driver
     *  For de-initialization it is required to set the mode of the sensor as "SLEEP"
     *  In SLEEP mode no measurements are performed
     */
    ESP_LOGD(TAG, "do bmp280_set_power_mode(BMP280_SLEEP_MODE)");
    com_rslt = bmp280_set_power_mode(BMP280_SLEEP_MODE);
    if (com_rslt != SUCCESS) {
        ESP_LOGE(TAG, "ABORT. bmp280_set_power_mode(BMP280_SLEEP_MODE) failed err %i", com_rslt);
        f_retval = ESP_FAIL;
        // LABEL
        goto cleanup;
    }

    /*  DEBUG Power mode.
     *  @brief This API used to get the Operational Mode from the sensor in the register 0xF4 bit 0 and 1
     *  @param v_power_mode_u8 : The value of power mode value
     *  value            |   Power mode
     * ------------------|------------------
     *  0x00             | BMP280_SLEEP_MODE
     *  0x01 and 0x02    | BMP280_FORCED_MODE
     *  0x03             | BMP280_NORMAL_MODE
     *
     *  @return results of bus communication function
     *  @retval 0 -> Success
     *  @retval -1 -> Error
     */
    ESP_LOGD(TAG, "do bmp280_get_power_mode()");

    com_rslt = bmp280_get_power_mode(&power_mode);
    ESP_LOGD(TAG, "  power_mode: 0x%hhx (d %hhu) [EXPECT 0x00 BMP280_SLEEP_MODE]", power_mode, power_mode);
    if (com_rslt != SUCCESS) {
        ESP_LOGE(TAG, "ABORT. bmp280_get_power_mode() failed err %i", com_rslt);
        f_retval = ESP_FAIL;
        // LABEL
        goto cleanup;
    }

    // LABEL
    cleanup:;

    return f_retval;
}
