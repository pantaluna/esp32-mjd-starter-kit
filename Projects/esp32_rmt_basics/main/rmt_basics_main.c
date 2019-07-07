#include "mjd.h"

/*
 * Logging
 */
static const char TAG[] = "myapp";

/*
 * KConfig: LED
 */
#define MY_LED_ON_DEVBOARD_GPIO_NUM (CONFIG_MY_LED_ON_DEVBOARD_GPIO_NUM)
#define MY_LED_ON_DEVBOARD_WIRING_TYPE (CONFIG_MY_LED_ON_DEVBOARD_WIRING_TYPE)

// default 14
#define MY_1WIRE_GPIO_NUM (CONFIG_MY_1WIRE_GPIO_NUM)

/*
 * FreeRTOS settings
 */
#define MYAPP_RTOS_TASK_STACK_SIZE_LARGE (8192)
#define MYAPP_RTOS_TASK_PRIORITY_NORMAL (RTOS_TASK_PRIORITY_NORMAL)

/*
 * DECLS
 */

#define LEVEL_HIGH (1)
#define LEVEL_LOW  (0)

/*
 * TASKS
 */
void main_task(void *pvParameter)
{
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    mjd_log_memory_statistics();

    /**************************************************************************
     * Reuseable variables
     */
    esp_err_t f_retval;

    /**************************************************************************
     * INIT CH0
     *
     * @doc RMT source clock = APB Clock 80Mhz.
     *      It ticks 80,000,000 times a second or 80,000 times a MILLIsecond or 80 times a MICROsecond or 0.08 times a NANOsecond.
     *
     * @source my_spreadsheet
     *      APB Clock Divider: 4 => Nanoseconds per Tick: 50.0ns
     *
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
#define MY_RMT_CLK_DIV (4)
#define MY_RMT_ONE_TICK_DURATION_NANOSEC (50.0)

    rmt_config_t rmt_cfg_ch0 =
                {
                        0
                };
    rmt_cfg_ch0.channel = RMT_CHANNEL_0;
    rmt_cfg_ch0.gpio_num = MY_1WIRE_GPIO_NUM;
    rmt_cfg_ch0.rmt_mode = RMT_MODE_TX;
    rmt_cfg_ch0.clk_div = MY_RMT_CLK_DIV; /*!< [value 1..255] RMT ABP clock divider */
    rmt_cfg_ch0.mem_block_num = 1; /*!< RMT memory block number */
    rmt_cfg_ch0.tx_config.loop_en = false; /*!< Enable sending RMT items in a loop */
    rmt_cfg_ch0.tx_config.idle_output_en = true; /*!< RMT idle level output enable */
    rmt_cfg_ch0.tx_config.idle_level = RMT_IDLE_LEVEL_LOW; /*!< RMT idle level */

    f_retval = rmt_config(&rmt_cfg_ch0);
    if (f_retval != ESP_OK)
    {
        ESP_LOGE(TAG, "rmt_config(CH0) err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    f_retval = rmt_driver_install(rmt_cfg_ch0.channel, 0, 0);
    if (f_retval != ESP_OK)
    {
        ESP_LOGE(TAG, "rmt_driver_install(CH0) err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /**************************************************************************
     * RMT TX and measure timings with logic analyzer.
     *
     * The initializer structure represents the rmt_item32_t struct:
     *      {duration, level, duration, level}
     *
     * Test the values of 1, 10, 100 and validate them using a logic analyzer. One tick is 50 nanoseconds (see section rmt_config()).
     */
    rmt_item32_t items[] =
                {
                            {
                                        {
                                                    {
                                                            1, LEVEL_HIGH, 1, LEVEL_LOW
                                                    }
                                        }
                            },
                            {
                                        {
                                                    {
                                                            1, LEVEL_HIGH, 1, LEVEL_LOW
                                                    }
                                        }
                            },
                            {
                                        {
                                                    {
                                                            1, LEVEL_HIGH, 1, LEVEL_LOW
                                                    }
                                        }
                            },
                            {
                                        {
                                                    {
                                                            1, LEVEL_HIGH, 1, LEVEL_LOW
                                                    }
                                        }
                            },
                            {
                                        {
                                                    {
                                                            1, LEVEL_HIGH, 1, LEVEL_LOW
                                                    }
                                        }
                            },
                            {
                                        {
                                                    {
                                                            10, LEVEL_HIGH, 10, LEVEL_LOW
                                                    }
                                        }
                            },
                            {
                                        {
                                                    {
                                                            10, LEVEL_HIGH, 10, LEVEL_LOW
                                                    }
                                        }
                            },
                            {
                                        {
                                                    {
                                                            10, LEVEL_HIGH, 10, LEVEL_LOW
                                                    }
                                        }
                            },
                            {
                                        {
                                                    {
                                                            10, LEVEL_HIGH, 10, LEVEL_LOW
                                                    }
                                        }
                            },
                            {
                                        {
                                                    {
                                                            10, LEVEL_HIGH, 10, LEVEL_LOW
                                                    }
                                        }
                            },
                            {
                                        {
                                                    {
                                                            100, LEVEL_HIGH, 100, LEVEL_LOW
                                                    }
                                        }
                            },
                            {
                                        {
                                                    {
                                                            100, LEVEL_HIGH, 100, LEVEL_LOW
                                                    }
                                        }
                            },
                            {
                                        {
                                                    {
                                                            100, LEVEL_HIGH, 100, LEVEL_LOW
                                                    }
                                        }
                            },
                            {
                                        {
                                                    {
                                                            100, LEVEL_HIGH, 100, LEVEL_LOW
                                                    }
                                        }
                            },
                            {
                                        {
                                                    {
                                                            100, LEVEL_HIGH, 100, LEVEL_LOW
                                                    }
                                        }
                            },
                };

    uint8_t nbr_of_items = ARRAY_SIZE(items);

    ESP_LOGD(TAG, "DEBUG");
    ESP_LOGD(TAG, "  sizeof(rmt_item32_t) = %i", sizeof(rmt_item32_t));  // sizeof(rmt_item32_t) = 4
    ESP_LOGD(TAG, "  nbr_of_items         = %i", nbr_of_items);

    rmt_item32_t *ptr_tmp_items = items; // Create a temporary pointer (=pointing to the beginning of the item array)
    for (uint8_t i = 0; i < nbr_of_items; i++)
            {
        ESP_LOGD(TAG, "  %3i :: [lvl0] %4u - %4i (%6.1f ns) | [lvl1] %4u - %4i (%6.1f ns) | value uint32: %u", i,
                ptr_tmp_items->level0, ptr_tmp_items->duration0, ptr_tmp_items->duration0 * MY_RMT_ONE_TICK_DURATION_NANOSEC,
                ptr_tmp_items->level1, ptr_tmp_items->duration1, ptr_tmp_items->duration1 * MY_RMT_ONE_TICK_DURATION_NANOSEC,
                ptr_tmp_items->val);
        ptr_tmp_items++; // Advance ptr
    }

    f_retval = rmt_write_items(rmt_cfg_ch0.channel, items, nbr_of_items, true);
    if (f_retval != ESP_OK)
    {
        ESP_LOGE(TAG, "rmt_write_items(CH0) err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // LABEL
    cleanup: ;

    if (f_retval != ESP_OK)
    {
        mjd_led_mark_error(MY_LED_ON_DEVBOARD_GPIO_NUM);
    }

    ESP_LOGI(TAG, "@doc Wait 1 sec after the RMT session");
    vTaskDelay(RTOS_DELAY_1SEC);

    /********************************************************************************
     * Task Delete
     * @doc Passing NULL will end the current task
     */
    vTaskDelete(NULL);
}

/*
 * MAIN
 */
void app_main()
{
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    BaseType_t xReturned;

    /********************************************************************************
     * STANDARD Init
     */
    mjd_log_wakeup_details();
    mjd_log_chip_info();
    mjd_log_time();
    mjd_log_memory_statistics();
    ESP_LOGI(TAG, "@doc Wait 2 seconds after power-on (start logic analyzer, let sensors become active, ...)");
    vTaskDelay(RTOS_DELAY_2SEC);

    /********************************************************************************
     * LED
     */
    ESP_LOGI(TAG, "\n\n***SECTION: LED***");
    ESP_LOGI(TAG, "MY_LED_ON_DEVBOARD_GPIO_NUM:    %i", MY_LED_ON_DEVBOARD_GPIO_NUM);
    ESP_LOGI(TAG, "MY_LED_ON_DEVBOARD_WIRING_TYPE: %i", MY_LED_ON_DEVBOARD_WIRING_TYPE);

    mjd_log_memory_statistics();

    mjd_led_config_t led_cfg =
                {
                        0
                };
    led_cfg.gpio_num = MY_LED_ON_DEVBOARD_GPIO_NUM;
    led_cfg.wiring_type = MY_LED_ON_DEVBOARD_WIRING_TYPE; // 1 GND MCU Huzzah32 | 2 VCC MCU Lolin32lite
    mjd_led_config(&led_cfg);

    /**********
     * TASK: main_task
     *  @important For stability (RMT + Wifi etc.): always use xTaskCreatePinnedToCore(APP_CPU_NUM) [Opposed to xTaskCreate() which might run the code on PRO_CPU_NUM...]
     */
    xReturned = xTaskCreatePinnedToCore(&main_task, "main_task (name)", MYAPP_RTOS_TASK_STACK_SIZE_LARGE, NULL,
            MYAPP_RTOS_TASK_PRIORITY_NORMAL, NULL, APP_CPU_NUM);
    if (xReturned == pdPASS)
    {
        ESP_LOGI(TAG, "OK Task main_task has been created, and is running right now");
    }

    /**********
     * END
     */
    ESP_LOGI(TAG, "END %s()", __FUNCTION__);
}
