#include "mjd.h"
#include "mjd_lorabee.h"

/*
 * Logging
 */
static const char TAG[] = "myapp";

/*
 * KConfig:
 * - UART1 settings: on which UART device and on which GPIO pins the LoraBee breakout board is wired up.
 */
static const int MY_LED_ON_DEVBOARD_GPIO_NUM = CONFIG_MY_LED_ON_DEVBOARD_GPIO_NUM;
static const int MY_LED_ON_DEVBOARD_WIRING_TYPE = CONFIG_MY_LED_ON_DEVBOARD_WIRING_TYPE;

static const int MY_LORABEE_UART_PORT_NUM = CONFIG_MY_LORABEE_UART_PORT_NUM; // @default UART_NUM_1
static const int MY_LORABEE_UART_TX_GPIO_NUM = CONFIG_MY_LORABEE_UART_TX_GPIO_NUM;// @default 22
static const int MY_LORABEE_UART_RX_GPIO_NUM = CONFIG_MY_LORABEE_UART_RX_GPIO_NUM;// @default 23
static const int MY_LORABEE_RESET_GPIO_NUM = CONFIG_MY_LORABEE_RESET_GPIO_NUM;// @default 14

/*
 * FreeRTOS settings
 */
#define MYAPP_RTOS_TASK_STACK_SIZE_LARGE (8192)
#define MYAPP_RTOS_TASK_PRIORITY_NORMAL (RTOS_TASK_PRIORITY_NORMAL)


/*
 * LORA settings
 * #define MY_LORABEE_RADIO_POWER (-3)
 *
 */
#define MY_LORABEE_RADIO_POWER (14)
#define MY_LORABEE_RADIO_FREQUENCY (865100000)
#define MY_LORABEE_RADIO_SPREADING_FACTOR ("sf7")
#define MY_LORABEE_RADIO_BANDWIDTH (125)


/*
 * TASKS
 */
void main_task(void *pvParameter) {
    ESP_LOGI(TAG, "%s()", __FUNCTION__);

    /********************************************************************************
     * Reuseable variables
     */
    esp_err_t f_retval = ESP_OK;

    /********************************************************************************
     * STANDARD Init
     */
    mjd_log_wakeup_details();
    mjd_log_chip_info();
    mjd_log_time();
    mjd_log_memory_statistics();
    ESP_LOGI(TAG, "@doc Wait X seconds after power-on (start logic analyzer, let peripherals become active, ...)");
    vTaskDelay(RTOS_DELAY_1SEC);

    /********************************************************************************
     * LED
     *
     */
    mjd_led_config_t led_config =
        { 0 };
    led_config.gpio_num = MY_LED_ON_DEVBOARD_GPIO_NUM;
    led_config.wiring_type = MY_LED_ON_DEVBOARD_WIRING_TYPE; // 1 GND MCU Huzzah32 | 2 VCC MCU Lolin32lite
    mjd_led_config(&led_config);

    ESP_LOGI(TAG, "LED blink 1 time");
    mjd_led_blink_times(MY_LED_ON_DEVBOARD_GPIO_NUM, 1);

    /********************************************************************************
     * Init LORABEE component
     */
    mjd_lorabee_config_t lorabee_config = MJD_LORABEE_CONFIG_DEFAULT();
    lorabee_config.uart_port_num = MY_LORABEE_UART_PORT_NUM;
    lorabee_config.uart_tx_gpio_num = MY_LORABEE_UART_TX_GPIO_NUM;
    lorabee_config.uart_rx_gpio_num = MY_LORABEE_UART_RX_GPIO_NUM;
    lorabee_config.reset_gpio_num = MY_LORABEE_RESET_GPIO_NUM;

    lorabee_config.radio_power = MY_LORABEE_RADIO_POWER;
    lorabee_config.radio_frequency = MY_LORABEE_RADIO_FREQUENCY;
    lorabee_config.radio_spreading_factor = MY_LORABEE_RADIO_SPREADING_FACTOR;
    lorabee_config.radio_bandwidth = MY_LORABEE_RADIO_BANDWIDTH;

    f_retval = mjd_lorabee_init(&lorabee_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "mjd_lorabee_init() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    mjd_lorabee_log_config(&lorabee_config);

    /**********
     * LORA COMMAND: get version info
     *   @doc https://www.disk91.com/2015/technology/networks/first-step-in-lora-land-microchip-rn2483-test/
     */

    mjd_lorabee_version_info_t microtech_info =
        { 0 };
    f_retval = mjd_lorabee_sys_get_version(&lorabee_config, &microtech_info);
    if (f_retval != ESP_OK) {
        // GOTO
        goto cleanup;
    }
    ESP_LOGI(TAG, "mjd_lorabee_version_info_t * microtech_info:");
    ESP_LOGI(TAG, "    microtech_info.raw              %s", microtech_info.raw);
    ESP_LOGI(TAG, "    microtech_info.model            %s", microtech_info.model);
    ESP_LOGI(TAG, "    microtech_info.firmware_version %s", microtech_info.firmware_version);
    ESP_LOGI(TAG, "    microtech_info.firmware_date    %s", microtech_info.firmware_date);



    /**********
     * COMMAND: RADIO RX *
     * TODO Implement a way to stop the for-loop manually (pushbutton? timer?)
     *
     */
    const uint32_t NBR_OF_RX = 1000; // 10 50 100 1000 10000
    ESP_LOGI(TAG, "radio rx LOOP: NBR_OF_RX %u", NBR_OF_RX);

    mjd_log_memory_statistics();

    // @doc Largest payload 114 chars
    char radio_rx_result[230] = "";
    uint32_t radio_rx_len = 0;

    for (uint32_t i = 1; i <= NBR_OF_RX; ++i) {
        ESP_LOGI(TAG, "");
        ESP_LOGI(TAG, "***radio rx LOOP ITEM #%u of %u***", i, NBR_OF_RX);

        f_retval = mjd_lorabee_radio_rx(&lorabee_config, (uint8_t *) radio_rx_result, &radio_rx_len);
        if (f_retval == ESP_FAIL) {
            // GOTO
            goto cleanup;
        }
        ESP_LOGI(TAG, "  Details of the received PAYLOAD:");
        ESP_LOGI(TAG, "    radio_rx_len:    %i", radio_rx_len);
        ESP_LOGI(TAG, "    radio_rx_result: HEXDUMP");
        ESP_LOG_BUFFER_HEXDUMP(TAG, radio_rx_result, radio_rx_len, ESP_LOG_INFO);

        // COMMAND "sys set pindig GPIO0 0": SET LED *ON, delay, SET LED *OFF
        // TODO Remove the delay in Production.
        f_retval = mjd_lorabee_sys_set_pindig(&lorabee_config, MJD_LORABEE_GPIO_NUM_0, MJD_LORABEE_GPIO_LEVEL_HIGH);
        if (f_retval != ESP_OK) {
            // GOTO
            goto cleanup;
        }
        vTaskDelay(RTOS_DELAY_100MILLISEC);
        f_retval = mjd_lorabee_sys_set_pindig(&lorabee_config, MJD_LORABEE_GPIO_NUM_0, MJD_LORABEE_GPIO_LEVEL_LOW);
        if (f_retval != ESP_OK) {
            // GOTO
            goto cleanup;
        }
    }
    mjd_log_memory_statistics();

    /********************************************************************************
     * LABEL
     */
    cleanup: ;

    mjd_log_time();

    mjd_lorabee_log_config(&lorabee_config);

    mjd_lorabee_deinit(&lorabee_config); // @doc also puts the device in sleep mode

    mjd_log_memory_statistics();

    /********************************************************************************
     * Task Delete
     * @doc Passing NULL will end the current task
     */
    ESP_LOGI(TAG, "END OF %s()", __FUNCTION__);
    vTaskDelete(NULL);
}

/*
 * MAIN
 */
void app_main() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    mjd_log_memory_statistics();

    /**********
     * CREATE TASK:
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
