/*
 * WIFI
 * @doc static <global var>/<global func>: its scope is restricted to the file in which it is declared.
 */

// Component header file(s)
#include "mjd.h"
#include "mjd_net.h"
#include "mjd_wifi.h"

/**********
 * Logging
 */
static const char TAG[] = "mjd_wifi";

/**********
 * MAIN
 */
static EventGroupHandle_t _wifi_event_group;
static const int WIFI_CONNECTED_BIT = BIT0;
static const int WIFI_DISCONNECTED_BIT = BIT1;

static bool _is_wifi_sta_initialized = false;

static uint32_t _total_nbr_of_first_connect_warnings = 0;
static uint32_t _total_nbr_of_fatal_connect_errors = 0;

static mjd_wifi_sta_info_t _mjd_wifi_sta_info =
            { 0 };

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

static const mjd_wifi_reason_msg_t _mjd_wifi_reason_msg_table[] =
            {
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
static IRAM_ATTR esp_err_t _mjd_wifi_sta_event_handler(void *ctx, system_event_t *event) {
    esp_err_t retval = ESP_OK;

    switch (event->event_id) {
    case SYSTEM_EVENT_STA_START:
        ESP_LOGI(TAG, "%s case SYSTEM_EVENT_STA_START", __FUNCTION__);

        _mjd_wifi_sta_info.sta_is_connected = false;

        ESP_ERROR_CHECK(esp_wifi_connect())
        ;

        break;
    case SYSTEM_EVENT_STA_CONNECTED:
        ESP_LOGI(TAG, "%s case SYSTEM_EVENT_STA_CONNECTED", __FUNCTION__);

        retval = esp_wifi_get_mac(ESP_IF_WIFI_STA, _mjd_wifi_sta_info.sta_mac);
        if (retval != ESP_OK) {
            ESP_LOGE(TAG, "%s esp_wifi_get_mac() err %d %s", __FUNCTION__, retval, esp_err_to_name(retval));
            memset(_mjd_wifi_sta_info.sta_mac, 0, sizeof(_mjd_wifi_sta_info.sta_mac)); // @doc sizeof (a) gives you a size in bytes of full array (opposed to the number of array elements).
        }

        memcpy(_mjd_wifi_sta_info.ap_ssid, event->event_info.connected.ssid, sizeof(_mjd_wifi_sta_info.ap_ssid)); // (to,from)
        _mjd_wifi_sta_info.ap_ssid_len = event->event_info.connected.ssid_len;
        memcpy(_mjd_wifi_sta_info.ap_bssid, event->event_info.connected.bssid, sizeof(_mjd_wifi_sta_info.ap_bssid)); // (to,from)
        _mjd_wifi_sta_info.ap_channel = event->event_info.connected.channel;

        wifi_ap_record_t ap_record;
        retval = esp_wifi_sta_get_ap_info(&ap_record);
        if (retval == ESP_OK) {
            _mjd_wifi_sta_info.ap_rssi = ap_record.rssi;
        } else {
            _mjd_wifi_sta_info.ap_rssi = 0;
        }

        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "%s case SYSTEM_EVENT_STA_GOT_IP", __FUNCTION__);

        _mjd_wifi_sta_info.sta_is_connected = true;

        ip4_addr_copy(_mjd_wifi_sta_info.ip_address, event->event_info.got_ip.ip_info.ip);
        ip4_addr_copy(_mjd_wifi_sta_info.gateway_address, event->event_info.got_ip.ip_info.gw);
        ip4_addr_copy(_mjd_wifi_sta_info.subnet_mask, event->event_info.got_ip.ip_info.netmask);

        xEventGroupClearBits(_wifi_event_group, WIFI_DISCONNECTED_BIT);
        xEventGroupSetBits(_wifi_event_group, WIFI_CONNECTED_BIT);

        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        ESP_LOGI(TAG, "%s case SYSTEM_EVENT_STA_DISCONNECTED", __FUNCTION__);

        _mjd_wifi_sta_info.sta_is_connected = false;

        // @debug show reason code!
        system_event_sta_disconnected_t *disconnected = &event->event_info.disconnected;
        ESP_LOGW(TAG, "  SYSTEM_EVENT_STA_DISCONNECTED: ssid: %s | ssid_len: %d | reason: %d (%s)", disconnected->ssid,
                disconnected->ssid_len,
                disconnected->reason, mjd_wifi_reason_to_msg(disconnected->reason));

        xEventGroupClearBits(_wifi_event_group, WIFI_CONNECTED_BIT);
        xEventGroupSetBits(_wifi_event_group, WIFI_DISCONNECTED_BIT);

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

    esp_err_t f_retval = ESP_OK;

    // CHECK init already done?
    if (_is_wifi_sta_initialized == true) {
        ESP_LOGW(TAG, "OK. Wifi has already been initialized");
        // GOTO
        goto cleanup;
    }

    /*
     * Optional for Production: dump less messages
     *   @doc It is possible to lower the log level for specific modules (wifi and tcpip_adapter are strong candidates)
     */
    /////esp_log_level_set("wifi", ESP_LOG_WARN); // @important Disable INFO messages which are too detailed for me.
    /////esp_log_level_set("tcpip_adapter", ESP_LOG_WARN); // @important Disable INFO messages which are too detailed for me.
    /////esp_log_level_set("phy_init", ESP_LOG_WARN); // @important Disable INFO messages which are too detailed for me.
    /**/

    _wifi_event_group = xEventGroupCreate();

    f_retval = esp_event_loop_init(_mjd_wifi_sta_event_handler, NULL);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "esp_event_loop_init() err %d %s", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    tcpip_adapter_init();

    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT()
            ;

    f_retval = esp_wifi_init(&wifi_init_config);
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

    f_retval = esp_wifi_set_mode(WIFI_MODE_STA);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_set_mode() err %d %s", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    wifi_config_t wifi_config =
                { 0 };    // init struct fields for this variable
    strcpy((char *) wifi_config.sta.ssid, param_ssid);  // (to,from)
    strcpy((char *) wifi_config.sta.password, param_password);  // (to,from)

    f_retval = esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_set_config() err %d %s", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // MARK init done
    _is_wifi_sta_initialized = true;

    ESP_LOGI(TAG, "OK: WIFI initialized!");

    // LABEL
    cleanup: ;

    return f_retval;
}

esp_err_t mjd_wifi_sta_start() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    EventBits_t uxBits;

    ESP_LOGI(TAG, "Connecting to the WIFI network...");
    f_retval = esp_wifi_start();
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_start() err %d %s", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    uxBits = xEventGroupWaitBits(_wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, RTOS_DELAY_6SEC); // (1-3sec=AUTH_FAIL!) RTOS_DELAY_1SEC RTOS_DELAY_6SEC
    if ((uxBits & WIFI_CONNECTED_BIT) == 0) {
        ESP_LOGW(TAG, "FIRST TIME esp_wifi_start() failed to connect. Wait 5 seconds and try a 2nd time...");

        ++_total_nbr_of_first_connect_warnings;

        f_retval = esp_wifi_stop();
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "esp_wifi_stop() err %d %s", f_retval, esp_err_to_name(f_retval));
            // GOTO
            goto cleanup;
        }

        vTaskDelay(RTOS_DELAY_5SEC); // @important delay 5 seconds

        f_retval = esp_wifi_start();
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "esp_wifi_start() err %d %s", f_retval, esp_err_to_name(f_retval));
            // GOTO
            goto cleanup;
        }

        uxBits = xEventGroupWaitBits(_wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, RTOS_DELAY_5SEC); // (1-3sec=AUTH_FAIL!) RTOS_DELAY_5SEC RTOS_DELAY_10SEC
        if ((uxBits & WIFI_CONNECTED_BIT) == 0) {
            ++_total_nbr_of_fatal_connect_errors;

            ESP_LOGE(TAG, "ERROR: SECOND TIME esp_wifi_start() failed to connect. => ABORT");
            ESP_LOGI(TAG, "  @stats _total_nbr_of_first_connect_warnings (retried): %u",
                    _total_nbr_of_first_connect_warnings);
            ESP_LOGI(TAG, "  @stats _total_nbr_of_fatal_connect_errors:             %u", _total_nbr_of_fatal_connect_errors);

            f_retval = MJD_ERR_ESP_WIFI; // mark error code
            // GOTO
            goto cleanup;
        }
    }

    // LABEL
    cleanup: ;

    return f_retval;
}

esp_err_t mjd_wifi_sta_disconnect_stop() {
    // @doc This function can be invoked even if mjd_wifi_sta is not yet init'd.
    // @doc Checks also if _wifi_event_group is NULL. Then certainly no connection is active.
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    EventBits_t uxBits;

    if (_wifi_event_group == NULL) { // @context Not yet initialized by mjd_wifi_sta_init()
        ESP_LOGD(TAG, "  (ok) NULL PTR _wifi_event_group %p => certainly not connected to Wifi", _wifi_event_group);
        // GOTO
        goto cleanup;
    }

    ESP_LOGI(TAG, "Disconnecting from WIFI network...");

    f_retval = esp_wifi_disconnect();
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_connect() err %d %s", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    f_retval = esp_wifi_stop();
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_stop() err %d %s", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    uxBits = xEventGroupWaitBits(_wifi_event_group, WIFI_DISCONNECTED_BIT, pdFALSE, pdTRUE, RTOS_DELAY_5SEC); // @important Wait at the most 5 sec
    if ((uxBits & WIFI_DISCONNECTED_BIT) == 0) {
        ESP_LOGE(TAG, "esp_wifi_disconnect() & esp_wifi_stop() failed to disconnect or stop. Aborting...");

        f_retval = MJD_ERR_ESP_WIFI; // mark error code
        // GOTO
        goto cleanup;
    }

    ESP_LOGI(TAG, "OK: WIFI disconnected!");
    ESP_LOGI(TAG, "  @stats _total_nbr_of_first_connect_warnings (retried): %u", _total_nbr_of_first_connect_warnings);
    ESP_LOGI(TAG, "  @stats _total_nbr_of_fatal_connect_errors:             %u", _total_nbr_of_fatal_connect_errors);

    // LABEL
    cleanup: ;

    return f_retval;
}

bool mjd_wifi_sta_is_connected() {
    // @doc This function can be invoked even if mjd_wifi_sta is not yet init'd.
    // @doc Checks also if _wifi_event_group is NULL. Then certainly no connection is active.
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    bool b_retval = true;
    EventBits_t uxBits;

    if (_wifi_event_group == NULL) { // @context Not yet initialized by mjd_wifi_sta_init()
        ESP_LOGD(TAG, "  PTR _wifi_event_group is NULL %p => NOT connected to Wifi", _wifi_event_group);
        b_retval = false;
        // GOTO
        goto cleanup;
    }

    uxBits = xEventGroupWaitBits(_wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdTRUE, RTOS_DELAY_0);
    if ((uxBits & WIFI_CONNECTED_BIT) == 0) {
        ESP_LOGD(TAG, "  WIFI_CONNECTED_BIT) is 0 => NOT connected to Wifi");
        b_retval = false;
        // GOTO
        goto cleanup;
    }

    // TRUE
    ESP_LOGD(TAG, "  OK Connected to Wifi");

    // LABEL
    cleanup: ;

    return b_retval;
}

esp_err_t mjd_wifi_sta_get_info(mjd_wifi_sta_info_t* param_ptr_info) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    if (param_ptr_info == NULL) {
        ESP_LOGE(TAG, "param_ptr_info is NULL");
        f_retval = ESP_FAIL;
        // GOTO
        goto cleanup;
    }

    param_ptr_info->sta_is_connected = _mjd_wifi_sta_info.sta_is_connected;
    memcpy(param_ptr_info->sta_mac, _mjd_wifi_sta_info.sta_mac, sizeof(_mjd_wifi_sta_info.sta_mac)); // (to,from)
    memcpy(param_ptr_info->ap_ssid, _mjd_wifi_sta_info.ap_ssid, sizeof(_mjd_wifi_sta_info.ap_ssid)); // (to,from)
    param_ptr_info->ap_ssid_len = _mjd_wifi_sta_info.ap_ssid_len;
    memcpy(param_ptr_info->ap_bssid, _mjd_wifi_sta_info.ap_bssid, sizeof(_mjd_wifi_sta_info.ap_bssid)); // (to,from)
    param_ptr_info->ap_channel = _mjd_wifi_sta_info.ap_channel;
    param_ptr_info->ap_rssi = _mjd_wifi_sta_info.ap_rssi;
    ip4_addr_copy(param_ptr_info->ip_address, _mjd_wifi_sta_info.ip_address);
    ip4_addr_copy(param_ptr_info->gateway_address, _mjd_wifi_sta_info.gateway_address);
    ip4_addr_copy(param_ptr_info->subnet_mask, _mjd_wifi_sta_info.subnet_mask);

    // LABEL
    cleanup: ;

    return f_retval;
}

esp_err_t mjd_wifi_log_sta_info() {
    // @important The inet_ntoa() function returns a string in a statically allocated buffer, which subsequent calls will overwrite. So copy the string for later use!
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    ESP_LOGI(TAG, "Wifi Station Info:");
    ESP_LOGI(TAG, "  Station::");
    ESP_LOGI(TAG, "      Is Connected: "MJDBOOLEANFMT, MJDBOOLEAN2STR(_mjd_wifi_sta_info.sta_is_connected));
    ESP_LOGI(TAG, "      MAC address:  "MJDMACFMT, MJDMAC2STR(_mjd_wifi_sta_info.sta_mac));
    ESP_LOGI(TAG, "  WiFi::");
    ESP_LOGI(TAG, "      SSID:     %s", _mjd_wifi_sta_info.ap_ssid);
    ESP_LOGI(TAG, "      SSID len: %u", _mjd_wifi_sta_info.ap_ssid_len);
    ESP_LOGI(TAG, "      BSSID:    "MJDMACFMT, MJDMAC2STR(_mjd_wifi_sta_info.ap_bssid));
    ESP_LOGI(TAG, "      Channel:  %u", _mjd_wifi_sta_info.ap_channel);
    ESP_LOGI(TAG, "      RSSI:     %i", _mjd_wifi_sta_info.ap_rssi);
    ESP_LOGI(TAG, "  Net::");
    ESP_LOGI(TAG, "      IPv4 address:         %s", inet_ntoa(_mjd_wifi_sta_info.ip_address));
    ESP_LOGI(TAG, "      IPv4 subnet mask:     %s", inet_ntoa(_mjd_wifi_sta_info.subnet_mask));
    ESP_LOGI(TAG, "      IPv4 gateway address: %s", inet_ntoa(_mjd_wifi_sta_info.gateway_address));
    ESP_LOGI(TAG, "  @stats _total_nbr_of_first_connect_warnings (retried): %u", _total_nbr_of_first_connect_warnings);
    ESP_LOGI(TAG, "  @stats _total_nbr_of_fatal_connect_errors:             %u", _total_nbr_of_fatal_connect_errors);

    return ESP_OK;
}
