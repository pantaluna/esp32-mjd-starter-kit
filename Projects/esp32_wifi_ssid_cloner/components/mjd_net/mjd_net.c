/*
 * Component: NET
 *  @doc static <global var>/<global func>: its scope is restricted to the file in which it is declared.
 */

// Component header file(s)
#include "mjd.h"
#include "mjd_net.h"

/**********
 * Logging
 */
static const char TAG[] = "mjd_net";

/*
 * MAIN
 */

/**********
 * INTERNET (opposed to LAN)
 */
esp_err_t mjd_net_is_internet_reachable() {
    // @dependency A working Internet connection
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    const char DNS_CHECK_HOST_NAME[] = "www.google.com";
    char ip_address[128];

    f_retval = mjd_net_resolve_dns_name(DNS_CHECK_HOST_NAME, ip_address);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "mjd_net_resolve_dns_name() FAILED err %i", f_retval);
        f_retval = ESP_FAIL;
    }

    return f_retval;
}

/**********
 * DNS
 */
esp_err_t mjd_net_resolve_dns_name(const char * host_name, char * ip_address) {
    // @dependency A working Internet connection
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    ip_addr_t addr_netconn;
    err_t err_netconn = netconn_gethostbyname_addrtype(host_name, &addr_netconn, NETCONN_DNS_IPV4);
    if (err_netconn != ERR_OK) {
        ESP_LOGE(TAG, "DNS lookup FAIL netconn_gethostbyname_addrtype() for %s: err=%d", host_name, err_netconn);
        f_retval = MJD_ERR_NETCONN;
    }
    if (f_retval != ESP_OK) {
        strcpy(ip_address, ""); // SPECIAL
        return f_retval; // EXIT
    }

    const struct addrinfo hints = { .ai_family = AF_INET, .ai_socktype = SOCK_STREAM };
    struct addrinfo *res;
    struct in_addr *addr;
    err_t err_addr = lwip_getaddrinfo(host_name, NULL, &hints, &res);   // @decl int lwip_getaddrinfo(const char *nodename, const char *servname, const struct addrinfo *hints, struct addrinfo **res);
    if (err_addr != ERR_OK || res == NULL) {
        ESP_LOGE(TAG, "DNS lookup FAIL lwip_getaddrinfo() for %s: err=%d res=%p", host_name, err_addr, res);
        strcpy(ip_address, ""); // SPECIAL
        f_retval = MJD_ERR_LWIP;
    } else {
        addr = &((struct sockaddr_in *) res->ai_addr)->sin_addr;
        strcpy(ip_address, inet_ntoa(*addr));
    }
    freeaddrinfo(res);

    return f_retval;
}

/**********
 * Date, Time, RTC, NTP
 */
esp_err_t mjd_net_sync_current_datetime() {
    // @dependency A working Internet connection
    // @doc The time_t variable represents the time as the number of seconds from a date called Epoch (01/01/1970).
    // @doc To split the time_t variable into the different time values (year, month, day, ...) you use the localtime_r method which updates a tm struct.
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    time_t now;
    struct tm timeinfo;
    char display_buffer[100];

    // WAIT for the LWIP App SNTP to set the system time
    // @doc Use an ESP32 Timer to avoid waiting forever for an SNTP response, and hanging up the MCU!
    // @doc The standard system time of the MCU after power-on is Thu Jan 1 00:00:00 1970

    // MJD_ERR_ESP_SNTP

    time(&now);
    localtime_r(&now, &timeinfo);

    if (timeinfo.tm_year >= (2015 - 1900)) {    // The initial datetime of the MCU is +-y1970, check if it is later or not.
        ESP_LOGI(TAG, "OK. SNTP Sync is NOT NEEDED - timeinfo.tm_year is %d (baseyear is 1900) \n", timeinfo.tm_year);
    } else {
        ESP_LOGI(TAG, "SNTP Sync is REQUIRED - timeinfo.tm_year is %d (baseyear is 1900) \n", timeinfo.tm_year);

        // init the lwip App SNTP
        sntp_setoperatingmode(SNTP_OPMODE_POLL);
        sntp_setservername(0, "pool.ntp.org");
        sntp_init();

        // timer init
        timer_config_t tconfig = {};
        tconfig.divider = esp_clk_apb_freq() / 10000; // Let the timer tick on a slow pace. 1 Mhz: esp_clk_apb_freq() / 1000000 = 80 ticks
        tconfig.counter_dir = TIMER_COUNT_UP;
        tconfig.counter_en = TIMER_PAUSE;
        tconfig.alarm_en = TIMER_ALARM_DIS;
        tconfig.auto_reload = false;
        timer_init(TIMER_GROUP_0, TIMER_0, &tconfig);

        // timer start with this value (uint64_t).
        timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 00000000ULL);
        timer_start(TIMER_GROUP_0, TIMER_0);

        // LOOP wait for updated datetime XOR timeout from esptimer
        bool has_timed_out = false;
        const double SNTP_TIMEOUT_SECONDS = 60.0; // 60.0 | @tip Test timeout with value 0.0 (it will fail immediately after the delay of 5sec).
        double timer_counter_value_seconds = 0;

        while (timeinfo.tm_year < (2015 - 1900)) {
            ESP_LOGI(TAG, "Time not set yet by lwip-SNTP, waiting for response...");

            timer_get_counter_time_sec(TIMER_GROUP_0, TIMER_0, &timer_counter_value_seconds);
            if (timer_counter_value_seconds > SNTP_TIMEOUT_SECONDS) {
                has_timed_out = true;
                break; // BREAK WHILE
            }

            vTaskDelay(RTOS_DELAY_5SEC);

            time(&now);
            localtime_r(&now, &timeinfo);
        }

        if (has_timed_out == false) {
            ESP_LOGI(TAG, "OK time synced with SNTP");
        } else {
            ESP_LOGE(TAG, "ESP32 Timer timed out (%5f seconds), no response from SNTP, time is not synced with SNTP!", timer_counter_value_seconds);
            f_retval = MJD_ERR_ESP_SNTP;
        }

        // pause timer (stop = n.a.)
        timer_pause(TIMER_GROUP_0, TIMER_0);

        // stop the lwip App SNTP
        sntp_stop();
    }

    // logging UTC
    ESP_LOGI(TAG, "Actual time (UTC):");
    strftime(display_buffer, sizeof(display_buffer), "%d/%m/%Y %H:%M:%S", &timeinfo);
    ESP_LOGI(TAG, "  - %s", display_buffer);
    strftime(display_buffer, sizeof(display_buffer), "%A, %d %B %Y", &timeinfo);
    ESP_LOGI(TAG, "  - %s", display_buffer);
    strftime(display_buffer, sizeof(display_buffer), "Today is day %j of year %Y", &timeinfo);
    ESP_LOGI(TAG, "  - %s", display_buffer);

    return f_retval;
}
