#include "mjd.h"
#include "mjd_bh1750fvi.h"

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

static const int MY_LIGHT_SENSOR_I2C_MASTER_NUM = CONFIG_MY_LIGHT_SENSOR_I2C_MASTER_NUM;
static const int MY_LIGHT_SENSOR_I2C_SLAVE_ADDRESS = CONFIG_MY_LIGHT_SENSOR_I2C_SLAVE_ADDRESS;
static const int MY_LIGHT_SENSOR_I2C_SCLK_GPIO_NUM = CONFIG_MY_LIGHT_SENSOR_I2C_SCLK_GPIO_NUM;
static const int MY_LIGHT_SENSOR_I2C_SDA_GPIO_NUM = CONFIG_MY_LIGHT_SENSOR_I2C_SDA_GPIO_NUM;

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

    mjd_bh1750fvi_config_t bh1750fvi_config = MJD_BH1750FVI_CONFIG_DEFAULT(); // @important Do not use ={} or ={0}
    bh1750fvi_config.i2c_port_num = MY_LIGHT_SENSOR_I2C_MASTER_NUM;
    bh1750fvi_config.i2c_slave_addr = MY_LIGHT_SENSOR_I2C_SLAVE_ADDRESS;
    bh1750fvi_config.i2c_scl_gpio_num = MY_LIGHT_SENSOR_I2C_SCLK_GPIO_NUM;
    bh1750fvi_config.i2c_sda_gpio_num = MY_LIGHT_SENSOR_I2C_SDA_GPIO_NUM;

    // Testing other sensor modes (HR2 = default)
    //bh1750fvi_config.bh1750fvi_mode = BH1750FVI_REGISTER_ONE_TIME_LOW_RES_MODE;
    //bh1750fvi_config.bh1750fvi_mode = BH1750FVI_REGISTER_ONE_TIME_HIGH_RES_MODE;

    retval = mjd_bh1750fvi_init(&bh1750fvi_config);
    if (retval != ESP_OK) {
        ESP_LOGE(TAG, "mjd_bh1750fvi_init() err %i %s", retval, esp_err_to_name(retval));
        goto cleanup;
        // GOTO
    }

    int total_nbr_of_error_readings = 0;

    while (1) {
        mjd_led_on(MY_LED_ON_DEVBOARD_GPIO_NUM);

        mjd_bh1750fvi_data_t bh1750fvi_data = { 0 };

        retval = mjd_bh1750fvi_read(&bh1750fvi_config, &bh1750fvi_data);
        if (retval != ESP_OK) {
            total_nbr_of_error_readings++;
            ESP_LOGE(TAG, "BH1750FVI Sensor failure | count error readings: %d\n", total_nbr_of_error_readings);
        } else {
            printf("***BH1750FVI SENSOR READING (I2C addr 0x%x): light intensity %.1f Lux | count error readings: %d\n", bh1750fvi_config.i2c_slave_addr, bh1750fvi_data.light_intensity_lux, total_nbr_of_error_readings);
        }

        mjd_led_off(MY_LED_ON_DEVBOARD_GPIO_NUM);

        // @doc technically no delay is needed for the sensor.
        //ESP_LOGD(TAG, "@doc Wait 2sec at the end of the while(1)");
        //vTaskDelay(RTOS_DELAY_2SEC);
    }

    retval = mjd_bh1750fvi_deinit(&bh1750fvi_config);
    if (retval != ESP_OK) {
        ESP_LOGE(TAG, "mjd_bh1750fvi_deinit() err %i %s", retval, esp_err_to_name(retval));
        goto cleanup;
        // GOTO
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
