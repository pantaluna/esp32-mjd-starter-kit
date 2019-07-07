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
 * MAC ADDRESSES
 *
 * @example 30:AE:A4:30:95:AC 00:00:00:00:00:00
 * @doc https://stackoverflow.com/questions/20553805/how-to-convert-a-mac-address-in-string-to-array-of-integers
 * @doc https://stackoverflow.com/questions/4162923/calculate-length-of-array-in-c-by-using-function
 *
 */
esp_err_t mjd_string_to_mac(const char * param_ptr_input, uint8_t param_ptr_mac[], size_t param_size_mac) {
    esp_err_t f_retval = ESP_OK;

    const uint32_t LEN_MAC_ARRAY = 6;
    uint8_t values[LEN_MAC_ARRAY];

    if (strlen(param_ptr_input) != strlen("00:00:00:00:00:00")) {
        memset(param_ptr_mac, 0, LEN_MAC_ARRAY);
        f_retval = ESP_ERR_INVALID_ARG;
        ESP_LOGE(TAG, "%s(). ABORT. param_ptr_input invalid string length", __FUNCTION__);
        // GOTO
        goto cleanup;
    }

    if (param_size_mac != LEN_MAC_ARRAY) {
        memset(param_ptr_mac, 0, LEN_MAC_ARRAY);
        f_retval = ESP_ERR_INVALID_ARG;
        ESP_LOGE(TAG, "%s(). ABORT. param_size_mac invalid length %zu (expected %zu)", __FUNCTION__, param_size_mac, LEN_MAC_ARRAY);
        // GOTO
        goto cleanup;
    }

    if (ARRAY_SIZE(values) != sscanf(param_ptr_input, "%hhX:%hhX:%hhX:%hhX:%hhX:%hhX",
            &values[0], &values[1], &values[2], &values[3], &values[4], &values[5])) {
        memset(param_ptr_mac, 0, LEN_MAC_ARRAY);
        f_retval = ESP_ERR_INVALID_ARG;
        ESP_LOGE(TAG, "%s(). ABORT. invalid mac address in input string", __FUNCTION__);
        // GOTO
        goto cleanup;
    }

    for (uint8_t i = 0; i < LEN_MAC_ARRAY; ++i) {
        param_ptr_mac[i] = values[i];
    }

    ESP_LOGV(TAG, "%s(). () param_ptr_input (HEXDUMP)", __FUNCTION__);
    ESP_LOG_BUFFER_HEXDUMP(TAG, param_ptr_input,  1 + strlen(param_ptr_input), ESP_LOG_VERBOSE);  // +1 to see the \0
    ESP_LOGV(TAG, "%s(). () param_ptr_mac (HEXDUMP)", __FUNCTION__);
    ESP_LOG_BUFFER_HEXDUMP(TAG, param_ptr_mac, LEN_MAC_ARRAY, ESP_LOG_VERBOSE); // @important Cannot use ARRAY_SIZE(param_ptr_mac)!

    // LABEL
    cleanup: ;

    return f_retval;
}

esp_err_t mjd_mac_to_string(const uint8_t param_ptr_input_mac[], size_t param_size_mac, char * param_ptr_output) {
    esp_err_t f_retval = ESP_OK;

    const size_t LEN_MAC_ARRAY = 6;
    const size_t LEN_STRING = 17;

    strcpy(param_ptr_output, "");

    if (param_size_mac != LEN_MAC_ARRAY) {
        f_retval = ESP_ERR_INVALID_ARG;
        ESP_LOGE(TAG, "%s(). ABORT. param_size_mac invalid length %zu (expected %zu)", __FUNCTION__, param_size_mac, LEN_MAC_ARRAY);
        // GOTO
        goto cleanup;
    }

    size_t len_sprintf = sprintf(param_ptr_output, "%hhX:%hhX:%hhX:%hhX:%hhX:%hhX",
            param_ptr_input_mac[0], param_ptr_input_mac[1], param_ptr_input_mac[2], param_ptr_input_mac[3], param_ptr_input_mac[4], param_ptr_input_mac[5]);
    if (len_sprintf != LEN_STRING) {
        f_retval = ESP_ERR_INVALID_ARG;
        ESP_LOGE(TAG, "%s(). ABORT. resulting string length is %zu (expecting %zu)", __FUNCTION__, len_sprintf, LEN_STRING);
        // GOTO
        goto cleanup;
    }

    ESP_LOGV(TAG, "%s(). () param_ptr_input_mac (HEXDUMP)", __FUNCTION__);
    ESP_LOG_BUFFER_HEXDUMP(TAG, param_ptr_input_mac, param_size_mac, ESP_LOG_VERBOSE); // @important Cannot use ARRAY_SIZE(param_ptr_mac)!
    ESP_LOGV(TAG, "%s(). () param_ptr_output (HEXDUMP)", __FUNCTION__);
    ESP_LOG_BUFFER_HEXDUMP(TAG, param_ptr_output,  1 + strlen(param_ptr_output), ESP_LOG_VERBOSE);  // +1 to see the \0

    // LABEL
    cleanup: ;

    return f_retval;
}


/**********
 * IP
 *   @tool https://www.browserling.com/tools/hex-to-ip
 */
esp_err_t mjd_net_get_ip_address(char * param_ptr_ip_address) {
    // @dependency A connected STA.
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    tcpip_adapter_ip_info_t ip_info;

    f_retval = tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ip_info);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "tcpip_adapter_get_ip_info() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        strcpy(param_ptr_ip_address, ""); // Clear receive string
        // GOTO
        goto cleanup;
    }
    if (ip_info.ip.addr == 0) {
        ESP_LOGE(TAG, "tcpip_adapter_get_ip_info() returned 'invalid' ip address 0.0.0.0 (not connected?)");
        strcpy(param_ptr_ip_address, ""); // Clear receive string
        // GOTO
        goto cleanup;
    }

    // @important The inet_ntoa() function returns a string in a statically allocated buffer, which subsequent calls will overwrite. So copy the resulting string!
    strcpy(param_ptr_ip_address, inet_ntoa(ip_info.ip.addr));

    // LABEL
    cleanup: ;

    return f_retval;
}

esp_err_t mjd_net_resolve_hostname_ipv4(const char * param_host_name, char * param_ip_address) {
    // @dependency A working Internet connection
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    int i_retval;
    struct addrinfo hints, *result;
    struct in_addr *addr;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; /* Only IPv4 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */

    i_retval = getaddrinfo(param_host_name, NULL, &hints, &result);
    if (i_retval != 0) {
        ESP_LOGE(TAG, "getaddrinfo() for hostname=%s: err=%i", param_host_name, i_retval);
        strcpy(param_ip_address, ""); // SPECIAL
        f_retval = ESP_FAIL; // SPECIAL
        // GOTO
        goto cleanup;
    }

    addr = &((struct sockaddr_in *) result->ai_addr)->sin_addr;
    // @important The inet_ntoa() function returns a string in a statically allocated buffer, which subsequent calls will overwrite. So copy the resulting string!
    strcpy(param_ip_address, inet_ntoa(*addr));

    // LABEL
    cleanup: ;

    freeaddrinfo(result);

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

    const struct addrinfo hints =
                { .ai_family = AF_INET, .ai_socktype = SOCK_STREAM };
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
 * NTP, Date, Time, RTC
 */
esp_err_t mjd_net_sync_current_datetime(bool param_forced) {
    // @dependency A working Internet connection
    // @doc The func only syncs the time when it is really necessary (unless param_forced is used).
    // @doc The time_t variable is an integral value that represents the time as the number of seconds
    //      from the date called Epoch aka. Unix Epoch (= 00:00 hours, Jan 1, 1970 UTC).
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
        struct timeval epoch_timeval =
                    { 0 };
        i_retval = settimeofday(&epoch_timeval, NULL);
        if (i_retval != 0) {
            ESP_LOGE(TAG, "settimeofday() FAILED | err %i", i_retval);
        }
    }

    time(&now);
    localtime_r(&now, &timeinfo);

    if (timeinfo.tm_year >= (2015 - 1900)) { // The initial datetime of the MCU is +-y1970, check if it is later or not.
        ESP_LOGI(TAG, "SNTP Sync is NOT NEEDED - timeinfo.tm_year is %d (baseyear is 1900)", timeinfo.tm_year);
    } else {
        ESP_LOGI(TAG, "SNTP Sync is REQUIRED - timeinfo.tm_year is %d (baseyear is 1900) or param_forced=true",
                timeinfo.tm_year);

        // init the lwip App SNTP
        sntp_setoperatingmode(SNTP_OPMODE_POLL);
        sntp_setservername(0, "pool.ntp.org");
        sntp_init();

        // timer init
        timer_config_t tconfig = {};
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
        const double SNTP_TIMEOUT_SECONDS = 15.0; // 15.0 @tip Test timeout with value 0.0 (it will fail immediately after the delay).
        double timer_counter_value_seconds = 0;

        while (timeinfo.tm_year < (2015 - 1900)) {
            ESP_LOGD(TAG, "Time not set yet by lwip-SNTP, waiting for response...");

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

/**********
 * UDP
 */
esp_err_t mjd_net_udp_send_buffer(const char *param_hostname, int param_port, uint8_t *param_buf, size_t param_len) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    /********************************************************************************
     * Reuseable variables
     */
    esp_err_t f_retval = ESP_OK;
    esp_err_t esp_retval = ESP_OK;
    int net_retval;

    /********************************************************************************
     * MAIN
     */
    ESP_LOGD(TAG, "Preparing destination address");

    char str_ip_address[128];
    esp_retval = mjd_net_resolve_hostname_ipv4(param_hostname, str_ip_address);
    if (esp_retval != ESP_OK) {
        ESP_LOGE(TAG, "Error mjd_net_resolve_hostname_ipv4(): errno %i (%s)", esp_retval, esp_err_to_name(esp_retval));
        f_retval = ESP_FAIL;
        // GOTO (NOT cleanup_sock!)
        goto cleanup;
    }
    ESP_LOGD(TAG, "  Resolved hostname %s to IPv4 %s", param_hostname, str_ip_address);

    struct sockaddr_in destAddr;
    const int addr_family = AF_INET;
    const int ip_protocol = IPPROTO_IP; // IPv4
    const int socket_style = SOCK_DGRAM;
    /* inet_aton() converts the Internet host address cp from the IPv4 numbers-and-dots notation into binary form (in network byte order)
     *             and stores it in the structure that inp points to. Returns zero if the address is invalid. */
    net_retval = inet_aton(str_ip_address, &destAddr.sin_addr.s_addr);
    if (net_retval == 0) {
        ESP_LOGE(TAG, "ERROR inet_aton(): invalid Internet host address (%s)", str_ip_address);
        f_retval = ESP_FAIL;
        // GOTO (NOT cleanup_sock!)
        goto cleanup;
    }
    destAddr.sin_family = AF_INET;
    /* htons() converts the unsigned short integer u16_t X from host byte order to network byte order.
     *         No error returned. */
    destAddr.sin_port = htons(param_port);

    ESP_LOGD(TAG, "Creating socket");
    int sock = socket(addr_family, socket_style, ip_protocol);
    if (sock < 0) {
        ESP_LOGE(TAG, "Error socket(): errno %i (%s)", errno, strerror(errno));
        f_retval = ESP_FAIL;
        // GOTO
        goto cleanup_sock;
    }

    ESP_LOGD(TAG, "Sending message");
    net_retval = sendto(sock, param_buf, param_len, 0, (struct sockaddr * ) &destAddr, sizeof(destAddr));
    if (net_retval < 0) {
        ESP_LOGE(TAG, "Error sendto(): errno %i (%s)", errno, strerror(errno));
        f_retval = ESP_FAIL;
        // GOTO
        goto cleanup_sock;
    }

    /*
     * CLEANUP
     */
    cleanup_sock: ;

    if (sock != -1) {
        ESP_LOGD(TAG, "Shutting down socket...");
        net_retval = shutdown(sock, 0);
        if (net_retval < 0) {
            ESP_LOGE(TAG, "Error shutdown(): errno %i (%s)", errno, strerror(errno));
            f_retval = ESP_FAIL;
            // GOTO (NOT cleanup_sock!)
            goto cleanup;
        }
        ESP_LOGD(TAG, "Closing socket...");
        net_retval = close(sock);
        if (net_retval < 0) {
            ESP_LOGE(TAG, "Error close(): errno %i (%s)", errno, strerror(errno));
            f_retval = ESP_FAIL;
        }
    }

    cleanup: ;

    return f_retval;
}
