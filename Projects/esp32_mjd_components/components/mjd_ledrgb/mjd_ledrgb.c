/*
 * Goto README.md for instructions
 */

// Component header file(s)
#include "mjd.h"
#include "mjd_ledrgb.h"

/*
 * Logging
 */
static const char TAG[] = "mjd_ledrgb";

/*
 * Data definitions
 */

// LED Strip characteristics | Unit = nanoseconds | Taken from the data sheets. foreach property: (min+max)/2
static const mjd_ledrgb_strip_characteristics_t STRIP_CHARACTERISTICS[] =
    {
            [MJD_LED_TYPE_WS2812_V2016] =
                { .color_sequence = MJD_COLOR_SEQUENCE_GRB, .T1high = 700, .T1low = 600, .T0high = 350, .T0low = 800, .Treset = 55000 },
            [MJD_LED_TYPE_WS2812B_V2017] =
                { .color_sequence = MJD_COLOR_SEQUENCE_GRB, .T1high = 1090, .T1low = 320, .T0high = 300, .T0low = 1090, .Treset = 290000 },
            [MJD_LED_TYPE_WS2813_V2017] =
                { .color_sequence = MJD_COLOR_SEQUENCE_GRB, .T1high = 1090, .T1low = 320, .T0high = 300, .T0low = 1090, .Treset = 290000 },
            [MJD_LED_TYPE_TEST] =
                { .color_sequence = MJD_COLOR_SEQUENCE_GRB, .T1high = 999, .T1low = 299, .T0high = 299, .T0low = 999, .Treset = 299999 } };

static mjd_ledrgb_strip_t STRIPS[8] =
    { 0 };

/**************************************
 * PUBLIC.
 *
 */
esp_err_t mjd_ledrgb_init(mjd_ledrgb_config_t* ptr_param_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    /**************************************************************************
     * Reuseable variables
     */
    esp_err_t f_retval = ESP_OK;

    /**************************************************************************
     * Check
     */

    if (ptr_param_config == NULL) {
        f_retval = ESP_FAIL;
        ESP_LOGE(TAG, "ABORT. ptr_param_config is NULL | err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    if (ptr_param_config->strip_id >= MJD_STRIP_IDENTIFIER_MAX) {
        f_retval = ESP_FAIL;
        ESP_LOGE(TAG, "ABORT. strip_id invalid | err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    if (ptr_param_config->gpio_num == 0) {
        f_retval = ESP_FAIL;
        ESP_LOGE(TAG, "ABORT. gpio_num 0 | err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    if (ptr_param_config->led_type >= MJD_LED_TYPES_MAX) {
        f_retval = ESP_FAIL;
        ESP_LOGE(TAG, "ABORT. led_type invalid | err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    if (ptr_param_config->nbr_of_leds == 0) {
        f_retval = ESP_FAIL;
        ESP_LOGE(TAG, "ABORT. nbr_of_leds 0 | err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /**************************************************************************
     * Process config input data
     */
    mjd_ledrgb_strip_t *ptr_strip = &STRIPS[ptr_param_config->strip_id];

    if (ptr_strip->is_init != false) {
        f_retval = ESP_FAIL;
        ESP_LOGE(TAG, "ABORT. Strip already initialized err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    ptr_strip->strip_id = ptr_param_config->strip_id;
    ptr_strip->gpio_num = ptr_param_config->gpio_num;
    ptr_strip->led_type = ptr_param_config->led_type;
    ptr_strip->nbr_of_leds = ptr_param_config->nbr_of_leds;
    ptr_strip->relative_brightness = ptr_param_config->relative_brightness;

    /**************************************************************************
     * Compute derived data
     */
    ptr_strip->rmt_channel = ptr_strip->strip_id;

    mjd_ledrgb_strip_characteristics_t strip_characteristics = STRIP_CHARACTERISTICS[ptr_strip->led_type];

    if (strip_characteristics.color_sequence == MJD_COLOR_SEQUENCE_GRB) {
        ptr_strip->color_sequence = strip_characteristics.color_sequence;
        ptr_strip->nbr_of_colors = 3;
    } else {
        f_retval = ESP_FAIL;
        ESP_LOGE(TAG, "ABORT. Invalid color_scheme err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    ptr_strip->rmt_item_pulse_pairs[MJD_RMT_PULSE_TYPE_BITVALUE_ZERO].level0 = MJD_LEVEL_HIGH;
    ptr_strip->rmt_item_pulse_pairs[MJD_RMT_PULSE_TYPE_BITVALUE_ZERO].duration0 = strip_characteristics.T0high / MY_RMT_ONE_TICK_DURATION_NANOSEC;
    ptr_strip->rmt_item_pulse_pairs[MJD_RMT_PULSE_TYPE_BITVALUE_ZERO].level1 = MJD_LEVEL_LOW;
    ptr_strip->rmt_item_pulse_pairs[MJD_RMT_PULSE_TYPE_BITVALUE_ZERO].duration1 = strip_characteristics.T0low / MY_RMT_ONE_TICK_DURATION_NANOSEC;

    ptr_strip->rmt_item_pulse_pairs[MJD_RMT_PULSE_TYPE_BITVALUE_ONE].level0 = MJD_LEVEL_HIGH;
    ptr_strip->rmt_item_pulse_pairs[MJD_RMT_PULSE_TYPE_BITVALUE_ONE].duration0 = strip_characteristics.T1high / MY_RMT_ONE_TICK_DURATION_NANOSEC;
    ptr_strip->rmt_item_pulse_pairs[MJD_RMT_PULSE_TYPE_BITVALUE_ONE].level1 = MJD_LEVEL_LOW;
    ptr_strip->rmt_item_pulse_pairs[MJD_RMT_PULSE_TYPE_BITVALUE_ONE].duration1 = strip_characteristics.T1low / MY_RMT_ONE_TICK_DURATION_NANOSEC;

    ptr_strip->rmt_item_pulse_pairs[MJD_RMT_PULSE_TYPE_RESET].level0 = MJD_LEVEL_LOW;
    ptr_strip->rmt_item_pulse_pairs[MJD_RMT_PULSE_TYPE_RESET].duration0 = 1; // send dummy lvl=0 dur=***1*** (I just want a low voltage level for Treset duration in the 2nd part of the item)(duration=0 does not work!)
    ptr_strip->rmt_item_pulse_pairs[MJD_RMT_PULSE_TYPE_RESET].level1 = MJD_LEVEL_LOW;
    ptr_strip->rmt_item_pulse_pairs[MJD_RMT_PULSE_TYPE_RESET].duration1 = strip_characteristics.Treset / MY_RMT_ONE_TICK_DURATION_NANOSEC;

    ptr_strip->rmt_item_pulse_pairs[MJD_RMT_PULSE_TYPE_END_MARKER].level0 = MJD_LEVEL_LOW;
    ptr_strip->rmt_item_pulse_pairs[MJD_RMT_PULSE_TYPE_END_MARKER].duration0 = 0;
    ptr_strip->rmt_item_pulse_pairs[MJD_RMT_PULSE_TYPE_END_MARKER].level1 = MJD_LEVEL_LOW;
    ptr_strip->rmt_item_pulse_pairs[MJD_RMT_PULSE_TYPE_END_MARKER].duration1 = 0;

    ESP_LOGD(TAG, "DEBUG rmt_item_pulses");
    for (uint8_t idx = 0; idx < MJD_RMT_PULSE_TYPE_MAX; idx++) {
        ESP_LOGD(TAG, "  J: %u, rmt_item_pulses :: [lvl0] %2u - %5i (%9.1f ns) | [lvl1] %2u - %5i (%9.1f ns) | value: %10u", idx,
                ptr_strip->rmt_item_pulse_pairs[idx].level0, ptr_strip->rmt_item_pulse_pairs[idx].duration0,
                ptr_strip->rmt_item_pulse_pairs[idx].duration0 * MY_RMT_ONE_TICK_DURATION_NANOSEC, ptr_strip->rmt_item_pulse_pairs[idx].level1,
                ptr_strip->rmt_item_pulse_pairs[idx].duration1, ptr_strip->rmt_item_pulse_pairs[idx].duration1 * MY_RMT_ONE_TICK_DURATION_NANOSEC,
                ptr_strip->rmt_item_pulse_pairs[idx].val);
    }

    /**************************************************************************
     * RMT INIT
     *
     * TODO mem_block_num. 1. Maybe larger number required for larger LED strips?
     * TODO mem_block_num. 2. Tune the number of blocks (max 8) depending on the actual number of RMT Channels used (=number of LED strips connected).
     *   DIVIDER examples for ABP clock freq of 80Mhz:
     *                                  --DIVIDER---
     *      80 Mhz: esp_clk_apb_freq() /        1  = 80000000 ticks/second  ERROR [@problem min=2]
     *      40 Mhz: esp_clk_apb_freq() /        2  = 40000000 ticks/second  OK
     *      10 Mhz: esp_clk_apb_freq() /        8  = 10000000 ticks/second  OK
     *       5 Mhz: esp_clk_apb_freq() /       16  =  5000000 ticks/second  OK
     *       1 Mhz: esp_clk_apb_freq() /       80  =  1000000 ticks/second  OK
     *     100 Khz: esp_clk_apb_freq() /      800  =   100000 ticks/second  OK
     *      10 Khz: esp_clk_apb_freq() /     8000  =    10000 ticks/second  OK
     *    1.25 Khz: esp_clk_apb_freq() /    64000  =     1250 ticks/second  OK
     *       1 Khz: esp_clk_apb_freq() /    80000  =     1000 ticks/second  ERROR [@problem max=65536]
     *     100 Hz : esp_clk_apb_freq() /   800000  =      100 ticks/second  ERROR [@problem max=65536]
     *      10 Hz : esp_clk_apb_freq() /  8000000  =       10 ticks/second  ERROR [@problem max=65536]
     *       1 Hz : esp_clk_apb_freq() / 80000000  =        1 ticks/second  ERROR [@problem max=65536]
     *
     *       Kolban:
     *       - The base clock runs by default at 80MHz. That means it ticks 80,000,000 times a second or 80,000 times a millisecond
     *         or 80 times a microsecond or 0.08 times a nano second.
     *         Flipping this around, our granularity of interval is 1/80,000,000 is 0.0000000125 seconds or 0.0000125 milliseconds
     *         or 0.0125 microseconds or 12.5 nanoseconds. This is fast.
     *       - About the clock divider value. If the base clock is 80MHz then a divisor of 80 gives us 1MHz.
     *
     */
    rmt_config_t rmt_cfg =
        { 0 };
    rmt_cfg.channel = ptr_strip->rmt_channel;
    rmt_cfg.gpio_num = ptr_strip->gpio_num;
    rmt_cfg.rmt_mode = RMT_MODE_TX;
    rmt_cfg.clk_div = MY_RMT_CLK_DIV; /*!< [value 1..255] RMT channel counter divider */
    rmt_cfg.mem_block_num = 1;
    rmt_cfg.tx_config.idle_output_en = true; /*!< RMT idle level output enable */
    rmt_cfg.tx_config.idle_level = RMT_IDLE_LEVEL_LOW; /*!< RMT idle level */
    rmt_cfg.tx_config.carrier_en = false;             // !< RMT carrier enable
    rmt_cfg.tx_config.loop_en = false;             // /*!< Enable sending RMT items in a loop */

    f_retval = rmt_config(&rmt_cfg);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "rmt_config() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    f_retval = rmt_driver_install(rmt_cfg.channel, 0, 0);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "rmt_driver_install() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /*************************************************************************
     * Allocate storage
     */
    ptr_strip->leds = (mjd_ledrgb_led_t*) calloc(ptr_strip->nbr_of_leds, sizeof(mjd_ledrgb_led_t));

    ptr_strip->len_transformation_buffer = ptr_strip->nbr_of_leds * sizeof(mjd_ledrgb_led_t);
    ptr_strip->transformation_buffer = (uint8_t*) calloc(ptr_strip->len_transformation_buffer, sizeof(uint8_t));

    ptr_strip->nbr_of_rmt_tx_items = 2 + (8 * ptr_strip->len_transformation_buffer);
    ptr_strip->rmt_tx_items = (rmt_item32_t*) calloc(ptr_strip->nbr_of_rmt_tx_items, sizeof(rmt_item32_t));

    /**************************************************************************
     * Init := true
     */
    ptr_strip->is_init = true;

    /*************************************************************************
     * Reset Strip
     */
    mjd_ledrgb_reset_strip(ptr_strip->strip_id);

    /**************************************************************************
     * LABEL cleanup
     */
    cleanup: ;

    return f_retval;
}

esp_err_t mjd_ledrgb_deinit(mjd_ledrgb_strip_identifier_t param_strip_id) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    /**************************************************************************
     * Reuseable variables
     */
    esp_err_t f_retval = ESP_OK;

    /**************************************************************************
     * Check
     */
    if (param_strip_id >= MJD_STRIP_IDENTIFIER_MAX) {
        f_retval = ESP_FAIL;
        ESP_LOGE(TAG, "ABORT. param_strip_id invalid | err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    mjd_ledrgb_strip_t *ptr_strip = &STRIPS[param_strip_id];

    if (ptr_strip->is_init != true) {
        f_retval = ESP_FAIL;
        ESP_LOGE(TAG, "ABORT. Strip was not initialized | err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /**************************************************************************
     * FREE memory
     */
    free(ptr_strip->transformation_buffer);
    ptr_strip->len_transformation_buffer = 0;

    free(ptr_strip->rmt_tx_items);
    ptr_strip->nbr_of_rmt_tx_items = 0;

    /**************************************************************************
     * RMT DRV UNINSTALL
     */
    f_retval = rmt_driver_uninstall(ptr_strip->rmt_channel);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "rmt_driver_uninstall() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /**************************************************************************
     * Init := false
     */
    ptr_strip->is_init = false;

    /**************************************************************************
     * LABEL cleanup
     */
    cleanup: ;

    // RETURN
    return f_retval;
}

esp_err_t mjd_ledrgb_get_strip_nbr_of_leds(mjd_ledrgb_strip_identifier_t param_strip_id, uint32_t * param_ptr_nbr_of_leds) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    /**************************************************************************
     * Reuseable variables
     */
    esp_err_t f_retval = ESP_OK;

    /**************************************************************************
     * Check
     */
    if (param_strip_id >= MJD_STRIP_IDENTIFIER_MAX) {
        f_retval = ESP_FAIL;
        ESP_LOGE(TAG, "ABORT. param_strip_id invalid | err");
        // GOTO
        goto cleanup;
    }

    /**************************************************************************
     * MAIN
     */
    mjd_ledrgb_strip_t *ptr_strip = &STRIPS[param_strip_id];

    *param_ptr_nbr_of_leds = ptr_strip->nbr_of_leds;

    /**************************************************************************
     * LABEL cleanup
     */
    cleanup: ;

    // RETURN
    return f_retval;
}

esp_err_t mjd_ledrgb_send_pixels_to_strip(mjd_ledrgb_strip_identifier_t param_strip_id) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    /**************************************************************************
     * Reuseable variables
     */
    esp_err_t f_retval = ESP_OK;

    /**************************************************************************
     * Check
     */
    if (param_strip_id >= MJD_STRIP_IDENTIFIER_MAX) {
        f_retval = ESP_FAIL;
        ESP_LOGE(TAG, "ABORT. param_strip_id invalid | err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    mjd_ledrgb_strip_t *ptr_strip = &STRIPS[param_strip_id];

    /*************************************************************************
     * Fill the .transformation_buffer (RGB to GRB)
     * @dep led strip's color sequence
     * @dep led strip's relative_brightness
     */
    for (uint32_t i = 0; i < ptr_strip->nbr_of_leds; i++) {
        if (ptr_strip->color_sequence == MJD_COLOR_SEQUENCE_GRB) { // WS2812B
            ptr_strip->transformation_buffer[0 + (i * ptr_strip->nbr_of_colors)] = (double) ptr_strip->leds[i].g * ptr_strip->relative_brightness
                    / 100.0;
            ptr_strip->transformation_buffer[1 + (i * ptr_strip->nbr_of_colors)] = (double) ptr_strip->leds[i].r * ptr_strip->relative_brightness
                    / 100.0;
            ptr_strip->transformation_buffer[2 + (i * ptr_strip->nbr_of_colors)] = (double) ptr_strip->leds[i].b * ptr_strip->relative_brightness
                    / 100.0;
        } else if (ptr_strip->color_sequence == MJD_COLOR_SEQUENCE_RGB) { // For future new led strip types
            ptr_strip->transformation_buffer[0 + (i * ptr_strip->nbr_of_colors)] = (double) ptr_strip->leds[i].r * ptr_strip->relative_brightness
                    / 100.0;
            ptr_strip->transformation_buffer[1 + (i * ptr_strip->nbr_of_colors)] = (double) ptr_strip->leds[i].g * ptr_strip->relative_brightness
                    / 100.0;
            ptr_strip->transformation_buffer[2 + (i * ptr_strip->nbr_of_colors)] = (double) ptr_strip->leds[i].b * ptr_strip->relative_brightness
                    / 100.0;
        } else {
            ESP_LOGW(TAG, "Invalid color sequence (not grb");
        }
    }

    ESP_LOGD(TAG, "DEBUG transformation_buffer");
    ESP_LOGD(TAG, "    ptr_strip->nbr_of_leds = %u", ptr_strip->nbr_of_leds);
    ESP_LOGD(TAG, "    ptr_strip->nbr_of_colors = %u", ptr_strip->nbr_of_colors);
    ESP_LOGD(TAG, "    ptr_strip->len_transformation_buffer = %u", ptr_strip->len_transformation_buffer);
    ESP_LOGD(TAG, "    ESP_LOG_BUFFER_HEX_LEVEL()");
    ESP_LOG_BUFFER_HEX_LEVEL(TAG, ptr_strip->transformation_buffer, ptr_strip->len_transformation_buffer, ESP_LOG_DEBUG);

    // DEVTEMP: HALT
    /////mjd_rtos_wait_forever();

    /*************************************************************************
     * Fill RMT TX items using NRZ 1-Wire protocol.
     *      Shift bits out, MSB first! Transform transformation_buffer bits to rmt_tx_items
     *      8 bits in each buffer byte. 1 extra item for the trailing Treset pulses. 1 extra item for the trailing RMT End Markers.
     *      @important rmt_tx_items MUST be on the heap (not the stack!)
     *      @doc https://overiq.com/c-programming/101/pointer-basics-in-c/
     */
    rmt_item32_t *ptr_fill_rmt_item = ptr_strip->rmt_tx_items;

    ESP_LOGD(TAG, "DEBUG (OK equal) Address of rmt_tx_items: %p | Address of ptr_fill_rmt_item %p", ptr_strip->rmt_tx_items, ptr_fill_rmt_item);

    for (uint32_t j = 0; j < ptr_strip->len_transformation_buffer; j++) {
        uint8_t input_byte = ptr_strip->transformation_buffer[j];
        ESP_LOGD(TAG, "  j: %u | input_byte: %u", j, input_byte);
        for (int8_t bit = 7; bit >= 0; --bit) {
            uint8_t bitvalue = (input_byte >> bit) & 0x01;
            *ptr_fill_rmt_item = ptr_strip->rmt_item_pulse_pairs[bitvalue]; // bitvalue~>array_index @important *ptr = indirection pointer

            ESP_LOGD(TAG, "    bit %u: ptr_strip->rmt_tx_items : [lvl0] %2u - %5i (%9.1f ns) | [lvl1] %2u - %5i (%9.1f ns) | value: %10u", bit,
                    ptr_fill_rmt_item->level0, ptr_fill_rmt_item->duration0, ptr_fill_rmt_item->duration0 * MY_RMT_ONE_TICK_DURATION_NANOSEC,
                    ptr_fill_rmt_item->level1, ptr_fill_rmt_item->duration1, ptr_fill_rmt_item->duration1 * MY_RMT_ONE_TICK_DURATION_NANOSEC,
                    ptr_fill_rmt_item->val);

            ++ptr_fill_rmt_item;
        }
    }

    // ADD pulse: Treset
    //      @doc All LED chips synchronously send the received data to each led-segment when the DIN port receives a Reset signal.
    ESP_LOGD(TAG, "Add rmt_item for TReset");
    *ptr_fill_rmt_item = ptr_strip->rmt_item_pulse_pairs[MJD_RMT_PULSE_TYPE_RESET]; // @important *ptr = indirection pointer

    ESP_LOGD(TAG,
            "    tReset: ptr_strip->rmt_item_pulse_pairs[RMT_PULSE_TYPE_RESET] : [lvl0] %2u - %5i (%9.1f ns) | [lvl1] %2u - %5i (%9.1f ns) | value: %10u",
            ptr_fill_rmt_item->level0, ptr_fill_rmt_item->duration0, ptr_fill_rmt_item->duration0 * MY_RMT_ONE_TICK_DURATION_NANOSEC,
            ptr_fill_rmt_item->level1, ptr_fill_rmt_item->duration1, ptr_fill_rmt_item->duration1 * MY_RMT_ONE_TICK_DURATION_NANOSEC,
            ptr_fill_rmt_item->val);

    ++ptr_fill_rmt_item;

    // ADD Pulse: RMT End Marker
    ESP_LOGD(TAG, "Add rmt_item for RMT End Marker");
    *ptr_fill_rmt_item = ptr_strip->rmt_item_pulse_pairs[MJD_RMT_PULSE_TYPE_END_MARKER]; // @important *ptr = indirection pointer

    ESP_LOGD(TAG,
            "    tReset: ptr_strip->rmt_item_pulse_pairs[RMT_PULSE_TYPE_END_MARKER] : [lvl0] %2u - %5i (%9.1f ns) | [lvl1] %2u - %5i (%9.1f ns) | value: %10u",
            ptr_fill_rmt_item->level0, ptr_fill_rmt_item->duration0, ptr_fill_rmt_item->duration0 * MY_RMT_ONE_TICK_DURATION_NANOSEC,
            ptr_fill_rmt_item->level1, ptr_fill_rmt_item->duration1, ptr_fill_rmt_item->duration1 * MY_RMT_ONE_TICK_DURATION_NANOSEC,
            ptr_fill_rmt_item->val);

    ++ptr_fill_rmt_item;

    //
    ESP_LOGD(TAG, "DEBUG param_ptr_led_strip->rmt_tx_items");
    ESP_LOGD(TAG, "    sizeof(rmt_item32_t)            = %u", sizeof(rmt_item32_t));
    ESP_LOGD(TAG, "    ptr_strip->nbr_of_rmt_tx_items  = %u", ptr_strip->nbr_of_rmt_tx_items);
    ESP_LOGD(TAG, "    ESP_LOG_BUFFER_HEX_LEVEL()");
    ESP_LOG_BUFFER_HEX_LEVEL(TAG, ptr_strip->rmt_tx_items, ptr_strip->nbr_of_rmt_tx_items, ESP_LOG_DEBUG);
    ESP_LOGD(TAG, "  ptr_strip->nbr_of_rmt_tx_items = %u", ptr_strip->nbr_of_rmt_tx_items);
    rmt_item32_t *ptr_report_rmt_item = ptr_strip->rmt_tx_items; // Create a temporary pointer (=pointing to the beginning of the item array)
    for (uint32_t i = 0; i < ptr_strip->nbr_of_rmt_tx_items; i++) {
        ESP_LOGD(TAG, "  %4u :: [lvl0] %2u - %5i (%9.1f ns) | [lvl1] %2u - %5i (%9.1f ns) | value: %10u", i, ptr_report_rmt_item->level0,
                ptr_report_rmt_item->duration0, ptr_report_rmt_item->duration0 * MY_RMT_ONE_TICK_DURATION_NANOSEC, ptr_report_rmt_item->level1,
                ptr_report_rmt_item->duration1, ptr_report_rmt_item->duration1 * MY_RMT_ONE_TICK_DURATION_NANOSEC, ptr_report_rmt_item->val);
        ptr_report_rmt_item++; // Advance ptr
    }

    /**************************************************************************
     * RMT WRITE
     */
    f_retval = rmt_write_items(ptr_strip->rmt_channel, ptr_strip->rmt_tx_items, ptr_strip->nbr_of_rmt_tx_items,
    true);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "rmt_write_items(CH1) err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    //    memset(param_ptr_led_strip->leds, 0, param_ptr_led_strip->len_leds * sizeof(led_t));
    //    digitalLeds_updatePixels(param_ptr_led_strip);

    /**************************************************************************
     * LABEL cleanup
     */
    cleanup: ;

    return f_retval;
}

esp_err_t mjd_ledrgb_reset_strip(mjd_ledrgb_strip_identifier_t param_strip_id) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    /**************************************************************************
     * Reuseable variables
     */
    esp_err_t f_retval = ESP_OK;

    /**************************************************************************
     * Check
     */
    if (param_strip_id >= MJD_STRIP_IDENTIFIER_MAX) {
        f_retval = ESP_FAIL;
        ESP_LOGE(TAG, "ABORT. param_strip_id invalid | err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    mjd_ledrgb_strip_t *ptr_strip = &STRIPS[param_strip_id];

    /*************************************************************************
     * Pick RGB COLORS & Send Pixels: 0%
     *      @range 0x00..0xFF
     */
    for (uint32_t j = 0; j < ptr_strip->nbr_of_leds; ++j) {
        ptr_strip->leds[j] = mjd_led_rgb(0, 0, 0);
    }

    ESP_LOGD(TAG, "DEBUG param_ptr_led_strip->leds");
    ESP_LOGD(TAG, "    sizeof(mjd_ledrgb_led_t)                                    = %u", sizeof(mjd_ledrgb_led_t));
    ESP_LOGD(TAG, "    sizeof(mjd_ledrgb_led_t) * param_ptr_led_strip->nbr_of_leds = %u", sizeof(mjd_ledrgb_led_t) * ptr_strip->nbr_of_leds);
    ESP_LOGD(TAG, "    ESP_LOG_BUFFER_HEX_LEVEL()");
    ESP_LOG_BUFFER_HEX_LEVEL(TAG, ptr_strip->leds, ptr_strip->nbr_of_leds * sizeof(mjd_ledrgb_led_t), ESP_LOG_DEBUG);

    mjd_ledrgb_send_pixels_to_strip(param_strip_id);

    /**************************************************************************
     * LABEL cleanup
     */
    cleanup: ;

    return f_retval;
}

esp_err_t mjd_ledrgb_set_strip_led(mjd_ledrgb_strip_identifier_t param_strip_id, mjd_ledrgb_led_identifier_t param_led_id,
                                   mjd_ledrgb_led_t param_led_color,
                                   bool param_send_pixels_to_strip) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    /**************************************************************************
     * Reuseable variables
     */
    esp_err_t f_retval = ESP_OK;

    /**************************************************************************
     * Check
     */
    if (param_strip_id >= MJD_STRIP_IDENTIFIER_MAX) {
        f_retval = ESP_FAIL;
        ESP_LOGE(TAG, "ABORT. param_strip_id invalid | err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    mjd_ledrgb_strip_t *ptr_strip = &STRIPS[param_strip_id];

    /**************************************************************************
     * MAIN
     */
    ptr_strip->leds[param_led_id] = param_led_color;

    ESP_LOGD(TAG, "DEBUG param_ptr_led_strip->leds");
    ESP_LOGD(TAG, "    sizeof(mjd_ledrgb_led_t)                          = %u", sizeof(mjd_ledrgb_led_t));
    ESP_LOGD(TAG, "    sizeof(mjd_ledrgb_led_t) * ptr_strip->nbr_of_leds = %u", sizeof(mjd_ledrgb_led_t) * ptr_strip->nbr_of_leds);
    ESP_LOGD(TAG, "    ESP_LOG_BUFFER_HEX_LEVEL()");
    ESP_LOG_BUFFER_HEX_LEVEL(TAG, ptr_strip->leds, ptr_strip->nbr_of_leds * sizeof(mjd_ledrgb_led_t), ESP_LOG_DEBUG);

    if (param_send_pixels_to_strip == true) {
        mjd_ledrgb_send_pixels_to_strip(param_strip_id);
    }

    /**************************************************************************
     * LABEL cleanup
     */
    cleanup: ;

    return f_retval;
}

mjd_ledrgb_led_t mjd_ledrgb_get_strip_led(mjd_ledrgb_strip_identifier_t param_strip_id, mjd_ledrgb_led_identifier_t param_led_id) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    /**************************************************************************
     * Reuseable variables
     */
    mjd_ledrgb_led_t new_led =
        { 0 };

    /**************************************************************************
     * Check
     */
    if (param_strip_id >= MJD_STRIP_IDENTIFIER_MAX) {
        ESP_LOGE(TAG, "ABORT. param_strip_id invalid | err");
        // GOTO
        goto cleanup;
    }

    mjd_ledrgb_strip_t *ptr_strip = &STRIPS[param_strip_id];

    /**************************************************************************
     * MAIN
     */
    new_led.r = ptr_strip->leds[param_led_id].r;
    new_led.g = ptr_strip->leds[param_led_id].g;
    new_led.b = ptr_strip->leds[param_led_id].b;

    ESP_LOGD(TAG, "DEBUG led");
    ESP_LOGD(TAG, "    sizeof(mjd_ledrgb_led_t) = %i", sizeof(mjd_ledrgb_led_t));
    ESP_LOGD(TAG, "    ESP_LOG_BUFFER_HEX_LEVEL()");
    ESP_LOG_BUFFER_HEX_LEVEL(TAG, &new_led, sizeof(mjd_ledrgb_led_t), ESP_LOG_DEBUG);

    /**************************************************************************
     * LABEL cleanup
     */
    cleanup: ;

    // RETURN
    return new_led;
}

esp_err_t mjd_ledrgb_set_strip_leds_one_color(mjd_ledrgb_strip_identifier_t param_strip_id, mjd_ledrgb_led_t param_led_color) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    /**************************************************************************
     * Reuseable variables
     */
    esp_err_t f_retval = ESP_OK;

    /**************************************************************************
     * Check
     */
    if (param_strip_id >= MJD_STRIP_IDENTIFIER_MAX) {
        f_retval = ESP_FAIL;
        ESP_LOGE(TAG, "ABORT. param_strip_id invalid | err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    mjd_ledrgb_strip_t *ptr_strip = &STRIPS[param_strip_id];

    /**************************************************************************
     * MAIN
     */
    for (uint8_t i = 0; i < ptr_strip->nbr_of_leds; i++) {
        ptr_strip->leds[i] = param_led_color;
    }

    ESP_LOGD(TAG, "DEBUG param_ptr_led_strip->leds");
    ESP_LOGD(TAG, "    sizeof(mjd_ledrgb_led_t)                          = %u", sizeof(mjd_ledrgb_led_t));
    ESP_LOGD(TAG, "    sizeof(mjd_ledrgb_led_t) * ptr_strip->nbr_of_leds = %u", sizeof(mjd_ledrgb_led_t) * ptr_strip->nbr_of_leds);
    ESP_LOGD(TAG, "    ESP_LOG_BUFFER_HEX_LEVEL()");
    ESP_LOG_BUFFER_HEX_LEVEL(TAG, ptr_strip->leds, ptr_strip->nbr_of_leds * sizeof(mjd_ledrgb_led_t), ESP_LOG_DEBUG);

    mjd_ledrgb_send_pixels_to_strip(param_strip_id);

    /**************************************************************************
     * LABEL cleanup
     */
    cleanup: ;

    return f_retval;
}

esp_err_t mjd_ledrgb_identify_led_positions(mjd_ledrgb_strip_identifier_t param_strip_id) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    /**************************************************************************
     * Reuseable variables
     */
    esp_err_t f_retval = ESP_OK;

    /**************************************************************************
     * Check
     */
    if (param_strip_id >= MJD_STRIP_IDENTIFIER_MAX) {
        f_retval = ESP_FAIL;
        ESP_LOGE(TAG, "ABORT. param_strip_id invalid | err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    mjd_ledrgb_strip_t *ptr_strip = &STRIPS[param_strip_id];

    /**************************************************************************
     * MAIN
     */
    mjd_ledrgb_reset_strip(param_strip_id);

    for (uint32_t el_led = 0; el_led < ptr_strip->nbr_of_leds; ++el_led) {
        /////for (uint32_t j = 0; j <= el_led; ++j) {
        mjd_ledrgb_set_strip_led(param_strip_id, el_led, mjd_led_rgb(255, 255, 255), true);
        vTaskDelay(RTOS_DELAY_100MILLISEC);
        mjd_ledrgb_set_strip_led(param_strip_id, el_led, mjd_led_rgb(0, 0, 0), true);
        vTaskDelay(RTOS_DELAY_100MILLISEC);
        /////}
    }
    /**************************************************************************
     * LABEL cleanup
     */
    cleanup: ;

    return f_retval;
}

