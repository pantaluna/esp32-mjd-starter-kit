/*
 * KY-032 Infrared Obstacle Avoidance Sensor For Arduino Smart Car Robot
 *
 */

// Component header file(s)
#include "mjd.h"
#include "mjd_ky032.h"

/*
 * Logging
 */
static const char TAG[] = "mjd_ky032";

/*
 * INTERRUPT
 */
static SemaphoreHandle_t static_ky032_config_gpio_isr_mux;

void IRAM_ATTR sensor_gpio_isr_handler(void* arg) {
    // DEBOUNCE HANDLER
    // @important Sometimes due to interference or ringing or something else, we might get two irqs quickly after each other.
    //            This is solved by looking at the time between interrupts and refusing any interrupt too close to another one.
    // @doc 1 Hz = the unit of frequency of one cycle per second.
    // @doc Compute in milliseconds:
    //          esp_clk_cpu_freq() = 160000000 (160Mhz) @source menuconfig
    //          cpu_ticks_per_ms   =    160000  @doc microsecond to millisecond
    const uint32_t SENSOR_INTERRUPT_DEBOUNCE_MS = 500; // @doc 100ms is usually high enough.
    const uint32_t cpu_ticks_per_ms = esp_clk_cpu_freq() / 1000;
    static uint32_t previous_time_ms = 0;
    uint32_t current_time_ms = xthal_get_ccount() / cpu_ticks_per_ms;
    if (current_time_ms - previous_time_ms < SENSOR_INTERRUPT_DEBOUNCE_MS) {
        return; //debounce: ignore everything < Xms after an earlier irq
    }
    previous_time_ms = current_time_ms;

    // @param pxHigherPriorityTaskWoken to pdTRUE if giving the semaphore caused a task to unblock, and the unblocked task has a priority higher than the currently running task.
    //        If xSemaphoreGiveFromISR() sets this value to pdTRUE then a context switch should be requested before the interrupt is exited.
    //        portYIELD_FROM_ISR() wakes up the imu_task immediately (instead of on next FreeRTOS tick).
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR(static_ky032_config_gpio_isr_mux, &xHigherPriorityTaskWoken);
    if (xHigherPriorityTaskWoken == pdTRUE) {
        portYIELD_FROM_ISR();
    }
}

/*********************************************************************************
 * PUBLIC.
 *
 *
 */
esp_err_t mjd_ky032_init(mjd_ky032_config_t* param_ptr_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    if (param_ptr_config->is_init == true) {
        f_retval = ESP_FAIL;
        ESP_LOGE(TAG, "ABORT. mjd_ky032_init() component is already init'd | err %i %s", f_retval,
                esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // WAIT 5 seconds so the Sensor can calibrate itself!
    vTaskDelay(RTOS_DELAY_5SEC);

    // Semaphore
    // !Clone locally (so the ISR can reference the semaphore handle).
    // @doc Binary semaphores created using xSemaphoreCreateBinary() are created in a state such that the semaphore must first be 'given' before it can be 'taken'!
    // @doc The required RAM is automatically allocated from the FreeRTOS heap (opposed the local stack!).
    param_ptr_config->gpio_isr_mux = xSemaphoreCreateBinary();
    if (param_ptr_config->gpio_isr_mux == NULL) {
        f_retval = ESP_FAIL;
        ESP_LOGE(TAG, "ABORT. xSemaphoreCreateBinary() failed (insufficient FreeRTOS heap available) | err %i %s", f_retval,
                esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    static_ky032_config_gpio_isr_mux = param_ptr_config->gpio_isr_mux;

    // GPIO
    // @important The pinvalue goes from 1 to 0 when obstacle is detected do use GPIO_INTR_NEGEDGE to capture that event.
    // @important GPIO_PULLUP_ENABLE is required.
    gpio_config_t io_conf = { 0 };
    io_conf.pin_bit_mask = (1ULL << param_ptr_config->data_gpio_num);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.intr_type = GPIO_INTR_NEGEDGE; // @validvalues GPIO_INTR_DISABLE GPIO_INTR_ANYEDGE GPIO_INTR_NEGEDGE GPIO_INTR_POSEDGE
    f_retval = gpio_config(&io_conf);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "ABORT. gpio_config() failed | err %i %s", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // ISR
    // @doc ESP_INTR_FLAG_LEVEL1 Accept a Level 1 interrupt vector (lowest priority)
    f_retval = gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "ABORT. gpio_install_isr_service() failed | err %i %s", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    f_retval = gpio_isr_handler_add(param_ptr_config->data_gpio_num, sensor_gpio_isr_handler, NULL);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "ABORT. gpio_isr_handler_add() failed | err %i %s", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // Mark OK
    param_ptr_config->is_init = true;

    // LABEL
    cleanup: ;

    return f_retval;
}

esp_err_t mjd_ky032_deinit(mjd_ky032_config_t* param_ptr_config) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    if (param_ptr_config->is_init == false) {
        f_retval = ESP_FAIL;
        ESP_LOGE(TAG, "ABORT. mjd_ky032_deinit() component was not init'd | err %i %s", f_retval,
                esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // ISR
    f_retval = gpio_isr_handler_remove(param_ptr_config->data_gpio_num);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "ABORT. gpio_isr_handler_remove() failed | err %i %s", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }
    gpio_uninstall_isr_service(); // @returns void

    // SEMAPHORE
    param_ptr_config->gpio_isr_mux = NULL;
    static_ky032_config_gpio_isr_mux = param_ptr_config->gpio_isr_mux;

    // Mark OK
    param_ptr_config->is_init = false;

    // LABEL
    cleanup: ;

    return f_retval;
}
