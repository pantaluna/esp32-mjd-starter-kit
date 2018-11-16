#include "mjd.h"
#include "mjd_net.h"
#include "mjd_wifi.h"

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
 * Main defines
 */

/*
 * WIFI
 */

// [NO HEADER FILE AVAIL YET] This is the (currently unofficial) 802.11 raw frame TX API, defined in esp32-wifi-lib's libnet80211.a/ieee80211_output.o
esp_err_t esp_wifi_80211_tx(wifi_interface_t ifx, const void *buffer, int len, bool en_sys_seq);

// Beacon format
//      https://mrncciew.com/2014/10/08/802-11-mgmt-beacon-frame/
uint8_t beacon_raw[] = { 0x80, 0x00,        // 0-1: Frame Control
        0x00, 0x00,                         // 2-3: Duration
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, // 4-9: Destination address (broadcast)
        0xba, 0xde, 0xaf, 0xfe, 0x00, 0x06, // 10-15: Source address 0xba, 0xde, 0xaf, 0xfe, 0x00, 0x06,
        0xba, 0xde, 0xaf, 0xfe, 0x00, 0x06, // 16-21: BSSID          0xba, 0xde, 0xaf, 0xfe, 0x00, 0x06,
        0x00, 0x00,                         // 22-23: Sequence / fragment number
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,  // 24-31: Timestamp (GETS OVERWRITTEN TO 0 BY HARDWARE)
        0x64, 0x00,                         // 32-33: Beacon interval
        0x31, 0x04,                         // 34-35: Capability info 0b0000 0000 0011 0001 0000 0100
        0x00, 0x00, /* FILL SSID HERE */    // 36-38: SSID parameter set, 0x00:length:content
        0x01, 0x08, 0x82, 0x84, 0x8b, 0x96, 0x0c, 0x12, 0x18, 0x24,  // 39-48: Supported rates
        0x03, 0x01, 0x01,                   // 49-51: DS Parameter set, current channel 1 (= 0x01),
        0x05, 0x04, 0x01, 0x02, 0x00, 0x00, // 52-57: Traffic Indication Map
        };
#define BEACON_SSID_OFFSET 38
#define BEACON_SRCADDR_OFFSET 10
#define BEACON_BSSID_OFFSET 16
#define BEACON_SEQNUM_OFFSET 22

// Cloned SSID's
typedef struct {
    char ssid[33];                     /**< SSID of original AP */
    uint8_t bssid[6];                  /**< MAC address of original AP */
} my_wifi_access_point_record_t;
#define MAX_NBR_OF_CLONED_ACCESS_POINTS 100
my_wifi_access_point_record_t cloned_access_points[MAX_NBR_OF_CLONED_ACCESS_POINTS] = { 0 };

// 15.0 300.0
#define CLONING_LOOP_TIMEOUT_SECONDS (300.0)

// WIFI Empty event handler
static IRAM_ATTR esp_err_t wifi_event_handler(void *ctx, system_event_t *event) {
    return ESP_OK;
}

// HELPER Wifi auth mode code to name
static const char* get_wifi_auth_mode_name(wifi_auth_mode_t auth_mode) {
    const char *names[] = { "OPEN", "WEP", "WPA PSK", "WPA2 PSK", "WPA WPA2 PSK", "MAX" };
    return names[auth_mode];
}

/*
 * TASK
 */
void step_find_active_ssids() {
    ESP_LOGI(TAG, "%s()", __FUNCTION__);

    int32_t retval, i;

    ESP_LOGI(TAG, "\n\n***SECTION: WIFI SCANNER***");

    mjd_led_on(MY_LED_ON_DEVBOARD_GPIO_NUM);

    // init receive vars
    for (i = 0; i < MAX_NBR_OF_CLONED_ACCESS_POINTS; i++) {
        strcpy(cloned_access_points[i].ssid, "");
        memset(cloned_access_points[i].bssid, 0, sizeof cloned_access_points[i].bssid);
    }

    // Set fixed index 0: LONELYRIDER
    strcpy(cloned_access_points[0].ssid, "LONELYRIDER"); // fixed ssid "LONELYRIDER" at [0]
    cloned_access_points[0].bssid[0] = 0x00; // fixed ssid "LONELYRIDER" at [0]  00:25:9c:ca:85:8e (a valid LinkSys MAC Address)
    cloned_access_points[0].bssid[1] = 0x25;
    cloned_access_points[0].bssid[2] = 0x9c;
    cloned_access_points[0].bssid[3] = 0xca;
    cloned_access_points[0].bssid[4] = 0x85;
    cloned_access_points[0].bssid[5] = 0xa0;

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
    ESP_LOGI(TAG, "Start scanning...");

    wifi_scan_config_t scan_config = { .ssid = 0, .bssid = 0, .channel = 0, .show_hidden = true, .scan_type =
            WIFI_SCAN_TYPE_PASSIVE };   // mode active by default
    retval = esp_wifi_scan_start(&scan_config, true);
    if (retval != ESP_ERR_WIFI_TIMEOUT && retval != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_scan_start() err %d %s", retval, esp_err_to_name(retval));
        goto cleanup;
        // GOTO
    }
    printf("  completed.\n");

    // get AP list
    uint16_t ap_num = MAX_NBR_OF_CLONED_ACCESS_POINTS - 1; // #0 = fixed LONELYRIDER
    wifi_ap_record_t ap_records[MAX_NBR_OF_CLONED_ACCESS_POINTS - 1]; // #0 = fixed LONELYRIDER
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_num, ap_records));

    // stop wifi
    ESP_ERROR_CHECK(esp_wifi_stop());

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
        printf("%32s | %4hhu | %4i | %12s | %4i | %4i | %4i | %4i | %4i | "MJDMACFMT" \n", (char *) ap_records[i].ssid,
                ap_records[i].primary, ap_records[i].rssi, get_wifi_auth_mode_name(ap_records[i].authmode),
                ap_records[i].phy_11b, ap_records[i].phy_11g, ap_records[i].phy_11n, ap_records[i].phy_lr, ap_records[i].wps,
                MJDMAC2STR(ap_records[i].bssid));
    }
    printf("---------------------------------\n");

    //
    ESP_LOGI(TAG, "Add all to cloned_access_points[]");
    for (int i = 0; i < ap_num; i++) {
        strcpy(cloned_access_points[i + 1].ssid, (char *) ap_records[i].ssid);  // cloned_access_points[0]=fixed "LONELYRIDER"
        memcpy(cloned_access_points[i + 1].bssid, ap_records[i].bssid, sizeof(ap_records[i].bssid)); // fixed ssid "LONELYRIDER" at [0]
    }

    // LABEL
    cleanup: ;

    mjd_led_off(MY_LED_ON_DEVBOARD_GPIO_NUM);

    ESP_LOGI(TAG, "END %s()", __FUNCTION__);
}

void step_send_beacons() {
    ESP_LOGI(TAG, "%s()", __FUNCTION__);

    ESP_LOGI(TAG, "\n\n***SECTION: WIFI SSID CLONER***");

    // configure, initialize and start the wifi driver
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT()
    ;
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    // Init dummy AP to specify a channel and get WiFi hardware into a mode where we can send the actual fake beacon frames.
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    wifi_config_t ap_config = { .ap = { .ssid = "cloner", .ssid_len = 0, .password = "dole90210", .channel = 1, .authmode =
            WIFI_AUTH_WPA2_PSK, .ssid_hidden = 1, .max_connection = 4, .beacon_interval = 60000 } };
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &ap_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));

    // timer init
    timer_config_t tconfig = { };
    tconfig.divider = 64000; // Slow pace. 1.25 Khz: esp_clk_apb_freq() / 64000 =  1.250 ticks/second
    tconfig.counter_dir = TIMER_COUNT_UP;
    tconfig.counter_en = TIMER_PAUSE;
    tconfig.alarm_en = TIMER_ALARM_DIS;
    tconfig.auto_reload = false;
    timer_init(TIMER_GROUP_0, TIMER_0, &tconfig);

    // timer start with this value (uint64_t).
    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 00000000ULL);
    timer_start(TIMER_GROUP_0, TIMER_0);
    double timer_counter_value_seconds = 0;

    ESP_LOGI(TAG, "  ...CLONING THESE SSID's for %.0f seconds...", CLONING_LOOP_TIMEOUT_SECONDS);
    ESP_LOGI(TAG, "  --------------------");
    for (int i = 0; i < MAX_NBR_OF_CLONED_ACCESS_POINTS; i++) {
        if (strlen(cloned_access_points[i].ssid) != 0) {
            ESP_LOGI(TAG, "  %s ", cloned_access_points[i].ssid);
        }
    }
    ESP_LOGI(TAG, "  --------------------");

    // Keep track of beacon sequence numbers on a per-ssid-basis
    uint16_t seqnum[MAX_NBR_OF_CLONED_ACCESS_POINTS] = { 0 };
    uint8_t current_ap = 0;

    while (1) {
        mjd_led_on(MY_LED_ON_DEVBOARD_GPIO_NUM);

        timer_get_counter_time_sec(TIMER_GROUP_0, TIMER_0, &timer_counter_value_seconds);
        if (timer_counter_value_seconds > CLONING_LOOP_TIMEOUT_SECONDS) {
            break; // BREAK WHILE
        }

        if (strlen(cloned_access_points[current_ap].ssid) == 0) { // skip empty ones
            goto loop_post;
            // GOTO NEXT ITER
        }

        ESP_LOGD(TAG, ". (beginofloop) current_ap: %i", current_ap);

        ESP_LOGD(TAG, "cloned_access_points[current_ap]: %s", cloned_access_points[current_ap].ssid);

        uint8_t the_beacon[200];
        memcpy(the_beacon, beacon_raw, BEACON_SSID_OFFSET - 1);
        the_beacon[BEACON_SSID_OFFSET - 1] = strlen(cloned_access_points[current_ap].ssid);
        memcpy(&the_beacon[BEACON_SSID_OFFSET], cloned_access_points[current_ap].ssid, strlen(cloned_access_points[current_ap].ssid));
        memcpy(&the_beacon[BEACON_SSID_OFFSET + strlen(cloned_access_points[current_ap].ssid)], &beacon_raw[BEACON_SSID_OFFSET],
                sizeof(beacon_raw) - BEACON_SSID_OFFSET);


        // OLD CODE Last byte of source address & BSSID will be the current AP number - emulate multiple APs broadcasting.  0xba, 0xde, 0xaf, 0xfe, 0x00, 0x??
        /////the_beacon[BEACON_SRCADDR_OFFSET + 5] = current_ap;
        /////the_beacon[BEACON_BSSID_OFFSET + 5] = current_ap;

        // NEW CODE fixed ssid "LONELYRIDER" at [0] 00:25:9c:ca:85:8e (a valid LinkSys ID) || Other AP's: Last byte of source address & BSSID will be the current_ap number e.g. 0xba, 0xde, 0xaf, 0xfe, 0x00, 0x??
        if (current_ap == 0) {
            memcpy(&the_beacon[BEACON_SRCADDR_OFFSET], cloned_access_points[0].bssid, sizeof(cloned_access_points[0].bssid));
            memcpy(&the_beacon[BEACON_BSSID_OFFSET], cloned_access_points[0].bssid, sizeof(cloned_access_points[0].bssid));
        } else {
            the_beacon[BEACON_SRCADDR_OFFSET + 5] = current_ap;
            the_beacon[BEACON_BSSID_OFFSET + 5] = current_ap;
        }

        // ***DOESNOTWORK***DUP-BSSIDS NOT ACCEPTED BY SMARTPHONE*** NEW CODE Duplicate SSID & BSSID (MACADDR) of the cloned AP's
        /////memcpy(&the_beacon[BEACON_SRCADDR_OFFSET], cloned_access_points[current_ap].ssid, strlen(cloned_access_points[current_ap].ssid));
        /////memcpy(&the_beacon[BEACON_BSSID_OFFSET], cloned_access_points[current_ap].bssid, sizeof(cloned_access_points[current_ap].bssid));

        // Update sequence number
        the_beacon[BEACON_SEQNUM_OFFSET] = (seqnum[current_ap] & 0x0f) << 4;
        the_beacon[BEACON_SEQNUM_OFFSET + 1] = (seqnum[current_ap] & 0xff0) >> 4;
        seqnum[current_ap]++;
        if (seqnum[current_ap] > 0xfff)
            seqnum[current_ap] = 0;

        esp_wifi_80211_tx(WIFI_IF_AP, the_beacon, sizeof(beacon_raw) + strlen(cloned_access_points[current_ap].ssid), false);

        loop_post: ;

        if (++current_ap >= MAX_NBR_OF_CLONED_ACCESS_POINTS) {
            ESP_LOGD(TAG, "RESET current_ap");
            current_ap = 0;
        }

        ESP_LOGD(TAG, "(endofloop) current_ap: %i", current_ap);

        mjd_led_off(MY_LED_ON_DEVBOARD_GPIO_NUM);

        vTaskDelay(RTOS_DELAY_10MILLISEC); // NOT slower!
    }

    // stop wifi
    ESP_ERROR_CHECK(esp_wifi_stop());

    ESP_LOGI(TAG, "END %s()", __FUNCTION__);
}

void main_task(void *pvParameter) {
    ESP_LOGI(TAG, "%s()", __FUNCTION__);

    while (1) {
        step_find_active_ssids();
        step_send_beacons();
    }

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

    mjd_led_config_t led_config = { 0 };
    led_config.gpio_num = MY_LED_ON_DEVBOARD_GPIO_NUM;
    led_config.wiring_type = MY_LED_ON_DEVBOARD_WIRING_TYPE; // 1 GND MCU Huzzah32 | 2 VCC MCU Lolin32lite
    mjd_led_config(&led_config);

    /**********
     * COMMON WIFI stuff for all tasks
     */
    // [ONCE] initialize the wifi event handler
    ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));

    // initialize the tcp stack
    tcpip_adapter_init();

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
