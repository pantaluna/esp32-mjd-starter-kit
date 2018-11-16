#include "mjd.h"

/*
 * Logging
 */
static const char TAG[] = "myapp";

/*
 * GPIO selector (Adafruit HUZZAH32)
 */

// @important Make sure no wire is connected to the *_SOLO pins!
#define GPIO_NUM_INPUT_SOLO    (GPIO_NUM_16)
#define GPIO_NUM_OUTPUT_LED   (GPIO_NUM_17)

// @important Make sure both pins are connected to each other!
#define GPIO_NUM_INPUT_WIRED   (GPIO_NUM_21)
#define GPIO_NUM_OUTPUT_WIRED  (GPIO_NUM_23)

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
void gpio_setup_gpiomux_sensor_to_led() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    mjd_led_off(HUZZAH32_GPIO_NUM_LED);

    // @special ESP32 GPIO-MUX routes the incoming signal to the outgoing on-board LED.
    // @doc Only the loopback signals 224-228 can be configured to be routed from one input GPIO and directly to another output GPIO.
    gpio_matrix_in(GPIO_NUM_INPUT_WIRED, SIG_IN_FUNC225_IDX, false);
    gpio_matrix_out(HUZZAH32_GPIO_NUM_LED, SIG_IN_FUNC225_IDX, false, false);
}

void app_main() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    gpio_config_t io_conf = { 0 };

    /* MY STANDARD Init */
    mjd_log_wakeup_details();
    mjd_log_chip_info();
    mjd_log_time();
    mjd_log_memory_statistics();
    ESP_LOGI(TAG, "@doc Wait 2 seconds after power-on (start logic analyzer, let sensors become active)");
    vTaskDelay(RTOS_DELAY_2SEC);

    /*
     * LED setup & quick test
     */
    ESP_LOGI(TAG, "\n\n***SECTION: LED***");

    mjd_led_config_t led_config = { 0 };
    led_config.gpio_num = HUZZAH32_GPIO_NUM_LED;
    led_config.wiring_type  = LED_WIRING_TYPE_DIODE_TO_GND; // 1 GND MCU Huzzah32 | 2 VCC MCU Lolin32lite
    mjd_led_config(&led_config);

    mjd_led_on(HUZZAH32_GPIO_NUM_LED);
    vTaskDelay(RTOS_DELAY_2SEC);
    mjd_led_off(HUZZAH32_GPIO_NUM_LED);

    /*
     * TODO NOTWORKING Find the reset button PIN# and try to manipulate it
     */

    /*
     *
     */
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "=====GPIO_MODE_OUTPUT for GPIO_NUM_OUTPUT_SOLO#%i (pullup & pulldown are not relevant for OUTPUT pins!)", GPIO_NUM_OUTPUT_LED);
    ESP_LOGW(TAG, "  CIRCUIT No wires connected!");
    ESP_LOGW(TAG, "  FAQ Default value if unassigned: unknown for OUTPUT pin");
    ESP_LOGW(TAG, "  FAQ you cannot use gpio_GET_level() on an OUTPUT-only pin, use multimeter");

    io_conf.pin_bit_mask = (1ULL << GPIO_NUM_OUTPUT_LED);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    ESP_LOGI(TAG, "gpio_config(&io_conf);");
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    ESP_LOGI(TAG, "  GPIO OUTPUT: gpio_GET_level(x) does not work, use multimeter | Expect 0 Actual=%i", gpio_get_level(GPIO_NUM_OUTPUT_LED));
    vTaskDelay(RTOS_DELAY_1SEC);

    gpio_set_level(GPIO_NUM_OUTPUT_LED, 0);
    ESP_LOGI(TAG, "gpio_set_level(GPIO_NUM_OUTPUT_SOLO, 0);");
    ESP_LOGI(TAG, "  GPIO OUTPUT: gpio_GET_level(x) does not work, use multimeter | Expect 0 Actual=%i", gpio_get_level(GPIO_NUM_OUTPUT_LED));
    vTaskDelay(RTOS_DELAY_1SEC);

    gpio_set_level(GPIO_NUM_OUTPUT_LED, 1);
    ESP_LOGI(TAG, "gpio_set_level(GPIO_NUM_OUTPUT_SOLO, 1);");
    ESP_LOGI(TAG, "  GPIO OUTPUT: gpio_GET_level(x) does not work, use multimeter | Expect 1 Actual=%i", gpio_get_level(GPIO_NUM_OUTPUT_LED));
    vTaskDelay(RTOS_DELAY_1SEC);

    gpio_set_level(GPIO_NUM_OUTPUT_LED, 0);
    ESP_LOGI(TAG, "gpio_set_level(GPIO_NUM_OUTPUT_SOLO, 0);");
    ESP_LOGI(TAG, "  GPIO OUTPUT: gpio_GET_level(x) does not work, use multimeter | Expect 0 Actual=%i", gpio_get_level(GPIO_NUM_OUTPUT_LED));
    vTaskDelay(RTOS_DELAY_1SEC);

    /*
     *
     */
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "=====GPIO_MODE_INPUT for GPIO_NUM_INPUT_SOLO#%i (pullup NO, pulldown NO)", GPIO_NUM_INPUT_SOLO);
    ESP_LOGW(TAG, "  CIRCUIT No wires connected!");
    ESP_LOGW(TAG, "  FAQ Default value if unassigned: 0");
    ESP_LOGW(TAG, "  FAQ you cannot use gpio_SET_level() on an INPUT-only pin!");

    io_conf.pin_bit_mask = (1ULL << GPIO_NUM_INPUT_SOLO);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    ESP_LOGI(TAG, "gpio_config(&io_conf);");
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    ESP_LOGI(TAG, "  GPIO INPUT after gpio_config(): Expect 0 Actual=%i", gpio_get_level(GPIO_NUM_INPUT_SOLO));
    vTaskDelay(RTOS_DELAY_1SEC);

    /*
     *
     */
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "=====GPIO_MODE_INPUT for GPIO_NUM_INPUT_SOLO#%i (pullup YES ?difference in initial pin value?, pulldown NO)", GPIO_NUM_INPUT_SOLO);
    ESP_LOGW(TAG, "  CIRCUIT No wires connected!");
    ESP_LOGW(TAG, "  FAQ Default value if unassigned: 1 (***pullup***)");
    ESP_LOGW(TAG, "  FAQ you cannot use gpio_SET_level() on an INPUT-only pin!");

    io_conf.pin_bit_mask = (1ULL << GPIO_NUM_INPUT_SOLO);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    ESP_LOGI(TAG, "gpio_config(&io_conf);");
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    ESP_LOGI(TAG, "  GPIO INPUT after gpio_config(): Expect 1 (pullup) Actual=%i", gpio_get_level(GPIO_NUM_INPUT_SOLO));

    /*
     * Check output -> input
     *
     */
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "=====Use GPIO_NUM_OUTPUT_WIRED #%i to drive GPIO_NUM_INPUT_WIRED #%i (pullup YES, pulldown NO)", GPIO_NUM_OUTPUT_WIRED, GPIO_NUM_INPUT_WIRED);
    ESP_LOGW(TAG, "  CIRCUIT Connect wire between the input-output pins!");
    vTaskDelay(RTOS_DELAY_5SEC);
    ESP_LOGW(TAG, "  FAQ you cannot use gpio_SET_level() on an INPUT-only pin!");
    ESP_LOGW(TAG, "  FAQ you cannot use gpio_GET_level() on an OUTPUT-only pin!");

    gpio_setup_gpiomux_sensor_to_led();

    io_conf.pin_bit_mask = (1ULL << GPIO_NUM_OUTPUT_WIRED);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    ESP_LOGI(TAG, "gpio_config(&io_conf);");
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    ESP_LOGI(TAG, "gpio_set_level(GPIO_NUM_OUTPUT_WIRED, 0);");
    gpio_set_level(GPIO_NUM_OUTPUT_WIRED, 0);
    vTaskDelay(RTOS_DELAY_5SEC);

    io_conf.pin_bit_mask = (1ULL << GPIO_NUM_INPUT_WIRED);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    ESP_LOGI(TAG, "gpio_config(&io_conf);");
    ESP_LOGI(TAG, "  GPIO INPUT after gpio_config(): Expect 0 (=value of gpio_output) Actual=%i", gpio_get_level(GPIO_NUM_INPUT_WIRED));
    vTaskDelay(RTOS_DELAY_5SEC);

    gpio_set_level(GPIO_NUM_OUTPUT_WIRED, 0);
    ESP_LOGI(TAG, "gpio_set_level(GPIO_NUM_OUTPUT_WIRED, 0);");
    ESP_LOGI(TAG, "  GPIO INPUT: Expect 0 Actual=%i", gpio_get_level(GPIO_NUM_INPUT_WIRED));
    vTaskDelay(RTOS_DELAY_5SEC);

    gpio_set_level(GPIO_NUM_OUTPUT_WIRED, 1);
    ESP_LOGI(TAG, "gpio_set_level(GPIO_NUM_OUTPUT_WIRED, 1);");
    ESP_LOGI(TAG, "  GPIO INPUT: Expect 1 Actual=%i", gpio_get_level(GPIO_NUM_INPUT_WIRED));
    vTaskDelay(RTOS_DELAY_5SEC);

    gpio_set_level(GPIO_NUM_OUTPUT_WIRED, 0);
    ESP_LOGI(TAG, "gpio_set_level(GPIO_NUM_OUTPUT_WIRED, 0);");
    ESP_LOGI(TAG, "  GPIO INPUT: Expect 0 Actual=%i", gpio_get_level(GPIO_NUM_INPUT_WIRED));
    vTaskDelay(RTOS_DELAY_5SEC);

    gpio_set_level(GPIO_NUM_OUTPUT_WIRED, 1);
    ESP_LOGI(TAG, "gpio_set_level(GPIO_NUM_OUTPUT_WIRED, 1);");
    ESP_LOGI(TAG, "  GPIO INPUT: Expect 1 Actual=%i", gpio_get_level(GPIO_NUM_INPUT_WIRED));
    vTaskDelay(RTOS_DELAY_5SEC);

    /*
     *
     */
    mjd_rtos_wait_forever();

    /*
     * END
     */
    ESP_LOGI(TAG, "END %s()", __FUNCTION__);
}
