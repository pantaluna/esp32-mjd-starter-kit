#include "mjd.h"
#include "mjd_list.h"
#include "mjd_net.h"
#include "mjd_wifi.h"

#include "driver/timer.h"
#include "esp_timer.h"
#include "freertos/ringbuf.h"

#include "include/mac_addresses.h"

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
 * defines typedefs vars
 */
RingbufHandle_t *ringbuf_packet;

static SemaphoreHandle_t _stations_data_semaphore = NULL;

typedef struct {
    uint8_t bssid[6];
    uint8_t channel;
    int8_t rssi;
    uint64_t timestamp_ms; // TODO 64b (paxcounter b64 function!)
    char timestamp_str[14 + 1];
    struct mjd_list_head list; // Linked List
} station_info_t;

typedef struct {
    uint8_t header[4];
    uint8_t dest_mac[6];
    uint8_t source_mac[6];
    uint8_t bssid[6];
    uint8_t payload[0];
} payload_t;

static MJD_LIST_HEAD(_stations_list);

// Purge params
// @rule Purge period > STA max age
const uint32_t STATION_MAXIMUM_AGE_MINUTES = 14; // DEV 1 minutes, PRD 14 minutes
const uint32_t STATION_PURGE_PERIOD_MINUTES = 15; // DEV 2 minutes, PRD 15 minutes

// Wifi Channel switcher params
// @doc Change channel regularly to find all devices
const uint32_t CHANNEL_SWITCHER_PERIOD_MS = RTOS_DELAY_1SEC;

/*
 * Helper funcs
 */
static uint64_t _get_log_timestamp64() {
    static uint32_t cur_log_stamp_msb = 0;
    static uint32_t cur_log_stamp_lsb = 0;

    uint32_t new_esp_log_stamp_32b = esp_log_timestamp();
    if (new_esp_log_stamp_32b < cur_log_stamp_lsb) { // the 32bit ticker (ms) overflowed
        ++cur_log_stamp_msb;
    }
    cur_log_stamp_lsb = new_esp_log_stamp_32b;
    return (uint64_t) cur_log_stamp_msb << 32 | cur_log_stamp_lsb;
}

/*
 * WIFI
 */
static esp_err_t _wifi_event_handler(void *ctx, system_event_t *event) {
    // A DUMMY BY DESIGN
    return ESP_OK;
}

static IRAM_ATTR void _promiscuous_rx_cb(void *recv_buf, wifi_promiscuous_pkt_type_t type) {
    wifi_promiscuous_pkt_t *ptr_packet = (wifi_promiscuous_pkt_t *) recv_buf;
    // @doc packet= wifi_pkt_rx_ctrl_t + payload. Length of payload is described by rx_ctrl.sig_len
    size_t len_packet = sizeof(wifi_pkt_rx_ctrl_t) + ptr_packet->rx_ctrl.sig_len;

    // DEVTEMP
    printf(".");
    fflush(stdout);

    // DEVTEMP
    /*printf("[CB: len_packet=%u]", len_packet); fflush(stdout);*/
    /*payload_t *ptr_payload = (payload_t *) ptr_packet->payload;
     printf("[CB: len_packet=%u MAC "MJDMACFMT"]", len_packet, MJDMAC2STR(ptr_payload->source_mac)); fflush(stdout);*/

    BaseType_t bt_retval = xRingbufferSend(ringbuf_packet, ptr_packet, len_packet, portMAX_DELAY);
    if (bt_retval != pdTRUE) {
        ESP_LOGE(TAG, "xRingbufferSend() err %d ?time-out or output buffer too large?", bt_retval);
        // GOTO
        goto cleanup;
    }

    // LABEL
    cleanup: ;
}

/*
 * TASK
 */
static void _packet_parser_task(void *pvParameter) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    /********************************************************************************
     * Reuseable variables
     *
     */
    wifi_promiscuous_pkt_t *ptr_packet;
    size_t len_packet;
    payload_t *ptr_payload;
    station_info_t *ptr_one_station = NULL;

    /********************************************************************************
     * MAIN
     *
     */
    while (1) {
        // TAKE Mutex!
        xSemaphoreTake(_stations_data_semaphore, portMAX_DELAY);

        // DEVTEMP
        /////printf("[parser]"); fflush(stdout);

        // Ringbuffer RX
        ptr_packet = (wifi_promiscuous_pkt_t *) xRingbufferReceive(ringbuf_packet, &len_packet, portMAX_DELAY);
        if (ptr_packet == NULL) {
            ESP_LOGE(TAG, "xRingbufferReceive() err TIMEOUT");
            // GOTO
            goto cleanup_inside_loop;

        }
        ptr_payload = (payload_t *) ptr_packet->payload;

        // DEVTEMP
        /*printf("[CB: len_packet=%u]", len_packet); fflush(stdout);*/
        /*printf("[CB: len_packet=%u MAC "MJDMACFMT"]", len_packet, MJDMAC2STR(ptr_payload->source_mac)); fflush(stdout);*/

        /* Filter out non-people sources (some ESP32 boards and some network equipment) */
        for (int i = 0; i < ARRAY_SIZE(not_people_mac_addresses); ++i) {
            if (memcmp(ptr_payload->source_mac, not_people_mac_addresses[i], 3) == 0) {  // first 3 bytes (not all 6!)
                /*ESP_LOGD(TAG, "Ignored a non-people source");
                 ESP_LOGD(TAG, "  header[0]: %hhu\n", ptr_payload->header[0]);
                 ESP_LOGD(TAG, "  MAC: "MJDMACFMT", rssi: %i", MJDMAC2STR(ptr_payload->source_mac), ptr_packet->rx_ctrl.rssi);*/
                // GOTO
                goto cleanup_inside_loop;
            }
        }

        /* Process already detected devices: update timestamp */
        mjd_list_for_each_entry(ptr_one_station, &_stations_list, list)
        {
            if (memcmp(ptr_one_station->bssid, ptr_payload->source_mac, sizeof(ptr_one_station->bssid)) == 0) { // all 6 bytes
                ESP_LOGD(TAG, "Update a device that was already detected");
                ptr_one_station->channel = ptr_packet->rx_ctrl.channel;
                ptr_one_station->rssi = ptr_packet->rx_ctrl.rssi;
                ptr_one_station->timestamp_ms = _get_log_timestamp64(); // 64b milliseconds
                mjd_get_current_time_yyyymmddhhmmss(ptr_one_station->timestamp_str); // fmt datetime string
                // log
                ESP_LOGD(TAG,
                        "  bssid/MAC: "MJDMACFMT" | channel: %u | rssi: %i | timestamp_ms: %" PRIu64 " | timestamp_str: %s",
                        MJDMAC2STR(ptr_one_station->bssid), ptr_one_station->channel, ptr_one_station->rssi,
                        ptr_one_station->timestamp_ms, ptr_one_station->timestamp_str);
                // GOTO
                goto cleanup_inside_loop;
            }
        }
        /* Add the device information to list */
        ESP_LOGI(TAG, "Adding a new device");
        ptr_one_station = malloc(sizeof(*ptr_one_station));
        memcpy(ptr_one_station->bssid, ptr_payload->source_mac, sizeof(ptr_one_station->bssid));
        ptr_one_station->channel = ptr_packet->rx_ctrl.channel;
        ptr_one_station->rssi = ptr_packet->rx_ctrl.rssi;
        ptr_one_station->timestamp_ms = _get_log_timestamp64(); // 64b milliseconds
        mjd_get_current_time_yyyymmddhhmmss(ptr_one_station->timestamp_str);
        mjd_list_add_tail(&ptr_one_station->list, &_stations_list);  // add_tail!
        // log
        ESP_LOGI(TAG, "  bssid/MAC: "MJDMACFMT" | channel: %u | rssi: %i | timestamp_ms: %" PRIu64 " | timestamp_str: %s",
                MJDMAC2STR(ptr_one_station->bssid), ptr_one_station->channel, ptr_one_station->rssi,
                ptr_one_station->timestamp_ms, ptr_one_station->timestamp_str);

        // LABEL
        cleanup_inside_loop: ;

        // GIVE Mutex!
        xSemaphoreGive(_stations_data_semaphore);

        // Return RB
        if (ptr_packet != NULL) {
            vRingbufferReturnItem(ringbuf_packet, ptr_packet);
        }

    }

    /********************************************************************************
     * TODO NEVER GETS HERE...
     * Task Cleanup & Delete
     * @doc Passing NULL will end the current task
     */
    vRingbufferDelete(ringbuf_packet);
    vTaskDelete(NULL);
}

static void _wifi_channel_switcher_task(void *pvParameter) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    /********************************************************************************
     * Reuseable variables
     *
     */
    esp_err_t f_retval;

    /********************************************************************************
     * INIT
     */

    /********************************************************************************
     * MAIN
     * Channels: 1..13 switch every 100ms
     * Bandwidth: always HT40 (no HT20 narrow bands on ESP32)
     *
     */
    const uint8_t MIN_CHANNEL_ID = 1;  // The min channel on EU Routers
    const uint8_t MAX_CHANNEL_ID = 11; // The max channel on EU Routers
    uint8_t primary_channel = MIN_CHANNEL_ID;
    const wifi_second_chan_t secondary_channel = WIFI_SECOND_CHAN_NONE;
    while (1) {
        f_retval = esp_wifi_set_channel(primary_channel, secondary_channel);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "esp_wifi_set_channel() err %d %s", f_retval, esp_err_to_name(f_retval));
            // GOTO
            goto cleanup_inside_loop;
        }
        ESP_LOGD(TAG, "[SWTCH: CH %u (%u)]", primary_channel, secondary_channel);

        // DEBUG
        /*uint8_t get_primary_channel;
         wifi_second_chan_t get_secondary_channel;
         f_retval = esp_wifi_get_channel(&get_primary_channel, &get_secondary_channel);
         if (f_retval != ESP_OK) {
         ESP_LOGE(TAG, "esp_wifi_get_channel() err %d %s", f_retval, esp_err_to_name(f_retval));
         // GOTO
         goto cleanup_inside_loop;
         }
         wifi_bandwidth_t get_bandwidth;
         f_retval = esp_wifi_get_bandwidth(WIFI_MODE_STA, &get_bandwidth);
         if (f_retval != ESP_OK) {
         ESP_LOGE(TAG, "esp_wifi_get_bandwidth() err %d %s", f_retval, esp_err_to_name(f_retval));
         // GOTO
         goto cleanup_inside_loop;
         }
         printf("[MON: CH %u (%u)| BW %u]", get_primary_channel, get_secondary_channel, get_bandwidth);
         fflush(stdout);*/

        // LABEL
        cleanup_inside_loop: ;

        // WAIT
        vTaskDelay(CHANNEL_SWITCHER_PERIOD_MS);

        // NEXT
        if (++primary_channel > MAX_CHANNEL_ID) {
            primary_channel = MIN_CHANNEL_ID;
        }
    }

    /********************************************************************************
     * TODO NEVER GETS HERE...
     * Task Delete
     * @doc Passing NULL will end the current task
     */
    vTaskDelete(NULL);
}

static void _log_stations() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    uint32_t nbr_of_stations;
    mjd_list_count(&_stations_list, &nbr_of_stations);
    ESP_LOGI(TAG, "#Stations: %u", nbr_of_stations);

    station_info_t *ptr_one_station = NULL;
    mjd_list_for_each_entry(ptr_one_station, &_stations_list, list)
    {
        ESP_LOGI(TAG, "  bssid/MAC: "MJDMACFMT"| channel: %4u | rssi: %4i | timestamp_ms: %" PRId64 " | timestamp_str: %s",
                MJDMAC2STR(ptr_one_station->bssid), ptr_one_station->channel, ptr_one_station->rssi,
                ptr_one_station->timestamp_ms, ptr_one_station->timestamp_str);
    }
}

static void _purge_stations() {
    uint64_t now_timestamp_ms = _get_log_timestamp64();

    ESP_LOGI(TAG, "PURGING...");

    const uint64_t STATION_MAXIMUM_AGE_MILLISEC = 60ULL * 1000ULL * STATION_MAXIMUM_AGE_MINUTES;

    ESP_LOGD(TAG, "  STATION_PURGE_PERIOD_MINUTES:   %u", STATION_PURGE_PERIOD_MINUTES);
    ESP_LOGD(TAG, "  STATION_MAXIMUM_AGE_MINUTES:    %u", STATION_MAXIMUM_AGE_MINUTES);
    ESP_LOGD(TAG, "  STATION_MAXIMUM_AGE_MILLISEC:   %" PRIu64 "", STATION_MAXIMUM_AGE_MILLISEC);
    ESP_LOGD(TAG, "  now_timestamp_ms:               %" PRIu64 "", now_timestamp_ms);
    ESP_LOGD(TAG, "  STA minimum absolute log stamp: %" PRIu64 "", (now_timestamp_ms - STATION_MAXIMUM_AGE_MILLISEC));

    if (now_timestamp_ms < STATION_MAXIMUM_AGE_MILLISEC) {
        ESP_LOGI(TAG, "Nothing to purge yet (too early in general)");
        // GOTO
        goto cleanup;
    }

    station_info_t *ptr_one_station = NULL;
    station_info_t *ptr_next_station = NULL;
    mjd_list_for_each_entry_safe(ptr_one_station, ptr_next_station, &_stations_list, list)
    {
        if (ptr_one_station->timestamp_ms < (now_timestamp_ms - STATION_MAXIMUM_AGE_MILLISEC)) {
            ESP_LOGI(TAG, "Yes, purge this old station:");
            ESP_LOGI(TAG,
                    "  bssid/MAC: "MJDMACFMT"| channel: %4u | rssi: %4i | timestamp_ms: %" PRId64 " | timestamp_str: %s",
                    MJDMAC2STR(ptr_one_station->bssid), ptr_one_station->channel, ptr_one_station->rssi,
                    ptr_one_station->timestamp_ms, ptr_one_station->timestamp_str);
            mjd_list_del(&ptr_one_station->list);
            free(ptr_one_station);
        }
    }

    // LABEL
    cleanup: ;
}

static void _station_purger_task(void *pvParameter) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    /********************************************************************************
     * Reuseable variables
     *
     */

    /********************************************************************************
     * VALIDATIONS
     */
    if (STATION_PURGE_PERIOD_MINUTES <= STATION_MAXIMUM_AGE_MINUTES) {
        ESP_LOGE(TAG, "INVALID combination. REFUSE STATION_PURGE_PERIOD_MINUTES <= STATION_MAXIMUM_AGE_MINUTES");
        // GOTO
        goto cleanup;
    }

    /********************************************************************************
     * MAIN Purge periodically
     *
     */
    while (1) {
        // WAIT for the next round
        ESP_LOGI(TAG, "_station_purger_task() WAITING %u minutes...", STATION_PURGE_PERIOD_MINUTES);
        vTaskDelay(STATION_PURGE_PERIOD_MINUTES * RTOS_DELAY_1MINUTE);

        // TAKE Mutex!
        xSemaphoreTake(_stations_data_semaphore, portMAX_DELAY);

        // MAIN
        _purge_stations();
        _log_stations();

        // GIVE Mutex!
        xSemaphoreGive(_stations_data_semaphore);

        // LABEL
        /////cleanup_inside_loop: ;
    }

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
     * Reuseable variables
     *
     */
    esp_err_t f_retval;
    BaseType_t xReturned;

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
    vTaskDelay(RTOS_DELAY_1MILLISEC);

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

    /********************************************************************************
     * PROJECT INIT
     *
     */

    // INIT Mutex data_stations
    _stations_data_semaphore = xSemaphoreCreateMutex();

    // INIT RingBuffer
    //  @doc Wifi promiscious packets are variable length
    ringbuf_packet = xRingbufferCreate(12 * 1024, RINGBUF_TYPE_NOSPLIT);
    if (ringbuf_packet == NULL) {
        ESP_LOGE(TAG, "xRingbufferCreate() returned NULL pointer");
        // GOTO
        goto cleanup;
    }

    // TASK MGT: packet_parser_task
    xReturned = xTaskCreatePinnedToCore(&_packet_parser_task, "_packet_parser_task (name)", MYAPP_RTOS_TASK_STACK_SIZE_16K,
    NULL,
    MYAPP_RTOS_TASK_PRIORITY_NORMAL, NULL, APP_CPU_NUM);
    if (xReturned != pdPASS) {
        ESP_LOGE(TAG, "Cannot create task _packet_parser_task");
        // GOTO
        goto cleanup;

    }

    // wifi init
    tcpip_adapter_init();

    ESP_ERROR_CHECK(esp_event_loop_init(_wifi_event_handler, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT()
    ;
    f_retval = esp_wifi_init(&cfg);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_init() err %d %s", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    f_retval = esp_wifi_set_storage(WIFI_STORAGE_RAM);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_set_storage() err %d %s", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Apr12,2018 WIFI_MODE_NULL is no longer supported in ESP-IDF, use WIFI_MODE_STA for sniffer mode (but do not connect to an AP)
    f_retval = esp_wifi_set_mode(WIFI_MODE_STA);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_set_mode() err %d %s", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // wifi kick off promiscious RX callback
    // @important Include only MGMT and AMPDU packets (low volume). Exclude DATA packets (too high volume!)
    f_retval = esp_wifi_set_promiscuous(false);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_set_promiscuous(false) err %d %s\n", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    wifi_promiscuous_filter_t filter =
        { .filter_mask = WIFI_PROMIS_FILTER_MASK_MGMT || WIFI_PROMIS_FILTER_MASK_DATA_AMPDU };
    f_retval = esp_wifi_set_promiscuous_filter(&filter);  // Only MGMT frames (no DATA or MISC frames)
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_set_promiscuous_filter(&filter) err %d %s\n", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    f_retval = esp_wifi_set_promiscuous_rx_cb(_promiscuous_rx_cb);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_set_promiscuous_rx_cb() err %d %s\n", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    f_retval = esp_wifi_set_promiscuous(true);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_set_promiscuous(true) err %d %s\n", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // TASK MGT: wifi_channel_switcher_task
    xReturned = xTaskCreatePinnedToCore(&_wifi_channel_switcher_task, "_wifi_channel_switcher_task (name)",
    MYAPP_RTOS_TASK_STACK_SIZE_16K, NULL,
    MYAPP_RTOS_TASK_PRIORITY_NORMAL, NULL, APP_CPU_NUM);
    if (xReturned != pdPASS) {
        ESP_LOGE(TAG, "Cannot create task _wifi_channel_switcher_task");
        // GOTO
        goto cleanup;
    }

    // TASK MGT: station_purger_task
    xReturned = xTaskCreatePinnedToCore(&_station_purger_task, "_station_purger_task (name)", MYAPP_RTOS_TASK_STACK_SIZE_16K,
    NULL,
    MYAPP_RTOS_TASK_PRIORITY_NORMAL, NULL, APP_CPU_NUM);
    if (xReturned != pdPASS) {
        ESP_LOGE(TAG, "Cannot create task _station_purger_task");
        // GOTO
        goto cleanup;
    }

    // STATS (initially empty)
    _log_stations();

    // LABEL
    cleanup: ;

    /**********
     * END
     */
    ESP_LOGI(TAG, "END %s()", __FUNCTION__);
}
