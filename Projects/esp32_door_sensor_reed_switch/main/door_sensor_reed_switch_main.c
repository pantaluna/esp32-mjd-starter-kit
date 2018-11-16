/*
 * MJD HARDWARE INSTRUCTIONS: README.MD
 *
 */
#include "mjd.h"

/*
 * Logging
 */
static const char TAG[] = "myapp";

/*
 * FreeRTOS settings
 */
#define MYAPP_RTOS_TASK_STACK_SIZE_LARGE (8192)
#define MYAPP_RTOS_TASK_PRIORITY_NORMAL (RTOS_TASK_PRIORITY_NORMAL)

/*
 * KConfig: LED, WIFI
 */
static const int MY_LED_ON_DEVBOARD_GPIO_NUM = CONFIG_MY_LED_ON_DEVBOARD_GPIO_NUM;
static const int MY_LED_ON_DEVBOARD_WIRING_TYPE = CONFIG_MY_LED_ON_DEVBOARD_WIRING_TYPE;
static const int MY_GPIO_INPUT_PIN = CONFIG_MY_GPIO_INPUT_PIN;

/**
 * SELECT the run mode of the app
 */
typedef enum {
    MJD_APP_MODE_SIMPLE_LOOP = 0,
    MJD_APP_MODE_INTERRUPT,
    MJD_APP_MODE_MAX
} mjd_app_mode_t;

/////mjd_app_mode_t mjd_app_mode = MJD_APP_MODE_SIMPLE_LOOP;
mjd_app_mode_t mjd_app_mode = MJD_APP_MODE_INTERRUPT;

/*
 * MAIN
 */
SemaphoreHandle_t xInterruptSemaphore = NULL;

void IRAM_ATTR sensor_isr_handler(void* arg) {
    // @important Sometimes due to interference (also called contact bounce), we might get two irqs quickly after each other.
    //            This is solved by looking at the time between interrupts and refusing any interrupt that occurs too fast after the previous one.
    // @doc 1 Hz = the unit of frequency of one cycle per second.
    // @doc Compute in milliseconds:
    //          esp_clk_cpu_freq() = 160000000 (160Mhz) @source menuconfig
    //          CPU_TICKS_PER_MS   = 160000000 / 1000 = 160000
    const int BUTTON_INTERRUPT_DEBOUNCE_MS = 250; // @doc PROD 250ms=0.25sec is usually high enough for BUTTONS
    const int CPU_TICKS_PER_MS = esp_clk_cpu_freq() / 1000;
    static uint32_t previous_time_ms = 0;
    uint32_t current_time_ms = xthal_get_ccount() / CPU_TICKS_PER_MS;
    if (current_time_ms - previous_time_ms < BUTTON_INTERRUPT_DEBOUNCE_MS) {
        return; //debounce: ignore interrupts occuring < Xms after the previous interrupt
    }
    previous_time_ms = current_time_ms;

    // @param pxHigherPriorityTaskWoken to pdTRUE if giving the semaphore caused a task to unblock, and the unblocked task has a priority higher than the currently running task.
    //        If xSemaphoreGiveFromISR() sets this value to pdTRUE then a context switch should be requested before the interrupt is exited.
    //        portYIELD_FROM_ISR() wakes up the imu_task immediately (instead of on next FreeRTOS tick).
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(xInterruptSemaphore, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}

void gpio_setup_gpiomux_sensor_to_led() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    // @doc ESP32 GPIO-MUX routes the incoming signal to the outgoing on-board LED.
    //   @special Only the loopback signals 224-228 can be used to route a signal from one input GPIO to another output GPIO.
    gpio_matrix_in(MY_GPIO_INPUT_PIN, SIG_IN_FUNC224_IDX, false);
    gpio_matrix_out(MY_LED_ON_DEVBOARD_GPIO_NUM, SIG_IN_FUNC224_IDX, false, false);
}

/*
 * TASK
 */
void main_task(void *pvParameter) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    gpio_config_t io_conf =
        { 0 };

    /* MY STANDARD Init */
    mjd_log_wakeup_details();
    mjd_log_time();
    mjd_log_memory_statistics();

    /*
     * LED setup & quick test
     */
    ESP_LOGI(TAG, "\n\n***SECTION: LED***");
    ESP_LOGI(TAG, "MY_LED_ON_DEVBOARD_GPIO_NUM:    %i", MY_LED_ON_DEVBOARD_GPIO_NUM);
    ESP_LOGI(TAG, "MY_LED_ON_DEVBOARD_WIRING_TYPE: %i", MY_LED_ON_DEVBOARD_WIRING_TYPE);

    mjd_led_config_t led_config =
        { 0 };
    led_config.gpio_num = MY_LED_ON_DEVBOARD_GPIO_NUM;
    led_config.wiring_type = MY_LED_ON_DEVBOARD_WIRING_TYPE; // 1 GND MCU Huzzah32 | 2 VCC MCU Lolin32lite
    mjd_led_config(&led_config);

    mjd_led_on(MY_LED_ON_DEVBOARD_GPIO_NUM);
    vTaskDelay(RTOS_DELAY_1SEC);
    mjd_led_off(MY_LED_ON_DEVBOARD_GPIO_NUM);
    vTaskDelay(RTOS_DELAY_1SEC);

    ESP_LOGI(TAG, "GPIO Mux sensor input pin to LED output pin");
    gpio_setup_gpiomux_sensor_to_led();

    if (mjd_app_mode == MJD_APP_MODE_SIMPLE_LOOP) {
        /*
         * MODE: Simple loop
         *
         */
        ESP_LOGI(TAG, "\n\n***SECTION: mode = simple loop***");
        ESP_LOGI(TAG, "  ==MY_GPIO_INPUT_PIN#%i (pullup ?, pulldown ?)", MY_GPIO_INPUT_PIN);

        io_conf.pin_bit_mask = (1ULL << MY_GPIO_INPUT_PIN);
        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE; // Use GPIO_PULLDOWN_ENABLE exceptionally for this Reed switch type NO (NormallyOpen)
        io_conf.intr_type = GPIO_INTR_DISABLE;

        ESP_LOGI(TAG, "gpio_config(&io_conf);");
        ESP_ERROR_CHECK(gpio_config(&io_conf));
        ESP_LOGI(TAG, "  GPIO INPUT pin value after gpio_config(): Actual=%i", gpio_get_level(MY_GPIO_INPUT_PIN));
        vTaskDelay(RTOS_DELAY_1SEC);

        while (1) {
            ESP_LOGI(TAG, "  GPIO INPUT: Expect ? - Actual=%i", gpio_get_level(MY_GPIO_INPUT_PIN));
            vTaskDelay(RTOS_DELAY_500MILLISEC);
        }

    } else if (mjd_app_mode == MJD_APP_MODE_INTERRUPT) {
        /*
         * MODE: Interrupt
         *
         */
        ESP_LOGI(TAG, "\n\n***SECTION: mode = interrupt***");
        ESP_LOGI(TAG, "  ==MY_GPIO_INPUT_PIN#%i (pullup ?, pulldown ?)", MY_GPIO_INPUT_PIN);

        io_conf.pin_bit_mask = (1ULL << MY_GPIO_INPUT_PIN);
        io_conf.mode = GPIO_MODE_INPUT;
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE; // Use GPIO_PULLDOWN_ENABLE for the Reed switch type NO (NormallyOpen)
        io_conf.intr_type = GPIO_INTR_POSEDGE; // @doc GPIO_INTR_DISABLE GPIO_INTR_ANYEDGE GPIO_INTR_POSEDGE GPIO_INTR_NEGEDGE GPIO_INTR_LOW_LEVEL GPIO_INTR_HIGH_LEVEL

        ESP_LOGI(TAG, "gpio_config(&io_conf);");
        ESP_ERROR_CHECK(gpio_config(&io_conf));
        ESP_LOGI(TAG, "  GPIO INPUT pin value after gpio_config(): Actual=%i", gpio_get_level(MY_GPIO_INPUT_PIN));
        vTaskDelay(RTOS_DELAY_1SEC);

        xInterruptSemaphore = xSemaphoreCreateBinary();

        ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1)); // @doc ESP_INTR_FLAG_LEVEL1 Accept a Level 1 interrupt vector (lowest priority)
        ESP_ERROR_CHECK(gpio_isr_handler_add(MY_GPIO_INPUT_PIN, sensor_isr_handler, NULL));

        while (true) {
            if (xSemaphoreTake(xInterruptSemaphore, portMAX_DELAY) == pdTRUE) {
                ESP_LOGI(TAG, "***semaphore taken***");
                ESP_LOGI(TAG, "    GPIO_INTR_POSEDGE (pin value changed from 0 to 1)");
            }
        }

    }

    /********************************************************************************
     * Task Delete (will not get here for this app)
     * @doc Passing NULL will end the current task
     *
     */
    vTaskDelete(NULL);

    /*
     * END
     */
    ESP_LOGI(TAG, "END %s()", __FUNCTION__);
}

/*
 * MAIN
 */
void app_main() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    mjd_log_memory_statistics();

    /**********
     * TASK:
     * @important For stability (RMT + Wifi): always use xTaskCreatePinnedToCore(APP_CPU_NUM) [Opposed to xTaskCreate()]
     */
    BaseType_t xReturned;
    xReturned = xTaskCreatePinnedToCore(&main_task, "main_task (name)", MYAPP_RTOS_TASK_STACK_SIZE_LARGE, NULL,
    MYAPP_RTOS_TASK_PRIORITY_NORMAL, NULL,
    APP_CPU_NUM);
    if (xReturned == pdPASS) {

        ESP_LOGI(TAG, "OK Task has been created, and is running right now");
    }

    //TEMP---Sprite@Espressif
    /*
     gpio_config_t io_conf={};
     io_conf.pin_bit_mask = (1ULL << 21);
     io_conf.mode = GPIO_MODE_OUTPUT;
     ESP_ERROR_CHECK(gpio_config(&io_conf));
     while(1) {
     vTaskDelay(RTOS_DELAY_1SEC);
     printf("On\n");
     WRITE_PERI_REG(GPIO_OUT_W1TS_REG, (1<<21));
     vTaskDelay(RTOS_DELAY_1SEC);
     printf("Off\n");
     WRITE_PERI_REG(GPIO_OUT_W1TC_REG, (1<<21));
     }
     */
    //TEMP---
    /**********
     * END
     */
    ESP_LOGI(TAG, "END %s()", __FUNCTION__);
}
