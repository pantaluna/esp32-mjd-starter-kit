#include "mjd.h"

#include "driver/ledc.h"

/*
 * Logging
 */
static const char TAG[] = "myapp";

/*
 * GPIO selector (Adafruit HUZZAH32)
 */

#define GPIO_NUM_OUTPUT_LED   (GPIO_NUM_14)

/*
 * FreeRTOS settings
 */

/*
 * Project Globs
 */

/*
 * MAIN
 */
void app_main() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    gpio_config_t io_conf =
        { 0 };

    /* MY STANDARD Init */
    mjd_log_wakeup_details();
    mjd_log_chip_info();
    mjd_log_time();
    mjd_log_memory_statistics();
    ESP_LOGI(TAG, "@doc Wait x seconds after power-on (start logic analyzer, let sensors become active)");
    vTaskDelay(RTOS_DELAY_1SEC);

    /*
     * LABEL
     */
    restart: ;

    /*
     * LED GPIO high/low
     */
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "=====GPIO_MODE_OUTPUT for GPIO_NUM_OUTPUT_SOLO#%i ", GPIO_NUM_OUTPUT_LED);
    ESP_LOGW(TAG, "  FAQ Pullup & pulldown settings are not relevant for OUTPUT pins");
    ESP_LOGW(TAG, "  FAQ gpio_GET_level() on an OUTPUT-only pin does not work, use multimeter");

    io_conf.pin_bit_mask = (1ULL << GPIO_NUM_OUTPUT_LED);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    ESP_LOGI(TAG, "\ngpio_config(&io_conf);");
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    vTaskDelay(RTOS_DELAY_1SEC);

    uint32_t counter = 0;
    while (++counter < 40) {
        gpio_set_level(GPIO_NUM_OUTPUT_LED, 1);
        /////ESP_LOGI(TAG, "\ngpio_set_level(GPIO_NUM_OUTPUT_SOLO, 1);");
        /////ESP_LOGI(TAG, "  GPIO OUTPUT: gpio_GET_level(x) does not work, use multimeter | Expect 1 Actual=%i", gpio_get_level(GPIO_NUM_OUTPUT_SOLO));
        vTaskDelay(RTOS_DELAY_50MILLISEC); // 10=no flickr 25=flickr

        gpio_set_level(GPIO_NUM_OUTPUT_LED, 0);
        /////ESP_LOGI(TAG, "\ngpio_set_level(GPIO_NUM_OUTPUT_SOLO, 0);");
        /////ESP_LOGI(TAG, "  GPIO OUTPUT: gpio_GET_level(x) does not work, use multimeter | Expect 0 Actual=%i", gpio_get_level(GPIO_NUM_OUTPUT_SOLO));
        vTaskDelay(RTOS_DELAY_50MILLISEC); // 10=no flickr 25=flickr
    }
    ESP_LOGI(TAG, "   wait 5sec...");
    vTaskDelay(RTOS_DELAY_5SEC);

    /*
     * LEDC PWM
     */
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "=====LEDC PWM for GPIO_NUM_OUTPUT_SOLO#%i ", GPIO_NUM_OUTPUT_LED);

    int32_t freq;
    int32_t duty;

    freq = 10;
    duty = 0;
    ESP_LOGI(TAG, "x. LEDC: ledc_timer_config() ledc_channel_config()");
    ESP_LOGI(TAG, "x. LEDC: LEDC_TIMER_8_BIT(duty 0..255) | freq=%u | duty = %u (LED stays *OFF)", freq, duty);
    vTaskDelay(RTOS_DELAY_2SEC);
    ledc_timer_config_t ledc_timer =
        { .speed_mode = LEDC_HIGH_SPEED_MODE,     // LEDC speed speed_mode, high-speed mode or low-speed mode
                .timer_num = LEDC_TIMER_0,              // The timer source of channel (0 - 3)
                .duty_resolution = LEDC_TIMER_8_BIT,    // range 0..255
                .freq_hz = freq,                        // changes/second
            };
    ledc_timer_config(&ledc_timer);
    ledc_channel_config_t ledc_channel =
        { .timer_sel = LEDC_TIMER_0, .speed_mode = LEDC_HIGH_SPEED_MODE, .channel = LEDC_CHANNEL_0, .gpio_num =
        GPIO_NUM_OUTPUT_LED, .duty = duty };
    ledc_channel_config(&ledc_channel);
    ESP_LOGI(TAG, "   wait 5sec...");
    vTaskDelay(RTOS_DELAY_5SEC);

    ledc_fade_func_install(0);

    // GOTO
    /////goto test;

    freq = 10;
    duty = 1;
    ESP_LOGI(TAG, "x. LEDC: LEDC_TIMER_8_BIT(duty 0..255) | freq=%u | duty = %u (flicker due to 10Hz)", freq, duty);
    vTaskDelay(RTOS_DELAY_2SEC);
    ledc_set_freq(LEDC_HIGH_SPEED_MODE, LEDC_TIMER_0, freq);
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
    ESP_LOGI(TAG, "   wait 5sec...");
    vTaskDelay(RTOS_DELAY_5SEC);

    freq = 10;
    duty = 128;
    ESP_LOGI(TAG, "x. LEDC: LEDC_TIMER_8_BIT(duty 0..255) | freq=%u | duty = %u (flicker due to 10Hz)", freq, duty);
    vTaskDelay(RTOS_DELAY_2SEC);
    ledc_set_freq(LEDC_HIGH_SPEED_MODE, LEDC_TIMER_0, freq);
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
    ESP_LOGI(TAG, "   wait 5sec...");
    vTaskDelay(RTOS_DELAY_5SEC);

    freq = 10;
    duty = 255;
ESP_LOGI(TAG, "x. LEDC: LEDC_TIMER_8_BIT(duty 0..255) | freq=%u | duty = %u (no flicker because duty = 100%%)", freq, duty);
    vTaskDelay(RTOS_DELAY_2SEC);
    ledc_set_freq(LEDC_HIGH_SPEED_MODE, LEDC_TIMER_0, freq);
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
    ESP_LOGI(TAG, "   wait 5sec...");
    vTaskDelay(RTOS_DELAY_5SEC);

    freq = 5;
    duty = 128;
    ESP_LOGI(TAG,
            "x. LEDC: LEDC_TIMER_8_BIT(duty 0..255) | vary freq 5..75 using ledc_set_freq() | duty=%u (no flicker above +-50Hz)",
            duty);
    vTaskDelay(RTOS_DELAY_2SEC);
    ledc_set_freq(LEDC_HIGH_SPEED_MODE, LEDC_TIMER_0, freq);
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
    for (freq = 5; freq < 75; ++freq) {
        ESP_LOGI(TAG, "  freq=%u duty=%u", freq, duty);
        ledc_set_freq(LEDC_HIGH_SPEED_MODE, LEDC_TIMER_0, freq);
        vTaskDelay(RTOS_DELAY_100MILLISEC);
    }
    ESP_LOGI(TAG, "   wait 5sec...");
    vTaskDelay(RTOS_DELAY_5SEC);

    freq = 10;
    duty = 0;
    ESP_LOGI(TAG, "x. LEDC: LEDC_TIMER_8_BIT(duty 0..255) | freq=%u | vary duty 0..255 using ledc_set_duty() (flicker due to 10Hz)",
            freq);
    vTaskDelay(RTOS_DELAY_2SEC);
    ledc_set_freq(LEDC_HIGH_SPEED_MODE, LEDC_TIMER_0, freq);
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
    for (duty = 0; duty < 256; ++duty) {
        ESP_LOGI(TAG, "  freq=%u duty=%u", freq, duty);
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty);
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
        vTaskDelay(RTOS_DELAY_25MILLISEC);
    }
    ESP_LOGI(TAG, "   wait 5sec...");
    vTaskDelay(RTOS_DELAY_5SEC);

    freq = 60;
    duty = 0;
    ESP_LOGI(TAG,
            "x. LEDC: LEDC_TIMER_8_BIT(duty 0..255) | freq=%u | vary duty 0..255 using ledc_set_duty() (no flicker due to 60Hz)",
            freq);
    vTaskDelay(RTOS_DELAY_2SEC);
    ledc_set_freq(LEDC_HIGH_SPEED_MODE, LEDC_TIMER_0, freq);
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
    for (duty = 0; duty < 256; ++duty) {
        ESP_LOGI(TAG, "  freq=%u duty=%u", freq, duty);
        ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty);
        ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
        vTaskDelay(RTOS_DELAY_25MILLISEC);
    }
    ESP_LOGI(TAG, "   wait 5sec...");
    vTaskDelay(RTOS_DELAY_5SEC);

    // LABEL
    /////test:;

    freq = 60;
    duty = 0;
    ESP_LOGI(TAG,
            "x. LEDC: LEDC_TIMER_8_BIT(duty 0..255) | freq=%u | vary duty 0..255 using ledc_set_fade_with_time(4 seconds LEDC_FADE_NO_WAIT)",
            freq);
    ledc_set_freq(LEDC_HIGH_SPEED_MODE, LEDC_TIMER_0, freq);
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
    ledc_set_fade_with_time(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 255, 4 * 1000);
    ledc_fade_start(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, LEDC_FADE_NO_WAIT); // LEDC_FADE_NO_WAIT LEDC_FADE_WAIT_DONE
    ESP_LOGI(TAG, "   wait 5 sec...");
    vTaskDelay(RTOS_DELAY_5SEC);

    freq = 60;
    duty = 255;
    ESP_LOGI(TAG,
            "x. LEDC: LEDC_TIMER_8_BIT(duty 0..255) | freq=%u | vary duty 255..0 using ledc_set_fade_with_time(4 seconds LEDC_FADE_WAIT_DONE)",
            freq);
    vTaskDelay(RTOS_DELAY_2SEC);
    ledc_set_freq(LEDC_HIGH_SPEED_MODE, LEDC_TIMER_0, freq);
    ledc_set_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0);
    ledc_set_fade_with_time(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, 0, 4 * 1000);
    ledc_fade_start(LEDC_HIGH_SPEED_MODE, LEDC_CHANNEL_0, LEDC_FADE_NO_WAIT); // LEDC_FADE_NO_WAIT LEDC_FADE_WAIT_DONE
    ESP_LOGI(TAG, "   wait 5 sec...");
    vTaskDelay(RTOS_DELAY_5SEC);

    ledc_fade_func_uninstall();

    // GOTO
    goto restart;

    /*
     *
     */
    // DEVTEMP
    mjd_rtos_wait_forever();
}
