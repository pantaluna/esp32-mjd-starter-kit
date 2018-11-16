/*
 * Goto README.md for instructions
 */
#ifndef __MJD_LEDRGB_H__
#define __MJD_LEDRGB_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * RMT PARAMS
 *
 * @doc RMT source clock = APB Clock 80Mhz.
 *      It ticks 80,000,000 times a second or 80,000 times a MILLIsecond or 80 times a MICROsecond or 0.08 times a NANOsecond.
 * @source my_spreadsheet
 *      APB Clock Divider: 4 => Nanoseconds per Tick: 50.0ns
 *
 */
#define MY_RMT_CLK_DIV (4)
#define MY_RMT_ONE_TICK_DURATION_NANOSEC (50.0)

/**
 * Data Definitions
 */

#define MJD_LEVEL_HIGH (1)
#define MJD_LEVEL_LOW  (0)

/**
 * @brief The list of LED Strip identifications that can be used in the app.
 */
typedef enum {
    MJD_STRIP_1 = 0,
    MJD_STRIP_2,
    MJD_STRIP_3,
    MJD_STRIP_4,
    MJD_STRIP_5,
    MJD_STRIP_6,
    MJD_STRIP_7,
    MJD_STRIP_8,
    MJD_STRIP_IDENTIFIER_MAX
} mjd_ledrgb_strip_identifier_t;


/**
 * @brief The list of each LED number of the LED strip/board.
 */
typedef enum {
    MJD_LED_1 = 0,
    MJD_LED_2,
    MJD_LED_3,
    MJD_LED_4,
    MJD_LED_5,
    MJD_LED_6,
    MJD_LED_7,
    MJD_LED_8,
    MJD_LED_9,
    MJD_LED_10,
    MJD_LED_11,
    MJD_LED_12,
    MJD_LED_13,
    MJD_LED_14,
    MJD_LED_15,
    MJD_LED_16,
    MJD_LED_17,
    MJD_LED_18,
    MJD_LED_19,
    MJD_LED_20,
    MJD_LED_21,
    MJD_LED_22,
    MJD_LED_23,
    MJD_LED_24,
    MJD_LED_25,
    MJD_LED_26,
    MJD_LED_27,
    MJD_LED_28,
    MJD_LED_29,
    MJD_LED_30,
    MJD_LED_31,
    MJD_LED_32,
    MJD_LED_33,
    MJD_LED_34,
    MJD_LED_35,
    MJD_LED_36,
    MJD_LED_37,
    MJD_LED_38,
    MJD_LED_39,
    MJD_LED_40,
    MJD_LED_41,
    MJD_LED_42,
    MJD_LED_43,
    MJD_LED_44,
    MJD_LED_45,
    MJD_LED_46,
    MJD_LED_47,
    MJD_LED_48,
    MJD_LED_49,
    MJD_LED_50,
    MJD_LED_51,
    MJD_LED_52,
    MJD_LED_53,
    MJD_LED_54,
    MJD_LED_55,
    MJD_LED_56,
    MJD_LED_57,
    MJD_LED_58,
    MJD_LED_59,
    MJD_LED_60,
    MJD_LED_61,
    MJD_LED_62,
    MJD_LED_63,
    MJD_LED_64,
    MJD_LED_IDENTIFIER_MAX
} mjd_ledrgb_led_identifier_t;

/**
 * @brief The list of color sequence protocols that must be used in the data protocol for a specific a LED strip/board.
 */
typedef enum {
    MJD_COLOR_SEQUENCE_GRB, // WS2812B
    MJD_COLOR_SEQUENCE_RGB, // TODO For future supported RGB LED chips
    MJD_COLOR_SEQUENCE_MAX
} mjd_ledrgb_color_sequence_t;

/**
 * @brief The specifics of a LED Strip/Board.
 *        This includes the color sequence protocol, the number of leds, and the timings of the data protocol
 */
typedef struct {
        mjd_ledrgb_color_sequence_t color_sequence;
        uint32_t T1high;
        uint32_t T1low;
        uint32_t T0high;
        uint32_t T0low;
        uint32_t Treset;

} mjd_ledrgb_strip_characteristics_t;

/**
 * @brief The list of supported LED Strips/Boards and their specific timings.
 *      See README for more details about the different packages.
 */
typedef enum {
    MJD_LED_TYPE_WS2812_V2016 = 0,      /*!< The WS2812 is the predecessor of the WS2812B */
    MJD_LED_TYPE_WS2812B_V2017, /*!< The most popular RGB LED package in 2017+ */
    MJD_LED_TYPE_WS2813_V2017, /*!< The WS2813 is the successor of the WS2812B (same timings, with an extra Backup Data In pin) */
    MJD_LED_TYPE_TEST,
    MJD_LED_TYPES_MAX
} mjd_ledrgb_led_type_t;

/**
 * @brief The list of RMT Data Pulses to support the NRZ protocol of the LED board.
 *        Each pulse defines a High and Low pulse for a bit value of 0 and 1.
 *        An extra pulse is added to mark the end of a RMT Transmission.
 */
typedef enum {
    MJD_RMT_PULSE_TYPE_BITVALUE_ZERO = 0,
    MJD_RMT_PULSE_TYPE_BITVALUE_ONE,
    MJD_RMT_PULSE_TYPE_RESET,
    MJD_RMT_PULSE_TYPE_END_MARKER,
    MJD_RMT_PULSE_TYPE_MAX
} mjd_ledrgb_rmt_pulse_type_t;

/**
 * @brief The color properties of a LED of a LED Strip/Board.
 *        TODO Support RGBW 4-color LED Strips as well
 */
typedef struct {
        uint8_t r, g, b;
} mjd_ledrgb_led_t;

/**
 * @brief A Helper func to create a led object
 */
inline mjd_ledrgb_led_t mjd_led_rgb(uint8_t r, uint8_t g, uint8_t b) {
    mjd_ledrgb_led_t led;
    led.r = r;
    led.g = g;
    led.b = b;
    return led;
}

/**
 * @brief Helpers for common RGB colors
 */
#define MJD_RGB_WHITE (mjd_led_rgb(255, 255, 255))
#define MJD_RGB_RED (mjd_led_rgb(255, 0, 0))
#define MJD_RGB_GREEN (mjd_led_rgb(0, 255, 0))
#define MJD_RGB_BLUE (mjd_led_rgb(0, 0, 255))
#define MJD_RGB_BLACK (mjd_led_rgb(0, 0, 255))

/**
 * @brief The properties of a LED configuration
 *        It is filled in by the app and send to mjd_ledrgb_init()
 */
typedef struct {
        mjd_ledrgb_strip_identifier_t strip_id; /*!< The unique ID of a LED Strip */
        gpio_num_t gpio_num; /*!< The GPIO NUM of the MCU that is wired to the LED Strip's DIN pad */
        mjd_ledrgb_led_type_t led_type; /*!< The LED Type */
        uint32_t nbr_of_leds; /*!< The number of LED chips that are integrated in this LED Strip. */
        uint8_t relative_brightness; /*!< A percentage that is applied to all LED color values before sending the pixels to the LED Strip. 25% = good value for indoors */
} mjd_ledrgb_config_t;

#define MJD_LEDRGB_CONFIG_DEFAULT() { \
    .relative_brightness = 25 \
}

typedef struct {
        // Public
        mjd_ledrgb_strip_identifier_t strip_id; /*!< The unique ID of a LED Strip */
        gpio_num_t gpio_num; /*!< The GPIO NUM of the MCU that is wired to the LED Strip's DIN pad */
        mjd_ledrgb_led_type_t led_type; /*!< The LED Type */
        uint32_t nbr_of_leds; /*!< The number of LED chips that are integrated in this LED Strip */
        uint8_t relative_brightness; /*!< A percentage that is applied to all LED color values before sending the pixels to the LED Strip. 25% = good value for indoors */
        // Private
        bool is_init;
        mjd_ledrgb_color_sequence_t color_sequence; /*!< Which color sequence protocol is used. @source led_type */
        uint8_t nbr_of_colors; /*!< The number of colors that are integrated in each LED chip of this LED Strip. @source led_type */
        mjd_ledrgb_led_t *leds; /*!< The actual LED color values */
        uint32_t len_transformation_buffer; /*!< 32bit! The length of the transformation buffer */
        uint8_t *transformation_buffer; /*!< The buffer to transform the input color values to the device-specific values. It also applies the relative brightness */
        rmt_channel_t rmt_channel; /*!< The RMT Channel to be used. @limitation Cannot use the same RMT Channel for multiple LED strips */
        rmt_item32_t rmt_item_pulse_pairs[MJD_RMT_PULSE_TYPE_MAX]; /*!< The elementary RMT Transmission pulses that are computed for this specific LED Strip */
        uint32_t nbr_of_rmt_tx_items; /*!< 32bit! The number of RMT Transmission Items to be sent to the LED Strip */
        rmt_item32_t *rmt_tx_items;/*!< The list of RMT Transmission Items to be sent to the LED Strip */
} mjd_ledrgb_strip_t;

/**
 * Function declarations
 */

/**
 * @brief Initialize a specific LED Strip
 *        Keep the strip_id for future reference
 *        It will also configure the ESP-IDF RMT component
 *
 * @param ptr_param_config A pointer to the config structure
 *
 * @return
 *     - ESP_OK Success
 */
esp_err_t mjd_ledrgb_init(mjd_ledrgb_config_t* ptr_param_config);

/**
 * @brief DeInitialize a specific LED Strip
 *
 * @param param_strip_id A reference to the LED Strip as used in mjd_ledrgb_init()
 *
 * @return
 *     - ESP_OK Success
 */
esp_err_t mjd_ledrgb_deinit(mjd_ledrgb_strip_identifier_t param_strip_id);

/**
 * @brief Get the number of leds in this strip
 *
 * @param param_strip_id A reference to the LED Strip as used in mjd_ledrgb_init()
 *
 * @return
 *     - ESP_OK Success
 */
esp_err_t mjd_ledrgb_get_strip_nbr_of_leds(mjd_ledrgb_strip_identifier_t param_strip_id, uint32_t * param_ptr_nbr_of_leds);

/**
 * @brief Send the pixels to the LED Strip
 *        The actual color values for all LEDS in the Strip structure are transmitted.
 *
 * @param param_strip_id A reference to the LED Strip as used in mjd_ledrgb_init()
 *
 * @return
 *     - ESP_OK Success
 */
esp_err_t mjd_ledrgb_send_pixels_to_strip(mjd_ledrgb_strip_identifier_t param_strip_id);

/**
 * @brief A helper func to reset the pixels of each LED of the LED Strip to zero (white).
 *
 * @param param_strip_id A reference to the LED Strip as used in mjd_ledrgb_init()
 *
 * @return
 */
esp_err_t mjd_ledrgb_reset_strip(mjd_ledrgb_strip_identifier_t param_strip_id);

/**
 * @brief Set the color value of a specific LED of a LED Strip
 *
 * @param param_strip_id              The ID of the LED Strip as used in mjd_ledrgb_init()
 * @param param_led_id                The ID of a LED chip
 * @param param_led_color             The color values to send
 * @param param_send_pixels_to_strip  Y/N Send the pixels in the buffer to the LED Strip. This is a speed optimization feature
 *
 * @return
 *     - ESP_OK Success
 */
esp_err_t mjd_ledrgb_set_strip_led(mjd_ledrgb_strip_identifier_t param_strip_id, mjd_ledrgb_led_identifier_t param_led_id,
                              mjd_ledrgb_led_t param_led_color, bool param_send_pixels_to_strip);

/**
 * @brief Get the color value of a specific LED of a LED Strip
 *
 * @param param_strip_id              The ID of the LED Strip as used in mjd_ledrgb_init()
 * @param param_led_id                The ID of a LED chip
 *
 * @return
 *     - ESP_OK Success
 */
mjd_ledrgb_led_t mjd_ledrgb_get_strip_led(mjd_ledrgb_strip_identifier_t param_strip_id,
                                          mjd_ledrgb_led_identifier_t param_led_id);

/**
 * @brief A helper func to assign the same color to all LEDs of a LED Strip
 *
 * @param param_strip_id              The ID of the LED Strip as used in mjd_ledrgb_init()
 * @param param_led_id                The ID of a LED chip
 *
 * @return
 *     - ...
 */
esp_err_t mjd_ledrgb_set_strip_leds_one_color(mjd_ledrgb_strip_identifier_t param_strip_id, mjd_ledrgb_led_t param_led_color);

/**
 * @brief A helper func to help identify the position of each LED of a LED Strip.
 *        This is very useful when the LEDs are aligned in a matrix and you wonder where each LED is mounted on the LED Strip (or LED board).
 *
 * @param param_strip_id              The ID of the LED Strip as used in mjd_ledrgb_init()
 *
 * @return
 *     - ESP_OK Success
 */
esp_err_t mjd_ledrgb_identify_led_positions(mjd_ledrgb_strip_identifier_t param_strip_id);


#ifdef __cplusplus
}
#endif

#endif /* __MJD_LEDRGB_H__ */
