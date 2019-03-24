#include "mjd.h"
#include "mjd_bme280.h"

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
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t retval;

    mjd_led_config_t led_config =
        { 0 };
    led_config.gpio_num = MY_LED_ON_DEVBOARD_GPIO_NUM; // (Huzzah32 #13) (Lolin32lite #22)
    led_config.wiring_type = MY_LED_ON_DEVBOARD_WIRING_TYPE; // (Huzzah32 1=GND) (Lolin32lite 2=VCC)
    mjd_led_config(&led_config);

    ESP_LOGI(TAG, "fill bme280_config");
    // @important Do not use ={} or ={0}
    mjd_bme280_config_t bme280_config = MJD_BME280_CONFIG_DEFAULT()
    ;
    bme280_config.i2c_port_num = MY_TEMPERATURE_SENSOR_I2C_MASTER_NUM;
    bme280_config.i2c_slave_addr = MY_TEMPERATURE_SENSOR_I2C_SLAVE_ADDRESS;
    bme280_config.i2c_scl_gpio_num = MY_TEMPERATURE_SENSOR_I2C_SCLK_GPIO_NUM;
    bme280_config.i2c_sda_gpio_num = MY_TEMPERATURE_SENSOR_I2C_SDA_GPIO_NUM;

    ESP_LOGI(TAG, "do mjd_bme280_init()");
    retval = mjd_bme280_init(&bme280_config);
    if (retval != ESP_OK) {
        ESP_LOGE(TAG, "mjd_bme280_init() err %i %s", retval, esp_err_to_name(retval));
        mjd_led_mark_error(MY_LED_ON_DEVBOARD_GPIO_NUM);
        // GOTO
        goto cleanup;
    }

    /*
     * TESTING 123: register BME280_CHIP_ID_ADDR
     */
    ESP_LOGI(TAG, "TESTING123: read register 0xD0 (BME280_CHIP_ID_ADDR):");

    int8_t bosch_retval;
    uint8_t reg_value;

    bosch_retval = bme280_get_regs(BME280_CHIP_ID_ADDR, &reg_value, 1, &bme280_config.bme280_device);
    if (bosch_retval != BME280_OK) {
        ESP_LOGE(TAG, "ABORT. bme280_get_regs(-chipid-) failed err %i", bosch_retval);
        mjd_led_mark_error(MY_LED_ON_DEVBOARD_GPIO_NUM);
        // LABEL
        goto cleanup;
    }
    ESP_LOGI(TAG, "  chip_id: 0x%x (dec %hhu) [EXPECT 0x60 for sensor BME280]", reg_value, reg_value);

    // DEVTEMP
    /////mjd_rtos_wait_forever();

    /*
     * LOOP
     * */
    ESP_LOGI(TAG, "LOOP:");

    uint32_t total_nbr_of_error_readings = 0;

    while (1) {
        mjd_led_on(MY_LED_ON_DEVBOARD_GPIO_NUM);

        mjd_bme280_data_t bme280_data =
            { 0 };

        retval = mjd_bme280_read_forced(&bme280_config, &bme280_data);
        if (retval != ESP_OK) {
            ++total_nbr_of_error_readings;
            ESP_LOGE(TAG, "BME280 Sensor failure");
            mjd_led_mark_error(MY_LED_ON_DEVBOARD_GPIO_NUM);
        } else {
            printf(
                    "***BME280 SENSOR READING (I2C addr 0x%x): humidity %.5f %% | pressure %.5f HPa | temperature %.2f *C | count error readings: %u\n",
                    bme280_config.i2c_slave_addr, bme280_data.humidity_percent, bme280_data.pressure_hpascal, bme280_data.temperature_celsius,
                    total_nbr_of_error_readings);
        }

        mjd_led_off(MY_LED_ON_DEVBOARD_GPIO_NUM);

        ESP_LOGD(TAG, "@doc Wait 3sec at the end of the while(1)");
        vTaskDelay(RTOS_DELAY_3SEC);
    }

    // Never gets here by design...
    retval = mjd_bme280_deinit(&bme280_config);
    if (retval != ESP_OK) {
        ESP_LOGE(TAG, "mjd_bme280_deinit() err %i %s", retval, esp_err_to_name(retval));
        mjd_led_mark_error(MY_LED_ON_DEVBOARD_GPIO_NUM);
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
    xReturned = xTaskCreatePinnedToCore(&sensor_task, "sensor_task (name)", MYAPP_RTOS_TASK_STACK_SIZE_LARGE, NULL, RTOS_TASK_PRIORITY_NORMAL, NULL,
    APP_CPU_NUM);
    if (xReturned == pdPASS) {
        printf("OK Task sensor_task has been created, and is running right now\n");
    }

    ESP_LOGI(TAG, "app_main() END");
}
