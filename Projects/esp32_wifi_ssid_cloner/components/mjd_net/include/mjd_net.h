/*
 *
 */
#ifndef __MJD_NET_H__
#define __MJD_NET_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "lwip/api.h"
#include "lwip/netdb.h"
#include "apps/sntp/sntp.h" // Component: lwip - App: sntp
#include "driver/timer.h"

/**********
 * Network helpers
 */

// Mac Address helper for printf
#ifndef MJDMAC2STR
#define MJDMACFMT "%02x:%02x:%02x:%02x:%02x:%02x"
#define MJDMAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#endif

/**********
 * INTERNET (opposed to LAN)
 */
esp_err_t mjd_net_is_internet_reachable();

/**********
 * DNS
 */
esp_err_t mjd_net_resolve_dns_name(const char * host_name, char * ip_address);

/**********
 * NTP
 */
esp_err_t mjd_net_sync_current_datetime();

#ifdef __cplusplus
}
#endif

#endif /* __MJD_NET_H__ */
