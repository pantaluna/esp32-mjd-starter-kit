#include "mjd_jsnsr04t.h"

/*
 * Logging
 */
static const char TAG[] = "myapp";

/*
 * KConfig:
 * - LED
 * - sensor GPIO's
 */
static const int MY_LED_ON_DEVBOARD_GPIO_NUM = CONFIG_MY_LED_ON_DEVBOARD_GPIO_NUM;
static const int MY_LED_ON_DEVBOARD_WIRING_TYPE = CONFIG_MY_LED_ON_DEVBOARD_WIRING_TYPE;

static const int MY_JSNSR04T_TRIGGER_GPIO_NUM = CONFIG_MY_JSNSR04T_TRIGGER_GPIO_NUM; // @default 16
static const int MY_JSNSR04T_ECHO_GPIO_NUM = CONFIG_MY_JSNSR04T_ECHO_GPIO_NUM; // @default 14

/*
 * FreeRTOS settings
 */
#define MYAPP_RTOS_TASK_STACK_SIZE_LARGE (8192)
#define MYAPP_RTOS_TASK_PRIORITY_NORMAL (RTOS_TASK_PRIORITY_NORMAL)

/*
 * TASKS
 */
void main_task(void *pvParameter) {
    ESP_LOGI(TAG, "%s()", __FUNCTION__);

    /********************************************************************************
     * Reuseable variables
     */
    esp_err_t f_retval = ESP_OK;

    /*********************************
     * LOGGING
     * Optional for Production: dump less messages
     * @doc It is possible to lower the log level for specific modules (wifi and tcpip_adapter are strong candidates)
     * @important Disable u8g2_hal DEBUG messages which are too detailed for me.
     */
    esp_log_level_set("u8g2_hal", ESP_LOG_INFO);

    /********************************************************************************
     * STANDARD Init
     */
    mjd_log_wakeup_details();
    mjd_log_chip_info();
    mjd_log_time();
    mjd_log_memory_statistics();
    /////ESP_LOGI(TAG, "@doc Wait X seconds after power-on (start logic analyzer, let peripherals become active, ...)");
    /////vTaskDelay(RTOS_DELAY_1SEC);

    /********************************************************************************
     * LED
     */
    mjd_led_config_t led_config =
                { 0 };
    led_config.gpio_num = MY_LED_ON_DEVBOARD_GPIO_NUM;
    led_config.wiring_type = MY_LED_ON_DEVBOARD_WIRING_TYPE; // 1 GND MCU Huzzah32 | 2 VCC MCU Lolin32lite
    mjd_led_config(&led_config);

    /********************************************************************************
     * Init component JSNSR04
     */
    mjd_jsnsr04t_config_t jsnsr04t_config = MJD_JSNSR04T_CONFIG_DEFAULT();
    jsnsr04t_config.trigger_gpio_num = MY_JSNSR04T_TRIGGER_GPIO_NUM;
    jsnsr04t_config.echo_gpio_num = MY_JSNSR04T_ECHO_GPIO_NUM;
    jsnsr04t_config.rmt_channel = RMT_CHANNEL_0;

    f_retval = mjd_jsnsr04t_init(&jsnsr04t_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "mjd_jsnsr04t_init() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    mjd_jsnsr04t_log_config(jsnsr04t_config);

    /********************************************************************************
     * Get data from JSNSR04
     */

    const uint32_t NBR_OF_READS = 100; // 1 10 50 100 500 1000 100000
    ESP_LOGI(TAG, "LOOP: NBR_OF_READS %u", NBR_OF_READS);

    mjd_log_memory_statistics();

    for (uint32_t j = 1; j <= NBR_OF_READS; ++j) {
        ESP_LOGI(TAG, "");
        ESP_LOGI(TAG, "***LOOP ITEM #%u of %u***", j, NBR_OF_READS);

        mjd_led_on(MY_LED_ON_DEVBOARD_GPIO_NUM);

        // MEASURE
        mjd_jsnsr04t_data_t jsnsr04t_data = MJD_JSNSR04T_DATA_DEFAULT();
        f_retval = mjd_jsnsr04t_get_measurement(&jsnsr04t_config, &jsnsr04t_data);
        if (f_retval == ESP_OK) {
            mjd_jsnsr04t_log_data(jsnsr04t_data);
        } else {
            ESP_LOGE(TAG, "[CONTINUE FOR-LOOP] mjd_jsnsr04t_get_measurement() failed | err %i (%s)", f_retval,
                    esp_err_to_name(f_retval));
            // GOTO
            /////goto cleanup;
        }

        mjd_led_off(MY_LED_ON_DEVBOARD_GPIO_NUM);
    }

    mjd_log_memory_statistics();

    /********************************************************************************
     * DeInit component JSNSR04
     */
    f_retval = mjd_jsnsr04t_deinit(&jsnsr04t_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "mjd_jsnsr04t_deinit() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    mjd_jsnsr04t_log_config(jsnsr04t_config);

    /********************************************************************************
     * LABEL
     */
    cleanup: ;

    /*
     * LOG TIME
     */
    mjd_log_time();

    /********************************************************************************
     * Task Delete
     * @doc Passing NULL will end the current task
     */
    ESP_LOGI(TAG, "END OF %s()", __FUNCTION__);
    vTaskDelete(NULL);
}

/*
 * MAIN
 */
void app_main() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    mjd_log_memory_statistics();

    /**********
     * CREATE TASK:
     * @important For stability (RMT + Wifi etc.): always use xTaskCreatePinnedToCore(APP_CPU_NUM) [Opposed to xTaskCreate()]
     */
    BaseType_t xReturned;
    xReturned = xTaskCreatePinnedToCore(&main_task, "main_task (name)", MYAPP_RTOS_TASK_STACK_SIZE_LARGE, NULL,
    MYAPP_RTOS_TASK_PRIORITY_NORMAL, NULL,
    APP_CPU_NUM);
    if (xReturned == pdPASS) {
        ESP_LOGI(TAG, "OK Task has been created, and is running right now");
    }

    /**********
     * END
     */
    ESP_LOGI(TAG, "END %s()", __FUNCTION__);
}
