/*
 * HARDWARE SETUP the MJD components:
 *  *NONE
 *
 */

/*
 * Includes: system, own
 */
#include "mjd.h"
#include "mjd_list.h"
#include "mjd_mqtt.h"
#include "mjd_net.h"
#include "mjd_wifi.h"

/*
 * Logging
 */
static const char TAG[] = "myapp";

/*
 * TODO DEVICE INFO
 */

/*
 * KConfig: LED, WIFI
 */
static const int MY_LED_ON_DEVBOARD_GPIO_NUM = CONFIG_MY_LED_ON_DEVBOARD_GPIO_NUM;
static const int MY_LED_ON_DEVBOARD_WIRING_TYPE = CONFIG_MY_LED_ON_DEVBOARD_WIRING_TYPE;

static const char *MY_WIFI_SSID = CONFIG_MY_WIFI_SSID;
/////static const char *MY_WIFI_SSID = "xxx"; // @test invalid

static const char *MY_WIFI_PASSWORD = CONFIG_MY_WIFI_PASSWORD;
/////static const char *MY_WIFI_PASSWORD = "yyy"; // @test invalid

/*
 * FreeRTOS settings
 */
#define MYAPP_RTOS_TASK_STACK_SIZE_16K (16 * 1024)
#define MYAPP_RTOS_TASK_PRIORITY_NORMAL (RTOS_TASK_PRIORITY_NORMAL)

/*
 * Config: MQTT
 */

#define MY_MQTT_HOST "broker.shiftr.io"
#define MY_MQTT_PORT "1883"
#define MY_MQTT_USER "try"
#define MY_MQTT_PASS "try"
/*
 #define MY_MQTT_HOST "192.168.0.95" // @important "s3 hostname" does not work on an MCU@LAN because it returns the ISP's WAN IP and this IP is not whitelisted in Ubuntu UFW!
 #define MY_MQTT_PORT "xxx"
 #define MY_MQTT_USER "xxx"
 #define MY_MQTT_PASS "xxx"
 */
#define MY_MQTT_BUFFER_SIZE  (4096)  // @suggested 256 @used 4096 [must be >= longest topic/payload that you will send]
#define MY_MQTT_TIMEOUT      (5000)  // @suggested 2000 @used Feb2018: 2000 @used Jun2018: 5000 (less timeouts now)

/*
 * TASK
 */
void main_task(void *pvParameter) {
    ESP_LOGI(TAG, "%s()", __FUNCTION__);

    mjd_log_memory_statistics();

    /********************************************************************************
     * Reuseable variables
     *
     */
    int i;
    int total;
    esp_err_t f_retval;

    /********************************************************************************
     * SOC init
     *
     */
    mjd_log_memory_statistics();

// DEVTEMP_BEGIN Fix after ESP-IDF github tag v3.2-dev
#ifndef ESP_ERR_NVS_NEW_VERSION_FOUND
#define ESP_ERR_NVS_NEW_VERSION_FOUND (ESP_ERR_NVS_BASE + 0xff)
#endif
// DEVTEMP-END

    ESP_LOGI(TAG, "@doc exec nvs_flash_init() - mandatory for Wifi to work later on");
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_LOGW(TAG, "  ESP_ERR_NVS_NO_FREE_PAGES - do nvs_flash_erase()");
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    } else if (err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "  ESP_ERR_NVS_NEW_VERSION_FOUND - do nvs_flash_erase()");
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    /********************************************************************************
     * MY STANDARD Init
     *
     */
    mjd_log_wakeup_details();
    mjd_increment_mcu_boot_count();
    mjd_log_mcu_boot_count();
    mjd_log_chip_info();
    mjd_log_clanguage_details();
    mjd_log_memory_statistics();
    mjd_set_timezone_utc();
    mjd_log_time();
    ESP_LOGI(TAG,
            "@tip You can also change the log level to DEBUG for more detailed logging and to get insights in what the component is actually doing.");
    ESP_LOGI(TAG, "@doc Wait 2 seconds after power-on (start logic analyzer, let peripherals become active, ...)");
    vTaskDelay(RTOS_DELAY_2SEC);

    /********************************************************************************
     * C Language: printf formats, utilities, stdlib, etc.
     *
     */
    ESP_LOGI(TAG, "\n\n***SECTION: C Language: printf formats, utilities, stdlib, etc.***");

    mjd_log_memory_statistics();

    /*
     * printf formats
     */
    ESP_LOGI(TAG, "\nprintf size_t values using format specifier %%zu:");
    uint8_t buf123[123];
    ESP_LOGI(TAG, "  buf123[123] : sizeof(buf123)=%zu", sizeof(buf123));

    /*
     * BYTES and BINARY REPRESENTATION
     */
    ESP_LOGI(TAG, "\nmjd_byte_to_binary_string():");

    uint8_t input_value_uint8;
    char output_string8[8 + 1] = "12345678";
    char small_output_string[2 + 1] = "12";

    input_value_uint8 = 0;
    if (mjd_byte_to_binary_string(input_value_uint8, output_string8) == ESP_OK) {
        ESP_LOGI(TAG, "OK for value: %3u - bin string 0b%s", input_value_uint8, output_string8);
    } else {
        ESP_LOGE(TAG, "ERROR for value: %3u - bin string 0b%s\n", input_value_uint8, output_string8);
    }

    input_value_uint8 = 1;
    if (mjd_byte_to_binary_string(input_value_uint8, output_string8) == ESP_OK) {
        ESP_LOGI(TAG, "OK for value: %3u - bin string 0b%s", input_value_uint8, output_string8);
    } else {
        ESP_LOGE(TAG, "ERROR for value: %3u - bin string 0b%s\n", input_value_uint8, output_string8);
    }

    input_value_uint8 = 254;
    if (mjd_byte_to_binary_string(input_value_uint8, output_string8) == ESP_OK) {
        ESP_LOGI(TAG, "OK for value: %3u - bin string 0b%s", input_value_uint8, output_string8);
    } else {
        ESP_LOGE(TAG, "ERROR for value: %3u - bin string 0b%s\n", input_value_uint8, output_string8);
    }

    input_value_uint8 = 255;
    if (mjd_byte_to_binary_string(input_value_uint8, output_string8) == ESP_OK) {
        ESP_LOGI(TAG, "OK for value: %3u - bin string 0b%s", input_value_uint8, output_string8);
    } else {
        ESP_LOGE(TAG, "ERROR for value: %3u - bin string 0b%s\n", input_value_uint8, output_string8);
    }

    // small_output_string (simulate an error)
    input_value_uint8 = 0;
    if (mjd_byte_to_binary_string(input_value_uint8, small_output_string) == ESP_OK) {
        ESP_LOGI(TAG, "OK for value: %3u - bin string 0b%s", input_value_uint8, small_output_string);
    } else {
        ESP_LOGE(TAG, "***EXPECTED ERROR*** (target string too small, left untouched) value: %3u - bin string 0b%s\n", input_value_uint8,
                small_output_string);
    }

    ESP_LOGI(TAG, "\nmjd_word_to_binary_string():");
    uint16_t input_value_uint16;
    char output_string16[16 + 1] = "1234567890123456";

    input_value_uint16 = 0;
    if (mjd_word_to_binary_string(input_value_uint16, output_string16) == ESP_OK) {
        ESP_LOGI(TAG, "OK for value: %5u - bin string 0b%s", input_value_uint16, output_string16);
    } else {
        ESP_LOGE(TAG, "ERROR for value: %5u - bin string 0b%s\n", input_value_uint16, output_string16);
    }

    input_value_uint16 = 1;
    if (mjd_word_to_binary_string(input_value_uint16, output_string16) == ESP_OK) {
        ESP_LOGI(TAG, "OK for value: %5u - bin string 0b%s", input_value_uint16, output_string16);
    } else {
        ESP_LOGE(TAG, "ERROR for value: %5u - bin string 0b%s\n", input_value_uint16, output_string16);
    }

    input_value_uint16 = 255;
    if (mjd_word_to_binary_string(input_value_uint16, output_string16) == ESP_OK) {
        ESP_LOGI(TAG, "OK for value: %5u - bin string 0b%s", input_value_uint16, output_string16);
    } else {
        ESP_LOGE(TAG, "ERROR for value: %5u - bin string 0b%s\n", input_value_uint16, output_string16);
    }

    input_value_uint16 = 256;
    if (mjd_word_to_binary_string(input_value_uint16, output_string16) == ESP_OK) {
        ESP_LOGI(TAG, "OK for value: %5u - bin string 0b%s", input_value_uint16, output_string16);
    } else {
        ESP_LOGE(TAG, "ERROR for value: %5u - bin string 0b%s\n", input_value_uint16, output_string16);
    }

    input_value_uint16 = 65534;
    if (mjd_word_to_binary_string(input_value_uint16, output_string16) == ESP_OK) {
        ESP_LOGI(TAG, "OK for value: %5u - bin string 0b%s", input_value_uint16, output_string16);
    } else {
        ESP_LOGE(TAG, "ERROR for value: %5u - bin string 0b%s\n", input_value_uint16, output_string16);
    }

    input_value_uint16 = 65535;
    if (mjd_word_to_binary_string(input_value_uint16, output_string16) == ESP_OK) {
        ESP_LOGI(TAG, "OK for value: %5u - bin string 0b%s", input_value_uint16, output_string16);
    } else {
        ESP_LOGE(TAG, "ERROR for value: %5u - bin string 0b%s\n", input_value_uint16, output_string16);
    }

    /**********
     * STRINGS
     */
    ESP_LOGI(TAG, "STRINGS");

    ESP_LOGI(TAG, "mjd_string_prepend():");
    char final_string[] = "123456789   ";
    char add_to_string[] = "ABC";
    ESP_LOGI(TAG, "  BEFORE");
    ESP_LOGI(TAG, "    final_string:  %s", final_string);
    ESP_LOGI(TAG, "    add_to_string: %s", add_to_string);
    mjd_string_prepend(final_string, add_to_string);
    ESP_LOGI(TAG, "  AFTER");
    ESP_LOGI(TAG, "    final_string:  %s", final_string);
    ESP_LOGI(TAG, "    add_to_string: %s", add_to_string);

    /*
     * HEX strings
     */
    mjd_log_memory_statistics();
    char hex_string[320];

    ESP_LOGI(TAG, "mjd_uint8s_to_hexstring():");
    uint8_t input_uint8s[] =
        { 0x00, 0x01, 0x0E, 0x0F, 0xF0, 0xF1, 0xFE, 0xFF };
    strcpy(hex_string, "");
    if (mjd_uint8s_to_hexstring(input_uint8s, ARRAY_SIZE(input_uint8s), hex_string) == ESP_OK) {
        ESP_LOGI(TAG, "    => input_uint8s[%u]:", ARRAY_SIZE(input_uint8s));
        for (uint32_t i = 0; i < ARRAY_SIZE(input_uint8s); ++i) {
            ESP_LOGI(TAG, "        idx %5i: %3u (0x%02X)", i, input_uint8s[i], input_uint8s[i]);
        }
        ESP_LOGI(TAG, "    <= hex_string: %s", hex_string);
    } else {
        ESP_LOGE(TAG, "mjd_uint8s_to_hexstring() ERROR");
    }

    ESP_LOGI(TAG, "mjd_hexstring_to_uint8s():");
    uint8_t output_uint8s[] =
        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    if (mjd_hexstring_to_uint8s(hex_string, strlen(hex_string), output_uint8s) == ESP_OK) {
        ESP_LOGI(TAG, "    => hex_string: %s", hex_string);
        ESP_LOGI(TAG, "    => output_uint8s[%u]:", strlen(hex_string) / 2);
        for (uint32_t i = 0; i < strlen(hex_string) / 2; ++i) {
            ESP_LOGI(TAG, "        idx %5i: %3u (0x%02X)", i, output_uint8s[i], output_uint8s[i]);
        }
    } else {
        ESP_LOGE(TAG, "mjd_uint8s_to_hexstring() ERROR");
    }

    ESP_LOGI(TAG, "mjd_string_to_hexstring()");

    char ascii_string[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    strcpy(hex_string, "");
    if (mjd_string_to_hexstring(ascii_string, strlen(ascii_string), hex_string) == ESP_OK) {
        ESP_LOGI(TAG, "String to HEX String:");
        ESP_LOGI(TAG, "    => ascii string %s", ascii_string);
        ESP_LOGI(TAG, "    <= hex string   %s", hex_string);
    } else {
        ESP_LOGE(TAG, "mjd_string_to_hexstring() ERROR for string: %s", ascii_string);
    }

    ESP_LOGI(TAG, "mjd_hexstring_to_string():");
    strcpy(ascii_string, "");
    if (mjd_hexstring_to_string(hex_string, strlen(hex_string), ascii_string) == ESP_OK) {
        ESP_LOGI(TAG, "HEX String to string:");
        ESP_LOGI(TAG, "    => hex string   %s", hex_string);
        ESP_LOGI(TAG, "    <= ascii string %s =>", ascii_string);
    } else {
        ESP_LOGE(TAG, "mjd_hexstring_to_string() ERROR for string: %s", ascii_string);
    }

    mjd_log_memory_statistics();

    // DEVTEMP: HALT
    /////mjd_rtos_wait_forever();

    /********************************************************************************
     * C Language: Linux Kernel linked list tool belt
     *
     */
    ESP_LOGI(TAG, "\n\n***SECTION: C Language: Linux Kernel linked list tool belt***");

    mjd_log_memory_statistics();

    static MJD_LIST_HEAD(fox_list);

    typedef struct {
        uint32_t code;
        float weight_kg;
        bool is_loyal;
        struct mjd_list_head list;
    } fox_t;

    fox_t *ptr_one_fox, *ptr_next_fox;
    uint32_t mycounter = 0; // mjd_list_count()

    ESP_LOGI(TAG, "mjd_list_empty() 0=no 1=yes");
    ESP_LOGI(TAG, "   %u", mjd_list_empty(&fox_list));

    ESP_LOGI(TAG, "ADD TAIL code 10");
    fox_t *ptr_first_fox;
    ptr_first_fox = malloc(sizeof(*ptr_first_fox));
    ptr_first_fox->code = 10;
    ptr_first_fox->weight_kg = 10.10;
    ptr_first_fox->is_loyal = false;
    ESP_LOGI(TAG, "  sizeof(*ptr_one_fox): %u bytes", sizeof(*ptr_one_fox));
    mjd_list_add_tail(&ptr_first_fox->list, &fox_list);

    ESP_LOGI(TAG, "ADD TAIL code 20");
    fox_t *ptr_second_fox;
    ptr_second_fox = malloc(sizeof(*ptr_second_fox));
    ptr_second_fox->code = 20;
    ptr_second_fox->weight_kg = 20.20;
    ptr_second_fox->is_loyal = false;
    mjd_list_add_tail(&ptr_second_fox->list, &fox_list);

    ESP_LOGI(TAG, "ADD TAIL code 30");
    fox_t *ptr_third_fox;
    ptr_third_fox = malloc(sizeof(*ptr_third_fox));
    ptr_third_fox->code = 30;
    ptr_third_fox->weight_kg = 30.30;
    ptr_third_fox->is_loyal = false;
    mjd_list_add_tail(&ptr_third_fox->list, &fox_list);

    ESP_LOGI(TAG, "ADD FRONT code 90");
    fox_t *ptr_zero_fox;
    ptr_zero_fox = malloc(sizeof(*ptr_zero_fox));
    ptr_zero_fox->code = 90;
    ptr_zero_fox->weight_kg = 90.90;
    ptr_zero_fox->is_loyal = false;
    mjd_list_add(&ptr_zero_fox->list, &fox_list);

    ESP_LOGI(TAG, "LIST (4)");
    mjd_list_for_each_entry(ptr_one_fox, &fox_list, list)
    {
        ESP_LOGI(TAG, "  fox->code %u", ptr_one_fox->code);
    }
    ESP_LOGI(TAG, "mjd_list_empty() 0=no 1=yes");
    ESP_LOGI(TAG, "   %u", mjd_list_empty(&fox_list));

    ESP_LOGI(TAG, "MJD_LIST_COUNT (=4!)");
    mjd_list_count(&fox_list, &mycounter);
    ESP_LOGI(TAG, "  mycounter: %u", mycounter);

    ESP_LOGI(TAG, "DELETE code 20");
    mjd_list_del(&ptr_second_fox->list);
    ESP_LOGI(TAG, "LIST");
    mjd_list_for_each_entry(ptr_one_fox, &fox_list, list)
    {
        ESP_LOGI(TAG, "  fox->code %u", ptr_one_fox->code);
    }
    ESP_LOGI(TAG, "mjd_list_empty() 0=no 1=yes");
    ESP_LOGI(TAG, "   %u", mjd_list_empty(&fox_list));

    ESP_LOGI(TAG, "MJD_LIST_COUNT (=3!)");
    mjd_list_count(&fox_list, &mycounter);
    ESP_LOGI(TAG, "  mycounter: %u", mycounter);

    ESP_LOGI(TAG, "DELETE *ALL");
    mjd_list_for_each_entry_safe(ptr_one_fox, ptr_next_fox, &fox_list, list)
    {
        mjd_list_del(&ptr_one_fox->list);
        free(ptr_one_fox);
    }
    ESP_LOGI(TAG, "LIST (should be empty)");
    mjd_list_for_each_entry(ptr_one_fox, &fox_list, list)
    {
        ESP_LOGI(TAG, "  fox->code %u", ptr_one_fox->code);
    }
    ESP_LOGI(TAG, "mjd_list_empty() 0=no 1=yes");
    ESP_LOGI(TAG, "   %u", mjd_list_empty(&fox_list));

    ESP_LOGI(TAG, "MJD_LIST_COUNT (=0!)");
    mjd_list_count(&fox_list, &mycounter);
    ESP_LOGI(TAG, "  mycounter: %u", mycounter);

    ESP_LOGI(TAG, "ADD 1000 ITEMS (limited FREE HEAP might be only +-200K)");
    mjd_log_memory_statistics();
    for (uint32_t j = 0; j < 1000; ++j) {
        //printf("%u ", j); fflush(stdout);
        ptr_one_fox = malloc(sizeof(*ptr_one_fox));
        ptr_one_fox->code = j;
        ptr_one_fox->weight_kg = j * 25.25;
        ptr_one_fox->is_loyal = false;
        mjd_list_add_tail(&ptr_one_fox->list, &fox_list);
    }
    mjd_log_memory_statistics();
    ESP_LOGI(TAG, "mjd_list_empty() 0=no 1=yes");
    ESP_LOGI(TAG, "   %u", mjd_list_empty(&fox_list));
    ESP_LOGI(TAG, "MJD_LIST_COUNT");
    mjd_list_count(&fox_list, &mycounter);
    ESP_LOGI(TAG, "  mycounter: %u", mycounter);
    mjd_log_memory_statistics();

    // DEVTEMP: HALT
    /////mjd_rtos_wait_forever();

    /********************************************************************************
     * RTOS
     *
     */
    ESP_LOGI(TAG, "\n\n***SECTION: RTOS***");

    ESP_LOGI(TAG, "Delay DEFINES:");
    ESP_LOGI(TAG, "  FYI: #define  portTICK_PERIOD_MS  ((TickType_t) 1000 / configTICK_RATE_HZ)");
    ESP_LOGI(TAG, "  FYI: (x / portTICK_PERIOD_MS) where portTICK_PERIOD_MS = %u", portTICK_PERIOD_MS);

    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "  RTOS_DELAY_0: %u", RTOS_DELAY_0);
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "  RTOS_DELAY_1MILLISEC (too small for RTOS, =0): %u", RTOS_DELAY_1MILLISEC);
    ESP_LOGI(TAG, "  RTOS_DELAY_5MILLISEC (too small for RTOS, =0): %u", RTOS_DELAY_5MILLISEC);
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "  RTOS_DELAY_10MILLISEC  : %u", RTOS_DELAY_10MILLISEC);
    ESP_LOGI(TAG, "  RTOS_DELAY_25MILLISEC  : %u", RTOS_DELAY_25MILLISEC);
    ESP_LOGI(TAG, "  RTOS_DELAY_50MILLISEC  : %u", RTOS_DELAY_50MILLISEC);
    ESP_LOGI(TAG, "  RTOS_DELAY_75MILLISEC  : %u", RTOS_DELAY_75MILLISEC);
    ESP_LOGI(TAG, "  RTOS_DELAY_100MILLISEC : %u", RTOS_DELAY_100MILLISEC);
    ESP_LOGI(TAG, "  RTOS_DELAY_125MILLISEC : %u", RTOS_DELAY_125MILLISEC);
    ESP_LOGI(TAG, "  RTOS_DELAY_150MILLISEC : %u", RTOS_DELAY_150MILLISEC);
    ESP_LOGI(TAG, "  RTOS_DELAY_200MILLISEC : %u", RTOS_DELAY_200MILLISEC);
    ESP_LOGI(TAG, "  RTOS_DELAY_250MILLISEC : %u", RTOS_DELAY_250MILLISEC);
    ESP_LOGI(TAG, "  RTOS_DELAY_500MILLISEC : %u", RTOS_DELAY_500MILLISEC);
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "  RTOS_DELAY_1SEC  : %u", RTOS_DELAY_1SEC);
    ESP_LOGI(TAG, "  RTOS_DELAY_2SEC  : %u", RTOS_DELAY_2SEC);
    ESP_LOGI(TAG, "  RTOS_DELAY_3SEC  : %u", RTOS_DELAY_3SEC);
    ESP_LOGI(TAG, "  RTOS_DELAY_5SEC  : %u", RTOS_DELAY_5SEC);
    ESP_LOGI(TAG, "  RTOS_DELAY_15SEC : %u", RTOS_DELAY_15SEC);
    ESP_LOGI(TAG, "  RTOS_DELAY_30SEC : %u", RTOS_DELAY_30SEC);
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "  RTOS_DELAY_1MINUTE   : %u", RTOS_DELAY_1MINUTE);
    ESP_LOGI(TAG, "  RTOS_DELAY_5MINUTES  : %u", RTOS_DELAY_5MINUTES);
    ESP_LOGI(TAG, "  RTOS_DELAY_15MINUTES : %u", RTOS_DELAY_15MINUTES);

    // DEVTEMP: HALT
    /////mjd_rtos_wait_forever();

    /********************************************************************************
     * ESP32 Logging
     *
     */
    ESP_LOGI(TAG, "\n\n***SECTION: ESP32 Logging***");

    /*
     * Optional for Production: dump less messages
     *  @doc It is possible to lower the log level for specific components (wifi and tcpip_adapter are strong candidates).
     */
    /////esp_log_level_set("wifi", ESP_LOG_WARN); // @important Disable INFO messages which are too detailed for me.
    /////esp_log_level_set("tcpip_adapter", ESP_LOG_WARN); // @important Disable INFO messages which are too detailed for me.
    /*
     *
     */
    ESP_LOGE(TAG, "This is an ERROR log message");
    ESP_LOGW(TAG, "This is a  WARNING log message");
    ESP_LOGI(TAG, "This is an INFO log message");
    ESP_LOGD(TAG, "This is a  DEBUG log message");
    ESP_LOGV(TAG, "This is a  VERBOSE log message");

    uint8_t bin_data[] =
        {
                1,
                2,
                3,
                4,
                5,
                6,
                7,
                8,
                9,
                0,
                '\r',
                '\n',
                '1',
                '2',
                '3',
                '4',
                '5',
                '6',
                '7',
                '8',
                '9',
                '0',
                '\r',
                '\n',
                'a',
                'b',
                'c',
                'd',
                'e',
                'f',
                'g',
                'h',
                '\r',
                '\n',
                'A',
                'B',
                'C',
                'D',
                'E',
                'F',
                'G',
                'H',
                '\r',
                '\n' };
    uint8_t ascii_data[] =
        { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H' };

    ESP_LOGI(TAG, "\nESP_LOG_BUFFER_HEXDUMP(): bin_data");
    ESP_LOG_BUFFER_HEXDUMP(TAG, bin_data, ARRAY_SIZE(bin_data), ESP_LOG_INFO);

    ESP_LOGI(TAG, "\nESP_LOG_BUFFER_HEX(): bin_data");
    ESP_LOG_BUFFER_HEX(TAG, bin_data, ARRAY_SIZE(bin_data));

    ESP_LOGI(TAG, "\nESP_LOG_BUFFER_CHAR(): ascii_data");
    ESP_LOG_BUFFER_CHAR(TAG, ascii_data, ARRAY_SIZE(ascii_data));

    // DEVTEMP: HALT
    /////mjd_rtos_wait_forever();

    /********************************************************************************
     * ESP32 timestamps - RTOS ticks - CPU ticks
     */
    ESP_LOGI(TAG, "\n\n***SECTION: ESP32 ticks and cpu cycles***");

    mjd_log_memory_statistics();

    ESP_LOGI(TAG, "  @doc #define portTICK_PERIOD_MS ((TickType_t) 1000 / configTICK_RATE_HZ )");
    for (i = 0; i < 5; ++i) {
        printf(
                "esp_log_timestamp(): %u millisec | xTaskGetTickCount() * portTICK_PERIOD_MS: %u millisec | xTaskGetTickCount(): %u | xthal_get_ccount(): %u \n",
                esp_log_timestamp(), xTaskGetTickCount() * portTICK_PERIOD_MS, xTaskGetTickCount(), xthal_get_ccount());
        vTaskDelay(RTOS_DELAY_1SEC);
    }

    // DEVTEMP: HALT
    /////mjd_rtos_wait_forever();

    /********************************************************************************
     * ESP32 cJSON
     *
     */
    ESP_LOGI(TAG, "\n\n***SECTION: ESP32 cJSON***");

    mjd_log_memory_statistics();

    ESP_LOGI(TAG, "* Example#2: Make JSON using cJSON_CreateObject()");
    /*
     * {"eventType":"logBegin","prop1":"this is prop1 value","prop2":"this is prop2 value"}
     */
    const uint32_t LEN_JSON_STRING = 2048;
    char *ptr_json_string = (char*) calloc(LEN_JSON_STRING, sizeof(char));
    cJSON *cjsonRoot = NULL;
    char cjson_prop1_value[] = "this is prop1 value";
    char cjson_prop2_value[] = "this is prop2 value";
    cjsonRoot = cJSON_CreateObject();
    cJSON_AddStringToObject(cjsonRoot, "eventType", "logBegin");
    cJSON_AddStringToObject(cjsonRoot, "prop1", cjson_prop1_value);
    cJSON_AddStringToObject(cjsonRoot, "prop2", cjson_prop2_value);
    cJSON_PrintPreallocated(cjsonRoot, ptr_json_string, LEN_JSON_STRING, MJD_CJSON_PRINT_UNFORMATTED);
    if (ptr_json_string == NULL) {
        strcpy(ptr_json_string, "{}");
    }
    cJSON_Delete(cjsonRoot);
    ESP_LOGI(TAG, "  ptr_json_string = %s", ptr_json_string);

    mjd_log_memory_statistics();

    ESP_LOGI(TAG, "* Example#2: Make JSON using cJSON_CreateObject()");
    /* {
     *     "eventType": "meteo",
     *     "time": "20181225235959",
     *     "measurements": {
     *         "valueInteger": 10,
     *         "valueIntegerNineDigits": 123456789,
     *         "valueFloat": 10,
     *         "valueFloatOneDigitBeforeAfterDecimalPoint": 1.5,
     *         "valueDoubleNineDigitsBeforeAfterDecimalPoint": 123456789.12345679,
     *         "valueDoubleEighteenDigits": 1.2345678912345678e+17,
     *         "valueDoubleEighteenDigitsBeforeAfterDecimalPoint": 1.2345678912345678e+17
     *     }
     * }
     */
    strcpy(ptr_json_string, "");

    cjsonRoot = cJSON_CreateObject();
    char cjson_time[] = "20181225235959";
    cJSON_AddStringToObject(cjsonRoot, "eventType", "meteo");
    cJSON_AddStringToObject(cjsonRoot, "time", cjson_time);

    cJSON *cjsonMeasurements = NULL;
    cjsonMeasurements = cJSON_CreateObject();
    cJSON_AddItemToObject(cjsonRoot, "measurements", cjsonMeasurements);
    cJSON_AddNumberToObject(cjsonMeasurements, "valueInteger", 10);
    cJSON_AddNumberToObject(cjsonMeasurements, "valueIntegerNineDigits", 123456789);
    cJSON_AddNumberToObject(cjsonMeasurements, "valueFloat", (float) 10);
    cJSON_AddNumberToObject(cjsonMeasurements, "valueFloatOneDigitBeforeAfterDecimalPoint", 1.5);
    cJSON_AddNumberToObject(cjsonMeasurements, "valueDoubleNineDigitsBeforeAfterDecimalPoint", (double) 123456789.123456789);
    cJSON_AddNumberToObject(cjsonMeasurements, "valueDoubleEighteenDigits", (double) 123456789123456789);
    cJSON_AddNumberToObject(cjsonMeasurements, "valueDoubleEighteenDigitsBeforeAfterDecimalPoint", (double) 123456789123456789.123456789123456789);

    cJSON_PrintPreallocated(cjsonRoot, ptr_json_string, LEN_JSON_STRING, MJD_CJSON_PRINT_UNFORMATTED);
    if (ptr_json_string == NULL) {
        strcpy(ptr_json_string, "{}");
    }
    cJSON_Delete(cjsonRoot);
    ESP_LOGI(TAG, "  ptr_json_string = %s", ptr_json_string);

    mjd_log_memory_statistics();

    /*
     * {"service": "http-input-adapter","status": "ok"}
     *     @important Do not forget to escape the double quotes in the json string, e.g. \"
     *
     *     \"status\": \"ok\"}
     */
    ESP_LOGI(TAG, "* Example#3: Parse JSON using cJSON_Parse(), [check propStatus equal to ok]");
    strcpy(ptr_json_string, "{\"service\": \"http-input-adapter\",\"status\": \"ok\"}}");

    const char expected_status_ok[] = "ok";

    cJSON *json_response = cJSON_Parse(ptr_json_string);
    if (json_response == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            ESP_LOGI(TAG, "Error cJSON_Parse() before: %s", error_ptr);
        }
        // GOTO
        goto example3_cleanup;
    }

    const cJSON *cjson_status = NULL;
    cjson_status = cJSON_GetObjectItemCaseSensitive(json_response, "status");
    if (!cJSON_IsString(cjson_status)) {
        ESP_LOGE(TAG, "Error cjson_status not data type String (or does not exist)");
        // GOTO
        goto example3_cleanup;
    }
    if ((cjson_status->valuestring == NULL)) {
        ESP_LOGE(TAG, "Error cjson_status valueString is NULL");
        // GOTO
        goto example3_cleanup;
    }

    if (strcmp(expected_status_ok, cjson_status->valuestring) != 0) {
        ESP_LOGE(TAG, "Error status value is not \"ok\" (actual value is \"%s\")", cjson_status->valuestring);
        // GOTO
        goto example3_cleanup;
    }

    ESP_LOGI(TAG, "OK status value is \"ok\"");

    // LABEL
    example3_cleanup:;

    cJSON_Delete(json_response);

    mjd_log_memory_statistics();

    // DEVTEMP: HALT
    /////mjd_rtos_wait_forever();

    /********************************************************************************
     * ESP32 mbedtls base64.h
     *
     * int mbedtls_base64_encode( unsigned char *dst, size_t dlen, size_t *olen, const unsigned char *src, size_t slen );
     * \brief          Encode a buffer into base64 format
     *
     * \param dst      destination buffer
     * \param dlen     size of the destination buffer
     * \param olen     number of bytes written
     * \param src      source buffer
     * \param slen     amount of data to be encoded
     *
     * \return         0 if successful, or MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL.
     */
    ESP_LOGI(TAG, "\n\n***SECTION: mbedtls base64.h***");

    mjd_log_memory_statistics();

    int mbedtls_retval = 0;
    char sstring_one[8 + 1] = "12345678";
    unsigned char ustring_two[128 + 1] = "";
    size_t base64_bytes_written = 0;

    ESP_LOGI(TAG, "mbedtls_base64_encode()");
    ESP_LOGI(TAG, "  sstring_one (len %3u): %s", strlen(sstring_one), sstring_one);

    mbedtls_retval = mbedtls_base64_encode(ustring_two, sizeof(ustring_two), &base64_bytes_written, (unsigned char *) sstring_one,
            strlen(sstring_one));
    if (mbedtls_retval != 0) {
        ESP_LOGE(TAG, "  mbedtls_base64_encode() failed err %i", mbedtls_retval);
    } else
        ESP_LOGI(TAG, "  ustring_two (len %3u): %s", base64_bytes_written, ustring_two);

    // DEVTEMP: HALT
    /////mjd_rtos_wait_forever();

    /********************************************************************************
     * LED
     *
     */
    ESP_LOGI(TAG, "\n\n***SECTION: LED***");
    ESP_LOGI(TAG, "MY_LED_ON_DEVBOARD_GPIO_NUM:    %i", MY_LED_ON_DEVBOARD_GPIO_NUM);
    ESP_LOGI(TAG, "MY_LED_ON_DEVBOARD_WIRING_TYPE: %i", MY_LED_ON_DEVBOARD_WIRING_TYPE);

    mjd_log_memory_statistics();

    mjd_led_config_t led_config =
        { 0 };
    led_config.gpio_num = MY_LED_ON_DEVBOARD_GPIO_NUM;
    led_config.wiring_type = MY_LED_ON_DEVBOARD_WIRING_TYPE; // 1 GND MCU Huzzah32 | 2 VCC MCU Lolin32lite
    mjd_led_config(&led_config);

    ESP_LOGI(TAG, "LED on off");
    mjd_led_on(MY_LED_ON_DEVBOARD_GPIO_NUM);
    vTaskDelay(RTOS_DELAY_500MILLISEC);
    mjd_led_off(MY_LED_ON_DEVBOARD_GPIO_NUM);
    vTaskDelay(RTOS_DELAY_1SEC);

    ESP_LOGI(TAG, "LED blink 3 times");
    mjd_led_blink_times(MY_LED_ON_DEVBOARD_GPIO_NUM, 3);
    vTaskDelay(RTOS_DELAY_1SEC);

    ESP_LOGI(TAG, "LED mark error");
    mjd_led_mark_error(MY_LED_ON_DEVBOARD_GPIO_NUM);

    vTaskDelay(RTOS_DELAY_1SEC);

    // DEVTEMP: HALT
    /////mjd_rtos_wait_forever();

    /********************************************************************************
     * WIFI general
     *
     */
    ESP_LOGI(TAG, "\n\n***SECTION: WIFI general***");

    mjd_log_memory_statistics();

    ESP_LOGI(TAG, "  mjd_wifi_reason_to_msg() reason: %d (msg %s)", WIFI_REASON_UNSPECIFIED, mjd_wifi_reason_to_msg(WIFI_REASON_UNSPECIFIED));
    ESP_LOGI(TAG, "  mjd_wifi_reason_to_msg() reason: %d (msg %s)", WIFI_REASON_HANDSHAKE_TIMEOUT,
            mjd_wifi_reason_to_msg(WIFI_REASON_HANDSHAKE_TIMEOUT));

    ESP_LOGI(TAG, "  mjd_wifi_sta_is_connected(): "MJDBOOLEANFMT, MJDBOOLEAN2STR(mjd_wifi_sta_is_connected()));

    // DEVTEMP: HALT
    /////mjd_rtos_wait_forever();

    /********************************************************************************
     * WIFI
     *   @important Do wifi-init only ONCE in the app.
     *   @important Do delay between reconnect cycles ELSE connecting to the Wifi AP might fail sometimes (depends on the HW Router Device model).
     *
     */
    ESP_LOGI(TAG, "\n\n***SECTION: WIFI***");
    ESP_LOGI(TAG, "MY_WIFI_SSID:     %s", MY_WIFI_SSID);
    ESP_LOGI(TAG, "MY_WIFI_PASSWORD: %s", MY_WIFI_PASSWORD);

    mjd_log_memory_statistics();

    f_retval = mjd_wifi_sta_init(MY_WIFI_SSID, MY_WIFI_PASSWORD);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "mjd_wifi_sta_init() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        goto wifi_cleanup;
        // GOTO
    }

    total = 3;  // DEVTEMP 3 100 1000
    ESP_LOGI(TAG, "WIFI: start stop: %i times", total);
    i = 0;
    while (++i <= total) {
        ESP_LOGI(TAG, "\n\nWIFI: LOOP#%i of %i", i, total);

        mjd_log_memory_statistics();

        mjd_led_on(MY_LED_ON_DEVBOARD_GPIO_NUM);

        f_retval = mjd_wifi_sta_start();
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "mjd_wifi_sta_start() err %i (%s)", f_retval, esp_err_to_name(f_retval));

            mjd_led_off(MY_LED_ON_DEVBOARD_GPIO_NUM);
            continue; // CONTINUE
        }

        mjd_log_memory_statistics();

        ESP_LOGI(TAG, "Helper: Is station connected to an AP? - mjd_wifi_sta_is_connected()");
        ESP_LOGI(TAG, "  mjd_wifi_sta_is_connected(): "MJDBOOLEANFMT, MJDBOOLEAN2STR(mjd_wifi_sta_is_connected()));

        ESP_LOGI(TAG, "Helper: Show IP Address via STA - mjd_net_get_ip_address()");
        char ip_address[32] = "";
        f_retval = mjd_net_get_ip_address(ip_address);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "mjd_net_get_ip_address() err %i (%s)", f_retval, esp_err_to_name(f_retval));

            mjd_led_off(MY_LED_ON_DEVBOARD_GPIO_NUM);
            continue; // CONTINUE
        }
        ESP_LOGI(TAG, "  ip_address = %s", ip_address);

        ESP_LOGI(TAG, "Helper: Log Wifi Station Info - mjd_wifi_log_sta_info()");
        mjd_wifi_log_sta_info();

        ESP_LOGI(TAG, "Helper: Get Station info (Device, connected Access Point) - mjd_wifi_sta_get_info()");
        //   @purpose Use it in my project meteohub to upload this device data
        mjd_wifi_sta_info_t mjd_wifi_sta_info = { 0 };
        mjd_wifi_sta_get_info(&mjd_wifi_sta_info);

        //
        ESP_LOGI(TAG, "Helper: Internet Check - mjd_net_is_internet_reachable");
        f_retval = mjd_net_is_internet_reachable();
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "mjd_net_is_internet_reachable() err %i (%s)", f_retval, esp_err_to_name(f_retval));

            mjd_led_off(MY_LED_ON_DEVBOARD_GPIO_NUM);
            continue; // CONTINUE
        }
        ESP_LOGI(TAG, "  OK Internet reachable");

        ESP_LOGI(TAG, "Helper: Resolve hostname to its IPv4 address - mjd_net_resolve_hostname_ipv4()");
        char str_hostname[] = "www.google.com";
        char str_ip_address[128];
        f_retval = mjd_net_resolve_hostname_ipv4(str_hostname, str_ip_address);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "Error mjd_net_resolve_hostname_ipv4(): errno %i (%s)", f_retval, esp_err_to_name(f_retval));

            mjd_led_off(MY_LED_ON_DEVBOARD_GPIO_NUM);
            continue; // CONTINUE
        }
        ESP_LOGI(TAG, "  hostname %s => IPv4 address %s", str_hostname, str_ip_address);


        mjd_log_memory_statistics();


        // ***Disconnect
        f_retval = mjd_wifi_sta_disconnect_stop();
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "mjd_wifi_sta_disconnect_stop() err %i ", f_retval);
            ESP_LOGE(TAG, "esp_err_to_name(err) = %s", esp_err_to_name(f_retval));

            mjd_led_off(MY_LED_ON_DEVBOARD_GPIO_NUM);
            continue; // CONTINUE
        }

        mjd_led_off(MY_LED_ON_DEVBOARD_GPIO_NUM);

        mjd_log_memory_statistics();

        // @important Delay 5 sec always works fine (delay 1 sec = often errors!)
        vTaskDelay(RTOS_DELAY_5SEC);
    }

    wifi_cleanup: ;

    mjd_log_memory_statistics();

    // DEVTEMP: HALT
    /////mjd_rtos_wait_forever();

    /********************************************************************************
     * NET - SNTP sync datetime - Part 1
     *
     */
    ESP_LOGI(TAG, "\n\n***SECTION: NET - SNTP sync datetime***");

    mjd_log_memory_statistics();

    mjd_led_on(MY_LED_ON_DEVBOARD_GPIO_NUM);

    f_retval = mjd_wifi_sta_start();
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "mjd_wifi_sta_start() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        ESP_LOGE(TAG, "SKIPPING sync datetime due to wifi error");
        // GOTO
        goto sntp_cleanup;
    }

    f_retval = mjd_net_sync_current_datetime(false);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "mjd_net_sync_current_datetime(false) err %i (%s)", f_retval, esp_err_to_name(f_retval));
        ESP_LOGE(TAG, "COULD NOT sync datetime due to error");
    }

    sntp_cleanup: ;

    mjd_log_time();
    mjd_wifi_sta_disconnect_stop();

    mjd_led_off(MY_LED_ON_DEVBOARD_GPIO_NUM);

    vTaskDelay(RTOS_DELAY_5SEC);

    /*
     * (UNCOMMENT THIS PART#2 ONLY FOR TESTING)
     * NET - SNTP sync datetime - Part#2 Ensure the current datetime is preserved after a deepsleep-restart
     */
    /*
     #define SLEEP_TIME  10 * 1000 * 000000  // MICROseconds
     esp_sleep_enable_timer_wakeup(SLEEP_TIME);
     ESP_LOGI(TAG, "Entering deep sleep (should wake up 10 seconds later)(what is current datetime then?)...\n\n");
     esp_deep_sleep_start();
     */

    // DEVTEMP: HALT
    /////mjd_rtos_wait_forever();
    /********************************************************************************
     * MQTT INIT
     * @important Do this only ONCE in the whole app!
     *
     */
    ESP_LOGI(TAG, "\n\n***SECTION: MQTT INIT***");

    mjd_log_memory_statistics();

    f_retval = mjd_mqtt_init(MY_MQTT_BUFFER_SIZE, MY_MQTT_TIMEOUT);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "ABORT. mjd_mqtt_init() failed");
        // GOTO
        goto mqtt_cleanup1;
    }

    /********************************************************************************
     * MQTT
     * @dep mjd_wifi_sta_init() in previous section.
     *
     */
    ESP_LOGI(TAG, "\n\n***SECTION: MQTT***");

    mjd_log_memory_statistics();

    // Wifi
    f_retval = mjd_wifi_sta_start();
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "mjd_wifi_sta_start() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto mqtt_cleanup1;
    }

    // MQTT Start
    f_retval = mjd_mqtt_start(MY_MQTT_HOST, MY_MQTT_PORT, "esp32_mjd_components_main", MY_MQTT_USER, MY_MQTT_PASS);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "ABORT. mjd_mqtt_start() failed");
        // GOTO
        goto mqtt_cleanup2;
    }

    // LOOP: MQTT Publish
    char topic[] = "meteohub/current/temporary";

    char payload[MY_MQTT_BUFFER_SIZE - 1] = ""; // IF task stackoverflow Then increase mqtt buffer size
    memset(payload, '-', 1024);

    total = 100;  // DEVTEMP 10 25 100 1000 50000
    ESP_LOGI(TAG, "MQTT publish: %i times", total);
    i = 0;
    while (++i <= total) {
        ESP_LOGI(TAG, "MQTT: LOOP#%i of %i", i, total);

        mjd_log_memory_statistics();

        /////ESP_LOGI(TAG, "MQTT publish: (%u) topic=%s => (%u) payload=%s \n", strlen(topic), topic, strlen(payload), payload);

        f_retval = mjd_mqtt_publish(topic, (uint8_t *) payload, strlen(payload), MJD_MQTT_QOS_1, false);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "ABORT. mjd_mqtt_publish() failed");
            // GOTO (ERROR)
            goto mqtt_cleanup2;
        }
    }

    //---LABEL---
    mqtt_cleanup2: ;

    mjd_log_memory_statistics();

    // MQTT Stop
    f_retval = mjd_mqtt_stop(); // @important Under label mqtt_cleanup2 section. NOT under label mqtt_cleanup1!
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "(continuing) mjd_mqtt_stop() failed");
    }

    //---LABEL---
    mqtt_cleanup1: ;

    mjd_wifi_sta_disconnect_stop();

    mjd_log_memory_statistics();

    // DEVTEMP: HALT
    /////mjd_rtos_wait_forever();

    /********************************************************************************
     * DEEP SLEEP & RESTART TIMER
     * @sop Put this section in comments when testing other things afterwards (else the MCU restarts every time...)
     * @important In deep sleep mode, wireless peripherals are powered down. Before entering sleep mode, applications must disable WiFi and BT using appropriate calls ( esp_bluedroid_disable(), esp_bt_controller_disable(), esp_wifi_stop()).
     * @doc https://esp-idf.readthedocs.io/en/latest/api-reference/system/sleep_modes.html
     *
     */
    ESP_LOGI(TAG, "\n\n***SECTION: DEEP SLEEP***");

    mjd_log_memory_statistics();

    const uint32_t MY_DEEP_SLEEP_TIME_SEC = 15; // 15 15*60 30*60
    esp_sleep_enable_timer_wakeup(mjd_seconds_to_microseconds(MY_DEEP_SLEEP_TIME_SEC));

    // @important Wait a bit so that the ESP_LOGI() can really dump to UART before deep sleep kicks in!
    ESP_LOGI(TAG, "Entering deep sleep (the MCU should wake up %u seconds later)...\n\n", MY_DEEP_SLEEP_TIME_SEC);
    vTaskDelay(RTOS_DELAY_1SEC);

    // DEVTEMP-BEGIN WORKAROUND for "The system does not wake up properly anymore after the ***2nd*** deep sleep period (and any deep sleep period after that) using ESP-IDF v3.2-dev-607-gb14e87a6."
    //     A temporary workaround is to call esp_set_deep_sleep_wake_stub(NULL); before entering deep sleep
    //     https://www.esp32.com/viewtopic.php?f=13&t=6919&p=29714
    /////esp_set_deep_sleep_wake_stub(NULL);
    // DEVTEMP-END

    esp_deep_sleep_start();

    // DEVTEMP @important I never get to this code line if deep sleep is initiated :P
    /////mjd_rtos_wait_forever();

    /********************************************************************************
     * Task Delete
     * @doc Passing NULL will end the current task
     *
     */
    vTaskDelete(NULL);
}

/*
 * MAIN
 */
void app_main() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    mjd_log_memory_statistics();

    /**********
     * TASK:
     * @important For stability (RMT + Wifi): always use xTaskCreatePinnedToCore(APP_CPU_NUM) [Opposed to xTaskCreate()]
     */
    BaseType_t xReturned;
    xReturned = xTaskCreatePinnedToCore(&main_task, "main_task (name)", MYAPP_RTOS_TASK_STACK_SIZE_16K, NULL,
    MYAPP_RTOS_TASK_PRIORITY_NORMAL, NULL,
    APP_CPU_NUM);
    if (xReturned == pdPASS) {

        ESP_LOGI(TAG, "OK Task has been created, and is running right now");
    }

    /**********
     * END
     */
    ESP_LOGI(TAG, "END %s()", __FUNCTION__);
}
