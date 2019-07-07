#include "mjd_jsnsr04t.h"
#include "mjd_ssd1306.h"

/*
 * Logging
 */
static const char TAG[] = "myapp";

/*
 * KConfig:
 * - LED
 * - sensor GPIO's
 * - SD1306
 * - POWER C Channel MOSFET to give power to the sensor (or not, to save power consumption in battery mode)
 */
static const int MY_LED_ON_DEVBOARD_GPIO_NUM = CONFIG_MY_LED_ON_DEVBOARD_GPIO_NUM;
static const int MY_LED_ON_DEVBOARD_WIRING_TYPE = CONFIG_MY_LED_ON_DEVBOARD_WIRING_TYPE;

static const int MY_JSNSR04T_TRIGGER_GPIO_NUM = CONFIG_MY_JSNSR04T_TRIGGER_GPIO_NUM; // @default 16
static const int MY_JSNSR04T_ECHO_GPIO_NUM = CONFIG_MY_JSNSR04T_ECHO_GPIO_NUM; // @default 14

static const int MY_SSD1306_OLED_IS_USED = CONFIG_MY_SSD1306_OLED_IS_USED;
static const int MY_SSD1306_I2C_SLAVE_ADDRESS = CONFIG_MY_SSD1306_I2C_SLAVE_ADDRESS;
static const int MY_SSD1306_I2C_MASTER_PORT_NUM = CONFIG_MY_SSD1306_I2C_MASTER_PORT_NUM;
static const int MY_SSD1306_I2C_SCL_GPIO_NUM = CONFIG_MY_SSD1306_I2C_SCL_GPIO_NUM;
static const int MY_SSD1306_I2C_SDA_GPIO_NUM = CONFIG_MY_SSD1306_I2C_SDA_GPIO_NUM;
static const int MY_SSD1306_OLED_DIMENSION_NUM = CONFIG_MY_SSD1306_OLED_DIMENSION_NUM;

static const int MY_POWER_MOSFET_IS_USED = CONFIG_MY_POWER_MOSFET_IS_USED;
static const int MY_POWER_MOSFET_GATE_GPIO_NUM = CONFIG_MY_POWER_MOSFET_GATE_GPIO_NUM;

/*
 * FreeRTOS settings
 */
#define MYAPP_RTOS_TASK_STACK_SIZE_LARGE (8192)
#define MYAPP_RTOS_TASK_PRIORITY_NORMAL (RTOS_TASK_PRIORITY_NORMAL)

/*
 * TASKS
 */
void main_task(void *pvParameter) {
    ESP_LOGI(TAG, "%s()", __FUNCTION__);

    /********************************************************************************
     * Reuseable variables
     */
    esp_err_t f_retval = ESP_OK;

    /*********************************
     * LOGGING
     * Optional for Production: dump less messages
     * @doc It is possible to lower the log level for specific modules (wifi and tcpip_adapter are strong candidates)
     * @important Disable u8g2_hal DEBUG messages which are too detailed for me.
     */
    esp_log_level_set("u8g2_hal", ESP_LOG_INFO);

    /********************************************************************************
     * STANDARD Init
     */
    mjd_log_wakeup_details();
    mjd_log_chip_info();
    mjd_log_time();
    mjd_log_memory_statistics();
    /////ESP_LOGI(TAG, "@doc Wait X seconds after power-on (start logic analyzer, let peripherals become active, ...)");
    /////vTaskDelay(RTOS_DELAY_1SEC);

    /********************************************************************************
     * LED
     */
    mjd_led_config_t led_config =
                { 0 };
    led_config.gpio_num = MY_LED_ON_DEVBOARD_GPIO_NUM;
    led_config.wiring_type = MY_LED_ON_DEVBOARD_WIRING_TYPE; // 1 GND MCU Huzzah32 | 2 VCC MCU Lolin32lite
    mjd_led_config(&led_config);

    /********************************************************************************
     * Init POWER MOSFET
     *
     * @important I have to use a "weak" external 1M Ohm pulldown resistor with this MOSFET to pull the line down and guarantee low power consumption (battery mode).
     *            In deinit() do not use gpio_reset_pin()! "I (12790) gpio: GPIO[4]| InputEn: 0| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:0"
     *            For GPIO#4 the func gpio_reset_pin() pulls UP the signal with a "strong" 10-50K Ohm, overriding my "weaker" 1M Ohm pull DOWN, meaning the line is always UP and so the MOSFET is always ON!
     *
     */
    ESP_LOGI(TAG, "  Init POWER MOSFET...");

    gpio_config_t mosfet_config;

    mosfet_config.pin_bit_mask = (1ULL << MY_POWER_MOSFET_GATE_GPIO_NUM);
    mosfet_config.mode = GPIO_MODE_OUTPUT;
    mosfet_config.pull_down_en = GPIO_PULLDOWN_DISABLE; // @important I have to use an external 1M Ohm pulldown resistor to pull the line down (!M Ohm = low power consumption in battery mode).
    mosfet_config.pull_up_en = GPIO_PULLUP_DISABLE;
    mosfet_config.intr_type = GPIO_PIN_INTR_DISABLE;

    if (MY_POWER_MOSFET_IS_USED == 1) {
        f_retval = gpio_config(&mosfet_config);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "%s(). ABORT. gpio_config(mosfet_config) | err %i (%s)", __FUNCTION__, f_retval,
                    esp_err_to_name(f_retval));
            // GOTO
            goto cleanup;
        }
        f_retval = gpio_set_level(MY_POWER_MOSFET_GATE_GPIO_NUM, 1);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "%s(). ABORT. gpio_set_level(MY_POWER_MOSFET_GATE_GPIO_NUM) | err %i (%s)", __FUNCTION__, f_retval,
                    esp_err_to_name(f_retval));
            // GOTO
            goto cleanup;
        }
    }

    /********************************************************************************
     * Init OLED
     */
    ESP_LOGI(TAG, "  Init OLED SSD1306...");

    // @important Do not use ={} or ={0}
    mjd_ssd1306_config_t ssd1306_config =
    MJD_SSD1306_CONFIG_DEFAULT()
            ;
    ssd1306_config.i2c_slave_addr = MY_SSD1306_I2C_SLAVE_ADDRESS;
    ssd1306_config.i2c_port_num = MY_SSD1306_I2C_MASTER_PORT_NUM;
    ssd1306_config.i2c_scl_gpio_num = MY_SSD1306_I2C_SCL_GPIO_NUM;
    ssd1306_config.i2c_sda_gpio_num = MY_SSD1306_I2C_SDA_GPIO_NUM;

    ssd1306_config.oled_dimension = MY_SSD1306_OLED_DIMENSION_NUM;
    ssd1306_config.oled_flip_mode = 0; /*!< 0: default, the screen is at the right of the pin row. 1: flip it (if you mounted the oled board the other way around). */

    if (MY_SSD1306_OLED_IS_USED == 1) {
        f_retval = mjd_ssd1306_init(&ssd1306_config);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "%s(). mjd_ssd1306_init() err %i %s", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
            // GOTO
            goto cleanup;
        }
    }

    /********************************************************************************
     * Init component JSNSR04
     */
    ESP_LOGI(TAG, "  Init JSNSR04...");

    mjd_jsnsr04t_config_t jsnsr04t_config = MJD_JSNSR04T_CONFIG_DEFAULT();
    jsnsr04t_config.trigger_gpio_num = MY_JSNSR04T_TRIGGER_GPIO_NUM;
    jsnsr04t_config.echo_gpio_num = MY_JSNSR04T_ECHO_GPIO_NUM;
    jsnsr04t_config.rmt_channel = RMT_CHANNEL_0;

    f_retval = mjd_jsnsr04t_init(&jsnsr04t_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "mjd_jsnsr04t_init() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    mjd_jsnsr04t_log_config(jsnsr04t_config);

    /********************************************************************************
     * Get data from JSNSR04
     */
    const uint32_t NBR_OF_READS = 10; // 1 5 10 50 100 500 1000 100000
    ESP_LOGI(TAG, "LOOP: NBR_OF_READS %u", NBR_OF_READS);

    mjd_log_memory_statistics();

    for (uint32_t j = 1; j <= NBR_OF_READS; ++j) {
        ESP_LOGI(TAG, "");
        ESP_LOGI(TAG, "***LOOP ITEM #%u of %u***", j, NBR_OF_READS);

        mjd_led_on(MY_LED_ON_DEVBOARD_GPIO_NUM);

        // MEASURE
        mjd_jsnsr04t_data_t jsnsr04t_data = MJD_JSNSR04T_DATA_DEFAULT();
        f_retval = mjd_jsnsr04t_get_measurement(&jsnsr04t_config, &jsnsr04t_data);
        if (f_retval == ESP_OK) {
            // LOG
            /////ESP_LOGI(TAG, "  %30s = %f", "distance_cm", jsnsr04t_data.distance_cm);
            mjd_jsnsr04t_log_data(jsnsr04t_data);
            // OLED
            if (MY_SSD1306_OLED_IS_USED == true) {
                /////mjd_ssd1306_cmd_clear_screen(&ssd1306_config);
                char str_line[80];
                sprintf(str_line, "#%u:", j);
                mjd_ssd1306_cmd_write_line(&ssd1306_config, MJD_SSD1306_LINE_NR_1, str_line);
                sprintf(str_line, "%6.2f cm", jsnsr04t_data.distance_cm);
                mjd_ssd1306_cmd_write_line(&ssd1306_config, MJD_SSD1306_LINE_NR_2, str_line);
            }
        } else {
            ESP_LOGE(TAG, "[CONTINUE FOR-LOOP] mjd_jsnsr04t_get_measurement() failed | err %i (%s)", f_retval,
                    esp_err_to_name(f_retval));
            // OLED
            if (MY_SSD1306_OLED_IS_USED == true) {
                /////mjd_ssd1306_cmd_clear_screen(&ssd1306_config);
                char str_line[80];
                sprintf(str_line, "#%u cm:", j);
                mjd_ssd1306_cmd_write_line(&ssd1306_config, MJD_SSD1306_LINE_NR_1, str_line);
                mjd_ssd1306_cmd_write_line(&ssd1306_config, MJD_SSD1306_LINE_NR_2, "ERROR");
            }
            // GOTO
            /////goto cleanup;
        }

        mjd_led_off(MY_LED_ON_DEVBOARD_GPIO_NUM);
    }

    mjd_log_memory_statistics();

    /********************************************************************************
     * DeInit component JSNSR04
     */
    ESP_LOGI(TAG, "  DeInit JSNSR04...");

    f_retval = mjd_jsnsr04t_deinit(&jsnsr04t_config);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "mjd_jsnsr04t_deinit() err %i (%s)", f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    mjd_jsnsr04t_log_config(jsnsr04t_config);

    /********************************************************************************
     * DeInit OLED
     */
    ESP_LOGI(TAG, "  Deinit OLED SSD1306...");

    if (MY_SSD1306_OLED_IS_USED == 1) {
        f_retval = mjd_ssd1306_deinit(&ssd1306_config);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "%s(). mjd_ssd1306_deinit() err %i %s", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
            // GOTO
            goto cleanup;
        }
    }

    /********************************************************************************
     * DeInit POWER MOSFET
     */
    ESP_LOGI(TAG, "  DeInit POWER MOSFET...");

    if (MY_POWER_MOSFET_IS_USED == 1) {
        f_retval = gpio_set_level(MY_POWER_MOSFET_GATE_GPIO_NUM, 0);
        if (f_retval != ESP_OK) {
            ESP_LOGE(TAG, "%s(). ABORT. gpio_set_level(MY_POWER_MOSFET_GATE_GPIO_NUM) | err %i (%s)", __FUNCTION__, f_retval,
                    esp_err_to_name(f_retval));
            // GOTO
            goto cleanup;
        }
    }

    /********************************************************************************
     * DEEP SLEEP & RESTART TIMER
     * @sop Put this section in comments when testing other things afterwards (else the MCU restarts every time...).
     * @important In deep sleep mode, wireless peripherals are powered down. Before entering sleep mode, applications must disable WiFi and BT using appropriate calls (esp_wifi_stop(), esp_bluedroid_disable(), esp_bt_controller_disable()).
     * @doc https://esp-idf.readthedocs.io/en/latest/api-reference/system/sleep_modes.html
     *
     */
    ESP_LOGI(TAG, "\n\n***SECTION: DEEP SLEEP***");

    mjd_log_memory_statistics();

    const uint32_t MY_DEEP_SLEEP_TIME_SEC = 15; // 15 15*60 30*60
    f_retval = esp_sleep_enable_timer_wakeup(mjd_seconds_to_microseconds(MY_DEEP_SLEEP_TIME_SEC));
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "%s(). esp_sleep_enable_timer_wakeup() err %i %s", __FUNCTION__, f_retval, esp_err_to_name(f_retval));
        // GOTO
        goto cleanup;
    }

    // @important Give ESP_LOGI() some time to log the information to UART before deep sleep kicks in!
    ESP_LOGI(TAG, "Entering deep sleep (the MCU should wake up %u seconds later)...\n\n", MY_DEEP_SLEEP_TIME_SEC);
    vTaskDelay(RTOS_DELAY_100MILLISEC);

    // DEVTEMP-BEGIN WORKAROUND for ESP-IDF v3.2-dev-607-gb14e87a6 "The system does not wake up properly anymore after the ***2nd*** deep sleep period (and any deep sleep period after that)."
    //     A temporary workaround is to call esp_set_deep_sleep_wake_stub(NULL); before entering deep sleep
    //     https://www.esp32.com/viewtopic.php?f=13&t=6919&p=29714
    /////esp_set_deep_sleep_wake_stub(NULL);
    // DEVTEMP-END

    esp_deep_sleep_start();

    // DEVTEMP @important I never get to this code line if deep sleep is initiated :P

    /********************************************************************************
     * LABEL
     */
    cleanup: ;

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
