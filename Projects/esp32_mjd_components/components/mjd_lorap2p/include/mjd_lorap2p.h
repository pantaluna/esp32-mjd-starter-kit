/*
 * Goto the README.md for instructions
 *
 */
#ifndef __MJD_LORAP2P_H__
#define __MJD_LORAP2P_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "mjd_lorabee.h"

/******************************************************************************
 * My LORAP2
 *
 */
// TODO TO be exacted later (somewhere between 200 and 230)
#define MJD_LORAP2P_TX_PAYLOAD_MAX_BYTES (220)

// Various radio net/device addresses (can be changed in the app)
#define MJD_LORAP2P_ADDR_NET_1_EDGE_GATEWAY ("01:00:00")
#define MJD_LORAP2P_ADDR_NET_1_DEVICE_1 ("01:00:01")
#define MJD_LORAP2P_ADDR_NET_1_DEVICE_2 ("01:00:02")

// Address helper for printf
#define MJDLORAP2PADDRFMT "%02X:%02X:%02X"
#define MJDLORAP2PADDRSTR(a) (a)[0], (a)[1], (a)[2]

typedef enum {
    MJD_LORAP2P_RADIO_CHANNEL_1 = 0, /*!< Matches indices of mjd_lorabee_p2p_channel_configs[] */
    MJD_LORAP2P_RADIO_CHANNEL_2 = 1,
    MJD_LORAP2P_RADIO_CHANNEL_3 = 2,
    MJD_LORAP2P_RADIO_CHANNEL_4 = 3,
    MJD_LORAP2P_RADIO_CHANNEL_MAX,
} mjd_lorap2p_radio_channel_t;

typedef struct {
        uint32_t frequency;
        mjd_lorabee_spreading_factor_t spreading_factor;
        mjd_lorabee_bandwidth_t bandwidth;
        mjd_lorabee_coding_rate_t coding_rate;
} mjd_lorap2p_radio_channel_config_t;

typedef struct {
        bool _is_init; /*!< Has the component been initialized? */

        uart_port_t uart_port_num; /*!< The ESP32 UART interface being used (typically UART_NUM1 or UART_NUM2) */
        gpio_num_t uart_tx_gpio_num; /*!< The ESP32 GPIO Number for the UART TX pin (GPIO#22 for an Adafruit Huzzah32) */
        gpio_num_t uart_rx_gpio_num; /*!< The ESP32 GPIO Number for the UART RX pin (GPIO#23 for an Adafruit Huzzah32) */
        gpio_num_t reset_gpio_num; /*!< The ESP32 GPIO pin that is wired to the LoraBee Reset pin (GPIO#14 for an Adafruit Huzzah32) */

        uint8_t radio_device_address[3]; /*!< NN:DD:DD Net (01) + Device (00:01) */
        int32_t radio_power;
        mjd_lorap2p_radio_channel_t radio_channel;
        uint32_t radio_watchdog_timeout; /*!< milliseconds (60000=1minute), decimal number representing the time-out length for the Watchdog Timer, from 0 to 4294967295. Set to ‘0’ to disable this functionality. */

        mjd_lorabee_config_t lorabee_config; /*!< */

        uint8_t tx_x_times; /*!< The nbr of times a packet is transmitted. Ensure at least one is received correctly by the receiver. The frame's is_retry flag is 0 for the 1st one and 0 for all others. */
        uint32_t _nbr_of_errors; /*!< Runtime Statistics: the total number of errors */
} mjd_lorap2p_config_t;

#define MJD_LORAP2P_CONFIG_DEFAULT() { \
    ._is_init = false, \
    .uart_port_num = UART_NUM_MAX, \
    .uart_tx_gpio_num = GPIO_NUM_MAX, \
    .uart_rx_gpio_num = GPIO_NUM_MAX, \
    .reset_gpio_num = GPIO_NUM_MAX, \
    .radio_device_address = {0}, \
    .radio_power = 10, \
    .radio_channel = MJD_LORAP2P_RADIO_CHANNEL_MAX, \
    .radio_watchdog_timeout = 0, \
    .lorabee_config = MJD_LORABEE_CONFIG_DEFAULT(), \
    .tx_x_times = 3, \
    ._nbr_of_errors = 0, \
}

typedef struct {
        uint8_t destination_address[3];
        uint8_t len_payload; /*<! */
        uint8_t* payload; /*<! PTR to payload. Typically a Protobuf buffer */
} mjd_lorap2p_data_frame_input_t;

#define MJD_LORAP2P_DATA_FRAME_INPUT_DEFAULT() { \
    .destination_address = {0}, \
    .len_payload = 0, \
    .payload = NULL, \
}

typedef struct {
        uint8_t _prefix[3]; /*!< <~> */
        uint8_t _frame_type; /*!< D=Data */
        uint8_t source_address[3]; /*<! source device addr */
        uint8_t seq_nr; /*!< tx frame sequence nr */
        uint8_t is_retry; /*!< 0=no 1=yes */
        uint8_t destination_address[3]; /*<! destination device addr @source mjd_lorap2p_data_frame_input_t */
        uint8_t len_payload; /*<! @source mjd_lorap2p_data_frame_input_t */
        uint8_t* payload; /*<! PTR to payload @source mjd_lorap2p_data_frame_input_t */
} mjd_lorap2p_data_frame_t;

#define MJD_LORAP2P_DATA_FRAME_DEFAULT() { \
    ._prefix = {'<','~','>'}, \
    ._frame_type = 'D', \
    .source_address = {0}, \
    .seq_nr = 0, \
    .is_retry = 0, \
    .destination_address = {0}, \
    .len_payload = 0, \
    .payload = NULL, \
}

typedef struct { /* TODO */
        uint8_t _prefix[3]; /*!< <~> */
        uint8_t frame_type; /*!< 'A'=ACK */
} mjd_lorap2p_ack_frame_t;

#define MJD_LORAP2P_ACK_FRAME_DEFAULT() { \
}

/**
 * Function declarations
 */
esp_err_t mjd_lorap2p_string_to_addr(const char * param_ptr_input, uint8_t param_ptr_addr[], size_t param_size_addr) ;
esp_err_t mjd_lorap2p_addr_to_string(const uint8_t param_ptr_input_addr[], size_t param_size_addr, char * param_ptr_output);
esp_err_t mjd_lorap2p_log_config(mjd_lorap2p_config_t* param_ptr_config);
esp_err_t mjd_lorap2p_log_data_frame_input(mjd_lorap2p_data_frame_input_t *param_ptr_data_frame);
esp_err_t mjd_lorap2p_log_data_frame(mjd_lorap2p_data_frame_t *param_ptr_data_frame);
esp_err_t mjd_lorap2p_init(mjd_lorap2p_config_t* param_ptr_config);
esp_err_t mjd_lorap2p_deinit(mjd_lorap2p_config_t* param_ptr_config);
esp_err_t mjd_lorap2p_tx(mjd_lorap2p_config_t* param_ptr_config,
                         const mjd_lorap2p_data_frame_input_t *param_ptr_data_frame_input);

#ifdef __cplusplus
}
#endif

#endif /* __MJD_LORAP2P_H__ */
