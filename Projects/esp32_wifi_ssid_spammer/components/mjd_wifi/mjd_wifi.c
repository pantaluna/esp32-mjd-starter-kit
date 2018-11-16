/*
 * WIFI
 * @doc static <global var>/<global func>: its scope is restricted to the file in which it is declared.
 */

// Component header file(s)
#include "mjd.h"
#include "mjd_wifi.h"

/**********
 * Logging
 */
static const char TAG[] = "mjd_wifi";

/**********
 * MAIN
 */
static EventGroupHandle_t wifi_event_group;
static const int WIFI_CONNECTED_BIT = BIT0;
static const int WIFI_DISCONNECTED_BIT = BIT1;

static uint32_t total_nbr_of_connect_errors = 0;
static uint32_t total_nbr_of_first_connect_errors = 0;

ip4_addr_t ip;
ip4_addr_t gw;
ip4_addr_t msk;

/***
 * Messages
 *   @source esp_wifi_types.h
 *   @doc Special feature of the C Preprocessor to convert a defined NAME to a string representation of that name (handy!).
 */
#define MJD_WIFI_ADD_ERROR_ITEM(err)  {err, #err}

typedef struct {
    esp_err_t code;
    const char *msg;
} mjd_wifi_reason_msg_t;

static const char _mjd_wifi_unknown_msg[] = "UNKNOWN ERROR MSG";

static const mjd_wifi_reason_msg_t _mjd_wifi_reason_msg_table[] = {
MJD_WIFI_ADD_ERROR_ITEM( WIFI_REASON_UNSPECIFIED),               // 1
MJD_WIFI_ADD_ERROR_ITEM( WIFI_REASON_AUTH_EXPIRE),               // 2
MJD_WIFI_ADD_ERROR_ITEM( WIFI_REASON_AUTH_LEAVE),                // 3
MJD_WIFI_ADD_ERROR_ITEM( WIFI_REASON_ASSOC_EXPIRE),              // 4
MJD_WIFI_ADD_ERROR_ITEM( WIFI_REASON_ASSOC_TOOMANY),             // 5
MJD_WIFI_ADD_ERROR_ITEM( WIFI_REASON_NOT_AUTHED),                // 6
MJD_WIFI_ADD_ERROR_ITEM( WIFI_REASON_NOT_ASSOCED),               // 7
MJD_WIFI_ADD_ERROR_ITEM( WIFI_REASON_ASSOC_LEAVE),               // 8
MJD_WIFI_ADD_ERROR_ITEM( WIFI_REASON_ASSOC_NOT_AUTHED),          // 9
MJD_WIFI_ADD_ERROR_ITEM( WIFI_REASON_DISASSOC_PWRCAP_BAD),      // 10
MJD_WIFI_ADD_ERROR_ITEM( WIFI_REASON_DISASSOC_SUPCHAN_BAD),     // 11
MJD_WIFI_ADD_ERROR_ITEM( WIFI_REASON_IE_INVALID),               // 13
MJD_WIFI_ADD_ERROR_ITEM( WIFI_REASON_MIC_FAILURE),              // 14
MJD_WIFI_ADD_ERROR_ITEM( WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT),   // 15
MJD_WIFI_ADD_ERROR_ITEM( WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT), // 16
MJD_WIFI_ADD_ERROR_ITEM( WIFI_REASON_IE_IN_4WAY_DIFFERS),       // 17
MJD_WIFI_ADD_ERROR_ITEM( WIFI_REASON_GROUP_CIPHER_INVALID),     // 18
MJD_WIFI_ADD_ERROR_ITEM( WIFI_REASON_PAIRWISE_CIPHER_INVALID),  // 19
MJD_WIFI_ADD_ERROR_ITEM( WIFI_REASON_AKMP_INVALID),             // 20
MJD_WIFI_ADD_ERROR_ITEM( WIFI_REASON_UNSUPP_RSN_IE_VERSION),    // 21
MJD_WIFI_ADD_ERROR_ITEM( WIFI_REASON_INVALID_RSN_IE_CAP),       // 22
MJD_WIFI_ADD_ERROR_ITEM( WIFI_REASON_802_1X_AUTH_FAILED),       // 23
MJD_WIFI_ADD_ERROR_ITEM( WIFI_REASON_CIPHER_SUITE_REJECTED),    // 24

MJD_WIFI_ADD_ERROR_ITEM( WIFI_REASON_BEACON_TIMEOUT),      // 200
MJD_WIFI_ADD_ERROR_ITEM( WIFI_REASON_NO_AP_FOUND),         // 201
MJD_WIFI_ADD_ERROR_ITEM( WIFI_REASON_AUTH_FAIL),           // 202
MJD_WIFI_ADD_ERROR_ITEM( WIFI_REASON_ASSOC_FAIL),          // 203
MJD_WIFI_ADD_ERROR_ITEM( WIFI_REASON_HANDSHAKE_TIMEOUT ),  // 204
};

const char *mjd_wifi_reason_to_msg(uint8_t code) {
    int i;

    for (i = 0; i < ARRAY_SIZE(_mjd_wifi_reason_msg_table); ++i) {
        if (_mjd_wifi_reason_msg_table[i].code == code) {
            return _mjd_wifi_reason_msg_table[i].msg; // RETURN
        }
    }
    return _mjd_wifi_unknown_msg;
}

/*********************************************************************************
 * event handler
 * @doc Use IRAM_ATTR to reduce the penalty associated with loading the code from flash. Cases when parts of application should or may be placed into IRAM:
 *          - Interrupt handlers (SHOULD).
 *          - Some timing critical code (MAY).
 *
 *********************************************************************************/
static IRAM_ATTR esp_err_t mjd_wifi_sta_event_handler(void *ctx, system_event_t *event) {
    switch (event->event_id) {
    case SYSTEM_EVENT_STA_START:
        ESP_LOGI(TAG, "mjd_wifi_sta_event_handler case SYSTEM_EVENT_STA_START")
        ;

        ESP_ERROR_CHECK(esp_wifi_connect())
        ;
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "mjd_wifi_sta_event_handler case SYSTEM_EVENT_STA_GOT_IP")
        ;

        ip = event->event_info.got_ip.ip_info.ip;
        gw = event->event_info.got_ip.ip_info.gw;
        msk = event->event_info.got_ip.ip_info.netmask;

        xEventGroupClearBits(wifi_event_group, WIFI_DISCONNECTED_BIT);
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        ESP_LOGI(TAG, "mjd_wifi_sta_event_handler case SYSTEM_EVENT_STA_DISCONNECTED")
        ;

        // @debug show reason code!
        system_event_sta_disconnected_t *disconnected = &event->event_info.disconnected;
        ESP_LOGI(TAG, "  SYSTEM_EVENT_STA_DISCONNECTED: ssid:%s, ssid_len:%d, reason:%d (%s)", disconnected->ssid,
                disconnected->ssid_len, disconnected->reason, mjd_wifi_reason_to_msg(disconnected->reason))
        ;

        xEventGroupClearBits(wifi_event_group, WIFI_CONNECTED_BIT);
        xEventGroupSetBits(wifi_event_group, WIFI_DISCONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;
}

/*********************************************************************************
 * PUBLIC
 *********************************************************************************/

esp_err_t mjd_wifi_sta_init(const char *param_ssid, const char *param_password) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t retval = ESP_OK;

    esp_log_level_set("wifi", ESP_LOG_WARN); // @important Disable INFO messages to lower cpu usage of the wifi component (Wifi on core0 easily conflicts with other components such as RMT on core1!).

    wifi_event_group = xEventGroupCreate();
    tcpip_adapter_init();

    retval = esp_event_loop_init(mjd_wifi_sta_event_handler, NULL);
    if (retval != ESP_OK) {
        ESP_LOGE(TAG, "esp_event_loop_init() err %d %s", retval, esp_err_to_name(retval));
        return retval; // EXIT
    }

    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT()
    ;

    retval = esp_wifi_init(&wifi_init_config);
    if (retval != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_init() err %d %s", retval, esp_err_to_name(retval));
        return retval; // EXIT
    }

    retval = esp_wifi_set_storage(WIFI_STORAGE_RAM);
    if (retval != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_set_storage() err %d %s", retval, esp_err_to_name(retval));
        return retval; // EXIT
    }

    retval = esp_wifi_set_mode(WIFI_MODE_STA);
    if (retval != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_set_mode() err %d %s", retval, esp_err_to_name(retval));
        return retval; // EXIT
    }

    wifi_config_t wifi_config = { 0 };    // init struct fields for this variable

    strcpy((char *) wifi_config.sta.ssid, param_ssid);  // (to,from)
    strcpy((char *) wifi_config.sta.password, param_password);  // (to,from)

    retval = esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    if (retval != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_set_config() err %d %s", retval, esp_err_to_name(retval));
        return retval; // EXIT
    }

    ESP_LOGI(TAG, "OK: WIFI initialized!");

    return ESP_OK;
}

esp_err_t mjd_wifi_sta_start() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t retval = ESP_OK;

    EventBits_t uxBits;

    ESP_LOGI(TAG, "Connecting to the WIFI network...");
    retval = esp_wifi_start();
    if (retval != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_start() err %d %s", retval, esp_err_to_name(retval));
        return retval; // EXIT
    }

    uxBits = xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, RTOS_DELAY_15SEC); // RTOS_DELAY_10SEC RTOS_DELAY_15SEC
    if ((uxBits & WIFI_CONNECTED_BIT) == 0) {
        ESP_LOGW(TAG, "FIRST TIME esp_wifi_start() failed to connect. Wait 30 seconds and try again a 2nd time...");

        total_nbr_of_first_connect_errors++;

        retval = esp_wifi_stop();
        if (retval != ESP_OK) {
            ESP_LOGE(TAG, "esp_wifi_stop() err %d %s", retval, esp_err_to_name(retval));
            return retval; // EXIT
        }

        vTaskDelay(RTOS_DELAY_30SEC); // @important

        retval = esp_wifi_start();
        if (retval != ESP_OK) {
            ESP_LOGE(TAG, "esp_wifi_start() err %d %s", retval, esp_err_to_name(retval));
            return retval; // EXIT
        }

        uxBits = xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, RTOS_DELAY_15SEC); // RTOS_DELAY_10SEC RTOS_DELAY_15SEC
        if ((uxBits & WIFI_CONNECTED_BIT) == 0) {
            total_nbr_of_connect_errors++;

            ESP_LOGE(TAG, "ERROR: SECOND TIME esp_wifi_start() failed to connect. => ABORT");
            ESP_LOGI(TAG, "  @doc total_nbr_of_connect_errors:       %u", total_nbr_of_connect_errors);
            ESP_LOGI(TAG, "  @doc total_nbr_of_first_connect_errors: %u", total_nbr_of_first_connect_errors);

            return MJD_ERR_ESP_WIFI; // EXIT
        }
    }

    ESP_LOGI(TAG, "OK: WIFI connected! Network connection details:");
    ESP_LOGI(TAG, "  IP:       %s", inet_ntoa(ip));
    ESP_LOGI(TAG, "  Net mask: %s", inet_ntoa(msk));
    ESP_LOGI(TAG, "  Gateway:  %s", inet_ntoa(gw));
    ESP_LOGI(TAG, "  @doc total_nbr_of_connect_errors:       %u", total_nbr_of_connect_errors);
    ESP_LOGI(TAG, "  @doc total_nbr_of_first_connect_errors: %u", total_nbr_of_first_connect_errors);

    return ESP_OK;
}

esp_err_t mjd_wifi_sta_disconnect_stop() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t retval = ESP_OK;

    EventBits_t uxBits;

    ESP_LOGI(TAG, "Disconnecting from WIFI network...");

    retval = esp_wifi_disconnect();
    if (retval != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_connect() err %d %s", retval, esp_err_to_name(retval));
        return retval; // EXIT
    }

    retval = esp_wifi_stop();
    if (retval != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_stop() err %d %s", retval, esp_err_to_name(retval));
        return retval; // EXIT
    }

    uxBits = xEventGroupWaitBits(wifi_event_group, WIFI_DISCONNECTED_BIT, pdFALSE, pdTRUE, RTOS_DELAY_10SEC);
    if ((uxBits & WIFI_DISCONNECTED_BIT) == 0) {
        ESP_LOGE(TAG, "esp_wifi_disconnect() & esp_wifi_stop() failed to disconnect or stop. Aborting...");
        return MJD_ERR_ESP_WIFI;
    }

    ESP_LOGI(TAG, "OK: WIFI disconnected!");

    return ESP_OK;
}

bool mjd_wifi_sta_is_connected() {
    // @doc Check wifi_event_group is already init'd (!=NULL). If not then certainly no connection active, and cannot use xEventGroupWaitBits(wifi_event_group) at all)!
    // @req This function can be invoked even if mjd_wifi_sta is not yet init'd.
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    bool bretval = true;
    EventBits_t uxBits;

    if (wifi_event_group == NULL) { // Not yet initialized by mjd_wifi_init()
        ESP_LOGD(TAG, "  PTR wifi_event_group %p", wifi_event_group);
        bretval = false;
    } else {
        uxBits = xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, RTOS_DELAY_0);
        if ((uxBits & WIFI_CONNECTED_BIT) == 0) {
            bretval = false;
        }
    }

    return bretval;
}
