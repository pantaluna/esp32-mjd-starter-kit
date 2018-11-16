#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#include "esp_event_loop.h"
#include "esp_clk.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "esp_system.h"

#include "driver/rmt.h"
#include "nvs_flash.h"
#include "soc/rmt_reg.h"
#include "soc/soc.h"

/*
 * Logging
 */
static const char *TAG = "myapp";

/*
 * FreeRTOS settings
 */
#define MYAPP_RTOS_TASK_STACK_SIZE_LARGE (8192)

/*
 * MJDLIB: *BEGIN
 */
// RTOS
#define xDelay1s (1000 / portTICK_PERIOD_MS)
#define xDelay3s (3000 / portTICK_PERIOD_MS)
#define TASK_PRIORITY_NORMAL (5)

uint32_t mjdlib_seconds_to_milliseconds(uint32_t seconds) {
    return seconds * 1000;
}

uint32_t mjdlib_seconds_to_microseconds(uint32_t seconds) {
    return seconds * 1000 * 1000;
}

/*
 * MJDLIB: *END
 */

/*
 * GPIO selector (Adafruit HUZZAH32)
 *   @doc #13 = Blue LED on the PCB
 *   @doc #21 = DHT11 temperature sensor / Sw180 tilt sensor
 */
#define GPIO_NUM_LED    (GPIO_NUM_13)
#define GPIO_NUM_SENSOR (GPIO_NUM_21)

/*
 * Project Globs
 */
SemaphoreHandle_t xSensorInterruptSemaphore = NULL;

/*
 * INIT
 */
void gpio_setup_led() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    gpio_config_t io_conf;

    io_conf.pin_bit_mask = (1ULL << GPIO_NUM_LED);
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&io_conf);
}

void gpio_setup_sensor_sw180() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    gpio_config_t io_conf;

    io_conf.pin_bit_mask = (1ULL << GPIO_NUM_SENSOR);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE; // @important
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_ANYEDGE; // @important
    gpio_config(&io_conf);
}

void gpio_setup_gpiomux_sensor_to_led() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    // @special ESP32 GPIO-MUX routes the incoming sensor signal to the on-board LED.
    // @doc Only the peripheral output signals 224 to 228 can be configured to be routed in from one GPIO and output directly from another GPIO.
    // @doc There are two ways of mapping pins: the GPIO Mux (preferred) and the IO_MUX.
    //          The GPIO mux allows you to map a peripheral to any pin, but it has the disadvantage that it introduces an APB clock (80MHz) tick delay in the signal;
    //          for some purposes this can be disadvantageous.
    //          We also have the IO_MUX, which can map the signals of some peripherals to a very limited amount of IO pins; this reduces flexibility
    //          but because the IO_MUX does not have the delay, it can be useful for some purposes.
    gpio_matrix_in(GPIO_NUM_SENSOR, SIG_IN_FUNC228_IDX, true);  // @special param3=true => invert because 1=nothing detected and 0=obstacle detected
    gpio_matrix_out(GPIO_NUM_LED, SIG_IN_FUNC228_IDX, false, false);
}

/*
 * Interrupt Services
 */
#define SENSOR_INTERRUPT_DEBOUNCE_MS (250) // 1000 = 1 sec

void sensor_isr_handler(void* arg) {
    // @important Sometimes due to interference or ringing or something else, we might get two irqs quickly after each other.
    //            This is solved by looking at the time between interrupts and refusing any interrupt too close to another one.
    // @doc 1 Hz = the unit of frequency of one cycle per second.
    // @doc Compute in milliseconds:
    //          esp_clk_cpu_freq() = 160000000 (160Mhz) @source menuconfig
    //          cpu_ticks_per_ms   =    160000
    const int cpu_ticks_per_ms = esp_clk_cpu_freq() / 1000;
    static uint32_t previous_time_ms = 0;
    uint32_t current_time_ms = xthal_get_ccount() / cpu_ticks_per_ms;
    if (current_time_ms - previous_time_ms < SENSOR_INTERRUPT_DEBOUNCE_MS) {
        return; //debounce: ignore everything < Xms after an earlier irq
    }
    previous_time_ms = current_time_ms;

    // @param pxHigherPriorityTaskWoken to pdTRUE if giving the semaphore caused a task to unblock, and the unblocked task has a priority higher than the currently running task.
    //        If xSemaphoreGiveFromISR() sets this value to pdTRUE then a context switch should be requested before the interrupt is exited.
    //        portYIELD_FROM_ISR() wakes up the imu_task immediately (instead of on next FreeRTOS tick).
    static BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xSensorInterruptSemaphore, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}

/*
 * TASK
 */
void gpio_loop_get_level_task(void *pvParameter) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    gpio_setup_led();
    gpio_setup_sensor_sw180();

    // @special ESP32 GPIO-MUX routes the incoming sensor signal to the on-board LED.
    // @doc Only the peripheral output signals 224 to 228 can be configured to be routed in from one GPIO and output directly from another GPIO.
    // @doc There are two ways of mapping pins: the GPIO Mux (preferred) and the IO_MUX.
    //          The GPIO mux allows you to map a peripheral to any pin, but it has the disadvantage that it introduces an APB clock (80MHz) tick delay in the signal;
    //          for some purposes this can be disadvantageous.
    //          We also have the IO_MUX, which can map the signals of some peripherals to a very limited amount of IO pins; this reduces flexibility
    //          but because the IO_MUX does not have the delay, it can be useful for some purposes.
    gpio_matrix_in(GPIO_NUM_SENSOR, SIG_IN_FUNC228_IDX, false);
    gpio_matrix_out(GPIO_NUM_LED, SIG_IN_FUNC228_IDX, false, false);

    while (1) {
        printf("GPIO#%d (tilt sensor), current value: %d\n", GPIO_NUM_SENSOR, gpio_get_level(GPIO_NUM_SENSOR));
        printf("  LED GPIO#13 should blink when the sensor signal = 1 ON\n");
        printf("  Wait a bit at the end of the loop\n\n");
        vTaskDelay(2 * 1000 / portTICK_PERIOD_MS);
    }
}

/*
 * TASK
 */
void gpio_semaphore_interrupt_task(void *pvParameter) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    gpio_setup_led();
    gpio_setup_sensor_sw180();
    gpio_setup_gpiomux_sensor_to_led();

    while (true) {
        if (xSemaphoreTake(xSensorInterruptSemaphore,portMAX_DELAY) == pdTRUE) {
            ESP_LOGI(TAG, "***semaphore taken***");
            ESP_LOGI(TAG, "  GPIO#%d (tilt sensor), current value: %d\n", GPIO_NUM_SENSOR, gpio_get_level(GPIO_NUM_SENSOR));
        }
    }
}

/*
 * MAIN
 */
void app_main() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);
    BaseType_t xReturned;

    /* SOC Init */
    ESP_LOGI(TAG, "@doc exec nvs_flash_init() - mandatory for Wifi");
    nvs_flash_init();

    /*
     * Task
     */
     /*xReturned = xTaskCreatePinnedToCore(&loop_gpio_get_level_task, "loop_gpio_get_level_task (name)", 2048, NULL, TASK_PRIORITY_NORMAL, NULL, APP_CPU_NUM);
     if (xReturned == pdPASS) {
     printf("OK Task has been created, and is running right now\n");
     }
     ESP_LOGI(TAG, "app_main() END");*/

    /*
     * Task
     */
    xSensorInterruptSemaphore = xSemaphoreCreateBinary();

    xReturned = xTaskCreatePinnedToCore(&gpio_semaphore_interrupt_task, "gpio_semaphore_interrupt_task (name)", MYAPP_RTOS_TASK_STACK_SIZE_LARGE, NULL, TASK_PRIORITY_NORMAL, NULL, APP_CPU_NUM);
    if (xReturned == pdPASS) {
        printf("OK Task has been created, and is running right now\n");
    }

    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1); ///< Accept a Level 1 interrupt vector (lowest priority)
    gpio_isr_handler_add(GPIO_NUM_SENSOR, sensor_isr_handler, NULL);

    /*
     * END
     */
    ESP_LOGI(TAG, "app_main() END");

}
