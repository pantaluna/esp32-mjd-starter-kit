#include "mjd.h"
#include "mjd_bmp280.h"

/*
 * Logging
 */
/////static const char *TAG = "myapp";
static const char TAG[] = "myapp";

/*
 * KConfig: LED, SENSOR
 */
static const int MY_LED_ON_DEVBOARD_GPIO_NUM = CONFIG_MY_LED_ON_DEVBOARD_GPIO_NUM;
static const int MY_LED_ON_DEVBOARD_WIRING_TYPE = CONFIG_MY_LED_ON_DEVBOARD_WIRING_TYPE;

static const int MY_TEMPERATURE_SENSOR_I2C_MASTER_NUM = CONFIG_MY_TEMPERATURE_SENSOR_I2C_MASTER_NUM;
static const int MY_TEMPERATURE_SENSOR_I2C_SLAVE_ADDRESS = CONFIG_MY_TEMPERATURE_SENSOR_I2C_SLAVE_ADDRESS;
static const int MY_TEMPERATURE_SENSOR_I2C_SCLK_GPIO_NUM = CONFIG_MY_TEMPERATURE_SENSOR_I2C_SCLK_GPIO_NUM;
static const int MY_TEMPERATURE_SENSOR_I2C_SDA_GPIO_NUM = CONFIG_MY_TEMPERATURE_SENSOR_I2C_SDA_GPIO_NUM;

/*
 * FreeRTOS settings
 */
#define MYAPP_RTOS_TASK_STACK_SIZE_LARGE (8192)

/*
 * Project Globs
 */


/*
 * INIT
 */

/*
 * TASK
 */
void sensor_task(void *pvParameter) {
    esp_err_t retval;

    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    mjd_led_config_t led_config = { 0 };
    led_config.gpio_num = MY_LED_ON_DEVBOARD_GPIO_NUM; // (Huzzah32 #13) (Lolin32lite #22)
    led_config.wiring_type = MY_LED_ON_DEVBOARD_WIRING_TYPE; // (Huzzah32 1=GND) (Lolin32lite 2=VCC)
    mjd_led_config(&led_config);

    mjd_bmp280_config_t bmp280_config = MJD_BMP280_CONFIG_DEFAULT(); // @important Do not use ={} or ={0}
    bmp280_config.i2c_port_num = MY_TEMPERATURE_SENSOR_I2C_MASTER_NUM;
    bmp280_config.i2c_slave_addr = MY_TEMPERATURE_SENSOR_I2C_SLAVE_ADDRESS;
    bmp280_config.i2c_scl_gpio_num = MY_TEMPERATURE_SENSOR_I2C_SCLK_GPIO_NUM;
    bmp280_config.i2c_sda_gpio_num = MY_TEMPERATURE_SENSOR_I2C_SDA_GPIO_NUM;
    retval = mjd_bmp280_init(&bmp280_config);
    if (retval != ESP_OK) {
        ESP_LOGE(TAG, "mjd_bmp280_init() err %i %s", retval, esp_err_to_name(retval));
        // GOTO
        goto cleanup;
    }

    int total_nbr_of_error_readings = 0;

    while (1) {
        mjd_led_on(MY_LED_ON_DEVBOARD_GPIO_NUM);

        mjd_bmp280_data_t bmp280_data = { 0 };

        retval = mjd_bmp280_read_forced(&bmp280_config, &bmp280_data);
        if (retval != ESP_OK) {
            total_nbr_of_error_readings++;
            ESP_LOGE(TAG, "BMP280 Sensor failure");
        } else {
            printf("***BMP280 SENSOR READING (I2C addr 0x%x): pressure %.5f HPa | temperature %.2f *C | count error readings: %d\n", bmp280_config.i2c_slave_addr, bmp280_data.pressure_hpascal, bmp280_data.temperature_celsius, total_nbr_of_error_readings);
        }

        mjd_led_off(MY_LED_ON_DEVBOARD_GPIO_NUM);

        ESP_LOGD(TAG, "@doc Wait 5sec at the end of the while(1)");
        vTaskDelay(RTOS_DELAY_5SEC);
    }

    retval = mjd_bmp280_deinit(&bmp280_config);
    if (retval != ESP_OK) {
        ESP_LOGE(TAG, "mjd_bmp280_deinit() err %i %s", retval, esp_err_to_name(retval));
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
    mjd_log_wakeup_details();
    mjd_log_chip_info();
    mjd_log_time();
    mjd_log_memory_statistics();
    ESP_LOGI(TAG, "@doc Wait 2 seconds after power-on (start logic analyzer, let sensors become active!)");
    vTaskDelay(RTOS_DELAY_2SEC);

    /*
     * Sensor Task
     */
    xReturned = xTaskCreatePinnedToCore(&sensor_task, "sensor_task (name)", MYAPP_RTOS_TASK_STACK_SIZE_LARGE, NULL, RTOS_TASK_PRIORITY_NORMAL, NULL, APP_CPU_NUM);
    if (xReturned == pdPASS) {
        printf("OK Task sensor_task has been created, and is running right now\n");
    }

    ESP_LOGI(TAG, "app_main() END");
}
