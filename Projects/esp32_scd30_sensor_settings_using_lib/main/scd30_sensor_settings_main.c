/*
 * Includes
 *
 */
#include "mjd.h"
#include "mjd_scd30.h"
#include "mjd_ssd1306.h"

/*
 * Logging
 *
 */
static const char TAG[] = "myapp";

/*
 * KConfig: LED SCD340 SSD1306
 *
 */
static const int MY_LED_ON_DEVBOARD_GPIO_NUM = CONFIG_MY_LED_ON_DEVBOARD_GPIO_NUM;
static const int MY_LED_ON_DEVBOARD_WIRING_TYPE = CONFIG_MY_LED_ON_DEVBOARD_WIRING_TYPE;

static const int MY_SCD30_CALIBRATION_COMMAND = CONFIG_MY_SCD30_CALIBRATION_COMMAND;

static const int MY_SCD30_I2C_SLAVE_ADDRESS = CONFIG_MY_SCD30_I2C_SLAVE_ADDRESS;
static const int MY_SCD30_I2C_MASTER_PORT_NUM = CONFIG_MY_SCD30_I2C_MASTER_PORT_NUM;
static const int MY_SCD30_I2C_SCL_GPIO_NUM = CONFIG_MY_SCD30_I2C_SCL_GPIO_NUM;
static const int MY_SCD30_I2C_SDA_GPIO_NUM = CONFIG_MY_SCD30_I2C_SDA_GPIO_NUM;

static const int MY_SSD1306_OLED_ENABLED = CONFIG_MY_SSD1306_OLED_ENABLED;
static const int MY_SSD1306_I2C_SLAVE_ADDRESS = CONFIG_MY_SSD1306_I2C_SLAVE_ADDRESS;
static const int MY_SSD1306_I2C_MASTER_PORT_NUM = CONFIG_MY_SSD1306_I2C_MASTER_PORT_NUM;
static const int MY_SSD1306_I2C_SCL_GPIO_NUM = CONFIG_MY_SSD1306_I2C_SCL_GPIO_NUM;
static const int MY_SSD1306_I2C_SDA_GPIO_NUM = CONFIG_MY_SSD1306_I2C_SDA_GPIO_NUM;

/*
 * FreeRTOS settings
 */
#define MYAPP_RTOS_TASK_STACK_SIZE_8K (8192)

/*
 * Project Globs
 *
 */
typedef enum {
    MY_SCD30_CALIBRATION_COMMAND_SHOW_SETTINGS = 1,
    MY_SCD30_CALIBRATION_COMMAND_ASC_ON = 2,
    MY_SCD30_CALIBRATION_COMMAND_ASC_OFF = 3,
    MY_SCD30_CALIBRATION_COMMAND_FRC = 4,
} mjd_scd30_calibration_command_t;

// OLED
// @important Do not use ={} or ={0}
static mjd_ssd1306_config_t _ssd1306_config =
    MJD_SSD1306_CONFIG_DEFAULT()
            ;

/*
 * RUN CALIB MODE functions
 */
static esp_err_t _run_calibration_command_show_settings() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval;

    /*
     * OLED
     */
    if (MY_SSD1306_OLED_ENABLED == 1) {
        mjd_ssd1306_cmd_write_line(&_ssd1306_config, MJD_SSD1306_LINE_NR_1, "SCD30 INIT");
    }

    /*
     * SCD30 INIT
     */
    ESP_LOGI(TAG, "do mjd_scd30_init()");

    // @important Do not use ={} or ={0}
    mjd_scd30_config_t scd30_config =
    MJD_SCD30_CONFIG_DEFAULT()
            ;

    scd30_config.i2c_slave_addr = MY_SCD30_I2C_SLAVE_ADDRESS;
    scd30_config.i2c_port_num = MY_SCD30_I2C_MASTER_PORT_NUM;
    scd30_config.i2c_scl_gpio_num = MY_SCD30_I2C_SCL_GPIO_NUM;
    scd30_config.i2c_sda_gpio_num = MY_SCD30_I2C_SDA_GPIO_NUM;

    f_retval = mjd_scd30_init(&scd30_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_scd30_init() err %i %s", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /*
     * LOG DEVICE PARAMS (read registers)
     */
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "=> Note down the values of these device parameters:");
    ESP_LOGI(TAG, "     automatic_self_calibration");
    ESP_LOGI(TAG, "     forced_recalibration_value");
    ESP_LOGI(TAG, "");

    f_retval = mjd_scd30_log_device_parameters(&scd30_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_scd30_log_device_parameters() err %i %s", __FUNCTION__, f_retval,
                esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /*
     * DEVICE DE-INIT
     */
    ESP_LOGI(TAG, "  mjd_scd30_deinit()...");
    f_retval = mjd_scd30_deinit(&scd30_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_scd30_deinit() | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /*
     * OLED
     */
    if (MY_SSD1306_OLED_ENABLED == 1) {
        mjd_ssd1306_cmd_write_line(&_ssd1306_config, MJD_SSD1306_LINE_NR_1, "Show Settings");
        mjd_ssd1306_cmd_write_line(&_ssd1306_config, MJD_SSD1306_LINE_NR_2, "on UART");
    }

    cleanup: ;

    return f_retval;
}

static esp_err_t _run_calibration_command_asc_on() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval;

    /*
     * SCD30 INIT
     */
    ESP_LOGI(TAG, "do mjd_scd30_init()");

    // @important Do not use ={} or ={0}
    mjd_scd30_config_t scd30_config =
    MJD_SCD30_CONFIG_DEFAULT()
            ;

    scd30_config.i2c_slave_addr = MY_SCD30_I2C_SLAVE_ADDRESS;
    scd30_config.i2c_port_num = MY_SCD30_I2C_MASTER_PORT_NUM;
    scd30_config.i2c_scl_gpio_num = MY_SCD30_I2C_SCL_GPIO_NUM;
    scd30_config.i2c_sda_gpio_num = MY_SCD30_I2C_SDA_GPIO_NUM;

    f_retval = mjd_scd30_init(&scd30_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_scd30_init() err %i %s", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /*
     * Calibration: ASC=On
     *
     */
    ESP_LOGI(TAG, "Setting ASC=On Activate continuous calculation of reference value for Automatic Self-Calibration");
    ESP_LOGI(TAG, "  - When activated for the first time a period of minimum *7* days is needed");
    ESP_LOGI(TAG, "  so that the algorithm can find its initial parameter set for ASC!");

    f_retval = mjd_scd30_cmd_set_automatic_self_calibration(&scd30_config, MJD_SCD30_ASC_AUTOMATIC_SELF_CALIBRATION_YES);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_scd30_cmd_set_automatic_self_calibration() err %i %s", __FUNCTION__, f_retval,
                esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /*
     * Start Continuous Measurement @ 0mBar
     * @doc Setting the argument to zero will deactivate the ambient pressure compensation.
     */
    ESP_LOGI(TAG, "ASC only works in continuous measurement mode!");
    ESP_LOGI(TAG, "  mjd_scd30_cmd_trigger_continuous_measurement()...");
    f_retval = mjd_scd30_cmd_trigger_continuous_measurement(&scd30_config, MJD_SCD30_AMBIENT_PRESSURE_DISABLED); // @examples MJD_SCD30_AMBIENT_PRESSURE_DISABLED 1015
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_scd30_cmd_trigger_continuous_measurement() err %i %s", __FUNCTION__, f_retval,
                esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /*
     * LOG DEVICE PARAMS (read registers)
     */
    f_retval = mjd_scd30_log_device_parameters(&scd30_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_scd30_log_device_parameters() err %i %s", __FUNCTION__, f_retval,
                esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /*
     * DEVICE DE-INIT
     */
    ESP_LOGI(TAG, "  mjd_scd30_deinit()...");
    f_retval = mjd_scd30_deinit(&scd30_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_scd30_deinit() | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /*
     * OLED
     */
    if (MY_SSD1306_OLED_ENABLED == 1) {
        mjd_ssd1306_cmd_write_line(&_ssd1306_config, MJD_SSD1306_LINE_NR_1, "ASC ON");
        mjd_ssd1306_cmd_write_line(&_ssd1306_config, MJD_SSD1306_LINE_NR_2, "Cont.Meas. ON");
    }

    cleanup: ;

    return f_retval;
}

static esp_err_t _run_calibration_command_asc_off() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval;

    /*
     * SCD30 INIT
     */
    ESP_LOGI(TAG, "do mjd_scd30_init()");

    // @important Do not use ={} or ={0}
    mjd_scd30_config_t scd30_config =
    MJD_SCD30_CONFIG_DEFAULT()
            ;

    scd30_config.i2c_slave_addr = MY_SCD30_I2C_SLAVE_ADDRESS;
    scd30_config.i2c_port_num = MY_SCD30_I2C_MASTER_PORT_NUM;
    scd30_config.i2c_scl_gpio_num = MY_SCD30_I2C_SCL_GPIO_NUM;
    scd30_config.i2c_sda_gpio_num = MY_SCD30_I2C_SDA_GPIO_NUM;

    f_retval = mjd_scd30_init(&scd30_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_scd30_init() err %i %s", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /*
     * Calibration: ASC=Off
     *
     */
    ESP_LOGI(TAG, "Setting ASC=Off Deactivate continuous calculation of reference value for Automatic Self-Calibration");
    f_retval = mjd_scd30_cmd_set_automatic_self_calibration(&scd30_config, MJD_SCD30_ASC_AUTOMATIC_SELF_CALIBRATION_NO);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_scd30_cmd_set_automatic_self_calibration() err %i %s", __FUNCTION__, f_retval,
                esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /*
     * STOP CONTINUOUS MEASUREMENT
     */
    ESP_LOGI(TAG, "  mjd_scd30_cmd_stop_continuous_measurement()...");
    f_retval = mjd_scd30_cmd_stop_continuous_measurement(&scd30_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_scd30_cmd_stop_continuous_measurement() err %i %s", __FUNCTION__, f_retval,
                esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /*
     * LOG DEVICE PARAMS (read registers)
     */
    f_retval = mjd_scd30_log_device_parameters(&scd30_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_scd30_log_device_parameters() err %i %s", __FUNCTION__, f_retval,
                esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /*
     * DEVICE DE-INIT
     */
    ESP_LOGI(TAG, "  mjd_scd30_deinit()...");
    f_retval = mjd_scd30_deinit(&scd30_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_scd30_deinit() | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /*
     * OLED
     */
    if (MY_SSD1306_OLED_ENABLED == 1) {
        mjd_ssd1306_cmd_write_line(&_ssd1306_config, MJD_SSD1306_LINE_NR_1, "ASC=OFF");
        mjd_ssd1306_cmd_write_line(&_ssd1306_config, MJD_SSD1306_LINE_NR_2, "Cont.Meas. OFF");
    }

    cleanup: ;

    return f_retval;
}

static esp_err_t _run_calibration_command_frc() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval;

    /*
     * LED
     */
    if (MY_SSD1306_OLED_ENABLED == 1) {
        mjd_ssd1306_cmd_write_line(&_ssd1306_config, MJD_SSD1306_LINE_NR_1, "No readings");
    }

    /*
     * SCD30 INIT
     */
    ESP_LOGI(TAG, "do mjd_scd30_init()");

    // @important Do not use ={} or ={0}
    mjd_scd30_config_t scd30_config =
    MJD_SCD30_CONFIG_DEFAULT()
            ;

    scd30_config.i2c_slave_addr = MY_SCD30_I2C_SLAVE_ADDRESS;
    scd30_config.i2c_port_num = MY_SCD30_I2C_MASTER_PORT_NUM;
    scd30_config.i2c_scl_gpio_num = MY_SCD30_I2C_SCL_GPIO_NUM;
    scd30_config.i2c_sda_gpio_num = MY_SCD30_I2C_SDA_GPIO_NUM;
    scd30_config.measurement_interval = 2; // @important for FRC: 2 seconds.

    f_retval = mjd_scd30_init(&scd30_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_scd30_init() err %i %s", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /*
     * Calibration: ASC=Off
     *
     */
    ESP_LOGI(TAG, "Setting ASC=Off Deactivate continuous calculation of reference value for Automatic Self-Calibration");

    // OLED
    if (MY_SSD1306_OLED_ENABLED == 1) {
        mjd_ssd1306_cmd_write_line(&_ssd1306_config, MJD_SSD1306_LINE_NR_1, "Set");
        mjd_ssd1306_cmd_write_line(&_ssd1306_config, MJD_SSD1306_LINE_NR_2, "ASC=Off");
    }

    f_retval = mjd_scd30_cmd_set_automatic_self_calibration(&scd30_config, MJD_SCD30_ASC_AUTOMATIC_SELF_CALIBRATION_NO);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_scd30_cmd_set_automatic_self_calibration() err %i %s", __FUNCTION__, f_retval,
                esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /*
     * Start Continuous Measurement
     *
     */
    ESP_LOGI(TAG, "mjd_scd30_cmd_trigger_continuous_measurement()...");

    // OLED
    if (MY_SSD1306_OLED_ENABLED == 1) {
        mjd_ssd1306_cmd_write_line(&_ssd1306_config, MJD_SSD1306_LINE_NR_1, "Stop");
        mjd_ssd1306_cmd_write_line(&_ssd1306_config, MJD_SSD1306_LINE_NR_2, "Cont.Meas.");
    }

    f_retval = mjd_scd30_cmd_trigger_continuous_measurement(&scd30_config, MJD_SCD30_AMBIENT_PRESSURE_DISABLED); // @examples MJD_SCD30_AMBIENT_PRESSURE_DISABLED 1015
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_scd30_cmd_trigger_continuous_measurement() err %i %s", __FUNCTION__, f_retval,
                esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /*
     * READ MEAS for 5 minutes, meas interval 2 seconds.
     *
     * @doc For best results, the sensor has to be run in a stable environment in continuous mode
     *      at a measurement rate of 2s for at least two minutes before applying the FRC command
     *      and sending the reference value.
     *
     * @formula NBR_OF_MEASUREMENT_RUNS
     *      Total Run Time:     150 seconds (3 minutes)
     *      Meas Interval:      2 seconds
     *      Nbr of Loop cycles => 150
     */
    ESP_LOGI(TAG, "Read Measurements for a total of 3 minutes (let the value stabilize)...");

    // OLED
    if (MY_SSD1306_OLED_ENABLED == 1) {
        mjd_ssd1306_cmd_write_line(&_ssd1306_config, MJD_SSD1306_LINE_NR_1, "Read.Meas.");
        mjd_ssd1306_cmd_write_line(&_ssd1306_config, MJD_SSD1306_LINE_NR_2, "3 minutes");
    }

    mjd_log_time();

    mjd_scd30_data_t scd30_data = { 0 };

    uint32_t nbr_of_valid_runs = 0;
    uint32_t nbr_of_errors = 0;

    static const uint32_t NBR_OF_MEASUREMENT_RUNS = (3 * 60) / 2;
    ESP_LOGI(TAG, "LOOP: NBR_OF_MEASUREMENT_RUNS %u", NBR_OF_MEASUREMENT_RUNS);

    for (uint32_t j = 1; j <= NBR_OF_MEASUREMENT_RUNS; ++j) {
        ESP_LOGI(TAG, "  ***SCD30 MEAS#%u of %u***...", j, NBR_OF_MEASUREMENT_RUNS);

        // GET "Data Ready Status" until := YES or timeout
        uint32_t max_nbr_of_status_checks;
        max_nbr_of_status_checks = scd30_config.measurement_interval * 3; // @example 30 = 30 seconds in combination with vTaskDelay(RTOS_DELAY_1SEC)
        mjd_scd30_data_ready_status_t data_ready_status = MJD_SCD30_DATA_READY_STATUS_NO;
        while ((data_ready_status != MJD_SCD30_DATA_READY_STATUS_YES) && (max_nbr_of_status_checks > 0)) {
            f_retval = mjd_scd30_cmd_get_data_ready_status(&scd30_config, &data_ready_status);
            if (f_retval != ESP_OK) {
                ++nbr_of_errors;
                mjd_led_mark_error(MY_LED_ON_DEVBOARD_GPIO_NUM);
                ESP_LOGE(TAG, "%s(). ABORT. mjd_scd30_cmd_get_data_ready_status() failed | err %i (%s)", __FUNCTION__,
                        f_retval,
                        esp_err_to_name(f_retval));
                // CONTINUE (not BREAK!)
                continue;
            }
            ESP_LOGD(TAG, "    ... data_ready_status: %u ...", data_ready_status);
            --max_nbr_of_status_checks;
            vTaskDelay(RTOS_DELAY_1SEC); // @important Do not change because it correlates to max_nbr_of_status_checks!
        }
        ESP_LOGI(TAG, "    ...data_ready_status: %u", data_ready_status);

        // Check no timeout
        if (data_ready_status != MJD_SCD30_DATA_READY_STATUS_YES) {
            f_retval = ESP_ERR_TIMEOUT;
            ++nbr_of_errors;
            mjd_led_mark_error(MY_LED_ON_DEVBOARD_GPIO_NUM);
            ESP_LOGE(TAG, "%s(). ABORT. Timeout checking mjd_scd30_cmd_get_data_ready_status() too many times | err %i (%s)",
                    __FUNCTION__,
                    f_retval,
                    esp_err_to_name(f_retval));
            // CONTINUE (not BREAK!)
            continue;
        }

        // Read Meas
        mjd_led_on(MY_LED_ON_DEVBOARD_GPIO_NUM);

        f_retval = mjd_scd30_cmd_read_measurement(&scd30_config, &scd30_data);
        if (f_retval != ESP_OK) {
            mjd_led_mark_error(MY_LED_ON_DEVBOARD_GPIO_NUM);
            ++nbr_of_errors;
            ESP_LOGE(TAG, "%s(). Cannot read measurement | err %i (%s)", __FUNCTION__, f_retval,
                    esp_err_to_name(f_retval));
            // CONTINUE (not BREAK!)
            continue;
        }

        ESP_LOGI(TAG, "    CO2: %6.1f | Temp C: %6.1f | Temp F: %6.1f | RelHum: %6.1f",
                scd30_data.co2_ppm,
                scd30_data.temperature_celsius, scd30_data.temperature_fahrenheit,
                scd30_data.relative_humidity);

        // OLED
        if (MY_SSD1306_OLED_ENABLED == 1) {
            char str_line[80];
            sprintf(str_line, "#%u CO2ppm", j);
            mjd_ssd1306_cmd_write_line(&_ssd1306_config, MJD_SSD1306_LINE_NR_1, str_line);
            sprintf(str_line, "%6.1f", scd30_data.co2_ppm);
            mjd_ssd1306_cmd_write_line(&_ssd1306_config, MJD_SSD1306_LINE_NR_2, str_line);
        }

        nbr_of_valid_runs++; // For calculating correct averages.

        mjd_led_off(MY_LED_ON_DEVBOARD_GPIO_NUM);

        // @optional A visual delay between reading loop items, is sometimes easier when debugging
        /////vTaskDelay(RTOS_DELAY_1SEC);

    }

    ESP_LOGI(TAG, "REPORT:");
    ESP_LOGI(TAG, "  NBR_OF_MEASUREMENT_RUNS: %u", NBR_OF_MEASUREMENT_RUNS);
    ESP_LOGI(TAG, "  nbr_of_valid_runs:       %u", nbr_of_valid_runs);
    ESP_LOGI(TAG, "  nbr_of_errors:           %u (1st and 2nd reading are always in error)", nbr_of_errors);

    /*
     * !!!SET FRC!!!
     */
    ESP_LOGI(TAG, "Set FRC = MJD_SCD30_CO2_PPM_OUTDOOR_FRESH_AIR 400ppm");

    f_retval = mjd_scd30_cmd_set_forced_recalibration_value(&scd30_config, MJD_SCD30_CO2_PPM_OUTDOOR_FRESH_AIR);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_scd30_cmd_get_forced_recalibration_value() err %i %s", __FUNCTION__, f_retval,
                esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /*
     * STOP CONTINUOUS MEASUREMENT
     */
    ESP_LOGI(TAG, "  mjd_scd30_cmd_stop_continuous_measurement()...");
    // OLED
    if (MY_SSD1306_OLED_ENABLED == 1) {
        mjd_ssd1306_cmd_write_line(&_ssd1306_config, MJD_SSD1306_LINE_NR_1, "Stop");
        mjd_ssd1306_cmd_write_line(&_ssd1306_config, MJD_SSD1306_LINE_NR_2, "Cont.Meas.");
    }


    f_retval = mjd_scd30_cmd_stop_continuous_measurement(&scd30_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_scd30_cmd_stop_continuous_measurement() err %i %s", __FUNCTION__, f_retval,
                esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /*
     * LOG DEVICE PARAMS (read registers)
     */
    f_retval = mjd_scd30_log_device_parameters(&scd30_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_scd30_log_device_parameters() err %i %s", __FUNCTION__, f_retval,
                esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /*
     * DEVICE DE-INIT
     */
    ESP_LOGI(TAG, "  mjd_scd30_deinit()...");
    f_retval = mjd_scd30_deinit(&scd30_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_scd30_deinit() | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // OLED
    if (MY_SSD1306_OLED_ENABLED == 1) {
        mjd_ssd1306_cmd_write_line(&_ssd1306_config, MJD_SSD1306_LINE_NR_1, "FRC set to");
        mjd_ssd1306_cmd_write_line(&_ssd1306_config, MJD_SSD1306_LINE_NR_2, "400 ppm");
    }

    cleanup: ;

    return f_retval;
}

/*
 * TASK
 *
 */
void main_task(void *pvParameter) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval;

    /*
     * LED
     */
    mjd_led_config_t led_config =
                { 0 };
    led_config.gpio_num = MY_LED_ON_DEVBOARD_GPIO_NUM; // (Huzzah32 #13) (Lolin32lite #22)
    led_config.wiring_type = MY_LED_ON_DEVBOARD_WIRING_TYPE; // (Huzzah32 1=GND) (Lolin32lite 2=VCC)
    mjd_led_config(&led_config);

    /*
     * OLED
     */
    if (MY_SSD1306_OLED_ENABLED == 1) {
        _ssd1306_config.i2c_slave_addr = MY_SSD1306_I2C_SLAVE_ADDRESS;
        _ssd1306_config.i2c_port_num = MY_SSD1306_I2C_MASTER_PORT_NUM;
        _ssd1306_config.i2c_scl_gpio_num = MY_SSD1306_I2C_SCL_GPIO_NUM;
        _ssd1306_config.i2c_sda_gpio_num = MY_SSD1306_I2C_SDA_GPIO_NUM;
        f_retval = mjd_ssd1306_init(&_ssd1306_config);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "%s(). mjd_ssd1306_init() err %i %s", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
            // GOTO
            goto cleanup;
        }
    }

    /*
     * Calibration Command <= menuconfig
     *
     */
    switch (MY_SCD30_CALIBRATION_COMMAND)
    {
    case MY_SCD30_CALIBRATION_COMMAND_SHOW_SETTINGS:
        f_retval = _run_calibration_command_show_settings();
        break;
    case MY_SCD30_CALIBRATION_COMMAND_ASC_ON:
        f_retval = _run_calibration_command_asc_on();
        break;
    case MY_SCD30_CALIBRATION_COMMAND_ASC_OFF:
        f_retval = _run_calibration_command_asc_off();
        break;
    case MY_SCD30_CALIBRATION_COMMAND_FRC:
        f_retval = _run_calibration_command_frc();
        break;
    default:
        ESP_LOGE(TAG, "ABORT. No task created. Invalid value for MY_SCD30_CALIBRATION_COMMAND");
    }
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). Error running the requested calibration command", __FUNCTION__);
        // GOTO
        goto cleanup;
    }

    // LABEL
    cleanup: ;

    mjd_log_time();

    /********************************************************************************
     * Task Delete
     * @doc Passing NULL will end the current task
     *
     */
    vTaskDelete(NULL);
}

/*
 * MAIN
 */
void app_main() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    BaseType_t xReturned;

    /* SOC init */
    ESP_LOGI(TAG, "@doc exec nvs_flash_init() - mandatory for Wifi to work later on");
    nvs_flash_init();

    /********************************************************************************
     * MY STANDARD Init
     *
     */
    mjd_log_chip_info();
    mjd_log_memory_statistics();
    mjd_set_timezone_utc();
    mjd_log_time();
    ESP_LOGI(TAG,
            "@tip You can also change the log level to DEBUG for more detailed logging and to get insights in what the component is actually doing.");
    ESP_LOGI(TAG, "@doc Wait 2 seconds after power-on (start logic analyzer, let peripherals become active, ...)");
    vTaskDelay(RTOS_DELAY_2SEC);

    /*
     * Task
     */
    xReturned = xTaskCreatePinnedToCore(&main_task, "main_task (name)", MYAPP_RTOS_TASK_STACK_SIZE_8K, NULL,
    RTOS_TASK_PRIORITY_NORMAL,
    NULL,
    APP_CPU_NUM);
    if (xReturned == pdPASS) {
        ESP_LOGI(TAG, "OK Task has been created, and is running right now");
    }

    ESP_LOGI(TAG, "app_main() END");
}
