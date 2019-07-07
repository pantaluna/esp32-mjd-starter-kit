/*
 * Goto the README.md for instructions
 *
 */
#ifndef __MJD_LORABEE_H__
#define __MJD_LORABEE_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * LORA settings
 *  @rule EU863-870 SF7 125Khz: maximum payload size is 230 bytes.
 */
#define MJD_LORABEE_LORA_TX_PAYLOAD_MAX_BYTES (230)
#define MJD_LORABEE_LORA_RX_PAYLOAD_MAX_BYTES (230)

/**
 * Data structs
 *
 */

// GPIO
typedef enum {
    MJD_LORABEE_GPIO_NUM_0 = 0, /*!< RN2483 GPIO0 (=LED on the LoraBee board) */
    MJD_LORABEE_GPIO_NUM_1, /*!< RN2483 GPIO1 */
    MJD_LORABEE_GPIO_NUM_2, /*!< RN2483 GPIO2 */
    MJD_LORABEE_GPIO_NUM_3, /*!< RN2483 GPIO3 */
    MJD_LORABEE_GPIO_NUM_MAX,
} mjd_lorabee_gpio_num_t;

typedef enum {
    MJD_LORABEE_GPIO_LEVEL_LOW = 0,
    MJD_LORABEE_GPIO_LEVEL_HIGH,
    MJD_LORABEE_GPIO_LEVEL_MAX,
} mjd_lorabee_gpio_level_t;

typedef enum {
    MJD_LORABEE_MODE_LORA = 0, /*!< */
    MJD_LORABEE_MODE_FSK, /*!< */
    MJD_LORABEE_MODE_MAX,
} mjd_lorabee_mode_t;

typedef enum {
    MJD_LORABEE_SPREADING_FACTOR_SF7 = 7, /*!< sf7, sf8, sf9, sf10, sf11, sf12 */
    MJD_LORABEE_SPREADING_FACTOR_SF8 = 8, /*!< */
    MJD_LORABEE_SPREADING_FACTOR_SF9 = 9, /*!< */
    MJD_LORABEE_SPREADING_FACTOR_SF10 = 10, /*!< */
    MJD_LORABEE_SPREADING_FACTOR_SF11 = 11, /*!< */
    MJD_LORABEE_SPREADING_FACTOR_SF12 = 12, /*!< */
    MJD_LORABEE_SPREADING_FACTOR_MAX,
} mjd_lorabee_spreading_factor_t;

typedef enum {
    MJD_LORABEE_BANDWIDTH_125KHZ = 125, /*!< 125, 250, 500 */
    MJD_LORABEE_BANDWIDTH_250KHZ = 250, /*!< */
    MJD_LORABEE_BANDWIDTH_500KHZ = 500, /*!< */
    MJD_LORABEE_BANDWIDTH_MAX,
} mjd_lorabee_bandwidth_t;

typedef enum {
    MJD_LORABEE_CODING_RATE_4_5 = 0, /*!< 4/5 (@default) Less error correcting bits -> fastest transmission */
    MJD_LORABEE_CODING_RATE_4_6,     /*!< 4/6 */
    MJD_LORABEE_CODING_RATE_4_7,     /*!< 4/7 */
    MJD_LORABEE_CODING_RATE_4_8,     /*!< 4/8 More error correcting bits -> slowest transmission */
    MJD_LORABEE_CODING_RATE_MAX,
} mjd_lorabee_coding_rate_t;

/*
 * RADIO CONFIG
 *
 */
typedef struct {
        bool is_init; /*!< Has the component been initialized? */
        uart_port_t uart_port_num; /*!< The ESP32 UART interface being used (typically UART_NUM1 or UART_NUM2) */
        gpio_num_t uart_tx_gpio_num; /*!< The ESP32 GPIO Number for the UART TX pin (GPIO#22 for an Adafruit Huzzah32) */
        gpio_num_t uart_rx_gpio_num; /*!< The ESP32 GPIO Number for the UART RX pin (GPIO#23 for an Adafruit Huzzah32) */
        gpio_num_t reset_gpio_num; /*!< The ESP32 GPIO pin that is wired to the LoraBee Reset pin (GPIO#14 for an Adafruit Huzzah32) */

        int32_t radio_power;
        mjd_lorabee_mode_t radio_mode;
        uint32_t radio_frequency;
        mjd_lorabee_spreading_factor_t radio_spreading_factor;
        mjd_lorabee_bandwidth_t radio_bandwidth;
        mjd_lorabee_coding_rate_t radio_coding_rate;
        uint32_t radio_watchdog_timeout; /*!< milliseconds (60000=1minute), decimal number representing the time-out length for the Watchdog Timer, from 0 to 4294967295. Set to ‘0’ to disable this functionality. */

        uint8_t max_nbr_of_radio_tx; /*!< Lora protocol: the max nbr of runs (includes retries) for 'radio tx` when transmitting */

        uint32_t nbr_of_errors; /*!< Runtime Statistics: the total number of errors when interacting with the Microchip RN2483 */
} mjd_lorabee_config_t;

#define MJD_LORABEE_CONFIG_DEFAULT() { \
    .is_init = false, \
    .uart_port_num = UART_NUM_MAX, \
    .uart_tx_gpio_num = GPIO_NUM_MAX, \
    .uart_rx_gpio_num = GPIO_NUM_MAX, \
    .reset_gpio_num = GPIO_NUM_MAX, \
    \
    .radio_power = 10, \
    .radio_mode = MJD_LORABEE_MODE_LORA, \
    .radio_frequency = 0, \
    .radio_spreading_factor = MJD_LORABEE_SPREADING_FACTOR_MAX, \
    .radio_bandwidth = MJD_LORABEE_BANDWIDTH_MAX, \
    .radio_coding_rate = MJD_LORABEE_CODING_RATE_MAX, \
    .radio_watchdog_timeout = 0, \
    \
    .max_nbr_of_radio_tx = 5, \
    \
    .nbr_of_errors = 0, \
}

// Microchip RN2843 RESPONSE
typedef struct {
        bool data_received; /*!< Has data been received from the device? */
        char response_1[16 + 2 * MJD_LORABEE_LORA_RX_PAYLOAD_MAX_BYTES]; /*!< RX#1 (optional) */
        char response_2[16 + 2 * MJD_LORABEE_LORA_RX_PAYLOAD_MAX_BYTES]; /*!< RX#2 (optional) */
        // size_t len_1;
        // size_t len_2;
} mjd_lorabee_response_t;

#define MJD_LORABEE_RESPONSE_DEFAULT() { \
    .data_received = false, \
    .response_1 = "", \
    .response_2 = "", \
}

// Microchip RN2843 version info (model, fw version, fw date)
// @example RN2483 1.0.3 Mar 22 2017 06:00:42
// @doc where X.Y.Z is firmware version, MMM is month, DD is day, HH:MM:SS is hour, minutes, seconds (format: [HW][FW] [Date] [Time]). [Date] and [Time] refer to the release of the firmware.
typedef struct {
        char raw[64]; /*!< The raw response */
        char model[16]; /*!< RN2483 */
        char firmware_version[16]; /*!< 1.0.3 */
        char firmware_date[24]; /*!< Mar 22 2017 06:00:42 */
} mjd_lorabee_version_info_t;




/**
 * Function declarations
 */
esp_err_t mjd_lorabee_cmd(mjd_lorabee_config_t* param_ptr_config, const char* param_ptr_command,
                          mjd_lorabee_response_t* param_ptr_response);

esp_err_t mjd_lorabee_sys_set_nvm(mjd_lorabee_config_t* param_ptr_config, uint32_t param_hex_address, uint8_t param_value);

esp_err_t mjd_lorabee_sys_set_pindig(mjd_lorabee_config_t* param_ptr_config, mjd_lorabee_gpio_num_t param_gpio_num,
                                     mjd_lorabee_gpio_level_t param_gpio_level);
// TODO sys_get_pindig()

esp_err_t mjd_lorabee_sys_get_nvm(mjd_lorabee_config_t* param_ptr_config, uint32_t param_hex_address,
                                  uint8_t * param_result);

esp_err_t mjd_lorabee_sys_get_hweui(mjd_lorabee_config_t* param_ptr_config, char * param_ptr_result);
esp_err_t mjd_lorabee_sys_get_version(mjd_lorabee_config_t* param_ptr_config, mjd_lorabee_version_info_t * param_ptr_result);
esp_err_t mjd_lorabee_sys_get_vdd(mjd_lorabee_config_t* param_ptr_config, uint32_t * param_ptr_result);

esp_err_t mjd_lorabee_radio_set_power(mjd_lorabee_config_t* param_ptr_config, int32_t param_value);
esp_err_t mjd_lorabee_radio_get_power(mjd_lorabee_config_t* param_ptr_config, int32_t * param_ptr_result);

esp_err_t mjd_lorabee_radio_set_mode(mjd_lorabee_config_t* param_ptr_config, mjd_lorabee_mode_t param_value);
esp_err_t mjd_lorabee_radio_get_mode(mjd_lorabee_config_t* param_ptr_config, mjd_lorabee_mode_t *param_ptr_value);

esp_err_t mjd_lorabee_radio_set_frequency(mjd_lorabee_config_t* param_ptr_config, uint32_t param_value);
esp_err_t mjd_lorabee_radio_get_frequency(mjd_lorabee_config_t* param_ptr_config, uint32_t * param_ptr_result);

esp_err_t mjd_lorabee_radio_set_spreading_factor(mjd_lorabee_config_t* param_ptr_config,
                                                 mjd_lorabee_spreading_factor_t param_value);
esp_err_t mjd_lorabee_radio_get_spreading_factor(mjd_lorabee_config_t* param_ptr_config,
                                                 mjd_lorabee_spreading_factor_t *param_ptr_value);

esp_err_t mjd_lorabee_radio_set_bandwidth(mjd_lorabee_config_t* param_ptr_config, mjd_lorabee_bandwidth_t param_value);
esp_err_t mjd_lorabee_radio_get_bandwidth(mjd_lorabee_config_t* param_ptr_config, mjd_lorabee_bandwidth_t * param_ptr_value);

esp_err_t mjd_lorabee_sys_set_coding_rate(mjd_lorabee_config_t* param_ptr_config, mjd_lorabee_coding_rate_t param_value);
esp_err_t mjd_lorabee_sys_get_coding_rate(mjd_lorabee_config_t* param_ptr_config, mjd_lorabee_coding_rate_t * param_ptr_value);

esp_err_t mjd_lorabee_radio_set_watchdog_timeout(mjd_lorabee_config_t* param_ptr_config, uint32_t param_value);
esp_err_t mjd_lorabee_radio_get_watchdog_timeout(mjd_lorabee_config_t* param_ptr_config, uint32_t * param_ptr_result);

esp_err_t mjd_lorabee_radio_get_signal_noise_ratio(mjd_lorabee_config_t* param_ptr_config, int32_t * param_ptr_result);

esp_err_t mjd_lorabee_radio_tx(mjd_lorabee_config_t* param_ptr_config, uint8_t* param_ptr_payload, size_t param_len);

esp_err_t mjd_lorabee_radio_rx(mjd_lorabee_config_t* param_ptr_config, uint8_t* param_ptr_result, size_t* param_len);

esp_err_t mjd_lorabee_mac_pause(mjd_lorabee_config_t* param_ptr_config);
esp_err_t mjd_lorabee_mac_resume(mjd_lorabee_config_t* param_ptr_config);

esp_err_t mjd_lorabee_reset(mjd_lorabee_config_t* param_ptr_config);
esp_err_t mjd_lorabee_sleep(mjd_lorabee_config_t* param_ptr_config);
esp_err_t mjd_lorabee_wakeup(mjd_lorabee_config_t* param_ptr_config);

esp_err_t mjd_lorabee_send_cmd(mjd_lorabee_config_t* param_ptr_config);

esp_err_t mjd_lorabee_log_config(mjd_lorabee_config_t* param_ptr_config);
esp_err_t mjd_lorabee_init(mjd_lorabee_config_t* param_ptr_config);
esp_err_t mjd_lorabee_deinit(mjd_lorabee_config_t* param_ptr_config);

#ifdef __cplusplus
}
#endif

#endif /* __MJD_LORABEE_H__ */
