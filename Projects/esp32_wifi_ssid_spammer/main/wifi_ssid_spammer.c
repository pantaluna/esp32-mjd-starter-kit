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

// [NO HEADER FILE AVAIL YET] This is the (currently unofficial) 802.11 raw frame TX API, defined in esp32-wifi-lib's libnet80211.a/ieee80211_output.o
esp_err_t esp_wifi_80211_tx(wifi_interface_t ifx, const void *buffer, int len, bool en_sys_seq);

uint8_t beacon_raw[] = { 0x80, 0x00,        // 0-1: Frame Control
        0x00, 0x00,                         // 2-3: Duration
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // 4-9: Destination address (broadcast)
        0xba, 0xde, 0xaf, 0xfe, 0x00, 0x06, // 10-15: Source address
        0xba, 0xde, 0xaf, 0xfe, 0x00, 0x06, // 16-21: BSSID
        0x00, 0x00,                         // 22-23: Sequence / fragment number
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,  // 24-31: Timestamp (GETS OVERWRITTEN TO 0 BY HARDWARE)
        0x64, 0x00,                         // 32-33: Beacon interval
        0x31, 0x04,                         // 34-35: Capability info
        0x00, 0x00, /* FILL SSID HERE */    // 36-38: SSID parameter set, 0x00:length:content
        0x01, 0x08, 0x82, 0x84, 0x8b, 0x96, 0x0c, 0x12, 0x18, 0x24,  // 39-48: Supported rates
        0x03, 0x01, 0x01,                   // 49-51: DS Parameter set, current channel 1 (= 0x01),
        0x05, 0x04, 0x01, 0x02, 0x00, 0x00, // 52-57: Traffic Indication Map
        };

char *fake_ssids[] = { "01-Never-gonna-give-you-up", "02-Never-gonna-let-you-down",
        "03-Never-gonna-give-you-up", "04-Never-gonna-let-you-down",
        "05-Never-gonna-give-you-up", "06-Never-gonna-let-you-down",
        "07-Never-gonna-give-you-up", "08-Never-gonna-let-you-down",
        "08-Never-gonna-give-you-up", "10-Never-gonna-let-you-down",
        "11-Never-gonna-give-you-up", "12-Never-gonna-let-you-down",
        "13-Never-gonna-give-you-up", "14-Never-gonna-let-you-down",
        "15-Never-gonna-give-you-up", "16-Never-gonna-let-you-down",
        "17-Never-gonna-give-you-up", "18-Never-gonna-let-you-down",
        "18-Never-gonna-give-you-up", "20-Never-gonna-let-you-down",
};

#define BEACON_SSID_OFFSET 38
#define SRCADDR_OFFSET 10
#define BSSID_OFFSET 16
#define SEQNUM_OFFSET 22
#define TOTAL_LINES (sizeof(fake_ssids) / sizeof(char *))

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

    mjd_led_config_t led_config = { 0 };
    led_config.gpio_num = MY_LED_ON_DEVBOARD_GPIO_NUM;
    led_config.wiring_type = MY_LED_ON_DEVBOARD_WIRING_TYPE; // 1 GND MCU Huzzah32 | 2 VCC MCU Lolin32lite
    mjd_led_config(&led_config);

    /********************************************************************************
     * WIFI SPAM SSID's
     *
     */
    ESP_LOGI(TAG, "\n\n***SECTION: WIFI SPAM SSID's***");

    // initialize the tcp stack
    tcpip_adapter_init();

    // initialize the wifi event handler
    ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));

    // configure, initialize and start the wifi driver
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT()
    ;
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    // Init dummy AP to specify a channel and get WiFi hardware into a mode where we can send the actual fake beacon frames.
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    wifi_config_t ap_config = { .ap = { .ssid = "esp32spammer", .ssid_len = 0, .password = "dummypassword", .channel = 1,
            .authmode = WIFI_AUTH_WPA2_PSK, .ssid_hidden = 1, .max_connection = 4, .beacon_interval = 60000 } };
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

    // Keep track of beacon sequence numbers on a per-songline-basis
    uint16_t seqnum[TOTAL_LINES] = { 0 };
    uint8_t line = 0;

    while (1) {
        vTaskDelay(RTOS_DELAY_10MILLISEC); // NOT slower!

        mjd_led_on(MY_LED_ON_DEVBOARD_GPIO_NUM);

        // Insert line of Rick Astley's "Never Gonna Give You Up" into beacon packet
        printf("%2i of %2i: (len %2u) %s\n", line+1, TOTAL_LINES, strlen(fake_ssids[line]), fake_ssids[line]);

        uint8_t the_beacon[200];
        memcpy(the_beacon, beacon_raw, BEACON_SSID_OFFSET - 1);
        the_beacon[BEACON_SSID_OFFSET - 1] = strlen(fake_ssids[line]);
        memcpy(&the_beacon[BEACON_SSID_OFFSET], fake_ssids[line], strlen(fake_ssids[line]));
        memcpy(&the_beacon[BEACON_SSID_OFFSET + strlen(fake_ssids[line])], &beacon_raw[BEACON_SSID_OFFSET],
                sizeof(beacon_raw) - BEACON_SSID_OFFSET);

        // Last byte of source address / BSSID will be line number - emulate multiple APs broadcasting one song line each
        the_beacon[SRCADDR_OFFSET + 5] = line;
        the_beacon[BSSID_OFFSET + 5] = line;

        // Update sequence number
        the_beacon[SEQNUM_OFFSET] = (seqnum[line] & 0x0f) << 4;
        the_beacon[SEQNUM_OFFSET + 1] = (seqnum[line] & 0xff0) >> 4;
        seqnum[line]++;
        if (seqnum[line] > 0xfff)
            seqnum[line] = 0;

        esp_wifi_80211_tx(WIFI_IF_AP, the_beacon, sizeof(beacon_raw) + strlen(fake_ssids[line]), false);

        if (++line >= TOTAL_LINES)
            line = 0;

        mjd_led_off(MY_LED_ON_DEVBOARD_GPIO_NUM);
    }

    // stop wifi
    ESP_ERROR_CHECK(esp_wifi_stop());

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
