#include "mjd.h"
#include "mjd_sht3x.h"

/*
 * Logging
 */
static const char TAG[] = "myapp";

/*
 * KConfig: LED
 */
static const int MY_LED_ON_DEVBOARD_GPIO_NUM = CONFIG_MY_LED_ON_DEVBOARD_GPIO_NUM;
static const int MY_LED_ON_DEVBOARD_WIRING_TYPE = CONFIG_MY_LED_ON_DEVBOARD_WIRING_TYPE;

static const int MY_SHT3X_I2C_SLAVE_ADDRESS = CONFIG_MY_SHT3X_I2C_SLAVE_ADDRESS;
static const int MY_SHT3X_I2C_MASTER_PORT_NUM = CONFIG_MY_SHT3X_I2C_MASTER_PORT_NUM;
static const int MY_SHT3X_I2C_SCL_GPIO_NUM = CONFIG_MY_SHT3X_I2C_SCL_GPIO_NUM;
static const int MY_SHT3X_I2C_SDA_GPIO_NUM = CONFIG_MY_SHT3X_I2C_SDA_GPIO_NUM;
static const int MY_SHT3X_SHT3X_REPEATABILITY = CONFIG_MY_SHT3X_REPEATABILITY;

/*
 * FreeRTOS settings
 */
#define MYAPP_RTOS_TASK_STACK_SIZE_8K (8192)

/*
 * Project Globs
 */

/*
 * INIT ONCE
 */

/*
 * TASK
 */
void peripheral_task(void *pvParameter) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval;

    mjd_led_config_t led_config =
        { 0 };
    led_config.gpio_num = MY_LED_ON_DEVBOARD_GPIO_NUM; // (Huzzah32 #13) (Lolin32lite #22)
    led_config.wiring_type = MY_LED_ON_DEVBOARD_WIRING_TYPE; // (Huzzah32 1=GND) (Lolin32lite 2=VCC)
    mjd_led_config(&led_config);

    /*
     * DEV INIT
     */
    ESP_LOGI(TAG, "do mjd_sht3x_init()");

    // @important Do not use ={} or ={0}
    mjd_sht3x_config_t sht3x_config =
    MJD_SHT3X_CONFIG_DEFAULT() ;

        sht3x_config.i2c_slave_addr = MY_SHT3X_I2C_SLAVE_ADDRESS;
        sht3x_config.i2c_port_num = MY_SHT3X_I2C_MASTER_PORT_NUM;
        sht3x_config.i2c_scl_gpio_num = MY_SHT3X_I2C_SCL_GPIO_NUM;
        sht3x_config.i2c_sda_gpio_num = MY_SHT3X_I2C_SDA_GPIO_NUM;
        // @tip Omit the following line if you want to use the default Repeatability
            sht3x_config.repeatability = MY_SHT3X_SHT3X_REPEATABILITY;

            f_retval = mjd_sht3x_init(&sht3x_config);
            if (f_retval != ESP_OK) {
                ESP_LOGE(TAG, "%s(). mjd_sht3x_init() err %i %s", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
                // GOTO
                goto cleanup;
            }

            /*
             * LOG DEVICE PARAMS (read registers)
             */
            mjd_sht3x_log_device_parameters(&sht3x_config);

            /*
             * LOOP
             */
            mjd_sht3x_data_t sht3x_data =
            {   0};
            uint32_t nbr_of_errors = 0;
            double sum_rh = 0,
    min_rh = FLT_MAX, max_rh = FLT_MIN;
    double sum_tc = 0, min_tc = FLT_MAX, max_tc = FLT_MIN;
    double sum_tf = 0, min_tf = FLT_MAX, max_tf = FLT_MIN;
    double sum_dpc = 0, min_dpc = FLT_MAX, max_dpc = FLT_MIN;
    mjd_log_time();
    const uint32_t NBR_OF_RUNS = 100; // 1 10 100 1000 10000
    ESP_LOGI(TAG, "LOOP: NBR_OF_RUNS %u", NBR_OF_RUNS);
    for (uint32_t j = 1; j <= NBR_OF_RUNS; ++j) {
        mjd_led_on(MY_LED_ON_DEVBOARD_GPIO_NUM);

        ESP_LOGI(TAG, "  ***SHT3X MEAS#%u***", j);

        // A0
        f_retval = mjd_sht3x_cmd_get_single_measurement(&sht3x_config, &sht3x_data);
        if (f_retval != ESP_OK) {
            mjd_led_mark_error(MY_LED_ON_DEVBOARD_GPIO_NUM);
            ++nbr_of_errors;
            ESP_LOGE(TAG, "%s(). Cannot get single measurement | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
            // CONTINUE
            continue;
        }
        ESP_LOGI(TAG, "    RelHum: %7.3f | Temp C: %7.3f | Temp F: %7.3f | DewPoint C: %7.3f", sht3x_data.relative_humidity,
                sht3x_data.temperature_celsius, sht3x_data.temperature_fahrenheit, sht3x_data.dew_point_celsius);

        sum_rh += sht3x_data.relative_humidity;
        if (sht3x_data.relative_humidity < min_rh) {
            min_rh = sht3x_data.relative_humidity;
        }
        if (sht3x_data.relative_humidity > max_rh) {
            max_rh = sht3x_data.relative_humidity;
        }

        sum_tc += sht3x_data.temperature_celsius;
        if (sht3x_data.temperature_celsius < min_tc) {
            min_tc = sht3x_data.temperature_celsius;
        }
        if (sht3x_data.temperature_celsius > max_tc) {
            max_tc = sht3x_data.temperature_celsius;
        }

        sum_tf += sht3x_data.temperature_fahrenheit;
        if (sht3x_data.temperature_fahrenheit < min_tf) {
            min_tf = sht3x_data.temperature_fahrenheit;
        }
        if (sht3x_data.temperature_fahrenheit > max_tf) {
            max_tf = sht3x_data.temperature_fahrenheit;
        }

        sum_dpc += sht3x_data.dew_point_celsius;
        if (sht3x_data.dew_point_celsius < min_dpc) {
            min_dpc = sht3x_data.dew_point_celsius;
        }
        if (sht3x_data.dew_point_celsius > max_dpc) {
            max_dpc = sht3x_data.dew_point_celsius;
        }

        mjd_led_off(MY_LED_ON_DEVBOARD_GPIO_NUM);

        // @optional A visual delay between reading loop items, is sometimes easier when debugging
        /////vTaskDelay(RTOS_DELAY_1SEC);

    }
    ESP_LOGI(TAG, "REPORT:");
    ESP_LOGI(TAG, "  NBR_OF_RUNS:   %u", NBR_OF_RUNS);
    ESP_LOGI(TAG, "  nbr_of_errors: %u", nbr_of_errors);
    ESP_LOGI(TAG, "    METRIC                        avg        min        max");
    ESP_LOGI(TAG, "    ------                 ---------- ---------- ----------");
    ESP_LOGI(TAG, "    Relative Humidity      %10.3f %10.3f %10.3f", sum_rh / NBR_OF_RUNS, min_rh, max_rh);
    ESP_LOGI(TAG, "    Temperature Celsius    %10.3f %10.3f %10.3f", sum_tc / NBR_OF_RUNS, min_tc, max_tc);
    ESP_LOGI(TAG, "    Temperature Fahrenheit %10.3f %10.3f %10.3f", sum_tf / NBR_OF_RUNS, min_tf, max_tf);
    ESP_LOGI(TAG, "    Dew Point Celsius      %10.3f %10.3f %10.3f", sum_dpc / NBR_OF_RUNS, min_dpc, max_dpc);

    /*
     * DEVICE DE-INIT
     */
    f_retval = mjd_sht3x_deinit(&sht3x_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_sht3x_deinit() | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    cleanup: ;

    mjd_log_time();

    // HALT (end of task)
    mjd_rtos_wait_forever();

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
    xReturned = xTaskCreatePinnedToCore(&peripheral_task, "peripheral_task (name)", MYAPP_RTOS_TASK_STACK_SIZE_8K, NULL, RTOS_TASK_PRIORITY_NORMAL,
    NULL,
    APP_CPU_NUM);
    if (xReturned == pdPASS) {
        ESP_LOGI(TAG, "OK Task has been created, and is running right now");
    }

    ESP_LOGI(TAG, "app_main() END");
}
