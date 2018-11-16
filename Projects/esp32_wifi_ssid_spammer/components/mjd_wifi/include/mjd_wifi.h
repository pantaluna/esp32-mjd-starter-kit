/*
 *
 */
#ifndef __MJD_WIFI_H__
#define __MJD_WIFI_H__

#include "esp_wifi.h"

#include "lwip/netdb.h"


#ifdef __cplusplus
extern "C" {
#endif


// Function Declarations
const char *mjd_wifi_reason_to_msg(uint8_t code);
esp_err_t mjd_wifi_sta_init(const char *param_ssid, const char *param_password);
esp_err_t mjd_wifi_sta_start();
esp_err_t mjd_wifi_sta_disconnect_stop();
bool mjd_wifi_sta_is_connected();


#ifdef __cplusplus
}
#endif

#endif /* __MJD_WIFI_H__ */
