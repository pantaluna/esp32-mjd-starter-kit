/*
 * @doc static <global var>/<global func>: its scope is restricted to the file in which it is declared.
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

/**********
 * STRINGS
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
    ESP_LOGI(TAG, "*** %s %s", buffer, current_time_string);
}

void mjd_get_current_time_yyyymmddhhmmss(char *ptr_buffer) {
    //@dep buffer 14+1
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
        vTaskDelay(RTOS_DELAY_5SEC);
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

    ESP_LOGI(TAG, "  This is an ESP32 chip");
    ESP_LOGI(TAG, "  APB clock frequency (Hz):    %d", esp_clk_apb_freq());

    ESP_LOGI(TAG, "  CPU clock frequency (Hz):   %d", esp_clk_cpu_freq());
    ESP_LOGI(TAG, "  CPU cores:    %d", chip_info.cores);
    ESP_LOGI(TAG, "  Silicon rev.: %d", chip_info.revision);
    ESP_LOGI(TAG, "  Features:     %s%s%s", (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WIFI" : "",
            (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "", (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");
    ESP_LOGI(TAG, "  Flash:        %dMB %s", spi_flash_get_chip_size() / 1024 / 1024,
            (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
    ESP_LOGI(TAG, "  [ESP-IDF Version: %s]", esp_get_idf_version());
    ESP_LOGI(TAG, "");
}

void mjd_log_clanguage_details() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    ESP_LOGI(TAG, "  __FILE__:     %s", __FILE__);
    ESP_LOGI(TAG, "  __LINE__:     %d", __LINE__);
    ESP_LOGI(TAG, "  __FUNCTION__: %s", __FUNCTION__);
    ESP_LOGI(TAG, "  char");
    ESP_LOGI(TAG, "    Storage size for char: %d", sizeof(char));
    ESP_LOGI(TAG, "    Number of bits in a char: %d", CHAR_BIT);
    ESP_LOGI(TAG, "  int");
    ESP_LOGI(TAG, "    Storage size for int: %d", sizeof(int));
    ESP_LOGI(TAG, "    Minimum value: %i", INT_MIN);
    ESP_LOGI(TAG, "    Maximum value: %i", INT_MAX);
    ESP_LOGI(TAG, "  unsigned int");
    ESP_LOGI(TAG, "    Storage size for unsigned int: %d", sizeof(unsigned int));
    ESP_LOGI(TAG, "    Minimum value: %u (unsigned is always positive)", 0);
    ESP_LOGI(TAG, "    Maximum value: %u", UINT_MAX);
    ESP_LOGI(TAG, "  long");
    ESP_LOGI(TAG, "    Storage size for long: %d", sizeof(long));
    ESP_LOGI(TAG, "    Minimum value: %ld", LONG_MIN);
    ESP_LOGI(TAG, "    Maximum value: %ld", LONG_MAX);
    ESP_LOGI(TAG, "  float");
    ESP_LOGI(TAG, "    Storage size for float: %d", sizeof(float));
    ESP_LOGI(TAG, "    Minimum positive value: %E", FLT_MIN);
    ESP_LOGI(TAG, "    Maximum positive value: %E", FLT_MAX);
    ESP_LOGI(TAG, "    Precision value: %d", FLT_DIG);
    ESP_LOGI(TAG, "");
}

void mjd_log_memory_statistics() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    // @doc uxTaskGetStackHighWaterMark(NULL) returns the minimum free stack space there has been (in words, so on a 32 bit machine a value of 1 means 4 bytes) since the task started.
    //          Stack for each task is allocated from the main heap in RAM.
    //          The smaller the returned number the closer the task has come to overflowing its stack.
    // @doc esp_get_free_heap_size(void) returns available heap size, in bytes.
    ESP_LOGI(TAG, "  FreeRTOS free STACK space: %u bytes | ESP free HEAP space: %u bytes", uxTaskGetStackHighWaterMark(NULL) * 4,
            esp_get_free_heap_size());
}

/**********
 * ESP32: BOOT INFO, DEEP SLEEP and WAKE UP
 */
static RTC_DATA_ATTR int mcu_boot_count = 0; //@important Allocated in RTC Fast Memory (=persistent after a deep sleep restart)

uint32_t mjd_increment_mcu_boot_count() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    return ++mcu_boot_count;
}

void mjd_log_mcu_boot_count() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    ESP_LOGI(TAG, "*** boot count: %i", mcu_boot_count);
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
    char wakeup_reason[20];

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
        strcpy(wakeup_reason, "UNKNOWN");
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
        vTaskDelay(RTOS_DELAY_500MILLISEC);
        mjd_led_off(gpio_nr);
        vTaskDelay(RTOS_DELAY_500MILLISEC);
    }
}

void mjd_led_mark_error(int gpio_nr) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    if (led_config_list[gpio_nr].is_initialized != 1) {
        ESP_LOGE(TAG, "  ABORT. mjd_led_config() was not called beforehand");
        return;
    }

    int i = 0;
    while (++i <= 15) {
        mjd_led_on(gpio_nr);
        vTaskDelay(RTOS_DELAY_50MILLISEC);
        mjd_led_off(gpio_nr);
        vTaskDelay(RTOS_DELAY_50MILLISEC);
    }
}
