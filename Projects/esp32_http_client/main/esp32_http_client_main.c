/*
 *
 */

/*
 * Includes: system, own
 */
#include "mjd.h"
#include "mjd_wifi.h"
#include "mjd_net.h"

/*
 * Logging
 */
static const char TAG[] = "myapp";

/*
 * KConfig: LED, WIFI
 */
static const int MY_LED_ON_DEVBOARD_GPIO_NUM = CONFIG_MY_LED_ON_DEVBOARD_GPIO_NUM;
static const int MY_LED_ON_DEVBOARD_WIRING_TYPE = CONFIG_MY_LED_ON_DEVBOARD_WIRING_TYPE;

static const char *MY_WIFI_SSID = CONFIG_MY_WIFI_SSID;
/////static const char *MY_WIFI_SSID = "xxx"; // @test invalid

static const char *MY_WIFI_PASSWORD = CONFIG_MY_WIFI_PASSWORD;
/////static const char *MY_WIFI_PASSWORD = "yyy"; // @test invalid

/*
 * FreeRTOS settings
 */
#define MYAPP_RTOS_TASK_STACK_SIZE_8K (8192)
#define MYAPP_RTOS_TASK_PRIORITY_NORMAL (RTOS_TASK_PRIORITY_NORMAL)

/*
 * Component: esp_http_client => HTTP Event Handler
 */
esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
        ESP_LOGD(TAG, "_http_event_handler() event - HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGD(TAG, "_http_event_handler() event - HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGD(TAG, "_http_event_handler() event - HTTP_EVENT_HEADER_SENT [BUG Event never comes] (after sending all the headers to the server)");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGD(TAG, "_http_event_handler() event - HTTP_EVENT_ON_HEADER (response header) key=%s value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGD(TAG, "_http_event_handler() event - HTTP_EVENT_ON_DATA (possible chunked) len=%d:", evt->data_len);
        if (!esp_http_client_is_chunked_response(evt->client)) {
            printf("  => %.*s\n", evt->data_len, (char*) evt->data);
        }
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGD(TAG, "_http_event_handler() event - HTTP_EVENT_ON_FINISH (when the HTTP session is finished)");
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGD(TAG, "_http_event_handler() event - HTTP_EVENT_DISCONNECTED (when the connection has been disconnected)");
        break;
    }
    return ESP_OK;
}

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
    esp_err_t f_retval;

    /********************************************************************************
     * SOC init
     *
     */
    mjd_log_memory_statistics();

// DEVTEMP_BEGIN Fix after ESP-IDF github tag v3.2-dev
#ifndef ESP_ERR_NVS_NEW_VERSION_FOUND
#define ESP_ERR_NVS_NEW_VERSION_FOUND (ESP_ERR_NVS_BASE + 0xff)
#endif
// DEVTEMP-END

    ESP_LOGI(TAG, "@doc exec nvs_flash_init() - mandatory for Wifi to work later on");
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_LOGW(TAG, "  ESP_ERR_NVS_NO_FREE_PAGES - do nvs_flash_erase()");
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    } else if (err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "  ESP_ERR_NVS_NEW_VERSION_FOUND - do nvs_flash_erase()");
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    /********************************************************************************
     * MY STANDARD Init
     *
     */
    mjd_log_memory_statistics();

    mjd_set_timezone_utc();
    mjd_log_time();

    ESP_LOGI(TAG, "@doc Wait X seconds after power-on (start logic analyzer, let sensors become active, ...)");
    vTaskDelay(RTOS_DELAY_1SEC);

    /********************************************************************************
     * LED
     *
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

    // DEVTEMP: HALT
    /////mjd_rtos_wait_forever();

    /********************************************************************************
     * WIFI
     *
     */
    ESP_LOGI(TAG, "\n\n***SECTION: WIFI***");
    ESP_LOGI(TAG, "MY_WIFI_SSID:     %s", MY_WIFI_SSID);
    ESP_LOGI(TAG, "MY_WIFI_PASSWORD: %s", MY_WIFI_PASSWORD);

    mjd_led_on(MY_LED_ON_DEVBOARD_GPIO_NUM);

    mjd_log_memory_statistics();

    f_retval = mjd_wifi_sta_init(MY_WIFI_SSID, MY_WIFI_PASSWORD);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "mjd_wifi_sta_init() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    f_retval = mjd_wifi_sta_start();
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "mjd_wifi_sta_start() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        mjd_led_mark_error(MY_LED_ON_DEVBOARD_GPIO_NUM);
        // GOTO
        goto cleanup;
    }

    // Helper: Is station connected to an AP?
    ESP_LOGI(TAG, "  mjd_wifi_sta_is_connected(): "MJDBOOLEANFMT, MJDBOOLEAN2STR(mjd_wifi_sta_is_connected()));

    // Helper: Log Wifi Station Info
    mjd_wifi_log_sta_info();

    // Helper: Internet Check
    f_retval = mjd_net_is_internet_reachable();
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "mjd_net_is_internet_reachable() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        mjd_led_mark_error(MY_LED_ON_DEVBOARD_GPIO_NUM);
        // GOTO
        goto cleanup;
    }
    ESP_LOGI(TAG, "OK Internet reachable");

    mjd_led_off(MY_LED_ON_DEVBOARD_GPIO_NUM);

    /********************************************************************************
     * MAIN: HTTP CLIENT
     *
     * 
     */
    ESP_LOGI(TAG, "===POST===");

    mjd_led_on(MY_LED_ON_DEVBOARD_GPIO_NUM);

    ESP_LOGI(TAG, "(esp_http_client_init)");
    esp_http_client_config_t config =
        { .url = "http://httpbin.org/post", .event_handler = _http_event_handler, .method = HTTP_METHOD_POST, };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE(TAG, "esp_http_client_init() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        mjd_led_mark_error(MY_LED_ON_DEVBOARD_GPIO_NUM);
        // GOTO
        goto cleanup;
    }

    const char *post_data = "ABCDEFGHIJ";
    f_retval = esp_http_client_set_post_field(client, post_data, strlen(post_data));
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "esp_http_client_set_post_field() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        mjd_led_mark_error(MY_LED_ON_DEVBOARD_GPIO_NUM);
        // GOTO
        goto cleanup;
    }

    esp_http_client_set_header(client, "Accept", "application/json"); // @seq After esp_http_client_set_post_field()
    esp_http_client_set_header(client, "Content-type", "application/octet-stream"); // @seq After esp_http_client_set_post_field()
    esp_http_client_set_header(client, "x-api-key", "1234567890"); // @seq After esp_http_client_set_post_field()

    ESP_LOGI(TAG, "esp_http_client_perform()");
    f_retval = esp_http_client_perform(client);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "esp_http_client_perform() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        mjd_led_mark_error(MY_LED_ON_DEVBOARD_GPIO_NUM);
        // GOTO
        goto cleanup;
    }
    ESP_LOGI(TAG, "HTTP POST HttpStatusCode=%d, content_length=%d", esp_http_client_get_status_code(client),
            esp_http_client_get_content_length(client));

    ESP_LOGI(TAG, "esp_http_client_cleanup()");
    f_retval = esp_http_client_cleanup(client);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "esp_http_client_cleanup() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        mjd_led_mark_error(MY_LED_ON_DEVBOARD_GPIO_NUM);
        // GOTO
        goto cleanup;
    }

    /*
     * CLEANUP
     */
    cleanup: ;

    // ***Disconnect
    f_retval = mjd_wifi_sta_disconnect_stop();
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "mjd_wifi_sta_disconnect_stop() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        mjd_led_mark_error(MY_LED_ON_DEVBOARD_GPIO_NUM);
    }

    mjd_led_off(MY_LED_ON_DEVBOARD_GPIO_NUM);

    mjd_log_memory_statistics();

    // DEVTEMP: HALT
    /////mjd_rtos_wait_forever();

    /********************************************************************************
     * DEEP SLEEP & RESTART TIMER
     * @sop Put this section in comments when testing other things afterwards (else the MCU restarts every time...)
     * @important In deep sleep mode, wireless peripherals are powered down. Before entering sleep mode, applications must disable WiFi and BT using appropriate calls ( esp_bluedroid_disable(), esp_bt_controller_disable(), esp_wifi_stop()).
     * @doc https://esp-idf.readthedocs.io/en/latest/api-reference/system/sleep_modes.html
     *
     */
    ESP_LOGI(TAG, "\n\n***SECTION: DEEP SLEEP***");

    mjd_log_memory_statistics();

    const uint32_t MY_DEEP_SLEEP_TIME_SEC = 15; // 15 15*60 30*60
    esp_sleep_enable_timer_wakeup(mjd_seconds_to_microseconds(MY_DEEP_SLEEP_TIME_SEC));

    // @important Wait a bit so that the ESP_LOGI() can really dump to UART before deep sleep kicks in!
    ESP_LOGI(TAG, "Entering deep sleep (the MCU should wake up %u seconds later)...\n\n", MY_DEEP_SLEEP_TIME_SEC);
    vTaskDelay(RTOS_DELAY_1SEC);

    // DEVTEMP-BEGIN WORKAROUND for "The system does not wake up properly anymore after the ***2nd*** deep sleep period (and any deep sleep period after that) using ESP-IDF v3.2-dev-607-gb14e87a6."
    //     A temporary workaround is to call esp_set_deep_sleep_wake_stub(NULL); before entering deep sleep
    //     https://www.esp32.com/viewtopic.php?f=13&t=6919&p=29714
    /////esp_set_deep_sleep_wake_stub(NULL);
    // DEVTEMP-END

    mjd_led_blink_times(MY_LED_ON_DEVBOARD_GPIO_NUM,2);

    esp_deep_sleep_start();

    // DEVTEMP @important I never get to this code line if deep sleep is initiated :P
    /////mjd_rtos_wait_forever();

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
    xReturned = xTaskCreatePinnedToCore(&main_task, "main_task (name)", MYAPP_RTOS_TASK_STACK_SIZE_8K, NULL,
    MYAPP_RTOS_TASK_PRIORITY_NORMAL, NULL,
    APP_CPU_NUM);
    if (xReturned == pdPASS) {
        ESP_LOGI(TAG, "OK Task has been created, and is running right now");
    }

    ESP_LOGI(TAG, "END %s()", __FUNCTION__);
}
