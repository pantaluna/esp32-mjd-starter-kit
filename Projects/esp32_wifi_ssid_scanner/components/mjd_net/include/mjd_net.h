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
#include "lwip/err.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"

#include "apps/sntp/sntp.h"      // ESP-IDF  < V3.2 Component: lwip - App: sntp
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

/**
 * @brief mjd_net_resolve_hostname_ipv4()
 *        Resolve a hostname to its IPv4 address in string format.
 *        Also works when providing an IPv4 address as input.
 *
 * @note  getaddrinfo() returns a linked list of address structures. Filter IPv4 addresses. Only use the first address (which is always the preferred address).
 *
 * @param      param_hostname  The hostname, or its IPV4 address, of the UDP Server
 * @param      param_port      The port of the UDP Server
 * @param      param_buf       The message buffer
 * @param      param_len       The length of the message buffer
 *
 * @return
 *  - ESP_OK
 *  - ESP_FAIL
 */
esp_err_t mjd_net_resolve_hostname_ipv4(const char * param_host_name, char * param_ip_address);

/**********
 * DNS
 */
esp_err_t mjd_net_resolve_dns_name(const char * host_name, char * ip_address);

/**********
 * INTERNET (opposed to LAN)
 */
esp_err_t mjd_net_is_internet_reachable();

/**********
 * NTP, Date, Time, RTC
 */
esp_err_t mjd_net_sync_current_datetime(bool param_forced);

/**********
 * UDP
 */

/**
 * @brief mjd_net_udp_send_buffer()
 *
 * @note
 *
 * @param      param_hostname  The hostname or its IPV4 address of the UDP Server
 * @param      param_port      The port of the UDP Server
 * @param      param_buf       The message buffer
 * @param      param_len       The length of the message buffer
 *
 * @return
 *  - ESP_OK
 *  - ESP_FAIL
 */
esp_err_t mjd_net_udp_send_buffer(const char *param_hostname, int param_port, uint8_t *param_buf, size_t param_len);

#ifdef __cplusplus
}
#endif

#endif /* __MJD_NET_H__ */
