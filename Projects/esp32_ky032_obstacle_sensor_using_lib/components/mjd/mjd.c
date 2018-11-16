/*
 *
 */

// Component header file
#include "mjd.h"

/**********
 * LOGGING
 */
static const char TAG[] = "mjd";

/**********
 * C Language: utilities, stdlib, etc.
 */

/**********
 * BINARY SEARCH / QUICK SORT
 * @doc http://www.cplusplus.com/reference/cstdlib/bsearch
 * @doc http://www.cplusplus.com/reference/cstdlib/qsort/
 *
 * @example pItem = (int*) bsearch (&gpio_num, lolin32lite_fatal_gpios, ARRAY_SIZE(lolin32lite_fatal_gpios), sizeof (int), mjd_compare_ints);
 * @example bsearch integer
 *      int * pItem;
 *      int values[] = { 50, 20, 60, 40, 10, 30 };
 *      pItem = (int*) bsearch (&key, values, ARRAY_SIZE(values), sizeof (int), compareints);
 *      if (pItem!=NULL) { printf ("%d is in the array.\n",*pItem) }
 *      else { printf ("%d is not in the array.\n",key) }
 *
 * @example For C strings, strcmp() can be used directly as the compare argument for bsearch
 *      pItem = (char*) bsearch (key, strvalues, 4, 20, (int(*)(const void*,const void*)) strcmp);
 *      if (pItem!=NULL) printf ("%s is in the array.\n",pItem);
 *      else printf ("%s is not in the array.\n",key);
 */
int mjd_compare_ints(const void * a, const void * b) {
    return (*(int*) a - *(int*) b);
}

/**********
 * BYTES and BINARY REPRESENTATION
 */

/*
 * Convert uint8_t to binary coded decimal
 */
uint8_t mjd_byte_to_bcd(uint8_t val) {
    return ((val / 10 * 16) + (val % 10));
}

/*
 * Convert binary coded decimal to uint8_t
 */
uint8_t mjd_bcd_to_byte(uint8_t val) {
    return ((val / 16 * 10) + (val % 16));
}

esp_err_t mjd_byte_to_binary_string(uint8_t input_byte, char * output_string) {
    // @doc http://c-faq.com/strangeprob/strlitnomod.html
    //          String constants are in fact constant. The compiler may place them in non-writable storage, and it is therefore not safe to modify them.
    //          When you need writable strings, you must allocate writable memory for them, either by declaring an array, or by calling malloc.

    if (strlen(output_string) < 8) {
        ESP_LOGE(TAG, "%s() ABORT. strlen output_string < 8", __FUNCTION__);
        return ESP_FAIL; // EXIT
    };

    strcpy(output_string, "--------");
    output_string[0] = (char) (input_byte & 0b10000000 ? '1' : '0');
    output_string[1] = (char) (input_byte & 0b01000000 ? '1' : '0');
    output_string[2] = (char) (input_byte & 0b00100000 ? '1' : '0');
    output_string[3] = (char) (input_byte & 0b00010000 ? '1' : '0');
    output_string[4] = (char) (input_byte & 0b00001000 ? '1' : '0');
    output_string[5] = (char) (input_byte & 0b00000100 ? '1' : '0');
    output_string[6] = (char) (input_byte & 0b00000010 ? '1' : '0');
    output_string[7] = (char) (input_byte & 0b00000001 ? '1' : '0');

    return ESP_OK;
}

esp_err_t mjd_word_to_binary_string(uint16_t input_word, char * output_string) {
    // @doc http://c-faq.com/strangeprob/strlitnomod.html
    //          String constants are in fact constant. The compiler may place them in non-writable storage, and it is therefore not safe to modify them.
    //          When you need writable strings, you must allocate writable memory for them, either by declaring an array, or by calling malloc.

    if (strlen(output_string) < 16) {
        ESP_LOGE(TAG, "%s() ABORT. strlen output_string < 16", __FUNCTION__);
        return ESP_FAIL; // EXIT
    };

    strcpy(output_string, "----------------");
    output_string[0] = (char) (input_word & 0b1000000000000000 ? '1' : '0');
    output_string[1] = (char) (input_word & 0b0100000000000000 ? '1' : '0');
    output_string[2] = (char) (input_word & 0b0010000000000000 ? '1' : '0');
    output_string[3] = (char) (input_word & 0b0001000000000000 ? '1' : '0');
    output_string[4] = (char) (input_word & 0b0000100000000000 ? '1' : '0');
    output_string[5] = (char) (input_word & 0b0000010000000000 ? '1' : '0');
    output_string[6] = (char) (input_word & 0b0000001000000000 ? '1' : '0');
    output_string[7] = (char) (input_word & 0b0000000100000000 ? '1' : '0');
    output_string[8] = (char) (input_word & 0b0000000010000000 ? '1' : '0');
    output_string[9] = (char) (input_word & 0b0000000001000000 ? '1' : '0');
    output_string[10] = (char) (input_word & 0b0000000000100000 ? '1' : '0');
    output_string[11] = (char) (input_word & 0b0000000000010000 ? '1' : '0');
    output_string[12] = (char) (input_word & 0b0000000000001000 ? '1' : '0');
    output_string[13] = (char) (input_word & 0b0000000000000100 ? '1' : '0');
    output_string[14] = (char) (input_word & 0b0000000000000010 ? '1' : '0');
    output_string[15] = (char) (input_word & 0b0000000000000001 ? '1' : '0');

    return ESP_OK;
}

/**********
 * STRING BASICS
 */
bool mjd_string_starts_with(const char *str, const char *prefix) {
    if (str == NULL || prefix == NULL) {
        return false;
    }
    size_t len_str = strlen(str);
    size_t len_prefix = strlen(prefix);
    if (len_prefix > len_str) {
        return false;
    }
    return strncmp(prefix, str, len_prefix) == 0;
}

bool mjd_string_ends_with(const char *str, const char *suffix) {
    if (str == NULL || suffix == NULL) {
        return false;
    }
    size_t len_str = strlen(str);
    size_t len_suffix = strlen(suffix);
    if (len_suffix > len_str) {
        return false;
    }
    return strncmp(str + len_str - len_suffix, suffix, len_suffix) == 0;
}

char * mjd_string_repeat(const char * s, int n) {
    size_t slen = strlen(s);
    char * dest = malloc(n * slen + 1);

    int i;
    char * p;
    for (i = 0, p = dest; i < n; ++i, p += slen) {
        memcpy(p, s, slen);
    }
    *p = '\0';
    return dest;
}

void mjd_string_prepend(char* param_ptr_string, const char* param_ptr_part) {
    // @important The original string must be allocated big enough to contain the original string + the part to be prepended.
    // https://stackoverflow.com/questions/2328182/prepending-to-a-string
    size_t len_part = strlen(param_ptr_part);
    size_t i;

    memmove(param_ptr_string + len_part, param_ptr_string, strlen(param_ptr_string) + 1);

    for (i = 0; i < len_part; ++i)
    {
        param_ptr_string[i] = param_ptr_part[i];
    }
}

/**********
 * HEX STRINGS
 */

esp_err_t mjd_uint8s_to_hexstring(const uint8_t * param_ptr_input, size_t param_len_input, char * param_ptr_output) {
    strcpy(param_ptr_output, "");

    uint32_t idx_src;
    uint32_t idx_dst;
    for (idx_src = 0, idx_dst = 0; idx_src < param_len_input; idx_src++, idx_dst += 2) {
        sprintf((char*) param_ptr_output + idx_dst, "%02X", param_ptr_input[idx_src]);
    }
    param_ptr_output[idx_dst] = '\0';

    ESP_LOGV(TAG, "mjd_uint8s_to_hexstring() param_ptr_input len=%u (HEXDUMP)", param_len_input);
    ESP_LOG_BUFFER_HEXDUMP(TAG, param_ptr_input, param_len_input, ESP_LOG_VERBOSE);
    ESP_LOGV(TAG, "mjd_uint8s_to_hexstring() param_ptr_output len=%u (HEXDUMP)", strlen(param_ptr_output));
    ESP_LOG_BUFFER_HEXDUMP(TAG, param_ptr_output, strlen(param_ptr_output) + 1, ESP_LOG_VERBOSE);  // +1 to see the \0

    return ESP_OK;
}

esp_err_t mjd_hexstring_to_uint8s(const char * param_ptr_input, size_t param_len_input, uint8_t * param_ptr_output) {
    esp_err_t f_retval = ESP_OK;

    if (param_len_input % 2 != 0) {
        ESP_LOGE(TAG, "%s(). ABORT. param_ptr_input has an uneven number of characters", __FUNCTION__);
        // GOTO
        goto cleanup;
    }

    char hex_chars[3];
    uint32_t idx_src, idx_dst;
    for (idx_src = 0, idx_dst = 0; idx_src < param_len_input; idx_src += 2, idx_dst++) {
        hex_chars[0] = *(param_ptr_input + idx_src);
        hex_chars[1] = *(param_ptr_input + idx_src + 1);
        hex_chars[2] = '\0';

        param_ptr_output[idx_dst] = (uint8_t) strtoul(hex_chars, NULL, 16);
    }

    ESP_LOGV(TAG, "mjd_hexstring_to_uint8s() param_ptr_input len=%u (HEXDUMP)", param_len_input);
    ESP_LOG_BUFFER_HEXDUMP(TAG, param_ptr_input, param_len_input + 1, ESP_LOG_VERBOSE);  // +1 to see the \0
    ESP_LOGV(TAG, "mjd_hexstring_to_uint8s() param_ptr_output len=%u (HEXDUMP)", param_len_input/2);
    ESP_LOG_BUFFER_HEXDUMP(TAG, param_ptr_output, param_len_input/2, ESP_LOG_VERBOSE);

    // LABEL
    cleanup: ;

    return f_retval;
}

esp_err_t mjd_string_to_hexstring(const char * param_ptr_input, size_t param_len_input, char * param_ptr_output) {

    return mjd_uint8s_to_hexstring((uint8_t *) param_ptr_input, param_len_input, param_ptr_output);
}

esp_err_t mjd_hexstring_to_string(const char * param_ptr_input, size_t param_len_input, char * param_ptr_output) {

    return mjd_hexstring_to_uint8s(param_ptr_input, param_len_input, (uint8_t *) param_ptr_output);
}

/**********
 * DATE TIME
 */
uint32_t mjd_seconds_to_milliseconds(uint32_t seconds) {
    return seconds * 1000;
}

uint32_t mjd_seconds_to_microseconds(uint32_t seconds) {
    return seconds * 1000 * 1000;
}

void mjd_log_time() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    time_t current_time;
    struct tm *ptr_local_time;
    char buffer[128];
    char *current_time_string;

    time(&current_time);
    ptr_local_time = localtime(&current_time);
    strftime(buffer, sizeof(buffer), "%Y%m%d%H%M%S", ptr_local_time);

    current_time_string = ctime(&current_time);
    if (current_time_string == NULL) {
        ESP_LOGE(TAG, "Error converting the current time using ctime().");
    }
    if (current_time_string[strlen(current_time_string)-1] == '\n') {
        current_time_string[strlen(current_time_string)-1] = '\0';
    }
    ESP_LOGI(TAG, "*** %s %s", buffer, current_time_string);
}

void mjd_get_current_time_yyyymmddhhmmss(char *ptr_buffer) {
    // @dep buffer 14+1
    // @example "*** 19700101000000 Thu Jan  1 00:00:00 1970\n"
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    time_t current_time;
    struct tm *ptr_local_time;

    time(&current_time);
    ptr_local_time = localtime(&current_time);
    strftime(ptr_buffer, 14 + 1, "%Y%m%d%H%M%S", ptr_local_time);
}

void mjd_set_timezone_utc() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    // @special "" means timezone UTC (@bug Specifying "UTC" does not work!)
    setenv("TZ", "", 1);
    tzset();
}

void mjd_set_timezone_amsterdam() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    // @doc https://remotemonitoringsystems.ca/time-zone-abbreviations.php
    // @doc timezone UTC = UTC
    setenv("TZ", "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00", 1);
    tzset();
}

/**********
 * RTOS
 */
void mjd_rtos_wait_forever() {
    ESP_LOGW(TAG, "%s()", __FUNCTION__);

    while (1) {
        vTaskDelay(RTOS_DELAY_15SEC); // @important Give all CPU time to other RTOS Tasks
    }
}

/**********
 * ESP32 SYSTEM
 */
/**********
 * ESP32 SYSTEM
 */
void mjd_log_chip_info() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);

    ESP_LOGI(TAG, "This is an ESP32 chip");
    ESP_LOGI(TAG, "  CPU cores:    %u", chip_info.cores);
    ESP_LOGI(TAG, "  Silicon rev.: %u", chip_info.revision);
    ESP_LOGI(TAG, "  CPU clock frequency (Hz):   %d", esp_clk_cpu_freq());
    ESP_LOGI(TAG, "  APB Advanced Peripheral Bus clock frequency (Hz):  %d", esp_clk_apb_freq());
    ESP_LOGI(TAG, "  Features:     %s%s%s", (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WIFI" : "",
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "", (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");
    ESP_LOGI(TAG, "  Flash:        %dMB %s", spi_flash_get_chip_size() / 1024 / 1024,
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
    ESP_LOGI(TAG, "  [ESP-IDF Version: %s]", esp_get_idf_version());
    ESP_LOGI(TAG, "");
}

void mjd_log_clanguage_details() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    ESP_LOGI(TAG, "C Lang details:");
    ESP_LOGI(TAG, "  __FILE__:     %s", __FILE__);
    ESP_LOGI(TAG, "  __LINE__:     %d", __LINE__);
    ESP_LOGI(TAG, "  __FUNCTION__: %s", __FUNCTION__);

    ESP_LOGI(TAG, "  char");
    ESP_LOGI(TAG, "    Storage size (bytes): %d", sizeof(char));
    ESP_LOGI(TAG, "    Number of bits in a char: %d", CHAR_BIT);

    // @doc printf integer datatype: uint8_t (%hhu) - uint16_t (%hu) - uint32_t (%u)

    ESP_LOGI(TAG, "  uint8_t unsigned");
    ESP_LOGI(TAG, "    Storage size (bytes): %u", sizeof(uint8_t));

    ESP_LOGI(TAG, "  uint32_t unsigned");
    ESP_LOGI(TAG, "    Storage size (bytes): %u", sizeof(uint32_t));
    ESP_LOGI(TAG, "    Minimum value: 0 (unsigned = always positive)");
    ESP_LOGI(TAG, "    UINT_MAX : %u", UINT_MAX);

    ESP_LOGI(TAG, "  int32_t");
    ESP_LOGI(TAG, "    Storage size (bytes) : %u", sizeof(int32_t));
    ESP_LOGI(TAG, "    INT_MIN: %i", INT_MIN);
    ESP_LOGI(TAG, "    INT_MAX: %i", INT_MAX);

    ESP_LOGI(TAG, "  uint64_t unsigned: bounds, how to print");
    ESP_LOGI(TAG, "    Storage size (bytes) : %u", sizeof(uint64_t));
    uint64_t u64_min = 0ULL;
    uint64_t u64_max = ULONG_LONG_MAX;
    ESP_LOGI(TAG, "    u64_min [EXPECT 0]:                    %" PRIu64 " (0x%" PRIX64 ")", u64_min, u64_min);
    ESP_LOGI(TAG, "    u64_max [EXPECT 18446744073709551615]: %" PRIu64 " (0x%" PRIX64 ")", u64_max, u64_max);

    ESP_LOGI(TAG, "  int64_t: bounds, how to print");
    ESP_LOGI(TAG, "    Storage size (bytes) : %u", sizeof(int64_t));
    int64_t s64_min = (-1 * __LONG_LONG_MAX__) - 1;
    int64_t s64_max = __LONG_LONG_MAX__; // 0x7FFFFFFFFFFFFFFFLL
    ESP_LOGI(TAG, "     s64_min [EXPECT  -9223372036854775808]: %" PRIi64 " (0x%" PRIX64 ")", s64_min, s64_min);
    ESP_LOGI(TAG, "     s64_max [EXPECT   9223372036854775807]: %" PRIi64 " (0x%" PRIX64 ")", s64_max, s64_max);

    ESP_LOGI(TAG, "  float");
    ESP_LOGI(TAG, "    Storage size (bytes): %d", sizeof(float));
    ESP_LOGI(TAG, "    FLT_MIN positive value: %E", FLT_MIN);
    ESP_LOGI(TAG, "    FLT_MAX maximum positive value: %E", FLT_MAX);
    ESP_LOGI(TAG, "    FLT_DIG precision value: %d", FLT_DIG);

    ESP_LOGI(TAG, "  size_t");
    size_t my_size = 999;
    ESP_LOGI(TAG, "    Storage size (bytes): %u", sizeof(my_size));
    ESP_LOGI(TAG, "    Example value (using fmt percent+zu): %zu", my_size);
    ESP_LOGI(TAG, "");

}

/**********
 * ESP32 SYSTEM & RTOS: Memory
 */
esp_err_t mjd_get_memory_statistics(mjd_meminfo_t* data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    data->free_esp_heap = esp_get_free_heap_size();
    data->free_rtos_stack = uxTaskGetStackHighWaterMark(NULL) * 4;

    return ESP_OK;
}

esp_err_t mjd_log_memory_statistics() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    mjd_meminfo_t meminfo;
    mjd_get_memory_statistics(&meminfo);
    ESP_LOGI(TAG, "ESP free HEAP space: %u bytes | FreeRTOS free STACK space (calling task): %u bytes", meminfo.free_esp_heap,
            meminfo.free_rtos_stack);

    return ESP_OK;
}

/**********
 * ESP32: BOOT INFO, DEEP SLEEP and WAKE UP
 */
static RTC_DATA_ATTR uint32_t mcu_boot_count = 0; //@important Allocated in RTC Fast Memory (= a persistent data area after a deep sleep restart)

uint32_t mjd_increment_mcu_boot_count() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    return ++mcu_boot_count;
}

void mjd_log_mcu_boot_count() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    ESP_LOGI(TAG, "*** boot count: %u", mcu_boot_count);
}

uint32_t mjd_get_mcu_boot_count() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    return mcu_boot_count;
}

void mjd_log_wakeup_details() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    /*
     * verify the wakeup reason
     * esp_sleep.h
         ESP_SLEEP_WAKEUP_UNDEFINED,    //! In case of deep sleep, reset was not caused by exit from deep sleep
         ESP_SLEEP_WAKEUP_EXT0,         //! Wakeup caused by external signal using RTC_IO
         ESP_SLEEP_WAKEUP_EXT1,         //! Wakeup caused by external signal using RTC_CNTL
         ESP_SLEEP_WAKEUP_TIMER,        //! Wakeup caused by timer
         ESP_SLEEP_WAKEUP_TOUCHPAD,     //! Wakeup caused by touchpad
         ESP_SLEEP_WAKEUP_ULP,          //! Wakeup caused by ULP program
     */
    char wakeup_reason[128];

    switch (esp_sleep_get_wakeup_cause()) {
    case ESP_SLEEP_WAKEUP_EXT0:
        strcpy(wakeup_reason, "EXT0 RTC_IO");
        break;

    case ESP_SLEEP_WAKEUP_EXT1:
        strcpy(wakeup_reason, "EXT1 RTC_CNTL");
        break;

    case ESP_SLEEP_WAKEUP_TIMER:
        strcpy(wakeup_reason, "TIMER");
        break;

    case ESP_SLEEP_WAKEUP_TOUCHPAD:
        strcpy(wakeup_reason, "TOUCHPAD");
        break;

    case ESP_SLEEP_WAKEUP_ULP:
        strcpy(wakeup_reason, "ULP");
        break;

    default:
        strcpy(wakeup_reason, "UNKNOWN (not after a deep sleep period, probably after a normal reset)");
        break;
    }

    ESP_LOGI(TAG, "*** Wakeup reason: %s", wakeup_reason);
}

/**********
 * ESP32: LED
 */
static mjd_led_config_t led_config_list[GPIO_PIN_COUNT]; // @dep gpio.h

void mjd_led_config(const mjd_led_config_t *led_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    gpio_config_t io_conf;
    io_conf.pin_bit_mask = (1ULL << led_config->gpio_num);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);

    led_config_list[led_config->gpio_num] = *led_config;
    led_config_list[led_config->gpio_num].is_initialized = 1; // Mark as in use.
}

void mjd_led_on(int gpio_nr) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    if (led_config_list[gpio_nr].is_initialized != 1) {
        ESP_LOGE(TAG, "  ABORT. mjd_led_config() was not called beforehand");
        return;
    }

    int level = 1;
    if (led_config_list[gpio_nr].wiring_type == LED_WIRING_TYPE_DIODE_FROM_VCC) {
        level = 0;
    }
    ESP_ERROR_CHECK(gpio_set_level(gpio_nr, level));
}

void mjd_led_off(int gpio_nr) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    if (led_config_list[gpio_nr].is_initialized != 1) {
        ESP_LOGE(TAG, "  ABORT. mjd_led_config() was not called beforehand");
        return;
    }

    int level = 0;
    if (led_config_list[gpio_nr].wiring_type == LED_WIRING_TYPE_DIODE_FROM_VCC) {
        level = 1;
    }
    ESP_ERROR_CHECK(gpio_set_level(gpio_nr, level));
}

void mjd_led_blink_times(int gpio_nr, int times) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    if (led_config_list[gpio_nr].is_initialized != 1) {
        ESP_LOGE(TAG, "  ABORT. mjd_led_config() was not called beforehand");
        return;
    }

    int i = 0;
    while (++i <= times) {
        mjd_led_on(gpio_nr);
        vTaskDelay(RTOS_DELAY_250MILLISEC);
        mjd_led_off(gpio_nr);
        vTaskDelay(RTOS_DELAY_250MILLISEC);
    }
}

void mjd_led_mark_error(int gpio_nr) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    if (led_config_list[gpio_nr].is_initialized != 1) {
        ESP_LOGE(TAG, "  ABORT. mjd_led_config() was not called beforehand");
        return;
    }

    int i = 0;
    while (++i <= 5) {
        mjd_led_on(gpio_nr);
        vTaskDelay(RTOS_DELAY_50MILLISEC);
        mjd_led_off(gpio_nr);
        vTaskDelay(RTOS_DELAY_50MILLISEC);
    }
}
