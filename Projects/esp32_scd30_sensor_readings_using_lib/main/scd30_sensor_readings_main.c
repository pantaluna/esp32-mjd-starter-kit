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
 */
static const int MY_LED_ON_DEVBOARD_GPIO_NUM = CONFIG_MY_LED_ON_DEVBOARD_GPIO_NUM;
static const int MY_LED_ON_DEVBOARD_WIRING_TYPE = CONFIG_MY_LED_ON_DEVBOARD_WIRING_TYPE;

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
 */
static const uint32_t NBR_OF_MEASUREMENT_RUNS = 100000; // 1 5 10 100 1000 10000 100000

/*
 * INIT ONCE
 */

/*
 * TASK
 */
void peripheral_task(void *pvParameter) {
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
    // @important Do not use ={} or ={0}
    mjd_ssd1306_config_t ssd1306_config =
    MJD_SSD1306_CONFIG_DEFAULT()
            ;
    if (MY_SSD1306_OLED_ENABLED == 1) {
        ssd1306_config.i2c_slave_addr = MY_SSD1306_I2C_SLAVE_ADDRESS;
        ssd1306_config.i2c_port_num = MY_SSD1306_I2C_MASTER_PORT_NUM;
        ssd1306_config.i2c_scl_gpio_num = MY_SSD1306_I2C_SCL_GPIO_NUM;
        ssd1306_config.i2c_sda_gpio_num = MY_SSD1306_I2C_SDA_GPIO_NUM;
        f_retval = mjd_ssd1306_init(&ssd1306_config);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "%s(). mjd_ssd1306_init() err %i %s", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
            // GOTO
            goto cleanup;
        }
        mjd_ssd1306_cmd_write_line(&ssd1306_config, MJD_SSD1306_LINE_NR_1, "No readings");
        mjd_ssd1306_cmd_write_line(&ssd1306_config, MJD_SSD1306_LINE_NR_2, "yet");
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
    f_retval = mjd_scd30_log_device_parameters(&scd30_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_scd30_log_device_parameters() err %i %s", __FUNCTION__, f_retval,
                esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /*
     * Start Continuous Measurement @ 0mBar
     * @doc Setting the argument to zero will deactivate the ambient pressure compensation.
     *
     * @examples param MJD_SCD30_AMBIENT_PRESSURE_DISABLED | 1020 (the average barometric pressure in Antwerp is 1020mBar).
     */
    ESP_LOGI(TAG, "  mjd_scd30_cmd_trigger_continuous_measurement()...");
    f_retval = mjd_scd30_cmd_trigger_continuous_measurement(&scd30_config, 1020);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_scd30_cmd_trigger_continuous_measurement() err %i %s", __FUNCTION__, f_retval,
                esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // DEVTEMP
    /////mjd_rtos_wait_forever();

    /*
     * LOOP
     */
    mjd_scd30_data_t scd30_data =
                { 0 };

    double sum_co2 = 0;
    double min_co2 = FLT_MAX;
    double max_co2 = FLT_MIN;
    double sum_rh = 0;
    double min_rh = FLT_MAX;
    double max_rh = FLT_MIN;
    double sum_tc = 0;
    double min_tc = FLT_MAX;
    double max_tc = FLT_MIN;
    double sum_tf = 0;
    double min_tf = FLT_MAX;
    double max_tf = FLT_MIN;
    double sum_dpc = 0;
    double min_dpc = FLT_MAX;
    double max_dpc = FLT_MIN;
    double sum_dpf = 0;
    double min_dpf = FLT_MAX;
    double max_dpf = FLT_MIN;

    uint32_t nbr_of_valid_runs = 0;
    uint32_t nbr_of_errors = 0;

    mjd_log_time();

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

        ESP_LOGI(TAG, "  mjd_scd30_cmd_read_measurement...");
        f_retval = mjd_scd30_cmd_read_measurement(&scd30_config, &scd30_data);
        if (f_retval != ESP_OK) {
            mjd_led_mark_error(MY_LED_ON_DEVBOARD_GPIO_NUM);
            ++nbr_of_errors;
            ESP_LOGE(TAG, "%s(). Cannot read measurement | err %i (%s)", __FUNCTION__, f_retval,
                    esp_err_to_name(f_retval));
            // CONTINUE (not BREAK!)
            continue;
        }

        ESP_LOGD(TAG, "    Dump raw data");
        for (uint32_t k = 0; k < ARRAY_SIZE(scd30_data.raw_data); k++) {
            ESP_LOGD(TAG, "      scd30_data->raw_data[%u]: 0x%04X %4u", k, scd30_data.raw_data[k], scd30_data.raw_data[k]);
        }

        ESP_LOGI(TAG, "    CO2: %6.1f | Temp C: %6.1f | Temp F: %6.1f | RelHum: %6.1f | DewPnt C: %6.1f | DewPnt F: %6.1f",
                scd30_data.co2_ppm,
                scd30_data.temperature_celsius, scd30_data.temperature_fahrenheit,
                scd30_data.relative_humidity,
                scd30_data.dew_point_celsius, scd30_data.dew_point_fahrenheit);
        ESP_LOGI(TAG, "    EU IDA Air Quality Category: %u - %s (%s)", scd30_data.eu_ida_category,
                scd30_data.eu_ida_category_code,
                scd30_data.eu_ida_category_desc);

        // OLED
        if (MY_SSD1306_OLED_ENABLED == 1) {
            char str_line[80];
            sprintf(str_line, "#%u ppm:", j);
            mjd_ssd1306_cmd_write_line(&ssd1306_config, MJD_SSD1306_LINE_NR_1, str_line);
            sprintf(str_line, "%6.1f", scd30_data.co2_ppm);
            mjd_ssd1306_cmd_write_line(&ssd1306_config, MJD_SSD1306_LINE_NR_2, str_line);
        }

        // STATS
        sum_co2 += scd30_data.co2_ppm;
        if (scd30_data.co2_ppm < min_co2) {
            min_co2 = scd30_data.co2_ppm;
        }
        if (scd30_data.co2_ppm > max_co2) {
            max_co2 = scd30_data.co2_ppm;
        }

        sum_tc += scd30_data.temperature_celsius;
        if (scd30_data.temperature_celsius < min_tc) {
            min_tc = scd30_data.temperature_celsius;
        }
        if (scd30_data.temperature_celsius > max_tc) {
            max_tc = scd30_data.temperature_celsius;
        }

        sum_tf += scd30_data.temperature_fahrenheit;
        if (scd30_data.temperature_fahrenheit < min_tf) {
            min_tf = scd30_data.temperature_fahrenheit;
        }
        if (scd30_data.temperature_fahrenheit > max_tf) {
            max_tf = scd30_data.temperature_fahrenheit;
        }

        sum_rh += scd30_data.relative_humidity;
        if (scd30_data.relative_humidity < min_rh) {
            min_rh = scd30_data.relative_humidity;
        }
        if (scd30_data.relative_humidity > max_rh) {
            max_rh = scd30_data.relative_humidity;
        }

        sum_dpc += scd30_data.dew_point_celsius;
        if (scd30_data.dew_point_celsius < min_dpc) {
            min_dpc = scd30_data.dew_point_celsius;
        }
        if (scd30_data.dew_point_celsius > max_dpc) {
            max_dpc = scd30_data.dew_point_celsius;
        }

        sum_dpf += scd30_data.dew_point_fahrenheit;
        if (scd30_data.dew_point_fahrenheit < min_dpf) {
            min_dpf = scd30_data.dew_point_fahrenheit;
        }
        if (scd30_data.dew_point_fahrenheit > max_dpf) {
            max_dpf = scd30_data.dew_point_fahrenheit;
        }

        nbr_of_valid_runs++; // For calculating correct averages.

        mjd_led_off(MY_LED_ON_DEVBOARD_GPIO_NUM);

        // @optional A visual delay between reading loop items, is sometimes easier when debugging
        /////vTaskDelay(RTOS_DELAY_1SEC);

    }

    ESP_LOGI(TAG, "REPORT:");
    ESP_LOGI(TAG, "  NBR_OF_MEASUREMENT_RUNS: %u", NBR_OF_MEASUREMENT_RUNS);
    ESP_LOGI(TAG, "  nbr_of_valid_runs:       %u", nbr_of_valid_runs);
    ESP_LOGI(TAG, "  nbr_of_errors:           %u", nbr_of_errors);
    ESP_LOGI(TAG, "    METRIC                        avg        min        max");
    ESP_LOGI(TAG, "    ------                 ---------- ---------- ----------");
    ESP_LOGI(TAG, "    CO2 ppm                %10.3f %10.3f %10.3f", sum_co2 / nbr_of_valid_runs, min_co2, max_co2);
    ESP_LOGI(TAG, "    Temperature Celsius    %10.3f %10.3f %10.3f", sum_tc / nbr_of_valid_runs, min_tc, max_tc);
    ESP_LOGI(TAG, "    Temperature Fahrenheit %10.3f %10.3f %10.3f", sum_tf / nbr_of_valid_runs, min_tf, max_tf);
    ESP_LOGI(TAG, "    Relative Humidity      %10.3f %10.3f %10.3f", sum_rh / nbr_of_valid_runs, min_rh, max_rh);
    ESP_LOGI(TAG, "    Dew Point Celsius      %10.3f %10.3f %10.3f", sum_dpc / nbr_of_valid_runs, min_dpc, max_dpc);
    ESP_LOGI(TAG, "    Dew Point Fahrenheit   %10.3f %10.3f %10.3f", sum_dpf / nbr_of_valid_runs, min_dpf, max_dpf);

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
     * DEVICE DE-INIT
     */
    ESP_LOGI(TAG, "  mjd_scd30_deinit()...");
    f_retval = mjd_scd30_deinit(&scd30_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_scd30_deinit() | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

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
     * Sensor Task
     */
    xReturned = xTaskCreatePinnedToCore(&peripheral_task, "peripheral_task (name)", MYAPP_RTOS_TASK_STACK_SIZE_8K, NULL,
    RTOS_TASK_PRIORITY_NORMAL,
    NULL,
    APP_CPU_NUM);
    if (xReturned == pdPASS) {
        ESP_LOGI(TAG, "OK Task has been created, and is running right now");
    }

    ESP_LOGI(TAG, "app_main() END");
}
