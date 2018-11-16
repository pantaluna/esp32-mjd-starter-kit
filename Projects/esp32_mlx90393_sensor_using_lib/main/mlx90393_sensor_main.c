#include "mjd.h"
#include "mjd_mlx90393.h"

/*
 * Logging
 */
static const char TAG[] = "myapp";

/*
 * KConfig: LED, SENSOR
 */
static const int MY_LED_ON_DEVBOARD_GPIO_NUM = CONFIG_MY_LED_ON_DEVBOARD_GPIO_NUM;
static const int MY_LED_ON_DEVBOARD_WIRING_TYPE = CONFIG_MY_LED_ON_DEVBOARD_WIRING_TYPE;

static const int MY_SENSOR_I2C_MASTER_NUM = CONFIG_MY_SENSOR_I2C_MASTER_NUM;
static const int MY_SENSOR_I2C_SLAVE_ADDRESS = CONFIG_MY_SENSOR_I2C_SLAVE_ADDRESS;
static const int MY_SENSOR_I2C_SCL_GPIO_NUM = CONFIG_MY_SENSOR_I2C_SCL_GPIO_NUM;
static const int MY_SENSOR_I2C_SDA_GPIO_NUM = CONFIG_MY_SENSOR_I2C_SDA_GPIO_NUM;
static const int MY_SENSOR_INT_GPIO_NUM = CONFIG_MY_SENSOR_INT_GPIO_NUM;

/*
 * FreeRTOS settings
 */
#define MYAPP_RTOS_TASK_STACK_SIZE_LARGE (8192)

/*
 * Project Globs
 */

/*
 * INIT ONCE
 */

/*
 * TASK
 */
void sensor_task(void *pvParameter) {
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
    ESP_LOGI(TAG, "do mjd_mlx90393_init()");

    // @important Do not use ={} or ={0}
    mjd_mlx90393_config_t mlx90393_config = MJD_MLX90393_CONFIG_DEFAULT()
    ;
    mlx90393_config.i2c_port_num = MY_SENSOR_I2C_MASTER_NUM;
    mlx90393_config.i2c_slave_addr = MY_SENSOR_I2C_SLAVE_ADDRESS;
    mlx90393_config.i2c_scl_gpio_num = MY_SENSOR_I2C_SCL_GPIO_NUM;
    mlx90393_config.i2c_sda_gpio_num = MY_SENSOR_I2C_SDA_GPIO_NUM;
    mlx90393_config.int_gpio_num = MY_SENSOR_INT_GPIO_NUM;

    f_retval = mjd_mlx90393_init(&mlx90393_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_mlx90393_init() err %i %s", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Optional: you can set various device parameters right now using the mjd_mlx90393_set_*() functions [The default params are set in the func mjd_mlx90393_init()].

    /*
     * LOG DEVICE PARAMS (registers)
     */
    mjd_mlx90393_log_device_parameters(&mlx90393_config);

    /*
     * LOOP
     */
    ESP_LOGI(TAG, "LOOP:");

    mjd_mlx90393_data_t mlx90393_data =
        { 0 };
    uint32_t nbr_of_error_start_meas = 0;
    uint32_t nbr_of_error_read_meas = 0;

    const uint32_t NBR_OF_RUNS = 10000;
    for (uint32_t j = 0; j < NBR_OF_RUNS; ++j) {
        mjd_led_on(MY_LED_ON_DEVBOARD_GPIO_NUM);

        ESP_LOGD(TAG, "Command: START MEASUREMENT");
        f_retval = mjd_mlx90393_cmd_start_measurement(&mlx90393_config);
        if (f_retval != ESP_OK) {
            ++nbr_of_error_start_meas;
            ESP_LOGW(TAG, "%s(). Cannot start Mode SM | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
            // CONTINUE
            continue;
        }

        ESP_LOGD(TAG, "Command: READ MEASUREMENT");
        f_retval = mjd_mlx90393_cmd_read_measurement(&mlx90393_config, &mlx90393_data);
        if (f_retval != ESP_OK) {
            ++nbr_of_error_read_meas;
            ESP_LOGE(TAG, "%s(). Cannot Read Measurement (the %u th RM failed) | err %i (%s)", __FUNCTION__, j, f_retval, esp_err_to_name(f_retval));
            // CONTINUE
            continue;
        }

        ESP_LOGI(TAG,
                "***MLX90393 MEAS*** X %.2f | Y %.2f | Z %.2f | T %.2f *C  ||  RAW X 0x%" PRIX16 " (%" PRIu16") | RAW Y 0x%" PRIX16 " (%" PRIu16") | RAW Z 0x%" PRIX16 " (%" PRIu16") | RAW T 0x%" PRIX16 " (%" PRIu16") ",
                mlx90393_data.x, mlx90393_data.y, mlx90393_data.z, mlx90393_data.t, mlx90393_data.x_raw, mlx90393_data.x_raw, mlx90393_data.y_raw,
                mlx90393_data.y_raw, mlx90393_data.z_raw, mlx90393_data.z_raw, mlx90393_data.t_raw, mlx90393_data.t_raw);

        mjd_led_off(MY_LED_ON_DEVBOARD_GPIO_NUM);

        // @optional A visual delay between reading loop items
        /////vTaskDelay(RTOS_DELAY_1SEC);

    }
    ESP_LOGI(TAG, "%s(). REPORT. NBR_OF_RUNS %u | nbr_of_error_start_meas %u | nbr_of_error_read_meas %u", __FUNCTION__, NBR_OF_RUNS,
            nbr_of_error_start_meas, nbr_of_error_read_meas);

    /*
     * DEVICE DE-INIT
     */
    f_retval = mjd_mlx90393_deinit(&mlx90393_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_mlx90393_deinit() | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    cleanup: ;

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

    /* MY STANDARD Init */
    mjd_log_chip_info();
    mjd_log_memory_statistics();
    ESP_LOGI(TAG, "@tip You can also change the log level to DEBUG for more detailed logging and to get insights in what the component is actually doing.");
    ESP_LOGI(TAG, "@doc Wait 2 seconds after power-on (start logic analyzer, let sensors become active, ...)");
    vTaskDelay(RTOS_DELAY_2SEC);

    /*
     * Sensor Task
     */
    xReturned = xTaskCreatePinnedToCore(&sensor_task, "sensor_task (name)", MYAPP_RTOS_TASK_STACK_SIZE_LARGE, NULL, RTOS_TASK_PRIORITY_NORMAL, NULL,
    APP_CPU_NUM);
    if (xReturned == pdPASS) {
        ESP_LOGI(TAG, "OK Task sensor_task has been created, and is running right now");
    }

    ESP_LOGI(TAG, "app_main() END");
}
