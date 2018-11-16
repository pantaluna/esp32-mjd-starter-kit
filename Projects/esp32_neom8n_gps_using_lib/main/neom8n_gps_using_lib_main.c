#include "mjd.h"
#include "mjd_neom8n.h"

/*
 * Logging
 */
static const char TAG[] = "myapp";

/*
 * KConfig: LED
 */
static const int MY_LED_ON_DEVBOARD_GPIO_NUM = CONFIG_MY_LED_ON_DEVBOARD_GPIO_NUM;
static const int MY_LED_ON_DEVBOARD_WIRING_TYPE = CONFIG_MY_LED_ON_DEVBOARD_WIRING_TYPE;

static const int MY_GPS_UART_NUM = CONFIG_MY_GPS_UART_NUM; // default UART_NUM_1
static const int MY_GPS_UART_RX_GPIO_NUM = CONFIG_MY_GPS_UART_RX_GPIO_NUM; // default 23
static const int MY_GPS_UART_TX_GPIO_NUM = CONFIG_MY_GPS_UART_TX_GPIO_NUM; // default 22

/*
 * FreeRTOS settings
 */
#define MYAPP_RTOS_TASK_STACK_SIZE_LARGE (8192)
#define MYAPP_RTOS_TASK_PRIORITY_NORMAL (RTOS_TASK_PRIORITY_NORMAL)

/*
 * TASKS
 */
void main_task(void *pvParameter) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    /********************************************************************************
     * Reuseable variables
     */
    esp_err_t f_retval;

    mjd_log_memory_statistics();

    /********************************************************************************
     * GPS NEO-M8N
     *
     */
    ESP_LOGI(TAG, "\n\n***SECTION: GPS NEO-M8N***");
    ESP_LOGI(TAG, "  TIP: Set the ESP_LOG_LEVEL to INFO to hide the massive DEBUG/VERBOSE messages");
    ESP_LOGI(TAG, "  TIP: Set the ESP_LOG_LEVEL to DEBUG to show the DEBUG messages");
    ESP_LOGI(TAG, "  TIP: Set the ESP_LOG_LEVEL to VERBOSE to also show a HEXDUMP of the incoming UAT buffers");

    ESP_LOGI(TAG, "INIT (default, without cold start)");
    mjd_neom8n_config_t neom8n_config = MJD_NEOM8N_CONFIG_DEFAULT();
    neom8n_config.uart_port = MY_GPS_UART_NUM;
    neom8n_config.uart_rx_gpio_num = MY_GPS_UART_RX_GPIO_NUM;
    neom8n_config.uart_tx_gpio_num = MY_GPS_UART_TX_GPIO_NUM;

    f_retval = mjd_neom8n_init(&neom8n_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "mjd_neom8n_init() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /********************************************************************************
     * MAIN LOOP: READ GPS DATA
     *
     */
    mjd_neom8n_data_t neom8n_data;

    ESP_LOGI(TAG,
            "LOOP: retrieve the current readings every 5 seconds (this interval can be changed), or retrieve the readings just once in your app");
    while (1) {
        mjd_led_on(MY_LED_ON_DEVBOARD_GPIO_NUM);

        f_retval = mjd_neom8n_read(&neom8n_config, &neom8n_data);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "mjd_neom8n_read() err %i (%s)", f_retval, esp_err_to_name(f_retval));
            // GOTO
            goto cleanup;
        }

        mjd_log_time();
        printf("*** GPS READING: data_received: %s | fix_quality %i | latitude %f | longitude %f | satellites_tracked %i\n",
                neom8n_data.data_received ? "true" : "false", neom8n_data.fix_quality, neom8n_data.latitude,
                neom8n_data.longitude, neom8n_data.satellites_tracked);
        printf("***              https://www.google.com/maps/search/%f°,%f°\n", neom8n_data.latitude, neom8n_data.longitude);

        mjd_led_off(MY_LED_ON_DEVBOARD_GPIO_NUM);

        vTaskDelay(RTOS_DELAY_5SEC);
    }

    // LABEL
    cleanup: ;

    mjd_led_mark_error(MY_LED_ON_DEVBOARD_GPIO_NUM);

    /********************************************************************************
     * Task Delete
     * @doc Passing NULL will end the current task
     */
    vTaskDelete(NULL);
}

void change_settings_task(void *pvParameter) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    mjd_log_memory_statistics();

    /********************************************************************************
     * GPS NEO-M8N
     *
     */
    mjd_neom8n_config_t neom8n_config = MJD_NEOM8N_CONFIG_DEFAULT();
    neom8n_config.uart_port = MY_GPS_UART_NUM;
    neom8n_config.uart_rx_gpio_num = MY_GPS_UART_RX_GPIO_NUM;
    neom8n_config.uart_tx_gpio_num = MY_GPS_UART_TX_GPIO_NUM;

    /********************************************************************************
     * Some useful commands to control the device startup, device power state, and state of the GNSS Receiver.
     * These commands are optional.
     *
     */
    ESP_LOGI(TAG,
            "[SKIPPED] Do a cold start - All satellite information would lost so it would take a while to get a 3D Fix on the satellites again");
    /*mjd_neom8n_cold_start_forced(&neom8n_config);
     vTaskDelay(RTOS_DELAY_15SEC);*/

    ESP_LOGI(TAG,
            "change_settings_task(): UBX Power down max 15 seconds and Power Up again after *5* seconds (you can still read out the data using the component)");
    mjd_neom8n_power_down_for_15_seconds(&neom8n_config);
    vTaskDelay(RTOS_DELAY_15SEC);
    mjd_neom8n_power_up(&neom8n_config);
    vTaskDelay(RTOS_DELAY_5SEC);

    ESP_LOGI(TAG,
            "change_settings_task(): UBX Power down infinitely and Power Up again after *15* seconds (you can still read out the data using the component)");
    mjd_neom8n_power_down(&neom8n_config);
    vTaskDelay(RTOS_DELAY_15SEC);
    mjd_neom8n_power_up(&neom8n_config);
    vTaskDelay(RTOS_DELAY_15SEC);

    ESP_LOGI(TAG,
            "change_settings_task(): UBX GNSS Stop for 15 seconds. The GPs will stop to take measurements and reporting data via UART (you can still read out the data using the component)");
    mjd_neom8n_gnss_stop(&neom8n_config);
    vTaskDelay(RTOS_DELAY_15SEC);

    ESP_LOGI(TAG,
            "change_settings_task(): UBX GNSS Start. The GPS will resume to take measurements and reporting data via UART (you can still read out the data using the component)");
    mjd_neom8n_gnss_start(&neom8n_config);
    vTaskDelay(RTOS_DELAY_15SEC);

    /********************************************************************************
     * Some useful commands to control the measurement rate related commands to control the GPS device.
     * These commands are optional.
     *
     */
    ESP_LOGI(TAG,
            "change_settings_task(): UBX Measurement Rate = 100 milliseconds (faster) - you can still read out the data using the component at your own pace)");
    mjd_neom8n_set_measurement_rate_100ms(&neom8n_config);
    vTaskDelay(RTOS_DELAY_15SEC);

    ESP_LOGI(TAG,
            "change_settings_task(): UBX Measurement Rate = 5000 milliseconds (slower) - you can still read out the data using the component at your own pace)");
    mjd_neom8n_set_measurement_rate_5000ms(&neom8n_config);
    vTaskDelay(RTOS_DELAY_15SEC);

    ESP_LOGI(TAG,
            "change_settings_task(): UBX Measurement Rate = 1000 milliseconds (default) - you can still read out the data using the component at your own pace)");
    mjd_neom8n_set_measurement_rate_1000ms(&neom8n_config);
    vTaskDelay(RTOS_DELAY_15SEC);

    /********************************************************************************
     * Task Delete
     * @doc Passing NULL will end the current task
     */
    ESP_LOGI(TAG, "change_settings_task(): deleting this task");
    ESP_LOGI(TAG, "The normal operation of the component is now active.");
    vTaskDelete(NULL);
}

/*
 * MAIN
 */
void app_main() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    BaseType_t xReturned;

    /********************************************************************************
     * STANDARD Init
     */
    mjd_log_wakeup_details();
    mjd_log_chip_info();
    mjd_log_time();
    mjd_log_memory_statistics();
    ESP_LOGI(TAG, "@doc Wait X seconds after power-on (start logic analyzer, let sensors become active!)");
    vTaskDelay(RTOS_DELAY_1SEC);

    /********************************************************************************
     * LED
     */
    ESP_LOGI(TAG, "\n\n***SECTION: LED***");
    ESP_LOGI(TAG, "MY_LED_ON_DEVBOARD_GPIO_NUM:    %i", MY_LED_ON_DEVBOARD_GPIO_NUM);
    ESP_LOGI(TAG, "MY_LED_ON_DEVBOARD_WIRING_TYPE: %i", MY_LED_ON_DEVBOARD_WIRING_TYPE);

    mjd_log_memory_statistics();

    mjd_led_config_t led_config =
        { 0 };
    led_config.gpio_num = MY_LED_ON_DEVBOARD_GPIO_NUM;
    led_config.wiring_type = MY_LED_ON_DEVBOARD_WIRING_TYPE; // 1 GND MCU Huzzah32 | 2 VCC MCU Lolin32lite
    mjd_led_config(&led_config);

    /**********
     * TASK: main_task
     *  @important For stability (RMT + Wifi etc.): always use xTaskCreatePinnedToCore(APP_CPU_NUM) [Opposed to xTaskCreate() which might run the code on PRO_CPU_NUM...]
     */
    xReturned = xTaskCreatePinnedToCore(&main_task, "main_task (name)", MYAPP_RTOS_TASK_STACK_SIZE_LARGE, NULL,
    MYAPP_RTOS_TASK_PRIORITY_NORMAL, NULL,
    APP_CPU_NUM);
    if (xReturned == pdPASS) {
        ESP_LOGI(TAG, "OK Task main_task has been created, and is running right now");
    }

    /**********
     * TASK: change_settings_task
     *  @important Start this task at least 5 seconds after starting the main_task so that the GPS-UART interface has been started by the main_task.
     *  @important For stability (RMT + Wifi etc.): always use xTaskCreatePinnedToCore(APP_CPU_NUM) [Opposed to xTaskCreate() which might run the code on PRO_CPU_NUM...]
     */
    vTaskDelay(RTOS_DELAY_5SEC);

    xReturned = xTaskCreatePinnedToCore(&change_settings_task, "change_settings_task (name)",
    MYAPP_RTOS_TASK_STACK_SIZE_LARGE, NULL,
    MYAPP_RTOS_TASK_PRIORITY_NORMAL, NULL,
    APP_CPU_NUM);
    if (xReturned == pdPASS) {
        ESP_LOGI(TAG, "OK Task change_settings_task has been created, and is running right now");
    }

    /**********
     * END
     */
    ESP_LOGI(TAG, "END %s()", __FUNCTION__);
}
