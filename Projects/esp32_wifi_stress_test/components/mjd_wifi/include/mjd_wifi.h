/*
 *
 */
#ifndef __MJD_WIFI_H__
#define __MJD_WIFI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_wifi.h"
#include "lwip/netdb.h"

// Types

/** @brief Save various info about the connected STA */
typedef struct {
        uint8_t sta_mac[6]; /**< STA MAC address */
        bool sta_is_connected; /**< is STA connnected to an AP? */
        ip4_addr_t ip_address; /**< STA IPv4 address */
        ip4_addr_t gateway_address; /**< Gateway IPv4 address */
        ip4_addr_t subnet_mask; /**< IPv4 subnet mask */
        uint8_t ap_bssid[6]; /**< BSSID of connected AP*/
        uint8_t ap_channel; /**< channel of connected AP*/
        int8_t ap_rssi; /**< signal strength of connected AP */
        uint8_t ap_ssid[32]; /**< SSID of connected AP */
        uint8_t ap_ssid_len; /**< SSID length of connected AP */
} mjd_wifi_sta_info_t;

// Function Declarations
const char *mjd_wifi_reason_to_msg(uint8_t code);
esp_err_t mjd_wifi_sta_init(const char *param_ssid, const char *param_password);
esp_err_t mjd_wifi_sta_start();
esp_err_t mjd_wifi_sta_disconnect_stop();
esp_err_t mjd_wifi_sta_get_info(mjd_wifi_sta_info_t* param_ptr_info);
bool mjd_wifi_sta_is_connected();
esp_err_t mjd_wifi_log_sta_info();

#ifdef __cplusplus
}
#endif

#endif /* __MJD_WIFI_H__ */
