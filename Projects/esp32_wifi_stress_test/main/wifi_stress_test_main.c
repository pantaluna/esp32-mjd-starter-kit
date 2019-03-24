/*
 * HARDWARE SETUP the MJD components:
 *  *NONE
 *
 */
#include "esp_http_client.h"

#include "mjd.h"
#include "mjd_huzzah32.h"
#include "mjd_net.h"
#include "mjd_wifi.h"

/*
 * Logging
 */
static const char TAG[] = "myapp";

/*
 * TODO DEVICE INFO
 */

/*
 * KConfig: LED, WIFI
 */
static const int MY_LED_ON_DEVBOARD_GPIO_NUM = CONFIG_MY_LED_ON_DEVBOARD_GPIO_NUM;
static const int MY_LED_ON_DEVBOARD_WIRING_TYPE = CONFIG_MY_LED_ON_DEVBOARD_WIRING_TYPE;

static const char *MY_WIFI_SSID = CONFIG_MY_WIFI_SSID;
static const char *MY_WIFI_PASSWORD = CONFIG_MY_WIFI_PASSWORD;

#define MAX_ACCESS_POINTS (30)

/*
 * FreeRTOS settings
 */
#define MYAPP_RTOS_TASK_STACK_SIZE_LARGE (8192)
#define MYAPP_RTOS_TASK_PRIORITY_NORMAL (RTOS_TASK_PRIORITY_NORMAL)

/*
 * HELPERS
 */
extern const char howsmyssl_com_root_cert_pem_start[] asm("_binary_howsmyssl_com_root_cert_pem_start");
extern const char howsmyssl_com_root_cert_pem_end[] asm("_binary_howsmyssl_com_root_cert_pem_end");

esp_err_t _http_event_handler(esp_http_client_event_t *evt) {
    switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
        ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        if (!esp_http_client_is_chunked_response(evt->client)) {
            // Write out data
            // printf("%.*s", evt->data_len, (char*)evt->data);
        }
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
        break;
    }
    return ESP_OK;
}

/*
 * WIFI SCANNER
 */
// Scanner: WIFI auth mode code to name
static const char* get_wifi_auth_mode_name(wifi_auth_mode_t auth_mode) {
    const char *names[] =
        { "OPEN", "WEP", "WPA PSK", "WPA2 PSK", "WPA WPA2 PSK", "MAX" };
    return names[auth_mode];
}

static esp_err_t _wifi_scanner() {
    esp_err_t f_retval = ESP_OK;

    // Start scan
    ESP_LOGI(TAG, "Start Wifi scan...");

    wifi_scan_config_t scan_config =
        { .ssid = 0, .bssid = 0, .channel = 0, .show_hidden = true, .scan_type = WIFI_SCAN_TYPE_ACTIVE };
    f_retval = esp_wifi_scan_start(&scan_config, true);
    if (f_retval != ESP_ERR_WIFI_TIMEOUT && f_retval != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_scan_start() err %d %s", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    ESP_LOGI(TAG, "  completed.");

    // Get AP list
    uint16_t ap_num = MAX_ACCESS_POINTS;
    wifi_ap_record_t ap_records[MAX_ACCESS_POINTS];
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_num, ap_records));

    // Report
    ESP_LOGI(TAG, "REPORT");
    ESP_LOGI(TAG, "------");
    ESP_LOGI(TAG, "  Legend: lr? = supports low rate?");
    ESP_LOGI(TAG, "  Nbr of access points found: %i", ap_num);
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "               SSID              | Chan | RSSI |    Auth Mode | 11b? | 11g? | 11n? |  lr? | WPS? | BSSID");
    ESP_LOGI(TAG, "---------------------------------");
    for (int i = 0; i < ap_num; i++) {
        ESP_LOGI(TAG, "%32s | %4hhu | %4i | %12s | %4i | %4i | %4i | %4i | %4i | "MJDMACFMT" ", (char *) ap_records[i].ssid, ap_records[i].primary,
                ap_records[i].rssi, get_wifi_auth_mode_name(ap_records[i].authmode), ap_records[i].phy_11b, ap_records[i].phy_11g,
                ap_records[i].phy_11n, ap_records[i].phy_lr, ap_records[i].wps, MJDMAC2STR(ap_records[i].bssid));
    }
    ESP_LOGI(TAG, "---------------------------------");

    // LABEL
    cleanup:;

    return f_retval;
}

/*
 * HTTPS
 */
static void _https() {
    /*esp_http_client_config_t config =
        { .url = "https://www.howsmyssl.com", .event_handler = _http_event_handler, .cert_pem = howsmyssl_com_root_cert_pem_start, };*/

    /*esp_http_client_config_t config =
        { .url = "http://ipv4.download.thinkbroadband.com/10MB.zip", .event_handler = _http_event_handler };*/

    esp_http_client_config_t config =
        { .url = "http://ipv4.download.thinkbroadband.com/1MB.zip", .event_handler = _http_event_handler };

    ESP_LOGI(TAG, "    URL: %s", config.url);
    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_err_t err = esp_http_client_perform(client);

    if (err == ESP_OK) {
        ESP_LOGI(TAG, "  OK. HTTP(S) Status = %d, content_length = %d", esp_http_client_get_status_code(client), esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "  ERROR. HTTP(S) request err %s", esp_err_to_name(err));
    }

    esp_http_client_cleanup(client);
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
    uint32_t i;
    uint32_t total;

    /********************************************************************************
     * SOC init
     *
     */
    mjd_log_memory_statistics();

    ESP_LOGI(TAG, "@doc exec nvs_flash_init() - mandatory for Wifi to work later on");
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    /********************************************************************************
     * MY STANDARD Init
     *
     */
    mjd_log_time();
    mjd_log_memory_statistics();

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

    /*
    ESP_LOGI(TAG, "***TEMPORARY: Routing the actual VREF Voltage Reference of the ESP32 to GPIO#26.");
    ESP_LOGI(TAG, "***    Use a multimeter to determine the voltage. The value will be around 1100mV.");
    ESP_LOGI(TAG, "");
    f_retval = mjd_huzzah32_route_vref_to_gpio();
    if (f_retval != ESP_OK) {
        goto wifi_cleanup;
    }
    // DEVTEMP: HALT
    mjd_rtos_wait_forever();
    */

    /********************************************************************************
     * WIFI
     *   @important Do wifi-init only ONCE in the app.
     *   @important Do delay between reconnect cycles ELSE connecting to the Wifi AP shall often fail (depends on the HW Router Device model).
     *
     */
    ESP_LOGI(TAG, "\n\n***SECTION: WIFI***");
    ESP_LOGI(TAG, "MY_WIFI_SSID:     %s", MY_WIFI_SSID);
    ESP_LOGI(TAG, "MY_WIFI_PASSWORD: %s", MY_WIFI_PASSWORD);

    /*
     * Optional for Production: dump less messages
     *  @doc Possible to lower the log level for specific modules (wifi and tcpip_adapter are strong candidates)
     */
    esp_log_level_set("phy", ESP_LOG_WARN); // @important  Disable INFO messages which are too detailed for me.
    esp_log_level_set("tcpip_adapter", ESP_LOG_WARN); // @important Disable INFO messages which are too detailed for me.
    esp_log_level_set("wifi", ESP_LOG_WARN); // @important Disable INFO messages which are too detailed for me.
    esp_log_level_set("mjd_wifi", ESP_LOG_WARN); // @important Disable INFO messages which are too detailed for me.

    mjd_log_memory_statistics();

    // [ONCE] Wifi Init
    f_retval = mjd_wifi_sta_init(MY_WIFI_SSID, MY_WIFI_PASSWORD);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "mjd_wifi_sta_init() err %i (%s)", f_retval, esp_err_to_name(f_retval));

        mjd_led_mark_error(MY_LED_ON_DEVBOARD_GPIO_NUM);
        // GOTO
        goto wifi_cleanup;
    }

    total = 1000000;  // TIMES 10 1000000
    ESP_LOGI(TAG, "WIFI: start stop: %i times", total);
    i = 0;
    while (++i <= total) {
        ESP_LOGI(TAG, "\n\nLOOP#%i of %i", i, total);

        mjd_log_memory_statistics();

        mjd_led_on(MY_LED_ON_DEVBOARD_GPIO_NUM);

        // Wifi start and do HTTPS
        ESP_LOGI(TAG, "Wifi start");
        f_retval = mjd_wifi_sta_start();
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "mjd_wifi_sta_start() err %i (%s)", f_retval, esp_err_to_name(f_retval));

            mjd_led_mark_error(MY_LED_ON_DEVBOARD_GPIO_NUM);
            // GOTO
            goto wifi_cleanup;
        }

        mjd_log_memory_statistics();

        // Helper: Is station connected to an AP?
        ESP_LOGI(TAG, "  mjd_wifi_sta_is_connected(): "MJDBOOLEANFMT, MJDBOOLEAN2STR(mjd_wifi_sta_is_connected()));

        // Adafruit HUZZAH32: display battery voltage
        ESP_LOGI(TAG, "Battery voltage %fV (only valid for a battery-powered Adafruit HUZZAH32 dev board)", mjd_huzzah32_get_battery_voltage());

        // Helper: Get IP Address
        char ip_address[32] = "";
        f_retval = mjd_net_get_ip_address(ip_address);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "mjd_net_get_ip_address() err %i (%s)", f_retval, esp_err_to_name(f_retval));

            mjd_led_mark_error(MY_LED_ON_DEVBOARD_GPIO_NUM);
            // GOTO
            goto wifi_cleanup;
        }
        ESP_LOGI(TAG, "ip_address = %s", ip_address);

        // Helper: Log Wifi Station Info
        mjd_wifi_log_sta_info();

        // Helper: Internet Check
        f_retval = mjd_net_is_internet_reachable();
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "mjd_net_is_internet_reachable() err %i (%s)", f_retval, esp_err_to_name(f_retval));

            mjd_led_mark_error(MY_LED_ON_DEVBOARD_GPIO_NUM);
            // GOTO
            goto wifi_cleanup;
        }
        ESP_LOGI(TAG, "OK Internet reachable");

        // ***Wifi scanner
        _wifi_scanner();

        // ***HTTPS/TLS REQUEST 2x times
        ESP_LOGI(TAG, "HTTPS requests (3x)");
        for (uint32_t u = 1; u <= 3; u++) {
            ESP_LOGI(TAG, "  START #%u", u);
            _https();
        }

        // Disconnect
        f_retval = mjd_wifi_sta_disconnect_stop();
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "mjd_wifi_sta_disconnect_stop() err %i ", f_retval);
            ESP_LOGE(TAG, "esp_err_to_name(err) = %s", esp_err_to_name(f_retval));

            mjd_led_mark_error(MY_LED_ON_DEVBOARD_GPIO_NUM);
            // GOTO
            goto wifi_cleanup;
        }

        mjd_led_off(MY_LED_ON_DEVBOARD_GPIO_NUM);

        // @important Delay 1 sec always works fine (delay 1 sec = often errors)
        vTaskDelay(RTOS_DELAY_1SEC);
    }

    wifi_cleanup: ;

    // [AFTER LOOP] Helper: Log Wifi Station Info
    mjd_wifi_log_sta_info();

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

    const uint32_t MY_DEEP_SLEEP_TIME_SEC = 15; // 15 60 30*60
    esp_sleep_enable_timer_wakeup(mjd_seconds_to_microseconds(MY_DEEP_SLEEP_TIME_SEC));

    // @important Wait a bit so that the ESP_LOGI() can really dump to UART before deep sleep kicks in!
    ESP_LOGI(TAG, "Entering deep sleep (the MCU should wake up %u seconds later)...\n\n", MY_DEEP_SLEEP_TIME_SEC);
    vTaskDelay(RTOS_DELAY_1SEC);

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
