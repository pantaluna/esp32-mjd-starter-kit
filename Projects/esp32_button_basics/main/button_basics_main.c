/*
 *
 */
#include "mjd.h"

/*
 * Logging
 */
static const char TAG[] = "myapp";

/*
 * GPIO selector (Adafruit HUZZAH32)
 */
#define GPIO_NUM_INPUT    (GPIO_NUM_16)
#define GPIO_BITPIN_INPUT (1ULL << GPIO_NUM_INPUT)

/*
 * FreeRTOS settings
 */
#define MYAPP_RTOS_TASK_STACK_SIZE_LARGE (8192)

/*
 * Project Globs
 */
SemaphoreHandle_t xButtonPressedInterruptSemaphore = NULL;

/*
 * Interrupts
 */
void IRAM_ATTR button_pressed_isr_handler(void* arg) {
    // @important Sometimes due to interference (also called contact bounce), we might get two irqs quickly after each other.
    //            This is solved by looking at the time between interrupts and refusing any interrupt too close to another one, and eventually comparing current with previous value.
    // @doc 1 Hz = the unit of frequency of one cycle per second.
    // @doc Compute in milliseconds:
    //          esp_clk_cpu_freq() = 160000000 (160Mhz) @source menuconfig
    //          CPU_TICKS_PER_MS   = 160000000 / 1000 = 160000
    const int BUTTON_INTERRUPT_DEBOUNCE_MS = 1000; // @doc 2000ms=2sec is usually high enough for BUTTONS (when you only want to know when it was pressed (ON)).
    const int CPU_TICKS_PER_MS = esp_clk_cpu_freq() / 1000;
    static uint32_t previous_time_ms = 0;
    uint32_t current_time_ms = xthal_get_ccount() / CPU_TICKS_PER_MS;
    if (current_time_ms - previous_time_ms < BUTTON_INTERRUPT_DEBOUNCE_MS) {
        return; //debounce: ignore everything < Xms after an earlier irq
    }
    previous_time_ms = current_time_ms;

    // @param pxHigherPriorityTaskWoken is pdTRUE if giving the semaphore caused a task to unblock, and the unblocked task has a priority higher than the currently running task.
    //        If xSemaphoreGiveFromISR() sets this value to pdTRUE then a context switch should be requested before the interrupt is exited.
    //        portYIELD_FROM_ISR() wakes up the imu_task immediately (instead of on next FreeRTOS tick).
    static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xButtonPressedInterruptSemaphore, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}

/*
 * MAIN
 */
void gpio_setup_gpiomux_sensor_to_led() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    mjd_led_off(HUZZAH32_GPIO_NUM_LED);

    // @special ESP32 GPIO-MUX routes the incoming signal to the outgoing on-board LED.
    //   @doc Only the loopback signals 224-228 can be configured to be routed from one input GPIO and directly to another output GPIO.
    gpio_matrix_in(GPIO_NUM_INPUT, SIG_IN_FUNC228_IDX, false);
    gpio_matrix_out(HUZZAH32_GPIO_NUM_LED, SIG_IN_FUNC228_IDX, false, false);
}

void app_main() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    gpio_config_t io_conf = { 0 };

    /* MY STANDARD Init */
    mjd_log_wakeup_details();
    mjd_log_chip_info();
    mjd_log_time();
    mjd_log_memory_statistics();
    ESP_LOGI(TAG, "@doc Wait 2 seconds after power-on (start logic analyzer, let sensors become active!)");
    vTaskDelay(RTOS_DELAY_2SEC);

    /*
     * LED setup & quick test
     */
    mjd_led_config_t led_config = { 0 };
    led_config.gpio_num = HUZZAH32_GPIO_NUM_LED;
    led_config.wiring_type  = LED_WIRING_TYPE_DIODE_TO_GND; // 1 GND MCU Huzzah32 | 2 VCC MCU Lolin32lite
    mjd_led_config(&led_config);

    mjd_led_on(HUZZAH32_GPIO_NUM_LED);
    vTaskDelay(RTOS_DELAY_1SEC);
    mjd_led_off(HUZZAH32_GPIO_NUM_LED);

    /*
     * INPUT GPIO#16
     */
    ESP_LOGI(TAG, "\n=====GPIO_MODE_INPUT (pullup YES, pulldown NO)");
    io_conf.pin_bit_mask = GPIO_BITPIN_INPUT;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;   // @important Enable PullDown so signal is LOW when button is not pressed
    io_conf.intr_type = GPIO_INTR_DISABLE;
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    ESP_LOGI(TAG, "GPIO_BITPIN_INPUT gpio_config(&io_conf);");
    ESP_LOGI(TAG, "  GPIO INPUT after gpio_config(): Expect 0 Actual=%i", gpio_get_level(GPIO_NUM_INPUT));
    vTaskDelay(RTOS_DELAY_1SEC);

    /*
     * SOP: button release & press
     */
    // Define Loopback input pin -> LED pin
    gpio_setup_gpiomux_sensor_to_led();

    ESP_LOGI(TAG, "\nACTION: Keep the button released (within 5 seconds please)...");
    vTaskDelay(RTOS_DELAY_5SEC);
    ESP_LOGI(TAG, "  GPIO INPUT: Expected 0 - Actual %i", gpio_get_level(GPIO_NUM_INPUT));

    ESP_LOGI(TAG, "\nACTION: Keep the button pressed (within 5 seconds please)...");
    vTaskDelay(RTOS_DELAY_5SEC);
    ESP_LOGI(TAG, "  GPIO INPUT: Expected 1 - Actual %i", gpio_get_level(GPIO_NUM_INPUT));

    /*
     * SOP: interrupts for edge changes
     */
    ESP_LOGI(TAG, "\n=====Button ISR");
    io_conf.pin_bit_mask = GPIO_BITPIN_INPUT;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;   // @important Enable internal PullDown so signal is LOW when button is not pressed
    io_conf.intr_type = GPIO_INTR_POSEDGE; // @doc GPIO_INTR_POSEDGE = GPIO interrupt type: rising edge
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    xButtonPressedInterruptSemaphore = xSemaphoreCreateBinary();
    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1); // @doc ESP_INTR_FLAG_LEVEL1 Accept a Level 1 interrupt vector (lowest priority)
    gpio_isr_handler_add(GPIO_NUM_INPUT, button_pressed_isr_handler, NULL);

    while (true) {
        if (xSemaphoreTake(xButtonPressedInterruptSemaphore, portMAX_DELAY) == pdTRUE) {
            ESP_LOGI(TAG, "***semaphore taken*** -> button was pressed :)"); // @important Gettting the current pin level is not possible (it is too late, it already happened!)
        }
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
