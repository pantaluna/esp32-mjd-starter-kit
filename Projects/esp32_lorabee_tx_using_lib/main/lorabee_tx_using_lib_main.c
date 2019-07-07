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
static const int MY_LORABEE_UART_TX_GPIO_NUM = CONFIG_MY_LORABEE_UART_TX_GPIO_NUM; // @default 22
static const int MY_LORABEE_UART_RX_GPIO_NUM = CONFIG_MY_LORABEE_UART_RX_GPIO_NUM; // @default 23
static const int MY_LORABEE_RESET_GPIO_NUM = CONFIG_MY_LORABEE_RESET_GPIO_NUM; // @default 14

/*
 * FreeRTOS settings
 */
#define MYAPP_RTOS_TASK_STACK_SIZE_LARGE (8192)
#define MYAPP_RTOS_TASK_PRIORITY_NORMAL (RTOS_TASK_PRIORITY_NORMAL)

/*
 * LORA settings
 *
 * - POWER (-3) = lowest 17mA supply current
 */
#define MY_LORABEE_RADIO_POWER (10)
#define MY_LORABEE_RADIO_FREQUENCY (868900000)
#define MY_LORABEE_RADIO_SPREADING_FACTOR (MJD_LORABEE_SPREADING_FACTOR_SF7)
#define MY_LORABEE_RADIO_BANDWIDTH (MJD_LORABEE_BANDWIDTH_125KHZ)
#define MY_LORABEE_RADIO_CODING_RATE (MJD_LORABEE_CODING_RATE_4_8)

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
    lorabee_config.radio_coding_rate = MY_LORABEE_RADIO_CODING_RATE;

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
     * COMMAND: RADIO TX *
     *
     */
    const uint32_t NBR_OF_TX = 10000; // 10 25 50 100 1000 10000
    ESP_LOGI(TAG, "radio tx LOOP: NBR_OF_TX %u", NBR_OF_TX);

    mjd_log_memory_statistics();

    // @doc Small payload 16 chars
    /////char radio_tx_string_payload[] = "0123456789ABCDEF";

    // @error Too large payload 256 chars
    /////char radio_tx_string_payload[] =
    /////        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    for (uint32_t i = 1; i <= NBR_OF_TX; ++i) {
        ESP_LOGI(TAG, "");
        ESP_LOGI(TAG, "***radio tx LOOP ITEM #%u of %u***", i, NBR_OF_TX);

        // @doc Payload 5 + 14 +
        char radio_tx_string_payload[1024] = "";
        char current_time[20];
        mjd_get_current_time_yyyymmddhhmmss(current_time);
        sprintf(radio_tx_string_payload, "%u %s"
                "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789"
                "0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789",
                i, current_time);

        ESP_LOGI(TAG, "  Details of the PAYLOAD which will be transmitted:");
        ESP_LOGI(TAG, "    radio_tx_string_payload:         %s", radio_tx_string_payload);
        ESP_LOGI(TAG, "    strlen(radio_tx_string_payload): %i", strlen(radio_tx_string_payload));
        ESP_LOGI(TAG, "    radio_tx_string_payload: HEXDUMP");
        ESP_LOG_BUFFER_HEXDUMP(TAG, radio_tx_string_payload, strlen(radio_tx_string_payload), ESP_LOG_INFO);

        // COMMAND "sys set pindig GPIO0 1": SET LED *ON
        f_retval = mjd_lorabee_sys_set_pindig(&lorabee_config, MJD_LORABEE_GPIO_NUM_0, MJD_LORABEE_GPIO_LEVEL_HIGH);
        if (f_retval != ESP_OK) {
            // GOTO
            goto cleanup;
        }

        f_retval = mjd_lorabee_radio_tx(&lorabee_config, (uint8_t *) radio_tx_string_payload,
                strlen(radio_tx_string_payload));
        if (f_retval == ESP_FAIL) {
            // GOTO
            goto cleanup;
        }

        // COMMAND "sys set pindig GPIO0 0": SET LED *OFF
        f_retval = mjd_lorabee_sys_set_pindig(&lorabee_config, MJD_LORABEE_GPIO_NUM_0, MJD_LORABEE_GPIO_LEVEL_LOW);
        if (f_retval != ESP_OK) {
            // GOTO
            goto cleanup;
        }

        /*
         * DELAY
         *   @important Send 'radio tx' too quickly causes error 'busy' for subsequent requests!
         *   @important Give receiver (single channel!) a time window to process the data and get ready to receive new packets.
         *   @important Respect state laws.
         */
        ESP_LOGI(TAG, "  Delay 3 seconds");
        vTaskDelay(RTOS_DELAY_3SEC);
    }

    mjd_log_memory_statistics();

    // COMMAND "sys set pindig GPIO0 0": SET LED *OFF
    f_retval = mjd_lorabee_sys_set_pindig(&lorabee_config, MJD_LORABEE_GPIO_NUM_0, MJD_LORABEE_GPIO_LEVEL_LOW);
    if (f_retval != ESP_OK) {
        // GOTO
        goto cleanup;
    }

    //
    // SLEEP

    // TX Command: SYS SLEEP [+-49 days]
    ESP_LOGI(TAG, "sys sleep");
    f_retval = mjd_lorabee_sleep(&lorabee_config);
    if (f_retval == ESP_FAIL) {
        // GOTO
        goto cleanup;
    }

    mjd_log_memory_statistics();

    /********************************************************************************
     * LABEL
     */
    cleanup: ;

    mjd_log_time();

    mjd_lorabee_log_config(&lorabee_config);

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
