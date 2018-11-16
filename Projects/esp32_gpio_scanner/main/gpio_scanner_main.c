#include "mjd.h"

/*
 * Logging
 */
static const char TAG[] = "myapp";

/*
 * FreeRTOS settings
 */
#define MYAPP_RTOS_TASK_STACK_SIZE_LARGE (8192)

/*
 * Project Globs
 */

/*
 * MAIN
 */
void app_main() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    /* MY STANDARD Init */
    mjd_log_wakeup_details();
    mjd_log_chip_info();
    mjd_log_time();
    mjd_log_memory_statistics();
    ESP_LOGI(TAG, "@doc Wait 2 seconds after power-on (start logic analyzer, let sensors become active!)");
    vTaskDelay(RTOS_DELAY_2SEC);

    /*
     * LOLIN32LITE LED GPIO#22 = known
     *
     */

    /*
     * Find the LED's GPIO#
     *  @doc http://esp-idf.readthedocs.io/en/latest/api-reference/peripherals/gpio.html
     *      You cannot gpio_get_level() of an OUTPUT PIN!
     *      GPIO6-11 are used for SPI flash.
     *      GPIO34-39 can only be set as input mode and do not have software pullup or pulldown functions.
     */
    ESP_LOGI(TAG, "START SCAN");

    int lolin32lite_fatal_gpios[] = {0,1,6,7,8,10,11,20,24,28,29,30,31,34,35,36,37,38,39};

    int gpio_num;
    for (gpio_num = 0; gpio_num <= 39; gpio_num++) {
        ESP_LOGI(TAG, "\n***GPIO#: %i***", gpio_num);

        // SKIP gpio's which crash the MCU when manipulated
        int * pItem;
        pItem = (int*) bsearch (&gpio_num, lolin32lite_fatal_gpios, ARRAY_SIZE(lolin32lite_fatal_gpios), sizeof (int), mjd_compare_ints);
        if (pItem!=NULL) {
            ESP_LOGW(TAG, "  SKIPPING (known not-a-LED)");
            continue; // NEXT
        }

        // SETUP (Lolin32lite: the LED is *OFF initially, and it goes *ON instantly on after gpio_config()!)
        gpio_config_t io_conf = { 0 };
        io_conf.pin_bit_mask = (1ULL << gpio_num);
        io_conf.mode = GPIO_MODE_OUTPUT;
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        io_conf.intr_type = GPIO_INTR_DISABLE;
        ESP_LOGI(TAG, "gpio_config()");;
        if (ESP_OK != gpio_config(&io_conf)) {
            ESP_LOGE(TAG, "  gpio_config() failed -> SKIP");
            continue; // NEXT
        }
        vTaskDelay(RTOS_DELAY_1SEC);

        // *ON *OFF (Lolin32lite: setting level1 makes LED *OFF, setting level0 makes LED *ON -the opposite of the Adafruit HUZZAH32 pffff @cause LED is connected to VCC @ Lolin32lite (connected to GND @ Huzzah32).
        ESP_LOGI(TAG, "Set pin *HIGH...");
        if (ESP_OK != gpio_set_level(gpio_num, 1)) {
            ESP_LOGE(TAG, "  gpio_set_level(gpio_num, 1) failed -> SKIP");
            continue; // NEXT
        }
        vTaskDelay(RTOS_DELAY_5SEC);

        ESP_LOGI(TAG, "Set pin *LOW...");
        if (ESP_OK != gpio_set_level(gpio_num, 0)) {
            ESP_LOGE(TAG, "  gpio_set_level(gpio_num, 0) failed -> SKIP");
            continue; // NEXT
        }
        vTaskDelay(RTOS_DELAY_5SEC);
    }

    /*
     *
     */
    mjd_rtos_wait_forever();

    /*
     * END
     */
    ESP_LOGI(TAG, "END %s()", __FUNCTION__);
}
