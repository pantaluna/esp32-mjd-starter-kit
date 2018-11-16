/*
 * HARDWARE SETUP the MJD components: README.MD
 *
 *
 */
#include "driver/rtc_io.h"

#include "mjd.h"

/*
 * Logging
 */
static const char TAG[] = "myapp";

/*
 * FreeRTOS settings
 */
#define MYAPP_RTOS_TASK_STACK_SIZE_LARGE (8192)
#define MYAPP_RTOS_TASK_PRIORITY_NORMAL (RTOS_TASK_PRIORITY_NORMAL)

/*
 * TODO DEVICE INFO
 */

/*
 * KConfig: LED, WIFI
 */
static const int MY_LED_ON_DEVBOARD_GPIO_NUM = CONFIG_MY_LED_ON_DEVBOARD_GPIO_NUM;
static const int MY_LED_ON_DEVBOARD_WIRING_TYPE = CONFIG_MY_LED_ON_DEVBOARD_WIRING_TYPE;
static const int MY_GPIO_INPUT_PIN = CONFIG_MY_GPIO_INPUT_PIN;

/*
 * TASK
 */
void main_task(void *pvParameter) {
    ESP_LOGI(TAG, "%s()", __FUNCTION__);

    mjd_log_memory_statistics();

    /********************************************************************************
     * Reuseable variables
     *
     */

    /********************************************************************************
     * MY STANDARD Init
     *
     */
    mjd_log_time();
    mjd_log_memory_statistics();

    mjd_log_wakeup_details();
    mjd_increment_mcu_boot_count();
    mjd_log_mcu_boot_count();
    vTaskDelay(RTOS_DELAY_2SEC);

    ESP_LOGI(TAG, "@doc Wait X seconds after power-on (start logic analyzer, let sensors become active, ...)");
    vTaskDelay(RTOS_DELAY_1SEC);

    /********************************************************************************
     * LED Config
     *
     */
    ESP_LOGI(TAG, "\n\n***SECTION: LED***");
    ESP_LOGI(TAG, "MY_LED_ON_DEVBOARD_GPIO_NUM:    %i", MY_LED_ON_DEVBOARD_GPIO_NUM);
    ESP_LOGI(TAG, "MY_LED_ON_DEVBOARD_WIRING_TYPE: %i", MY_LED_ON_DEVBOARD_WIRING_TYPE);

    mjd_led_config_t led_config =
        { 0 };
    led_config.gpio_num = MY_LED_ON_DEVBOARD_GPIO_NUM;
    led_config.wiring_type = MY_LED_ON_DEVBOARD_WIRING_TYPE; // 1 GND MCU Huzzah32 | 2 VCC MCU Lolin32lite
    mjd_led_config(&led_config);

    /********************************************************************************
     * DEEP SLEEP & RESTART TIMER
     *   @sop Put this section in comments when testing other things afterwards (else the MCU restarts every time...)
     *   @important In deep sleep mode, wireless peripherals are powered down. Before entering sleep mode, applications must disable WiFi and BT using appropriate calls ( esp_bluedroid_disable(), esp_bt_controller_disable(), esp_wifi_stop()).
     *   @doc https://esp-idf.readthedocs.io/en/latest/api-reference/system/sleep_modes.html
     *
     *   @doc esp_sleep_enable_ext0_wakeup @param level input level which will trigger wakeup (0=low, 1=high)
     *
     */
    ESP_LOGI(TAG, "\n\n***SECTION: DEEP SLEEP***");

    mjd_log_memory_statistics();

    ESP_LOGI(TAG, "LED on");
    mjd_led_on(MY_LED_ON_DEVBOARD_GPIO_NUM);

    // MODE: TIMER
    /*
     const uint32_t MY_DEEP_SLEEP_TIME_SEC = 5; // 5 15*60 30*60
     esp_sleep_enable_timer_wakeup(mjd_seconds_to_microseconds(MY_DEEP_SLEEP_TIME_SEC));
     ESP_LOGI(TAG, "Entering deep sleep (the MCU should wake up %u seconds later)...", MY_DEEP_SLEEP_TIME_SEC);
     vTaskDelay(RTOS_DELAY_1SEC);
     esp_deep_sleep_start();
     */

    // MODE: RTC GPIO
    //   @important Wait a bit so that the ESP_LOGI() can dump buffer to UART before deep sleep kicks in!
    if (rtc_gpio_is_valid_gpio(MY_GPIO_INPUT_PIN)) {
        rtc_gpio_pulldown_en(MY_GPIO_INPUT_PIN);
        esp_sleep_enable_ext0_wakeup(MY_GPIO_INPUT_PIN, 1);
        ESP_LOGI(TAG, "Entering deep sleep (the MCU should wake up when GPIO#%u is HIGH)..", MY_GPIO_INPUT_PIN);
        vTaskDelay(RTOS_DELAY_1SEC);
        esp_deep_sleep_start();
    } else {
        ESP_LOGE(TAG, "ABORT. GPIO#%u is not a valid RTC GPIO PIN#!", MY_GPIO_INPUT_PIN);
    }

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

    mjd_log_memory_statistics();

    /**********
     * TASK:
     * @important For stability (RMT + Wifi): always use xTaskCreatePinnedToCore(APP_CPU_NUM) [Opposed to xTaskCreate()]
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
