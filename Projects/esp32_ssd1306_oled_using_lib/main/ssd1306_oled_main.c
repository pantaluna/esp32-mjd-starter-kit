/*
 * Includes
 *
 */
#include "mjd.h"
#include "mjd_ssd1306.h"

/*
 * Logging
 *
 */
static const char TAG[] = "myapp";

/*
 * KConfig: LED
 */
static const int MY_LED_ON_DEVBOARD_GPIO_NUM = CONFIG_MY_LED_ON_DEVBOARD_GPIO_NUM;
static const int MY_LED_ON_DEVBOARD_WIRING_TYPE = CONFIG_MY_LED_ON_DEVBOARD_WIRING_TYPE;

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
    ESP_LOGI(TAG, "  mjd_ssd1306_init()...");

    // @important Do not use ={} or ={0}
    mjd_ssd1306_config_t ssd1306_config =
    MJD_SSD1306_CONFIG_DEFAULT()
            ;
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

    // DEVTEMP
    /////mjd_rtos_wait_forever();

    // OLED
    char str_line[80];
    for (uint32_t j=10; j > 0; j--) {
        ESP_LOGI(TAG, "Show message#%u on the OLED...",j);

        sprintf(str_line, "message#%u", j);
        mjd_ssd1306_cmd_write_line(&ssd1306_config, MJD_SSD1306_LINE_NR_1, str_line);
        mjd_ssd1306_cmd_write_line(&ssd1306_config, MJD_SSD1306_LINE_NR_2, "12345678901234567890");

        vTaskDelay(RTOS_DELAY_1SEC);
    }

    ESP_LOGI(TAG, "  mjd_ssd1306_cmd_clear_screen()...");
    mjd_ssd1306_cmd_clear_screen(&ssd1306_config);

    ESP_LOGI(TAG, "  mjd_ssd1306_cmd_write_line(), this text stays on the display...");
    mjd_ssd1306_cmd_write_line(&ssd1306_config, MJD_SSD1306_LINE_NR_1, "00 XX XX gg");
    mjd_ssd1306_cmd_write_line(&ssd1306_config, MJD_SSD1306_LINE_NR_2, "00 XX gg XX");

    /*
     * DE-INIT
     */
    ESP_LOGI(TAG, "  mjd_ssd1306_deinit()...");
    f_retval = mjd_ssd1306_deinit(&ssd1306_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). mjd_ssd1306_deinit() | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
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
    xReturned = xTaskCreatePinnedToCore(&peripheral_task, "peripheral_task (name)", MYAPP_RTOS_TASK_STACK_SIZE_8K, NULL,
    RTOS_TASK_PRIORITY_NORMAL,
    NULL,
    APP_CPU_NUM);
    if (xReturned == pdPASS) {
        ESP_LOGI(TAG, "OK Task has been created, and is running right now");
    }

    ESP_LOGI(TAG, "app_main() END");
}
