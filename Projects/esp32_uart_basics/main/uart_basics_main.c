#include "mjd.h"

/*
 * Logging
 */
static const char TAG[] = "myapp";

/*
 * KConfig: LED
 */
static const int MY_LED_ON_DEVBOARD_GPIO_NUM = CONFIG_MY_LED_ON_DEVBOARD_GPIO_NUM;
static const int MY_LED_ON_DEVBOARD_WIRING_TYPE = CONFIG_MY_LED_ON_DEVBOARD_WIRING_TYPE;

/*
 * FreeRTOS settings
 */
#define MYAPP_RTOS_TASK_STACK_SIZE_LARGE (8192)
#define MYAPP_RTOS_TASK_PRIORITY_NORMAL (RTOS_TASK_PRIORITY_NORMAL)

/*
 * UART1 settings
 *  @rule ring buffer size > 128
 */
static const uint32_t MY_UART1_TX_GPIO_NUM = 22;
static const uint32_t MY_UART1_RX_GPIO_NUM = 23;
static const uint32_t MY_UART1_RX_RINGBUFFER_SIZE = 1024;
static const uint32_t MY_UART1_READBYTES_BUF_SIZE = 5;
static const uint32_t MY_UART1_READBYTES_TIMEOUT = RTOS_DELAY_5SEC;

/*
 * TASK
 */
void main_task(void *pvParameter) {
    ESP_LOGI(TAG, "%s()", __FUNCTION__);

    /********************************************************************************
     * Reuseable variables
     */
    esp_err_t f_retval;

    /********************************************************************************
     * ~STANDARD Init
     */
    mjd_log_wakeup_details();
    mjd_log_chip_info();
    mjd_log_time();
    mjd_log_memory_statistics();
    ESP_LOGI(TAG, "@doc Wait X seconds after power-on (start logic analyzer, let peripherals become active, ...)");
    vTaskDelay(RTOS_DELAY_1SEC);

    /********************************************************************************
     * LED
     */
    ESP_LOGI(TAG, "\n\n***SECTION: LED***");
    ESP_LOGI(TAG, "MY_LED_ON_DEVBOARD_GPIO_NUM:    %i", MY_LED_ON_DEVBOARD_GPIO_NUM);
    ESP_LOGI(TAG, "MY_LED_ON_DEVBOARD_WIRING_TYPE: %i", MY_LED_ON_DEVBOARD_WIRING_TYPE);

    mjd_log_memory_statistics();

    mjd_led_config_t led_config =
        { 0 };
    led_config.gpio_num = MY_LED_ON_DEVBOARD_GPIO_NUM;
    led_config.wiring_type = MY_LED_ON_DEVBOARD_WIRING_TYPE; // 1 GND MCU Huzzah32 | 2 VCC MCU Lolin32lite
    mjd_led_config(&led_config);

    // DEVTEMP: HALT
    /////mjd_rtos_wait_forever();

    /********************************************************************************
     * MAIN
     *
     */

    // Configure the UART1 controller
    ESP_LOGW(TAG,
            "Make sure you set the baudspeed in the terminal programme to the same value as in the following UART_NUM1 configuration");
    uart_config_t uart_config =
        { .baud_rate = 115200, // @validvalues 9600 57600 115200 921600
                .data_bits = UART_DATA_8_BITS,
                .parity = UART_PARITY_DISABLE,
                .stop_bits = UART_STOP_BITS_1,
                .flow_ctrl = UART_HW_FLOWCTRL_DISABLE };
    f_retval = uart_param_config(UART_NUM_1, &uart_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "uart_param_config() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    f_retval = uart_set_pin(UART_NUM_1, MY_UART1_TX_GPIO_NUM, MY_UART1_RX_GPIO_NUM, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "uart_set_pin() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    f_retval = uart_driver_install(UART_NUM_1, MY_UART1_RX_RINGBUFFER_SIZE, 0, 0, NULL, 0);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "uart_driver_install() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    char welcome_text[1024] = "";

    // Send text to the UART* controller
    strcpy(welcome_text,
            "UART_NUM_1: type something in the terminal programme which is connected to the USB UART Board.\r\n");
    printf(welcome_text);
    f_retval = uart_write_bytes(UART_NUM_1, welcome_text, strlen(welcome_text));
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "uart_write_bytes() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    strcpy(welcome_text,
            "UART_NUM_1: its output should appear in the MCU make monitor AND also echoed back to UART_NUM_1, after timeout (RTOS_DELAY_5SEC) XOR buffer full,  ...\r\n");
    printf(welcome_text);
    f_retval = uart_write_bytes(UART_NUM_1, welcome_text, strlen(welcome_text));
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "uart_write_bytes() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    strcpy(welcome_text,
            "UART_NUM_1: the uart_read_bytes() buffer size is kept small so you can see the effect of full buffers\r\n");
    f_retval = uart_write_bytes(UART_NUM_1, welcome_text, strlen(welcome_text));
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "uart_write_bytes() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    strcpy(welcome_text,
            "UART_NUM_1: Ready! Type something in this terminal (it shall appear in the MCU 'make monitor' output)\r\n");
    f_retval = uart_write_bytes(UART_NUM_1, welcome_text, strlen(welcome_text));
    if (f_retval == ESP_FAIL) {
        ESP_LOGE(TAG, "uart_write_bytes() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    fflush(stdout);

    // TODO This is an inefficient polling loop: uart_read_bytes() returns after every timeout (last param).
    uint8_t *data = (uint8_t *) malloc(MY_UART1_READBYTES_BUF_SIZE + 1); // @doc +1 for the appended \0 character
    while (1) {
        // Start next Iteration
        ESP_LOGI(TAG, "loop next iter");

        // Read data from external UART1
        int len = uart_read_bytes(UART_NUM_1, data, MY_UART1_READBYTES_BUF_SIZE, MY_UART1_READBYTES_TIMEOUT);
        if (len == ESP_FAIL) {
            ESP_LOGE(TAG, "uart_read_bytes() err %i (%s)", f_retval, esp_err_to_name(f_retval));
            // GOTO
            goto cleanup;
        }

        if (len > 0) {
            // Write data to internal UART0
            data[len] = '\0';
            printf("%s", data);
            fflush(stdout);
            // Also echo back to external UART1
            f_retval = uart_write_bytes(UART_NUM_1, (char *) data, strlen((char *) data));
            if (f_retval == ESP_FAIL) {
                ESP_LOGE(TAG, "uart_write_bytes() err %i (%s)", f_retval, esp_err_to_name(f_retval));
                // GOTO
                goto cleanup;
            }
        }
    }

    // LABEL
    cleanup: ;

    mjd_led_mark_error(MY_LED_ON_DEVBOARD_GPIO_NUM);

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

    mjd_log_memory_statistics();

    /**********
     * TASK:
     * @important For stability (RMT + Wifi etc.): always use xTaskCreatePinnedToCore(APP_CPU_NUM) [Opposed to xTaskCreate()]
     */
    BaseType_t xReturned;
    xReturned = xTaskCreatePinnedToCore(&main_task, "main_task (name)", MYAPP_RTOS_TASK_STACK_SIZE_LARGE, NULL,
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
