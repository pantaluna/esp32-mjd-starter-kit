#include <inttypes.h>

#include "mjd.h"

#include "driver/timer.h"

/**
 * Logging
 */
static const char TAG[] = "myapp";

/*
 * KConfig: LED, WIFI
 */
static const int MY_LED_ON_DEVBOARD_GPIO_NUM = CONFIG_MY_LED_ON_DEVBOARD_GPIO_NUM;
static const int MY_LED_ON_DEVBOARD_WIRING_TYPE = CONFIG_MY_LED_ON_DEVBOARD_WIRING_TYPE;

/**
 * FreeRTOS settings
 */
#define MYAPP_RTOS_TASK_STACK_SIZE_16K (16384)
#define MYAPP_RTOS_TASK_PRIORITY_NORMAL (RTOS_TASK_PRIORITY_NORMAL)

/**
 * HELPERS
 */

/**
 * TASK
 */
void main_task(void *pvParameter) {
    ESP_LOGI(TAG, "%s()", __FUNCTION__);

    /********************************************************************************
     * Reuseable variables
     */
    esp_err_t f_retval;

    /********************************************************************************
     * MY STANDARD Init
     */
    mjd_log_wakeup_details();
    mjd_log_chip_info();
    mjd_log_time();
    mjd_log_memory_statistics();
    /////ESP_LOGI(TAG, "@doc Wait 2 seconds after power-on (start logic analyzer, let peripherals become active!)");
    /////vTaskDelay(RTOS_DELAY_2SEC);

    /********************************************************************************
     * LED
     */
    ESP_LOGI(TAG, "\n\n***SECTION: LED***");
    ESP_LOGI(TAG, "  MY_LED_ON_DEVBOARD_GPIO_NUM:    %i", MY_LED_ON_DEVBOARD_GPIO_NUM);
    ESP_LOGI(TAG, "  MY_LED_ON_DEVBOARD_WIRING_TYPE: %i", MY_LED_ON_DEVBOARD_WIRING_TYPE);

    mjd_led_config_t led_config = { 0 };
    led_config.gpio_num = MY_LED_ON_DEVBOARD_GPIO_NUM;
    led_config.wiring_type = MY_LED_ON_DEVBOARD_WIRING_TYPE; // 1 GND MCU Huzzah32 | 2 VCC MCU Lolin32lite
    mjd_led_config(&led_config);

    ESP_LOGI(TAG, "LED on off");
    mjd_led_on(MY_LED_ON_DEVBOARD_GPIO_NUM);
    vTaskDelay(RTOS_DELAY_500MILLISEC);
    mjd_led_off(MY_LED_ON_DEVBOARD_GPIO_NUM);

    // DEVTEMP: HALT
    /////mjd_rtos_wait_forever();

    /********************************************************************************
     * TIMER TESTING 123...
     */

    /* timer init
     * APB (Advanced Peripheral Bus) clock frequency (Hz): 80000000 ***KEEP THIS TECH INFO FOR FUTURE PROJECTS***
     *   timer.h #define TIMER_BASE_CLK  (APB_CLK_FREQ)  !< Frequency of the clock on the input of the timer groups !>
     *
     * config.divider The lower the divider, the faster the timer will tick. The total divider's range is [2..65536].
     *    => 1 Hz is one cycle per second
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
     *       tmrval => sec: *time = (double) timer_val * div / TIMER_BASE_CLK;
     */

    // timer init
    timer_config_t tconfig = {};
    tconfig.divider = 64000; // Let the timer tick on a relative slow pace. 1.25 Khz: esp_clk_apb_freq() / 64000 = 1250 ticks/second
    tconfig.counter_dir = TIMER_COUNT_UP;
    tconfig.counter_en = TIMER_PAUSE;
    tconfig.alarm_en = TIMER_ALARM_DIS;
    tconfig.auto_reload = false;
    f_retval = timer_init(TIMER_GROUP_0, TIMER_0, &tconfig);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). timer_init() err %d %s", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // timer starts with this value.
    timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 00000000ULL);
    timer_start(TIMER_GROUP_0, TIMER_0);

    // wait for updated datetime
    uint64_t timer_counter_value;
    double timer_counter_value_seconds;

    const uint64_t TEN_SECONDS_OF_TIMER = 10ULL;
    ESP_LOGI(TAG, "@doc Restart the timer after every %" PRIu64 " seconds", TEN_SECONDS_OF_TIMER);

    const uint32_t NBR_OF_CYCLES = 500;
    ESP_LOGI(TAG, "@doc LOOP: %u cycles (an arbitrary number of times to show how the timer works and restarts every X seconds)", NBR_OF_CYCLES);

    vTaskDelay(RTOS_DELAY_3SEC);

    for (uint32_t i = 1;  i <= NBR_OF_CYCLES; i++) {
        timer_get_counter_value(TIMER_GROUP_0, TIMER_0, &timer_counter_value);
        timer_get_counter_time_sec(TIMER_GROUP_0, TIMER_0, &timer_counter_value_seconds);
        ESP_LOGI(TAG, "cycle %5i: T0 counter value =  %" PRIu64 " | counter value seconds = %f", i, timer_counter_value, timer_counter_value_seconds);

        // Reset timer value to 0 after 10 seconds
        if (timer_counter_value_seconds >= TEN_SECONDS_OF_TIMER) {
            timer_set_counter_value(TIMER_GROUP_0, TIMER_0, 00000000ULL);
            ESP_LOGI(TAG, "\n\n***Timer reset to ZERO***\n");
        }

        vTaskDelay(RTOS_DELAY_100MILLISEC); // @important values lower than 10 millisec does not work accurately enough with this RTOS func()
    }

    // pause timer (stop = n.a.)
    //   @doc The timer may be paused at any time by calling timer_pause(). To start it again call timer_start()
    timer_pause(TIMER_GROUP_0, TIMER_0);

    // LABEL
    cleanup:;

    ESP_LOGI(TAG, "*DONE");

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
