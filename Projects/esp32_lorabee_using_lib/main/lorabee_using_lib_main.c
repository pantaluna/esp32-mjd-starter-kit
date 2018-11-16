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
 */
#define MY_LORABEE_RADIO_POWER (-3)
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

    /********************************************************************************
     * UART1: EEPROM nvm get/set icw pin-reset
     *   @doc The EEPROM is only erased when doing a factoryRESET (not with `sys reset` or my pin-reset)
     */

    ESP_LOGI(TAG, "COMMAND 'sys get nvm 300': This command returns the data stored in the user EEPROM of the RN2483 module at the requested <address> location.");
    uint8_t radio_nvm_address300_get_value;
    f_retval = mjd_lorabee_sys_get_nvm(&lorabee_config, 0x300, &radio_nvm_address300_get_value);
    if (f_retval != ESP_OK) {
        // GOTO
        goto cleanup;
    }
    ESP_LOGI(TAG, "retrieved sys GET nvm 300 (uint8 value): %u = hex %02X", radio_nvm_address300_get_value, radio_nvm_address300_get_value);

    ESP_LOGI(TAG, "'sys set nvm 300 BB': This command sets the data in the user EEPROM of the RN2483 module at the requested <address> location.");
    uint8_t radio_nvm_address300_set_value = 0xBB;
    ESP_LOGI(TAG, "Setting sys SET nvm 300 (uint8 value): %u = hex %02X", radio_nvm_address300_set_value, radio_nvm_address300_set_value);
    f_retval = mjd_lorabee_sys_set_nvm(&lorabee_config, 0x300, radio_nvm_address300_set_value);
    if (f_retval != ESP_OK) {
        // GOTO
        goto cleanup;
    }

    ESP_LOGI(TAG, "COMMAND 'sys get nvm 300': This command returns the data stored in the user EEPROM of the RN2483 module at the requested <address> location.");
    uint8_t radio_nvm_address300_get_value2;
    f_retval = mjd_lorabee_sys_get_nvm(&lorabee_config, 0x300, &radio_nvm_address300_get_value2);
    if (f_retval != ESP_OK) {
        // GOTO
        goto cleanup;
    }
    ESP_LOGI(TAG, "retrieved sys GET nvm 300 (uint8 value): %u = hex %02X", radio_nvm_address300_get_value2, radio_nvm_address300_get_value2);


    /********************************************************************************
     * UART1: send Lora radio commands
     *   @doc https://www.disk91.com/2015/technology/networks/first-step-in-lora-land-microchip-rn2483-test/
     */

    //
    ESP_LOGI(TAG, "COMMAND: sys set pindig GPIO0 1: SET LED *ON");
    f_retval = mjd_lorabee_sys_set_pindig(&lorabee_config, MJD_LORABEE_GPIO_NUM_0, MJD_LORABEE_GPIO_LEVEL_HIGH);
    if (f_retval != ESP_OK) {
        // GOTO
        goto cleanup;
    }

    ESP_LOGI(TAG, "COMMAND 'sys get ver': GET the version info");
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

    ESP_LOGI(TAG, "COMMAND 'sys get hweui': GET the preprogrammed EUI node address");
    char radio_hweui[16];
    f_retval = mjd_lorabee_sys_get_hweui(&lorabee_config, radio_hweui);
    if (f_retval != ESP_OK) {
        // GOTO
        goto cleanup;
    }
    ESP_LOGI(TAG, "retrieved hweui (string value): %s", radio_hweui);

    ESP_LOGI(TAG, "COMMAND 'sys get vdd': GET VOLTAGE");
    uint32_t sys_vdd_voltage;
    f_retval = mjd_lorabee_sys_get_vdd(&lorabee_config, &sys_vdd_voltage);
    if (f_retval != ESP_OK) {
        // GOTO
        goto cleanup;
    }
    ESP_LOGI(TAG, "retrieved vdd voltage (uint32_t value): %u", sys_vdd_voltage);

    //
    // MAC PAUSE RESUME PAUSE
    //

    ESP_LOGI(TAG, "COMMAND 'mac pause'");
    f_retval = mjd_lorabee_mac_pause(&lorabee_config);
    if (f_retval == ESP_FAIL) {
        // GOTO
        goto cleanup;
    }

    ESP_LOGI(TAG, "COMMAND 'mac resume'");
    f_retval = mjd_lorabee_mac_resume(&lorabee_config);
    if (f_retval == ESP_FAIL) {
        // GOTO
        goto cleanup;
    }

    ESP_LOGI(TAG, "COMMAND 'mac pause'");
    f_retval = mjd_lorabee_mac_pause(&lorabee_config);
    if (f_retval == ESP_FAIL) {
        // GOTO
        goto cleanup;
    }

    /*
     * RADIO GET *
     *
     */
    ESP_LOGI(TAG, "COMMAND 'radio get mod': get the mode");
    char radio_mode[16];
    f_retval = mjd_lorabee_radio_get_mode(&lorabee_config, radio_mode);
    if (f_retval != ESP_OK) {
        // GOTO
        goto cleanup;
    }
    ESP_LOGI(TAG, "retrieved mode (string value): %s", radio_mode);

    ESP_LOGI(TAG, "COMMAND 'radio get pwr': RADIO GET POWER");
    int32_t radio_power;
    f_retval = mjd_lorabee_radio_get_power(&lorabee_config, &radio_power);
    if (f_retval != ESP_OK) {
        // GOTO
        goto cleanup;
    }
    ESP_LOGI(TAG, "retrieved power (int32_t signed value): %i", radio_power);

    ESP_LOGI(TAG, "COMMAND 'radio get freq': RADIO GET FREQUENCY");
    uint32_t radio_frequency;
    f_retval = mjd_lorabee_radio_get_frequency(&lorabee_config, &radio_frequency);
    if (f_retval != ESP_OK) {
        // GOTO
        goto cleanup;
    }
    ESP_LOGI(TAG, "retrieved frequency (uint32_t value): %u", radio_frequency);

    ESP_LOGI(TAG, "COMMAND 'radio get sf': RADIO GET SF");
    char radio_spreading_factor[16];
    f_retval = mjd_lorabee_radio_get_spreading_factor(&lorabee_config, radio_spreading_factor);
    if (f_retval != ESP_OK) {
        // GOTO
        goto cleanup;
    }
    ESP_LOGI(TAG, "retrieved spreading factor (string value): %s", radio_spreading_factor);

    ESP_LOGI(TAG, "COMMAND 'radio get bw': RADIO GET BW");
    uint32_t radio_bandwidth;
    f_retval = mjd_lorabee_radio_get_bandwidth(&lorabee_config, &radio_bandwidth);
    if (f_retval != ESP_OK) {
        // GOTO
        goto cleanup;
    }
    ESP_LOGI(TAG, "retrieved bandwidth (uint32_t value): %u", radio_bandwidth);

    ESP_LOGI(TAG, "COMMAND 'radio get wdt': RADIO GET Watchdog timeout");
    uint32_t radio_watchdog_timeout;
    f_retval = mjd_lorabee_radio_get_watchdog_timeout(&lorabee_config, &radio_watchdog_timeout);
    if (f_retval != ESP_OK) {
        // GOTO
        goto cleanup;
    }
    ESP_LOGI(TAG, "retrieved watchdog_timeout (uint32_t value): %u", radio_watchdog_timeout);

    ESP_LOGI(TAG, "COMMAND 'radio get snr': RADIO GET signal-noise ratio");
    int32_t radio_signal_noise_ratio;
    f_retval = mjd_lorabee_radio_get_signal_noise_ratio(&lorabee_config, &radio_signal_noise_ratio);
    if (f_retval != ESP_OK) {
        // GOTO
        goto cleanup;
    }
    ESP_LOGI(TAG, "retrieved _signal_noise_ratio (int32_t signed value): %i", radio_signal_noise_ratio);

    /*
     * RADIO SET * (ALL are already done correctly in mjd_lorabee_init()
     *
     */

    ESP_LOGI(TAG, "COMMAND: 'RADIO SET mode'");
    char radio_mode2[] = "lora";
    ESP_LOGI(TAG, " setting mode (string value): %s", radio_mode2);
    f_retval = mjd_lorabee_radio_set_mode(&lorabee_config, radio_mode2);
    if (f_retval != ESP_OK) {
        // GOTO
        goto cleanup;
    }

    ESP_LOGI(TAG, "COMMAND: RADIO SET power");
    /////int32_t radio_power2 = 14;
    int32_t radio_power2 = -3;
    ESP_LOGI(TAG, " setting power (int32_t value): %i", radio_power2);
    f_retval = mjd_lorabee_radio_set_power(&lorabee_config, radio_power2);
    if (f_retval != ESP_OK) {
        // GOTO
        goto cleanup;
    }

    ESP_LOGI(TAG, "COMMAND 'radio set freq'");
    // # My Lora P2P freq=865.1 Khz sf=SF7 bw=125Khz
    uint32_t radio_frequency2 = 865100000;
    ESP_LOGI(TAG, " setting frequency (uint32_t value): %u", radio_frequency2);
    f_retval = mjd_lorabee_radio_set_frequency(&lorabee_config, radio_frequency2);
    if (f_retval != ESP_OK) {
        // GOTO
        goto cleanup;
    }

    ESP_LOGI(TAG, "RADIO SET spreading factor");
    char radio_spreading_factor2[] = "sf7";
    /////char radio_spreading_factor2[] = "sf12";
    ESP_LOGI(TAG, " setting spreading factor (string value): %s", radio_spreading_factor2);
    f_retval = mjd_lorabee_radio_set_spreading_factor(&lorabee_config, radio_spreading_factor2);
    if (f_retval != ESP_OK) {
        // GOTO
        goto cleanup;
    }

    ESP_LOGI(TAG, "RADIO SET bandwith");
    uint32_t radio_bandwidth2 = 125;
    ESP_LOGI(TAG, " setting bandwidth (uint32_t value): %u", radio_bandwidth2);
    f_retval = mjd_lorabee_radio_set_bandwidth(&lorabee_config, radio_bandwidth2);
    if (f_retval != ESP_OK) {
        // GOTO
        goto cleanup;
    }

    //
    // SLEEP
    //

    ESP_LOGI(TAG, "Command: RADIO SET watchdog timeout");
    uint32_t radio_watchdog_timeout3 = 0;
    ESP_LOGI(TAG, " setting watchdog timeout (uint32_t value): %u", radio_watchdog_timeout3);
    f_retval = mjd_lorabee_radio_set_watchdog_timeout(&lorabee_config, radio_watchdog_timeout3);
    if (f_retval != ESP_OK) {
        // GOTO
        goto cleanup;
    }

    ESP_LOGI(TAG, "Command: SYS SLEEP [+-49 days]");
    f_retval = mjd_lorabee_sleep(&lorabee_config);
    if (f_retval == ESP_FAIL) {
        // GOTO
        goto cleanup;
    }

    mjd_log_memory_statistics();

    /*ESP_LOGI(TAG, "***TEST SEND COMMAND (MUST FAIL AS DEVICE IS IN SLEEP MODE)");
    f_retval = mjd_lorabee_sys_set_pindig(&lorabee_config, MJD_LORABEE_GPIO_NUM_0, MJD_LORABEE_GPIO_LEVEL_LOW);
    if (f_retval != ESP_OK) {
        // GOTO
        goto cleanup;
    }*/

    /********************************************************************************
     * LABEL
     */
    cleanup: ;

    mjd_lorabee_log_config(&lorabee_config);

    /*
     * LOG TIME
     */
    mjd_log_time();

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
