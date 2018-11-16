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
 * IP
 *  @tool https://www.browserling.com/tools/hex-to-ip
 */
esp_err_t mjd_net_get_ip_address(char * param_ptr_ip_address) {
    // @dependency A connected STA.
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    tcpip_adapter_ip_info_t ipInfo;

    f_retval = tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "tcpip_adapter_get_ip_info() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        strcpy(param_ptr_ip_address, ""); // Clear receive string
        // GOTO
        goto cleanup;
    }
    if (ipInfo.ip.addr == 0) {
        ESP_LOGE(TAG, "tcpip_adapter_get_ip_info() returned 'invalid' ip address 0.0.0.0 (not connected?)");
        strcpy(param_ptr_ip_address, ""); // Clear receive string
        // GOTO
        goto cleanup;
    }

    // @important The inet_ntoa() function returns a string in a statically allocated buffer, which subsequent calls will overwrite. So copy the resulting string!
    strcpy(param_ptr_ip_address, inet_ntoa(ipInfo.ip.addr));

    // LABEL
    cleanup: ;

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
    err_t err_addr = lwip_getaddrinfo(host_name, NULL, &hints, &res); // @decl int lwip_getaddrinfo(const char *nodename, const char *servname, const struct addrinfo *hints, struct addrinfo **res);
    if (err_addr != ERR_OK || res == NULL) {
        ESP_LOGE(TAG, "DNS lookup FAIL lwip_getaddrinfo() for %s: err=%d res=%p", host_name, err_addr, res);
        strcpy(ip_address, ""); // SPECIAL
        f_retval = MJD_ERR_LWIP;
    } else {
        addr = &((struct sockaddr_in *) res->ai_addr)->sin_addr;
        // @important The inet_ntoa() function returns a string in a statically allocated buffer, which subsequent calls will overwrite. So copy the resulting string!
        strcpy(ip_address, inet_ntoa(*addr));
    }
    freeaddrinfo(res);

    return f_retval;
}

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
 * Date, Time, RTC, NTP
 */
esp_err_t mjd_net_sync_current_datetime(bool param_forced) {
    // @dependency A working Internet connection
    // @doc The func only syncs the time when it is really necessary (unless param_forced is used).
    // @doc The time_t variable represents the time as the number of seconds from a date called Epoch (01/01/1970).
    // @doc Use localtime_r() to split the time_t variable into the different time values (year, month, day, ...) of a tm struct.
    // @doc https://www.ibm.com/support/knowledgecenter/en/ssw_ibm_i_72/apis/settod.htm
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;
    int i_retval = 0;

    time_t now;
    struct tm timeinfo;
    char display_buffer[100];

    // WAIT for the LWIP App SNTP to set the system time
    // @doc Use an ESP32 Timer to avoid waiting forever for an SNTP response, and hanging up the MCU!
    // @doc The standard system time of the MCU after power-on is Thu Jan 1 00:00:00 1970

    // MJD_ERR_ESP_SNTP

    if (param_forced == true) {
        ESP_LOGI(TAG, "param_forced true => reset current datetime to epoch");
        struct timeval epoch_timeval = {0};
        i_retval = settimeofday(&epoch_timeval, NULL);
        if(i_retval != 0) {
            ESP_LOGE(TAG, "settimeofday() FAILED | err %i", i_retval);
        }
    }

    time(&now);
    localtime_r(&now, &timeinfo);

    if (timeinfo.tm_year >= (2015 - 1900)) { // The initial datetime of the MCU is +-y1970, check if it is later or not.
        ESP_LOGI(TAG, "SNTP Sync is NOT NEEDED - timeinfo.tm_year is %d (baseyear is 1900)",
                timeinfo.tm_year);
    } else {
        ESP_LOGI(TAG, "SNTP Sync is REQUIRED - timeinfo.tm_year is %d (baseyear is 1900) or param_forced=true", timeinfo.tm_year);

        // init the lwip App SNTP
        sntp_setoperatingmode(SNTP_OPMODE_POLL);
        sntp_setservername(0, "pool.ntp.org");
        sntp_init();

        // timer init
        timer_config_t tconfig = { };
        tconfig.divider = esp_clk_apb_freq() / 64000; // Let the timer tick on a slow pace. 1.25 Khz: esp_clk_apb_freq() / 64000 = 1.250 ticks/second
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
        const double SNTP_TIMEOUT_SECONDS = 15.0; // 15.0 | @tip Test timeout with value 0.0 (it will fail immediately after the delay).
        double timer_counter_value_seconds = 0;

        while (timeinfo.tm_year < (2015 - 1900)) {
            ESP_LOGI(TAG, "Time not set yet by lwip-SNTP, waiting for response...");

            timer_get_counter_time_sec(TIMER_GROUP_0, TIMER_0, &timer_counter_value_seconds);
            if (timer_counter_value_seconds > SNTP_TIMEOUT_SECONDS) {
                has_timed_out = true;
                break; // BREAK WHILE
            }

            vTaskDelay(RTOS_DELAY_1SEC); // Wait in increments of 1 second

            time(&now);
            localtime_r(&now, &timeinfo);
        }

        if (has_timed_out == false) {
            ESP_LOGI(TAG, "OK time synced with SNTP");
        } else {
            ESP_LOGE(TAG, "ESP32 Timer timed out (%5f seconds), no response from SNTP, time is not synced with SNTP!",
                    timer_counter_value_seconds);
            f_retval = MJD_ERR_ESP_SNTP;
        }

        // pause timer (stop = n.a.)
        timer_pause(TIMER_GROUP_0, TIMER_0);

        // stop the lwip App SNTP
        sntp_stop();
    }

    // logging UTC
    ESP_LOGI(TAG, "Actual time (UTC):");
    ESP_LOGI(TAG, "  - %s", asctime(&timeinfo));
    strftime(display_buffer, sizeof(display_buffer), "day %j of year %Y", &timeinfo);
    ESP_LOGI(TAG, "  - %s", display_buffer);
    strftime(display_buffer, sizeof(display_buffer), "%d/%m/%Y %H:%M:%S", &timeinfo);
    ESP_LOGI(TAG, "  - %s", display_buffer);
    strftime(display_buffer, sizeof(display_buffer), "%A, %d %B %Y", &timeinfo);
    ESP_LOGI(TAG, "  - %s", display_buffer);

    return f_retval;
}
