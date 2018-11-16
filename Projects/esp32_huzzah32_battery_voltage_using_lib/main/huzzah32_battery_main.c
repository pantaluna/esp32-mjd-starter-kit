/*
 * HARDWARE SETUP the MJD components:
 *  *NONE
 *
 */
#include "mjd.h"
#include "mjd_huzzah32.h"


/*
 * Logging
 */
static const char TAG[] = "myapp";

/*
 * KConfig: LED, WIFI
 */
static const int MY_LED_ON_DEVBOARD_GPIO_NUM = CONFIG_MY_LED_ON_DEVBOARD_GPIO_NUM;
static const int MY_LED_ON_DEVBOARD_WIRING_TYPE = CONFIG_MY_LED_ON_DEVBOARD_WIRING_TYPE;

/*
 * FreeRTOS settings
 */
#define MYAPP_RTOS_TASK_STACK_SIZE_LARGE (8192)
#define MYAPP_RTOS_TASK_PRIORITY_NORMAL (RTOS_TASK_PRIORITY_NORMAL)


/*
 * TASK
 */
void main_task(void *pvParameter) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    /********************************************************************************
     * MY STANDARD Init
     *
     */
    /* MY STANDARD Init */
    mjd_log_wakeup_details();
    mjd_log_chip_info();
    mjd_log_time();
    mjd_log_memory_statistics();
    ESP_LOGI(TAG, "@doc Wait X seconds after power-on (start logic analyzer, let peripherals active)");
    vTaskDelay(RTOS_DELAY_1SEC);

    /********************************************************************************
     * LED
     *
     */
    ESP_LOGI(TAG, "\n\n***SECTION: LED***");
    ESP_LOGI(TAG, "  MY_LED_ON_DEVBOARD_GPIO_NUM:    %i", MY_LED_ON_DEVBOARD_GPIO_NUM);
    ESP_LOGI(TAG, "  MY_LED_ON_DEVBOARD_WIRING_TYPE: %i", MY_LED_ON_DEVBOARD_WIRING_TYPE);

    mjd_led_config_t led_config =
        { 0 };
    led_config.gpio_num = MY_LED_ON_DEVBOARD_GPIO_NUM;
    led_config.wiring_type = MY_LED_ON_DEVBOARD_WIRING_TYPE; // 1 GND MCU Huzzah32 | 2 VCC MCU Lolin32lite
    mjd_led_config(&led_config);

    /**
     * @brief Route the actual VREF Voltage Reference of the ESP32 to GPIO#26. Then use a multimeter to determine the voltage. The value will be around 1100mV.
     *        PS This code is not needed to measure the actual battery voltage later on (we just need to measure the value once).
     *        GPIO#26 is the 5th pin down from the top left of the HUZZAH32 dev board.
     *
     * The VREF for the HUZZAH32 that I'm using right now is 1086mV. And this value is entered using `make menuconfig` => Components => Adafruit HUZZAH32
     * and used in the func that reads the battery level.
     *
     * @return
     *     - ESP_OK Success
     */
    ESP_LOGI(TAG, "***Routing the actual VREF Voltage Reference of the ESP32 to GPIO#26.");
    ESP_LOGI(TAG, "***    Use a multimeter to determine the voltage. The value will be around 1100mV.");
    ESP_LOGI(TAG, "");
    f_retval = mjd_huzzah32_route_vref_to_gpio();
    if (f_retval != ESP_OK) {
        goto cleanup;
    }

    /********************************************************************************
     * Adafruit HUZZAH32: Report the features of the current ESP32 board about the ADC Calibration Characterization.
     *  Figure out if the ESP32 eFuse contains specific information for that.
     */
    mjd_huzzah32_log_adc_efuses();
    vTaskDelay(RTOS_DELAY_1SEC);

    /********************************************************************************
     * Adafruit HUZZAH32: read battery voltage level
     */
    ESP_LOGI(TAG, "LOOP: battery measurements");
    ESP_LOGI(TAG, "");

    while (1) {
        mjd_led_on(MY_LED_ON_DEVBOARD_GPIO_NUM);
        ESP_LOGI(TAG, "Actual battery voltage (V): %f [Compare it with the measurement on your multimeter]", mjd_huzzah32_get_battery_voltage());
        mjd_led_off(MY_LED_ON_DEVBOARD_GPIO_NUM);

        vTaskDelay(RTOS_DELAY_2SEC);
    }

    // LABEL
    cleanup:

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
