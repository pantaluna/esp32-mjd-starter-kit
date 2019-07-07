/*
 * Goto the README.md for instructions
 *
 */

/*
 * Includes: system, own
 */
#include "mjd_jsnsr04t.h"

/*
 * Logging
 */
static const char TAG[] = "mjd_jsnsr04t";

/*
 * Sensor settings
 */
#define MJD_JSNSR04T_MINIMUM_SUPPORTED_DISTANCE_CM         (25.0)
#define MJD_JSNSR04T_MAXIMUM_SUPPORTED_DISTANCE_CM         (350.0)
// Spinlock for protecting concurrent register-level access
static portMUX_TYPE jsnsr04t_spinlock = portMUX_INITIALIZER_UNLOCKED;

/*
 * PRIVATE DATA TYPES
 *
 * @doc C lang: if you declare a typedef struct within a .c file, then it will be private for that source file.
 *
 */
typedef struct {
    bool data_received; /*!< Has data been received from the device? */
    bool is_an_error; /*!< Is the data an error? */
    uint32_t raw; /*!< The raw measured value (from RMT) */
    double distance_cm; /*!< This distance is adjusted with the distance_sensor_to_artifact_cm (subtracted). */
} mjd_jsnsr04t_raw_data_t;

/**************************************
 * PRIVATE STATIC.
 *
 * @brief helper log data
 *
 * @important LOG_DEBUG (not LOG_INFO)
 */
static esp_err_t _log_raw_data(mjd_jsnsr04t_raw_data_t param_raw_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    ESP_LOGD(TAG, "Log mjd_jsnsr04t_raw_data_t param_ptr_raw_data:");
    ESP_LOGD(TAG, "  %32s = %u", "(bool) data_received", param_raw_data.data_received);
    ESP_LOGD(TAG, "  %32s = %u", "(bool) is_an_error", param_raw_data.is_an_error);
    ESP_LOGD(TAG, "  %32s = %u", "(uint32_t) raw", param_raw_data.raw);
    ESP_LOGD(TAG, "  %32s = %f", "(double) distance_cm", param_raw_data.distance_cm);

    return f_retval;
}

/**************************************
 * PRIVATE STATIC.
 *
 * @brief Increase error count, reset distance value to 0.0, mark record in error
 */
static esp_err_t _mark_error_event(mjd_jsnsr04t_config_t* param_ptr_config,
                                       mjd_jsnsr04t_raw_data_t* param_ptr_raw_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    ++param_ptr_config->nbr_of_errors;

    param_ptr_raw_data->distance_cm = 0.0; // Reset distance
    param_ptr_raw_data->is_an_error = true; // Mark error

    return f_retval;
}

/**************************************
 * PUBLIC.
 *
 * @brief helper log config
 */
esp_err_t mjd_jsnsr04t_log_config(mjd_jsnsr04t_config_t param_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    ESP_LOGI(TAG, "Log mjd_jsnsr04t_config_t param_ptr_config:");
    ESP_LOGI(TAG, "  %32s = %u", "(bool) is_init", param_config.is_init);
    ESP_LOGI(TAG, "  %32s = %i", "(gpio_num_t) trigger_gpio_num", param_config.trigger_gpio_num);
    ESP_LOGI(TAG, "  %32s = %i", "(gpio_num_t) echo_gpio_num", param_config.echo_gpio_num);
    ESP_LOGI(TAG, "  %32s = %i", "(rmt_channel_t) rmt_channel", param_config.rmt_channel);
    ESP_LOGI(TAG, "  %32s = %f", "(double) distance_sensor_to_artifact_cm",
            param_config.distance_sensor_to_artifact_cm);
    ESP_LOGI(TAG, "  %32s = %u", "(uint32_t) nbr_of_errors", param_config.nbr_of_errors);
    ESP_LOGI(TAG, "  %32s = %u", "(uint32_t) nbr_of_samples", param_config.nbr_of_samples);

    return f_retval;
}

/**************************************
 * PUBLIC.
 *
 * @brief helper log data
 */
esp_err_t mjd_jsnsr04t_log_data(mjd_jsnsr04t_data_t param_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    ESP_LOGI(TAG, "Log mjd_jsnsr04t_data_t* param_ptr_data:");
    ESP_LOGI(TAG, "  %32s = %u", "(bool) data_received", param_data.data_received);
    ESP_LOGI(TAG, "  %32s = %u", "(bool) is_an_error", param_data.is_an_error);
    ESP_LOGI(TAG, "  %32s = %f", "(double) distance_cm", param_data.distance_cm);

    return f_retval;
}

/*********************************************************************************
 * PUBLIC.
 *
 * @brief JSN-SR04T: init
 *
 * RMT:
 * ----
 * - RMT source clock = APB Clock 80Mhz. It ticks 80,000,000 times a second or 80,000 times a MILLIsecond
 *   or 80 times a MICROsecond or 0.08 times a NANOsecond. This is FAST enough for meteo sensors.
 * - If we divide the base clock by 80 (giving 1Mhz) then the granularity unit
 *      becomes 1 microsecond (=enough granularity for our sensors, and very easy to reason with that).
 * - DIVIDER examples for ABP clock freq of 80Mhz:
 *                                  --DIVIDER---
 *      80 Mhz: esp_clk_apb_freq() /        1  = 80000000 ticks/second  ERROR [@problem min=2]
 *      40 Mhz: esp_clk_apb_freq() /        2  = 40000000 ticks/second  OK
 *      10 Mhz: esp_clk_apb_freq() /        8  = 10000000 ticks/second  OK
 *       5 Mhz: esp_clk_apb_freq() /       16  =  5000000 ticks/second  OK
 *       1 Mhz: esp_clk_apb_freq() /       80  =  1000000 ticks/second  OK ***microsecond***
 *     100 Khz: esp_clk_apb_freq() /      800  =   100000 ticks/second  OK
 *      10 Khz: esp_clk_apb_freq() /     8000  =    10000 ticks/second  OK
 *    1.25 Khz: esp_clk_apb_freq() /    64000  =     1250 ticks/second  OK
 *       1 Khz: esp_clk_apb_freq() /    80000  =     1000 ticks/second  ERROR [@problem max=65536]
 *     100 Hz : esp_clk_apb_freq() /   800000  =      100 ticks/second  ERROR [@problem max=65536]
 *      10 Hz : esp_clk_apb_freq() /  8000000  =       10 ticks/second  ERROR [@problem max=65536]
 *       1 Hz : esp_clk_apb_freq() / 80000000  =        1 ticks/second  ERROR [@problem max=65536]
 *
 *   Kolban:
 *   - The base clock runs by default at 80MHz. That means it ticks 80,000,000 times a second or 80,000 times a millisecond
 *     or 80 times a microsecond or 0.08 times a nano second.
 *     Flipping this around, our granularity of interval is 1/80,000,000 is 0.0000000125 seconds or 0.0000125 milliseconds
 *     or 0.0125 microseconds or 12.5 nanoseconds. This is fast.
 *   - About the clock divider value. If the base clock is 80MHz then a divisor of 80 gives us 1MHz.
 *
 *********************************************************************************/
esp_err_t mjd_jsnsr04t_init(mjd_jsnsr04t_config_t* param_ptr_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // TODO Check ALL params
    if (param_ptr_config->nbr_of_samples == 0) {
        f_retval = ESP_ERR_INVALID_ARG;
        ESP_LOGE(TAG, "%s(). param_ptr_config->nbr_of_samples cannot be 0 | err %i (%s)", __FUNCTION__, f_retval,
                esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    if (param_ptr_config->is_init == true) {
        f_retval = ESP_ERR_INVALID_STATE;
        ESP_LOGE(TAG, "%s(). The component has already been init'd | err %i (%s)", __FUNCTION__, f_retval,
                esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /*
     * GPIO's
     */
    gpio_config_t io_config;

    io_config.pin_bit_mask = (1ULL << param_ptr_config->trigger_gpio_num);
    io_config.mode = GPIO_MODE_OUTPUT;
    io_config.pull_down_en = GPIO_PULLDOWN_ENABLE; // @important
    io_config.pull_up_en = GPIO_PULLUP_DISABLE;
    io_config.intr_type = GPIO_PIN_INTR_DISABLE;
    f_retval = gpio_config(&io_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. gpio_config(trigger_gpio_num) | err %i (%s)", __FUNCTION__, f_retval,
                esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    io_config.pin_bit_mask = (1ULL << param_ptr_config->echo_gpio_num);
    io_config.mode = GPIO_MODE_INPUT;
    io_config.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_config.pull_up_en = GPIO_PULLUP_DISABLE;
    io_config.intr_type = GPIO_PIN_INTR_DISABLE;
    f_retval = gpio_config(&io_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. gpio_config(echo_gpio_num) | err %i (%s)", __FUNCTION__, f_retval,
                esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /*
     * RMT
     */
    // // Clock Divider (See func doc)
    const int JSNSR04T_RMT_CLK_DIV = esp_clk_apb_freq() / 1000000; // 1 tick = 1 MICROsec (value 80 => RMT Logical Clock @ 1Mhz)

    // // Pulses shorter than this setting will be filtered out.
    // // =200 MICROsec (1 tick = 1 MICROsec). @doc uint8_t [0..255]. @dependency My logical clock of 1Mhz.
    const int JSNSR04T_RMT_MIN_TICKS = 200;

    // When no edge is detected on the input signal for longer than idle_thres channel clock cycles, then the receive process stops.
    // // =60 MILLIsec, = 60*1000 MICROsec (1 tick = 1 MICROsec). @doc uint16_t [0..65535]. @dependency My logical clock of 1Mhz.
    const int JSNSR04T_RMT_TIMEOUT_US = 60 * 1000;

    rmt_config_t rx_config =
                { 0 };
    rx_config.gpio_num = param_ptr_config->echo_gpio_num;
    rx_config.channel = param_ptr_config->rmt_channel;
    rx_config.rmt_mode = RMT_MODE_RX;
    rx_config.clk_div = JSNSR04T_RMT_CLK_DIV;
    rx_config.mem_block_num = 1;
    rx_config.rx_config.filter_en = true;
    rx_config.rx_config.filter_ticks_thresh = JSNSR04T_RMT_MIN_TICKS;
    rx_config.rx_config.idle_threshold = JSNSR04T_RMT_TIMEOUT_US;
    f_retval = rmt_config(&rx_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. rmt_config() | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    f_retval = rmt_driver_install(rx_config.channel, 2048, 0); // rx_buf_size 2048 seems to the minimum (why? I do not know).
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. rmt_driver_install() | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Mark init-yes
    param_ptr_config->is_init = true;

    // LABEL
    cleanup: ;

    return f_retval;
}

esp_err_t mjd_jsnsr04t_deinit(mjd_jsnsr04t_config_t* param_ptr_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // NULL

    // Mark init'd false
    param_ptr_config->is_init = false;

    // LABEL
    /////cleanup: ;

    return f_retval;
}

/*
 * PRIVATE STATIC.
 *
 * @brief Get ONE raw measurement
 *
 * @doc Speed of sound = 0.0342cm per microsecond
 * @doc The duration is the time it takes to transmit and receive the ultrasonic signal (so the distance is related to half that time measurement).
 * @doc Minimum stable distance =  25cm
 * @doc Maximum stable distance = 350cm
 * @doc Reserve a minimum period of 60 millisec between measurements (avoid signal overlap, do not process the retured ultrasonic signal of the previous measurement).
 * @doc If no obstacle is detected, the output pin will give a 38 millisec high level signal (38 millisec = 6.5 meter which is out of range).
 * @doc If the sensor does not receive an echo within 60 millisec (range too big or too close or no object range) then the signal goes LOW after 60 millisec.
 * @important Use ets_delay_us(). Do not use vTaskDelay() https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/freertos-smp.html?highlight=vtaskdelay
 *
 */
static esp_err_t _get_one_measurement(mjd_jsnsr04t_config_t* param_ptr_config, mjd_jsnsr04t_raw_data_t* param_ptr_raw_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    bool do_trigger = true; // Set :=false to test use case that the trigger sequence is not executed.

    // Reset receive values
    param_ptr_raw_data->data_received = false;
    param_ptr_raw_data->is_an_error = false;
    param_ptr_raw_data->raw = 0;
    param_ptr_raw_data->distance_cm = 0.0;

    /*
     * # SENSOR RMT
     */
    size_t rx_size = 0;
    rmt_item32_t* ptr_rx_item;
    bool is_ringbuffer_received = false;
    RingbufHandle_t rb = NULL;
    f_retval = rmt_get_ringbuf_handle(param_ptr_config->rmt_channel, &rb);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. rmt_get_ringbuf_handle() | err %i (%s)", __FUNCTION__, f_retval,
                esp_err_to_name(f_retval));
        _mark_error_event(param_ptr_config, param_ptr_raw_data);
    }
    f_retval = rmt_rx_start(param_ptr_config->rmt_channel, true);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. rmt_rx_start() | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        _mark_error_event(param_ptr_config, param_ptr_raw_data);
    }

    /*
     * # SENSOR CMD Start Measurement:
     *      1. trigger_gpio level:=0 for at least 60 MILLIsec
     *      2. trigger_gpio level:=1 for at least 25 MICROsec
     */
    if (do_trigger == true) {
        portENTER_CRITICAL(&jsnsr04t_spinlock);
        gpio_set_level(param_ptr_config->trigger_gpio_num, 0);
        ets_delay_us(60 * 1000);
        gpio_set_level(param_ptr_config->trigger_gpio_num, 1);
        ets_delay_us(25);
        gpio_set_level(param_ptr_config->trigger_gpio_num, 0);
        portEXIT_CRITICAL(&jsnsr04t_spinlock);
    }

    /*
     * SENSOR Collect data:
     *   - Pull the data from the ring buffer.
     *
     *   @doc xRingbufferReceive() param#3 ticks_to_wait MAXIMUM nbr of Ticks to wait for items in the ringbuffer ELSE Timeout (NULL).
     */
    ptr_rx_item = (rmt_item32_t*) xRingbufferReceive(rb, &rx_size, RTOS_DELAY_100MILLISEC);
    if (ptr_rx_item == NULL) {
        f_retval = ESP_ERR_TIMEOUT;
        ESP_LOGE(TAG, "%s(). ABORT. xRingbufferReceive() | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        _mark_error_event(param_ptr_config, param_ptr_raw_data);
        // GOTO
        goto cleanup;
    }
    is_ringbuffer_received = true;
    int nbr_of_items = rx_size / sizeof(rmt_item32_t);

    // DEBUG Dump
    ESP_LOGD(TAG, "  nbr_of_items         = %i", nbr_of_items);
    ESP_LOGD(TAG, "  sizeof(rmt_item32_t) = %i", sizeof(rmt_item32_t));
    rmt_item32_t* temp_ptr = ptr_rx_item; // Use a temporary pointer (=pointing to the beginning of the item array)
    for (uint8_t i = 0; i < nbr_of_items; i++) {
        ESP_LOGD(TAG, "  %2i :: [level 0]: %1d - %5d microsec, [level 1]: %3d - %5d microsec",
                i,
                temp_ptr->level0, temp_ptr->duration0,
                temp_ptr->level1, temp_ptr->duration1);
        temp_ptr++;
    }

    // Check RMT nbr_of_items
    if (nbr_of_items != 1) {
        _mark_error_event(param_ptr_config, param_ptr_raw_data);
        f_retval = ESP_ERR_INVALID_RESPONSE;
        ESP_LOGE(TAG, "%s(). ABORT. RMT nbr_of_items != 1 (%i) | err %i (%s)", __FUNCTION__, nbr_of_items, f_retval,
                esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Check RMT first_item level0
    if (ptr_rx_item->level0 != 1) {
        f_retval = ESP_ERR_INVALID_RESPONSE;
        ESP_LOGE(TAG, "%s(). ABORT. RMT ptr_rx_item->level0 != 1 (%u) | err %i (%s)", __FUNCTION__, ptr_rx_item->level0,
                f_retval,
                esp_err_to_name(f_retval));
        _mark_error_event(param_ptr_config, param_ptr_raw_data);
        // GOTO
        goto cleanup;
    }

    // COMPUTE
    param_ptr_raw_data->data_received = true;
    param_ptr_raw_data->raw = ptr_rx_item->duration0; // Unit=microseconds
    param_ptr_raw_data->distance_cm = (param_ptr_raw_data->raw / 2) * 0.0342; // @uses Lightspeed

    if (param_ptr_raw_data->distance_cm < MJD_JSNSR04T_MINIMUM_SUPPORTED_DISTANCE_CM) {
        f_retval = ESP_ERR_INVALID_RESPONSE;
        ESP_LOGE(TAG, "%s(). ABORT. Out Of Range: distance_cm < %f (%f) | err %i (%s)", __FUNCTION__,
                MJD_JSNSR04T_MINIMUM_SUPPORTED_DISTANCE_CM, param_ptr_raw_data->distance_cm, f_retval,
                esp_err_to_name(f_retval));
        _mark_error_event(param_ptr_config, param_ptr_raw_data);
        // GOTO
        goto cleanup;
    }
    if (param_ptr_raw_data->distance_cm > MJD_JSNSR04T_MAXIMUM_SUPPORTED_DISTANCE_CM) {
        f_retval = ESP_ERR_INVALID_RESPONSE;
        ESP_LOGE(TAG, "%s(). ABORT. Out Of Range: distance_cm > %f (%f) | err %i (%s)", __FUNCTION__,
                MJD_JSNSR04T_MAXIMUM_SUPPORTED_DISTANCE_CM, param_ptr_raw_data->distance_cm, f_retval,
                esp_err_to_name(f_retval));
        _mark_error_event(param_ptr_config, param_ptr_raw_data);
        // GOTO
        goto cleanup;
    }

    // ADJUST with distance_sensor_to_artifact_cm (default 0cm).
    if (param_ptr_config->distance_sensor_to_artifact_cm != 0.0) {
        param_ptr_raw_data->distance_cm -= param_ptr_config->distance_sensor_to_artifact_cm;
        if (param_ptr_raw_data->distance_cm <= 0.0) {
            f_retval = ESP_ERR_INVALID_RESPONSE;
            ESP_LOGE(TAG,
                    "%s(). ABORT. Invalid value: adjusted distance <= 0 (subtracted sensor_artifact_cm) (%f) | err %i (%s)",
                    __FUNCTION__,
                    param_ptr_raw_data->distance_cm, f_retval, esp_err_to_name(f_retval));
            _mark_error_event(param_ptr_config, param_ptr_raw_data);
            // GOTO
            goto cleanup;
        }
    }

    // LABEL
    cleanup: ;

    if (is_ringbuffer_received == true) {
        // Compagnon func of xRingbufferReceive()
        // After parsing the data, return data to ringbuffer (as soon as possible).
        // Only return item when rigbuffer data was really received (error handling).
        vRingbufferReturnItem(rb, (void*) ptr_rx_item);
    }

    // @important Do NOT overwrite f_retval!
    if (rmt_rx_stop(param_ptr_config->rmt_channel) != ESP_OK) {
        ESP_LOGE(TAG, "%s(). ABORT. rmt_rx_stop() | err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        _mark_error_event(param_ptr_config, param_ptr_raw_data);
    }

    return f_retval;
}

/*
 * PUBLIC
 *
 * @brief Get ONE weighted Measurement
 *
 */
esp_err_t mjd_jsnsr04t_get_measurement(mjd_jsnsr04t_config_t* param_ptr_config, mjd_jsnsr04t_data_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    // Reset receive values
    param_ptr_data->data_received = false;
    param_ptr_data->is_an_error = false;
    param_ptr_data->distance_cm = 0.0;

    // Samples
    uint32_t count_errors = 0;
    double sum_distance_cm = 0;
    double min_distance_cm = FLT_MAX;
    double max_distance_cm = FLT_MIN;
    mjd_jsnsr04t_raw_data_t samples[param_ptr_config->nbr_of_samples];

    for (uint32_t j = 1; j <= param_ptr_config->nbr_of_samples; j++) {
        f_retval = _get_one_measurement(param_ptr_config, &samples[j]);
        if (f_retval != ESP_OK) {
            ESP_LOGW(TAG, "[CONTINUE-LOOP] %s(). WARNING. _get_one_measurement() | err %i (%s)", __FUNCTION__, f_retval,
                    esp_err_to_name(f_retval));
            // Do not exit loop!
        }
        _log_raw_data(samples[j]);

        // COUNT ERRORS
        if (samples[j].is_an_error == true) {
            count_errors += 1;
        }
        // SUM
        sum_distance_cm += samples[j].distance_cm;
        // IDENTIFY MIN VALUE and MAX VALUE
        if (samples[j].distance_cm < min_distance_cm) {
            min_distance_cm = samples[j].distance_cm;
        }
        if (samples[j].distance_cm > max_distance_cm) {
            max_distance_cm = samples[j].distance_cm;
        }
    }

    // Validation
    if (count_errors > 0) {
        ++param_ptr_config->nbr_of_errors;
        param_ptr_data->is_an_error = true; // Mark error
        f_retval = ESP_ERR_INVALID_RESPONSE;
        ESP_LOGE(TAG, "%s(). ABORT. At least one measurement in error (%u) | err %i (%s)", __FUNCTION__, count_errors,
                f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    if ((max_distance_cm - min_distance_cm) > param_ptr_config->max_range_allowed_in_samples_cm) {
        ++param_ptr_config->nbr_of_errors;
        param_ptr_data->is_an_error = true; // Mark error
        f_retval = ESP_ERR_INVALID_RESPONSE;
        ESP_LOGE(TAG,
                "%s(). ABORT. Statistics: range min-max is too big %f (max_range_allowed_in_samples_cm: %f) | err %i (%s)",
                __FUNCTION__,
                max_distance_cm - min_distance_cm, param_ptr_config->max_range_allowed_in_samples_cm, f_retval,
                esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Deduct one weighted measurement
    param_ptr_data->data_received = true;
    param_ptr_data->distance_cm = sum_distance_cm / param_ptr_config->nbr_of_samples;

    // LABEL
    cleanup: ;

    return f_retval;
}
