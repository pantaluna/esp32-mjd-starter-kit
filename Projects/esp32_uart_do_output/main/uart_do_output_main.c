/*
 * HARDWARE SETUP the MJD components:
 *  *NONE
 *
 */

/*
 * Includes: system, own
 */
#include "mjd.h"

/*
 * Logging
 */
static const char TAG[] = "myapp";

/*
 * KConfig: LED, WIFI
 */
static const int MY_LED_ON_DEVBOARD_GPIO_NUM = CONFIG_MY_LED_ON_DEVBOARD_GPIO_NUM;
static const int MY_LED_ON_DEVBOARD_WIRING_TYPE = CONFIG_MY_LED_ON_DEVBOARD_WIRING_TYPE;

static const int MY_UART_PORT_NUM = CONFIG_MY_UART_PORT_NUM;
static const int MY_UART_TX_GPIO_NUM = CONFIG_MY_UART_TX_GPIO_NUM;

/*
 * Settings (fixed)
 */
#define MY_UART_BAUD_SPEED         (115200)
#define MY_UART_RX_RINGBUFFER_SIZE (512 * 2)

/*
 * FreeRTOS settings
 */
#define MYAPP_RTOS_TASK_STACK_SIZE_16K  (16 * 1024)
#define MYAPP_RTOS_TASK_PRIORITY_NORMAL (RTOS_TASK_PRIORITY_NORMAL)

/*
 * TASK
 */
void main_task(void *pvParameter) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    /********************************************************************************
     * LED
     *
     */
    mjd_led_config_t led_config =
                { 0 };
    led_config.gpio_num = MY_LED_ON_DEVBOARD_GPIO_NUM;
    led_config.wiring_type = MY_LED_ON_DEVBOARD_WIRING_TYPE; // 1 GND MCU Huzzah32 | 2 VCC MCU Lolin32lite
    mjd_led_config(&led_config);

    /********************************************************************************
     * UART
     *
     */

    // Configure the UART1 controller
    uart_config_t uart_config =
                { .baud_rate = MY_UART_BAUD_SPEED,
                        .data_bits = UART_DATA_8_BITS,
                        .parity = UART_PARITY_DISABLE,
                        .stop_bits = UART_STOP_BITS_1,
                        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE };
    f_retval = uart_param_config(MY_UART_PORT_NUM, &uart_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). uart_param_config() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    f_retval = uart_set_pin(MY_UART_PORT_NUM,
            MY_UART_TX_GPIO_NUM, UART_PIN_NO_CHANGE,
            UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). uart_set_pin() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // @doc Param UART TX ring buffer size. OK If set to zero Then the driver will not use the TX buffer; the TX function will block the task until all data has been sent out.
    f_retval = uart_driver_install(MY_UART_PORT_NUM, MY_UART_RX_RINGBUFFER_SIZE, 0, 0, NULL, 0);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). uart_driver_install() err %i (%s)", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    /********************************************************************************
     * GEN OUTPUT
     *
     */
    char line[1024] = "";
    bool led_toggle = false;

    uint32_t iter = 0;
    while (1) {
        led_toggle = !led_toggle;
        if (led_toggle == true) {
            mjd_led_on(MY_LED_ON_DEVBOARD_GPIO_NUM);
        } else {
            mjd_led_off(MY_LED_ON_DEVBOARD_GPIO_NUM);

        }
        ++iter;
        sprintf(line, "<~>D%010u" "1234567890" "ABCDEFGHIJKLMNOPQRSTUVWXYZ" "abcdefghijklmnopqrstuvwxyz" "\r\n", iter);

        ESP_LOGI(TAG, "Send line to UART...");
        ESP_LOGI(TAG, "%s", line);

        f_retval = uart_write_bytes(MY_UART_PORT_NUM, line, strlen(line));
        if (f_retval == ESP_FAIL) {
            ESP_LOGE(TAG, "uart_write_bytes() err %i (%s)", f_retval, esp_err_to_name(f_retval));
            // GOTO
            goto cleanup;
        }

        vTaskDelay(RTOS_DELAY_1SEC);
    }

    /********************************************************************************
     * Cleanup
     *
     */
    // LABEL
    cleanup: ;

    /********************************************************************************
     * Task Delete
     * @doc Passing NULL will end the current task
     *
     */
    vTaskDelete(NULL);
}

/*
 * MAIN
 */
void app_main() {
    /**********
     * TASK:
     * @important For stability (RMT + Wifi): always use xTaskCreatePinnedToCore(APP_CPU_NUM) [Opposed to xTaskCreate()]
     */
    BaseType_t xReturned;
    xReturned = xTaskCreatePinnedToCore(&main_task, "main_task (name)", MYAPP_RTOS_TASK_STACK_SIZE_16K, NULL,
    MYAPP_RTOS_TASK_PRIORITY_NORMAL, NULL,
    APP_CPU_NUM);
    if (xReturned != pdPASS) {
        ESP_LOGE(TAG, "ABORT. Task cannot be created");
    }
}
