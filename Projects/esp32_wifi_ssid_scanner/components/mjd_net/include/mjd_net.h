/*
 *
 */
#ifndef __MJD_NET_H__
#define __MJD_NET_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "driver/timer.h"

#include "lwip/api.h"
#include "lwip/netdb.h"

#include "apps/sntp/sntp.h"      // ESP-IDF < V3.2 Component: lwip - App: sntp
/////#include "lwip/apps/sntp.h" // ESP-IDF >= V3.2 Component: lwip - App: sntp

/**********
 * Network helpers
 */

// Mac Address helper for printf
#define MJDMACFMT "%02X:%02X:%02X:%02X:%02X:%02X"
#define MJDMAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]

/**********
 * IP
 */
esp_err_t mjd_net_get_ip_address(char * param_ptr_ip_address);

/**********
 * DNS
 */
esp_err_t mjd_net_resolve_dns_name(const char * host_name, char * ip_address);

/**********
 * INTERNET (opposed to LAN)
 */
esp_err_t mjd_net_is_internet_reachable();

/**********
 * NTP
 */
esp_err_t mjd_net_sync_current_datetime(bool param_forced);

#ifdef __cplusplus
}
#endif

#endif /* __MJD_NET_H__ */
