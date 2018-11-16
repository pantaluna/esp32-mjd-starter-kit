#include "mjd.h"
#include "mjd_net.h"
#include "mjd_wifi.h"

/*
 * Main defines
 */
#define MAX_AP 30

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
#define MYAPP_RTOS_TASK_STACK_SIZE_16K (16384)
#define MYAPP_RTOS_TASK_PRIORITY_NORMAL (RTOS_TASK_PRIORITY_NORMAL)

/*
 * WIFI
 */
// auth mode code to name
static const char* get_wifi_auth_mode_name(wifi_auth_mode_t auth_mode) {
    const char *names[] =
        { "OPEN", "WEP", "WPA PSK", "WPA2 PSK", "WPA WPA2 PSK", "MAX" };
    return names[auth_mode];
}

// Empty event handler
static IRAM_ATTR esp_err_t wifi_event_handler(void *ctx, system_event_t *event) {
    return ESP_OK;
}

/*
 * TASK
 */
void main_task(void *pvParameter) {
    ESP_LOGI(TAG, "%s()", __FUNCTION__);

    /********************************************************************************
     * Reuseable variables
     */
    int32_t retval;

    /********************************************************************************
     * WIFI SCANNER
     *
     */
    ESP_LOGI(TAG, "\n\n***SECTION: WIFI SCANNER***");

    mjd_led_on(MY_LED_ON_DEVBOARD_GPIO_NUM);

    // initialize the tcp stack
    tcpip_adapter_init();

    // initialize the wifi event handler
    ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));

    // configure, initialize and start the wifi driver
    wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT()
    ;
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_config));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    // config and run the scan process in blocking mode
    //  Passive scanning will save power as it does not need to transmit (opposed to active scanning).
    //  ESP32: passive mode always returns with ESP_ERR_WIFI_TIMEOUT in blocking mode (but that is not an error in this mode so proceed)
    //  Some routers have active scanning disabled!
    //  @doc http://www.rfwireless-world.com/Terminology/WLAN-passive-scanning-vs-WLAN-active-scanning.html
    //  @doc https://www.wi-fi.org/knowledge-center/faq/what-are-passive-and-active-scanning
    //  @doc https://www.juniper.net/documentation/en_US/junos-space-apps/network-director3.1/topics/concept/wireless-scanning.html
    printf("Start scanning...\n");

    wifi_scan_config_t scan_config =
        { .ssid = 0, .bssid = 0, .channel = 0, .show_hidden = true, .scan_type = WIFI_SCAN_TYPE_PASSIVE }; // mode active by default
    retval = esp_wifi_scan_start(&scan_config, true);
    if (retval != ESP_ERR_WIFI_TIMEOUT && retval != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_scan_start() err %d %s", retval, esp_err_to_name(retval));
        goto cleanup;
        // GOTO
    }

    printf("  completed.\n");

    // get AP list
    uint16_t ap_num = MAX_AP;
    wifi_ap_record_t ap_records[MAX_AP];
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_num, ap_records));

    // stop wifi
    ESP_ERROR_CHECK(esp_wifi_stop());
    vTaskDelay(RTOS_DELAY_1SEC);

    // list
    printf("REPORT\n");
    printf("------\n");
    printf("  Scan Mode: passive (better than active)\n");
    printf("  Legend: lr? = supports low rate?\n");
    printf("  Nbr of access points found: %i\n", ap_num);
    printf("\n");
    printf("               SSID              | Chan | RSSI |    Auth Mode | 11b? | 11g? | 11n? |  lr? | WPS? | BSSID \n");
    printf("---------------------------------\n");
    for (int i = 0; i < ap_num; i++) {
        printf("%32s | %4hhu | %4i | %12s | %4i | %4i | %4i | %4i | %4i | "MJDMACFMT" \n", (char *) ap_records[i].ssid, ap_records[i].primary,
                ap_records[i].rssi, get_wifi_auth_mode_name(ap_records[i].authmode), ap_records[i].phy_11b, ap_records[i].phy_11g,
                ap_records[i].phy_11n, ap_records[i].phy_lr, ap_records[i].wps, MJDMAC2STR(ap_records[i].bssid));
    }
    printf("---------------------------------\n");

    mjd_led_off(MY_LED_ON_DEVBOARD_GPIO_NUM);

    // LABEL
    cleanup: ;

    /********************************************************************************
     * Task Delete
     * @doc Passing NULL will end the current task
     */
    vTaskDelete(NULL);
}

/*
 * MAIN
 */
void app_main() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    /********************************************************************************
     * SOC init
     */
    ESP_LOGI(TAG, "@doc exec nvs_flash_init() - mandatory for Wifi to work later on");
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    /********************************************************************************
     * MY STANDARD Init
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

    mjd_led_config_t led_config =
        { 0 };
    led_config.gpio_num = MY_LED_ON_DEVBOARD_GPIO_NUM;
    led_config.wiring_type = MY_LED_ON_DEVBOARD_WIRING_TYPE; // 1 GND MCU Huzzah32 | 2 VCC MCU Lolin32lite
    mjd_led_config(&led_config);

    /**********
     * TASK:
     * @important For stability (RMT + Wifi): always use xTaskCreatePinnedToCore(APP_CPU_NUM) [Opposed to xTaskCreate()]
     */
    BaseType_t xReturned;
    xReturned = xTaskCreatePinnedToCore(&main_task, "main_task (name)", MYAPP_RTOS_TASK_STACK_SIZE_16K, NULL,
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
