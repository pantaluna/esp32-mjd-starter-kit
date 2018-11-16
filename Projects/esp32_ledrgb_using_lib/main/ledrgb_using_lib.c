#include "mjd.h"
#include "mjd_ledrgb.h"

/*
 * Logging
 */
static const char TAG[] = "myapp";

/*
 * KConfig: GPIO
 */
// default 14
#define MY_1WIRE_GPIO_NUM (CONFIG_MY_1WIRE_GPIO_NUM)

/*
 * FreeRTOS settings
 */
#define MYAPP_RTOS_TASK_STACK_SIZE_LARGE (8192)
#define MYAPP_RTOS_TASK_PRIORITY_NORMAL (RTOS_TASK_PRIORITY_NORMAL)

/*
 * FUNCS
 */

void do_strip_sequences() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    /*************************************************************************
     * INIT: assign strip id, nbr of leds to process
     */
    ESP_LOGI(TAG, "INIT: assign strip id");
    uint8_t strip_id = MJD_STRIP_1; // @doc logical range [1..8]) | numerical range [0..7])

    /*************************************************************************
     * All leds := WHITE
     *      @range 0x00..0xFF (always considering brightness)
     *
     */
    ESP_LOGI(TAG, "All leds := WHITE 10 seconds");
    ESP_LOGI(TAG, "  @tip Now is a good time to measure the power consumption, and the stability of the LED's");

    mjd_ledrgb_set_strip_leds_one_color(strip_id, mjd_led_rgb(255, 255, 255)); // NICE: 20,20,20
    vTaskDelay(RTOS_DELAY_10SEC);

    // DEVTEMP: HALT
    /////mjd_rtos_wait_forever();

    mjd_ledrgb_reset_strip(strip_id);
    vTaskDelay(RTOS_DELAY_1SEC);

    /*************************************************************************
     * Identify position of each led on the board
     *
     */
    ESP_LOGI(TAG, "Identify position of each LED on the board/strip");

    mjd_ledrgb_identify_led_positions(strip_id);
    vTaskDelay(RTOS_DELAY_1SEC);

    // DEVTEMP: HALT
    /////mjd_rtos_wait_forever();

    mjd_ledrgb_reset_strip(strip_id);
    vTaskDelay(RTOS_DELAY_1SEC);

    /*************************************************************************
     * All leds := RED
     *      @range 0x00..0xFF (always considering brightness)
     *
     */

    ESP_LOGI(TAG, "All leds := RED");
    ESP_LOGI(TAG, "  @tip Check all LED's are RED");

    mjd_ledrgb_set_strip_leds_one_color(strip_id, mjd_led_rgb(255, 0, 0));
    vTaskDelay(RTOS_DELAY_5SEC);

    // DEVTEMP: HALT
    /////mjd_rtos_wait_forever();

    mjd_ledrgb_reset_strip(strip_id);
    vTaskDelay(RTOS_DELAY_1SEC);

    /*************************************************************************
     * Gradually all LED's := RED
     *   @doc color values 0x00..0xFF (always considering brightness)
     *
     */

    ESP_LOGI(TAG, "Gradually set all LED's := RED");
    {
        uint32_t nbr_of_leds = 0;
        mjd_ledrgb_get_strip_nbr_of_leds(strip_id, &nbr_of_leds);
        ESP_LOGI(TAG, "  nbr_of_leds %u", nbr_of_leds);

        for (uint32_t counter = 1; counter <= 2; --counter) {
            for (uint32_t led_id = 0; led_id < nbr_of_leds; ++led_id) {
                mjd_ledrgb_set_strip_led(strip_id, led_id, MJD_RGB_RED, true);
                vTaskDelay(RTOS_DELAY_50MILLISEC);
            }
            mjd_ledrgb_reset_strip(strip_id);
        }

    }

    // DEVTEMP: HALT
    /////mjd_rtos_wait_forever();

    mjd_ledrgb_reset_strip(strip_id);
    vTaskDelay(RTOS_DELAY_1SEC);

    /*************************************************************************
     * All leds := GREEN
     *      @range 0x00..0xFF (always considering brightness)
     *
     */
    ESP_LOGI(TAG, "All leds := GREEN");
    ESP_LOGI(TAG, "  @tip Check all LED's are GREEN");

    mjd_ledrgb_set_strip_leds_one_color(strip_id, mjd_led_rgb(0, 255, 0));
    vTaskDelay(RTOS_DELAY_5SEC);

    // DEVTEMP: HALT
    /////mjd_rtos_wait_forever();

    mjd_ledrgb_reset_strip(strip_id);
    vTaskDelay(RTOS_DELAY_1SEC);

    /*************************************************************************
     * Gradually all LED's := GREEN
     *   @doc color values 0x00..0xFF (always considering brightness)
     *
     */

    ESP_LOGI(TAG, "Gradually set all LED's := GREEN");
    {
        uint32_t nbr_of_leds = 0;
        mjd_ledrgb_get_strip_nbr_of_leds(strip_id, &nbr_of_leds);
        ESP_LOGI(TAG, "  nbr_of_leds %u", nbr_of_leds);

        for (uint32_t counter = 1; counter <= 2; --counter) {
            for (uint32_t led_id = 0; led_id < nbr_of_leds; ++led_id) {
                mjd_ledrgb_set_strip_led(strip_id, led_id, MJD_RGB_GREEN, true);
                vTaskDelay(RTOS_DELAY_50MILLISEC);
            }
            mjd_ledrgb_reset_strip(strip_id);
        }

    }

    // DEVTEMP: HALT
    /////mjd_rtos_wait_forever();

    mjd_ledrgb_reset_strip(strip_id);
    vTaskDelay(RTOS_DELAY_1SEC);

    /*************************************************************************
     * All leds := BLUE
     *      @range 0x00..0xFF (always considering brightness)
     *
     */
    ESP_LOGI(TAG, "All leds := BLUE");
    ESP_LOGI(TAG, "  @tip Check all LED's are BLUE");

    mjd_ledrgb_set_strip_leds_one_color(strip_id, mjd_led_rgb(0, 0, 255));
    vTaskDelay(RTOS_DELAY_5SEC);

    // DEVTEMP: HALT
    /////mjd_rtos_wait_forever();

    mjd_ledrgb_reset_strip(strip_id);
    vTaskDelay(RTOS_DELAY_1SEC);

    /*************************************************************************
     * Gradually all LED's := BLUE
     *   @doc color values 0x00..0xFF (always considering brightness)
     *
     */

    ESP_LOGI(TAG, "Gradually set all LED's := BLUE");
    {
        uint32_t nbr_of_leds = 0;
        mjd_ledrgb_get_strip_nbr_of_leds(strip_id, &nbr_of_leds);
        ESP_LOGI(TAG, "  nbr_of_leds %u", nbr_of_leds);

        for (uint32_t counter = 1; counter <= 2; --counter) {
            for (uint32_t led_id = 0; led_id < nbr_of_leds; ++led_id) {
                mjd_ledrgb_set_strip_led(strip_id, led_id, MJD_RGB_BLUE, true);
                vTaskDelay(RTOS_DELAY_50MILLISEC);
            }
            mjd_ledrgb_reset_strip(strip_id);
        }

    }

    // DEVTEMP: HALT
    /////mjd_rtos_wait_forever();

    mjd_ledrgb_reset_strip(strip_id);
    vTaskDelay(RTOS_DELAY_1SEC);

    /*************************************************************************
     * Set led1=red led2=green led3=blue led4=white
     *
     * param4 false: do not yet send the pixels to the strip (=speed optimization)
     * param4 true:  send pixels to strip right now
     */

    ESP_LOGI(TAG, "Set LED#1=red LED#2=green LED#3=blue LED#4=white 5 seconds");

    mjd_ledrgb_set_strip_led(strip_id, MJD_LED_1, MJD_RGB_RED, false);
    mjd_ledrgb_set_strip_led(strip_id, MJD_LED_2, MJD_RGB_GREEN, false);
    mjd_ledrgb_set_strip_led(strip_id, MJD_LED_3, MJD_RGB_BLUE, false);
    mjd_ledrgb_set_strip_led(strip_id, MJD_LED_4, MJD_RGB_WHITE, true);
    vTaskDelay(RTOS_DELAY_5SEC);

    mjd_ledrgb_reset_strip(strip_id);
    vTaskDelay(RTOS_DELAY_1SEC);

    /*************************************************************************
     * LED#1-4: Rotate
     *      @range 0x00..0xFF (always considering brightness)
     *
     */

    ESP_LOGI(TAG, "LED#1-4: Rotate");
    {
        mjd_ledrgb_set_strip_led(strip_id, MJD_LED_1, MJD_RGB_RED, false);
        mjd_ledrgb_set_strip_led(strip_id, MJD_LED_2, MJD_RGB_GREEN, false);
        mjd_ledrgb_set_strip_led(strip_id, MJD_LED_3, MJD_RGB_BLUE, false);
        mjd_ledrgb_set_strip_led(strip_id, MJD_LED_4, MJD_RGB_WHITE, false);
        mjd_ledrgb_send_pixels_to_strip(strip_id);

        for (uint32_t counter = 1; counter <= 100; ++counter) {
            mjd_ledrgb_led_t save_led = mjd_ledrgb_get_strip_led(strip_id, MJD_LED_4);
            mjd_ledrgb_set_strip_led(strip_id, MJD_LED_4, mjd_ledrgb_get_strip_led(strip_id, MJD_LED_3), false);
            mjd_ledrgb_set_strip_led(strip_id, MJD_LED_3, mjd_ledrgb_get_strip_led(strip_id, MJD_LED_2), false);
            mjd_ledrgb_set_strip_led(strip_id, MJD_LED_2, mjd_ledrgb_get_strip_led(strip_id, MJD_LED_1), false);
            mjd_ledrgb_set_strip_led(strip_id, MJD_LED_1, save_led, false);

            mjd_ledrgb_send_pixels_to_strip(strip_id);
            vTaskDelay(RTOS_DELAY_75MILLISEC);
        }
    }

    mjd_ledrgb_reset_strip(strip_id);
    vTaskDelay(RTOS_DELAY_1SEC);

    /*************************************************************************
     * Set color of specific LED's
     *   @doc color values 0x00..0xFF (always considering brightness)
     *
     */

    ESP_LOGI(TAG, "Set color of specific LED's (LED#s: 1-4)");
    {
        for (uint32_t counter = 0; counter < 2; --counter) {
            mjd_ledrgb_set_strip_led(strip_id, MJD_LED_1, mjd_led_rgb(0, 128, 128), true); // Teal #008080 (0,128,128)
            vTaskDelay(RTOS_DELAY_1SEC);

            mjd_ledrgb_set_strip_led(strip_id, MJD_LED_3, mjd_led_rgb(128, 128, 0), true); // Olive #808000 (128,128,0)
            vTaskDelay(RTOS_DELAY_1SEC);

            mjd_ledrgb_set_strip_led(strip_id, MJD_LED_2, mjd_led_rgb(112, 128, 144), true); //slate gray #708090 (112,128,144) | Purple #800080 (128,0,128)
            vTaskDelay(RTOS_DELAY_1SEC);

            mjd_ledrgb_set_strip_led(strip_id, MJD_LED_4, mjd_led_rgb(128, 0, 0), true); // Maroon #800000 (128,0,0)
            vTaskDelay(RTOS_DELAY_1SEC);

            mjd_ledrgb_reset_strip(strip_id);
            vTaskDelay(RTOS_DELAY_1SEC);
        }
    }

    /*************************************************************************
     * ROTATE all LEDS slowly
     *   @doc color values 0x00..0xFF (always considering brightness)
     *
     */

    ESP_LOGI(TAG, "ROTATE all LEDS slowly");
    {
        uint32_t nbr_of_leds = 0;
        mjd_ledrgb_get_strip_nbr_of_leds(strip_id, &nbr_of_leds);
        ESP_LOGI(TAG, "  nbr_of_leds %u", nbr_of_leds);

        for (uint32_t led_id = 0; led_id < nbr_of_leds; ++led_id) {
            uint32_t modulus = led_id % 8;
            if (modulus == 0) {
                mjd_ledrgb_set_strip_led(strip_id, led_id, MJD_RGB_RED, false);
            }
            if (modulus == 1) {
                mjd_ledrgb_set_strip_led(strip_id, led_id, MJD_RGB_RED, false);
            }
            if (modulus == 2) {
                mjd_ledrgb_set_strip_led(strip_id, led_id, MJD_RGB_GREEN, false);
            }
            if (modulus == 3) {
                mjd_ledrgb_set_strip_led(strip_id, led_id, MJD_RGB_GREEN, false);
            }
            if (modulus == 4) {
                mjd_ledrgb_set_strip_led(strip_id, led_id, MJD_RGB_BLUE, false);
            }
            if (modulus == 5) {
                mjd_ledrgb_set_strip_led(strip_id, led_id, MJD_RGB_BLUE, false);
            }
            if (modulus == 6) {
                mjd_ledrgb_set_strip_led(strip_id, led_id, MJD_RGB_WHITE, false);
            }
            if (modulus == 7) {
                mjd_ledrgb_set_strip_led(strip_id, led_id, MJD_RGB_BLACK, false);
            }
        }
        mjd_ledrgb_send_pixels_to_strip(strip_id);

        for (uint32_t counter = 1; counter <= 25; ++counter) {
            mjd_ledrgb_led_t saved_last_led = mjd_ledrgb_get_strip_led(strip_id, nbr_of_leds - 1);

            for (uint32_t led_id = nbr_of_leds - 1; led_id > 0; --led_id) {
                mjd_ledrgb_set_strip_led(strip_id, led_id, mjd_ledrgb_get_strip_led(strip_id, led_id - 1), false);
            }
            mjd_ledrgb_set_strip_led(strip_id, 0, saved_last_led, false);

            mjd_ledrgb_send_pixels_to_strip(strip_id);
            vTaskDelay(RTOS_DELAY_250MILLISEC);
        }

        mjd_ledrgb_reset_strip(strip_id);
        vTaskDelay(RTOS_DELAY_1SEC);
    }

    /*************************************************************************
     * ROTATE all LEDS extremely quickly (no more visible to the human eye)
     *   @doc color values 0x00..0xFF (always considering brightness)
     *
     */

    ESP_LOGI(TAG, "ROTATE all LEDS extremely quickly (no more visible to the human eye)");
    {
        uint32_t nbr_of_leds = 0;
        mjd_ledrgb_get_strip_nbr_of_leds(strip_id, &nbr_of_leds);
        ESP_LOGI(TAG, "  nbr_of_leds %u", nbr_of_leds);

        for (uint32_t led_id = 0; led_id < nbr_of_leds; ++led_id) {
            uint32_t modulus = led_id % 8;
            if (modulus == 0) {
                mjd_ledrgb_set_strip_led(strip_id, led_id, MJD_RGB_RED, false);
            }
            if (modulus == 1) {
                mjd_ledrgb_set_strip_led(strip_id, led_id, MJD_RGB_RED, false);
            }
            if (modulus == 2) {
                mjd_ledrgb_set_strip_led(strip_id, led_id, MJD_RGB_GREEN, false);
            }
            if (modulus == 3) {
                mjd_ledrgb_set_strip_led(strip_id, led_id, MJD_RGB_GREEN, false);
            }
            if (modulus == 4) {
                mjd_ledrgb_set_strip_led(strip_id, led_id, MJD_RGB_BLUE, false);
            }
            if (modulus == 5) {
                mjd_ledrgb_set_strip_led(strip_id, led_id, MJD_RGB_BLUE, false);
            }
            if (modulus == 6) {
                mjd_ledrgb_set_strip_led(strip_id, led_id, MJD_RGB_WHITE, false);
            }
            if (modulus == 7) {
                mjd_ledrgb_set_strip_led(strip_id, led_id, MJD_RGB_BLACK, false);
            }
        }
        mjd_ledrgb_send_pixels_to_strip(strip_id);

        for (uint32_t counter = 1; counter <= 750; ++counter) {
            mjd_ledrgb_led_t saved_last_led = mjd_ledrgb_get_strip_led(strip_id, nbr_of_leds - 1);

            for (uint32_t led_id = nbr_of_leds - 1; led_id > 0; --led_id) {
                mjd_ledrgb_set_strip_led(strip_id, led_id, mjd_ledrgb_get_strip_led(strip_id, led_id - 1), false);
            }
            mjd_ledrgb_set_strip_led(strip_id, 0, saved_last_led, false);

            mjd_ledrgb_send_pixels_to_strip(strip_id);
            vTaskDelay(RTOS_DELAY_10MILLISEC);
        }

        mjd_ledrgb_reset_strip(strip_id);
        vTaskDelay(RTOS_DELAY_1SEC);
    }

    /*************************************************************************
     * ROTATE all LEDS quickly (but still visible to the human eye)
     *   @doc color values 0x00..0xFF (always considering brightness)
     *
     */

    ESP_LOGI(TAG, "ROTATE all LEDS quickly (but still visible to the human eye)");
    {
        uint32_t nbr_of_leds = 0;
        mjd_ledrgb_get_strip_nbr_of_leds(strip_id, &nbr_of_leds);
        ESP_LOGI(TAG, "  nbr_of_leds %u", nbr_of_leds);

        for (uint32_t led_id = 0; led_id < nbr_of_leds; ++led_id) {
            uint32_t modulus = led_id % 8;
            if (modulus == 0) {
                mjd_ledrgb_set_strip_led(strip_id, led_id, MJD_RGB_RED, false);
            }
            if (modulus == 1) {
                mjd_ledrgb_set_strip_led(strip_id, led_id, MJD_RGB_RED, false);
            }
            if (modulus == 2) {
                mjd_ledrgb_set_strip_led(strip_id, led_id, MJD_RGB_GREEN, false);
            }
            if (modulus == 3) {
                mjd_ledrgb_set_strip_led(strip_id, led_id, MJD_RGB_GREEN, false);
            }
            if (modulus == 4) {
                mjd_ledrgb_set_strip_led(strip_id, led_id, MJD_RGB_BLUE, false);
            }
            if (modulus == 5) {
                mjd_ledrgb_set_strip_led(strip_id, led_id, MJD_RGB_BLUE, false);
            }
            if (modulus == 6) {
                mjd_ledrgb_set_strip_led(strip_id, led_id, MJD_RGB_WHITE, false);
            }
            if (modulus == 7) {
                mjd_ledrgb_set_strip_led(strip_id, led_id, MJD_RGB_BLACK, false);
            }
        }
        mjd_ledrgb_send_pixels_to_strip(strip_id);

        for (uint32_t counter = 1; counter <= 375; ++counter) {
            mjd_ledrgb_led_t saved_last_led = mjd_ledrgb_get_strip_led(strip_id, nbr_of_leds - 1);

            for (uint32_t led_id = nbr_of_leds - 1; led_id > 0; --led_id) {
                mjd_ledrgb_set_strip_led(strip_id, led_id, mjd_ledrgb_get_strip_led(strip_id, led_id - 1), false);
            }
            mjd_ledrgb_set_strip_led(strip_id, 0, saved_last_led, false);

            mjd_ledrgb_send_pixels_to_strip(strip_id);
            vTaskDelay(RTOS_DELAY_50MILLISEC);
        }

        mjd_ledrgb_reset_strip(strip_id);
        vTaskDelay(RTOS_DELAY_1SEC);
    }

    /*************************************************************************
     * Pulse Higher and Higher
     *      @range 0x00..0xFF (always considering brightness)
     *
     */

    ESP_LOGI(TAG, "Pulse Higher and Higher");
    {
        uint32_t nbr_of_leds = 0;
        mjd_ledrgb_get_strip_nbr_of_leds(strip_id, &nbr_of_leds);
        ESP_LOGI(TAG, "  nbr_of_leds %u", nbr_of_leds);

        for (uint16_t color_value = 0; color_value <= 255; ++color_value) {

            for (uint32_t led_id = 0; led_id < nbr_of_leds; ++led_id) {
                uint32_t modulus = led_id % 4;
                if (modulus == 0) {
                    mjd_ledrgb_set_strip_led(strip_id, led_id, mjd_led_rgb(color_value, 0, 0), false);
                }
                if (modulus == 1) {
                    mjd_ledrgb_set_strip_led(strip_id, led_id, mjd_led_rgb(0, color_value, 0), false);
                }
                if (modulus == 2) {
                    mjd_ledrgb_set_strip_led(strip_id, led_id, mjd_led_rgb(0, 0, color_value), false);
                }
                if (modulus == 3) {
                    mjd_ledrgb_set_strip_led(strip_id, led_id, mjd_led_rgb(color_value, color_value, color_value), false);
                }
                mjd_ledrgb_send_pixels_to_strip(strip_id);
            }

            vTaskDelay(RTOS_DELAY_10MILLISEC);
        }

    }

    mjd_ledrgb_reset_strip(strip_id);
    vTaskDelay(RTOS_DELAY_1SEC);

    // DEVTEMP: HALT
    /////mjd_rtos_wait_forever();

    /*************************************************************************
     * Pulse Lower and Lower
     *      @range 0x00..0xFF (always considering brightness)
     *
     */

    ESP_LOGI(TAG, "Pulse Lower and Lower");
    {
        uint32_t nbr_of_leds = 0;
        mjd_ledrgb_get_strip_nbr_of_leds(strip_id, &nbr_of_leds);
        ESP_LOGI(TAG, "  nbr_of_leds %u", nbr_of_leds);

        for (uint16_t color_value = 255; color_value > 0; --color_value) {

            for (uint32_t led_id = 0; led_id < nbr_of_leds; ++led_id) {
                uint32_t modulus = led_id % 4;
                if (modulus == 0) {
                    mjd_ledrgb_set_strip_led(strip_id, led_id, mjd_led_rgb(color_value, 0, 0), false);
                }
                if (modulus == 1) {
                    mjd_ledrgb_set_strip_led(strip_id, led_id, mjd_led_rgb(0, color_value, 0), false);
                }
                if (modulus == 2) {
                    mjd_ledrgb_set_strip_led(strip_id, led_id, mjd_led_rgb(0, 0, color_value), false);
                }
                if (modulus == 3) {
                    mjd_ledrgb_set_strip_led(strip_id, led_id, mjd_led_rgb(color_value, color_value, color_value), false);
                }
                mjd_ledrgb_send_pixels_to_strip(strip_id);
            }

            vTaskDelay(RTOS_DELAY_10MILLISEC);
        }

    }

    mjd_ledrgb_reset_strip(strip_id);
    vTaskDelay(RTOS_DELAY_1SEC);

    // DEVTEMP: HALT
    /////mjd_rtos_wait_forever();

    /*************************************************************************
     * Gradually shift colors
     *      @range 0x00..0xFF (always considering brightness)
     *
     * mjd_ledrgb_led_t saved_last_led = mjd_ledrgb_get_strip_led(strip_id, nbr_of_leds - 1);
     */

    ESP_LOGI(TAG, "Gradually shift colors");
    {
        uint32_t nbr_of_leds = 0;
        uint32_t i_section;
        uint8_t red, green, blue;

        mjd_ledrgb_get_strip_nbr_of_leds(strip_id, &nbr_of_leds);
        ESP_LOGI(TAG, "  nbr_of_leds %u", nbr_of_leds);

        for (uint32_t cycle = 0; cycle <= 255; ++cycle) {

            for (uint32_t led_id = 0; led_id < nbr_of_leds; ++led_id) {
                float f_section = (float) led_id / nbr_of_leds;
                if (f_section < 0.33) {
                    i_section = 1;
                    red = 0;
                    green = ((cycle + led_id + i_section) * 3) & 255;
                    blue = (i_section * 3) & 255;
                 } else if (f_section < 0.66) {
                    i_section = 2;
                    red = (i_section * 3) & 255;
                    green = 0;
                    blue =  ((cycle + led_id + i_section) * 3) & 255;
                } else {
                    i_section = 3;
                    red =  ((cycle + led_id + i_section) * 3) & 255;
                    green = (i_section * 3) & 255;
                    blue = 0;
                }
                mjd_ledrgb_set_strip_led(strip_id, led_id, mjd_led_rgb(red, green, blue), true);
            }
            vTaskDelay(RTOS_DELAY_10MILLISEC);
        }

    }

    mjd_ledrgb_reset_strip(strip_id);
    vTaskDelay(RTOS_DELAY_1SEC);

    // DEVTEMP: HALT
    /////mjd_rtos_wait_forever();

    /**********
     * END
     */
    ESP_LOGD(TAG, "END %s()", __FUNCTION__);
}

/*
 * TASKS
 */
void main_task(void *pvParameter) {

    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    mjd_log_memory_statistics();

    mjd_ledrgb_config_t config = MJD_LEDRGB_CONFIG_DEFAULT();

    /*
     * Guidelines:
     * - You can control up to 8 different LED boards/strips simultaneously. This app only uses MJD_STRIP_1.
     * - nbr_of_leds : Max +-20 LED's at 100% brightness when using a 3.3V data signal and 3.3V power line from the MCU (no auxiliary power line).
     * - nbr_of_leds : You do not have to use all LED's of the LED board or the LED strip.
     * - relative_brightness :=20 (%) is a good value for indoors and development.
     */

/*
    ESP_LOGI(TAG, "Product: CJMCU 4 Bit WS2812 5050 RGB LED Driver Development Board");
    config.strip_id = MJD_STRIP_1;
    config.led_type = MJD_LED_TYPE_WS2812B_V2017; // MJD_LED_TYPE_WS2813_V2017 MJD_LED_TYPE_WS2812B_V2017 MJD_LED_TYPE_WS2812_V2016;
    config.nbr_of_leds = 4; // 1..4..8..16..30..60..120
    config.relative_brightness = 50;
    config.gpio_num = MY_1WIRE_GPIO_NUM;
    mjd_ledrgb_init(&config);
*/

    ESP_LOGI(TAG, "Product: CJMCU 4x4 Bit WS2812 5050 RGB LED Driver Development Board");
    config.strip_id = MJD_STRIP_1;
    config.led_type = MJD_LED_TYPE_WS2812B_V2017; // MJD_LED_TYPE_WS2813_V2017 MJD_LED_TYPE_WS2812B_V2017 MJD_LED_TYPE_WS2812_V2016;
    config.nbr_of_leds = 16; // 1..4..8..16..30..60..120
    config.relative_brightness = 50;
    config.gpio_num = MY_1WIRE_GPIO_NUM;
    mjd_ledrgb_init(&config);

/*
    ESP_LOGI(TAG, "Product: BTF-LIGHTING WS2813 led pixel strip, DC 5V, length 1m, 30 pixels/strip/m, IP30");
    config.strip_id = MJD_STRIP_1;
    config.led_type = MJD_LED_TYPE_WS2813_V2017; // MJD_LED_TYPE_WS2813_V2017 MJD_LED_TYPE_WS2812B_V2017MJD_LED_TYPE_WS2812_V2016;
    config.nbr_of_leds = 30; // 1..4..8..16..30..60..120
    config.relative_brightness = 50;
    config.gpio_num = MY_1WIRE_GPIO_NUM;
    mjd_ledrgb_init(&config);
*/

    loop_this: ;
    do_strip_sequences();
    goto loop_this;

    mjd_ledrgb_deinit(MJD_STRIP_1);

    mjd_log_memory_statistics();

    ESP_LOGI(TAG, "DONE");

    /********************************************************************************
     * Task Delete
     * @doc Passing NULL will end the current task
     */
    vTaskDelete(NULL);
}

/*
 * MAIN
 */
void app_main() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    BaseType_t xReturned;

    /********************************************************************************
     * STANDARD Init
     */
    mjd_log_wakeup_details();
    mjd_log_chip_info();
    mjd_log_time();
    mjd_log_memory_statistics();
    ESP_LOGI(TAG,
            "@doc Wait 3 seconds after power-on (let optional power supply become active, start logic analyzer, let peripherals become active!)");
    vTaskDelay(RTOS_DELAY_3SEC);

    /**********
     * TASK: main_task
     *  @important For stability (RMT + Wifi etc.): always use xTaskCreatePinnedToCore(APP_CPU_NUM) [Opposed to xTaskCreate() which might run the code on PRO_CPU_NUM...]
     */
    xReturned = xTaskCreatePinnedToCore(&main_task, "main_task (name)",
    MYAPP_RTOS_TASK_STACK_SIZE_LARGE,
    NULL,
    MYAPP_RTOS_TASK_PRIORITY_NORMAL,
    NULL,
    APP_CPU_NUM);
    if (xReturned == pdPASS) {
        ESP_LOGI(TAG, "OK Task main_task has been created, and is running right now");
    }

    /**********
     * END
     */
    ESP_LOGD(TAG, "END %s()", __FUNCTION__);
}
