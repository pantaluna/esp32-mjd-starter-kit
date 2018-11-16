#include "mjd.h"
#include "mjd_ds3231.h"

/*
 * Logging
 */
static const char TAG[] = "myapp";

// My DS3231 Input Params
#define DS3231_I2C_SCL_GPIO_NUM              21          /*!< gpio number for I2C SCL clock pin */
#define DS3231_I2C_SDA_GPIO_NUM              17          /*!< gpio number for I2C SDA data pin */

/*
 * FreeRTOS settings
 */
#define MYAPP_RTOS_TASK_STACK_SIZE_LARGE (8192)

/*
 * MAIN
 */
void app_main() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    /* MY STANDARD Init */
    mjd_log_wakeup_details();
    mjd_log_chip_info();
    mjd_log_time();
    mjd_log_memory_statistics();
    ESP_LOGI(TAG, "@doc Wait 2 seconds after power-on (start logic analyzer, let sensors become active!)");
    vTaskDelay(RTOS_DELAY_2SEC);

    /*
     * MAIN
     */
    ESP_LOGI(TAG, "START...");

    ESP_LOGI(TAG, "***** init");
    mjd_ds3231_config_t config = MJD_DS3231_CONFIG_DEFAULT()
    ;  // @important Do not use ={} or ={0}
    ESP_LOGI(TAG, "***** RTC Module DS3231: I2C slave address = 0x%x", config.i2c_slave_addr);
    config.scl_io_num = DS3231_I2C_SCL_GPIO_NUM;
    config.sda_io_num = DS3231_I2C_SDA_GPIO_NUM;
    mjd_ds3231_init(&config);


#define MJD_DS3231_CONFIG_DEFAULT() { \
    .manage_i2c_driver = true,  \
    .i2c_port_num = I2C_NUM_0,  \
    .i2c_slave_addr = DS3231_I2C_ADDR  \
};

    ESP_LOGI(TAG, "***** GET datetime from the device after power-on (check datetime from previous session is preserved!)");
    mjd_ds3231_data_t data_first =
        { 0 };
    mjd_ds3231_get_datetime(&config, &data_first);

    // Response: year = implicitly 20XX (just the year, not the century)
    printf("RESPONSE\n");
    printf("  year 20%u | month %u | day %u | hours %u | minutes %u | seconds %u\n", data_first.year, data_first.month,
            data_first.day, data_first.hours, data_first.minutes, data_first.seconds);
    // Delay @ endOfLoop
    printf("\n");
    vTaskDelay(RTOS_DELAY_5SEC);

    //
    ESP_LOGI(TAG, "***** SET datetime Sat Dec 25, 2055 23:45:30h");
    mjd_ds3231_data_t data =
        { 0 };
    data.year = 55; // implicitly 20XX (just the year, not the century)
    data.month = 12;
    data.day = 25;
    data.weekday = 7; //7=Saturday
    data.hours = 23;
    data.minutes = 45;
    data.seconds = 30;
    mjd_ds3231_set_datetime(&config, &data);

    //
    ESP_LOGI(TAG, "***** GET datetime in a loop 5x");
    uint32_t i;
    for (i = 0; i < 5; i++) {
        mjd_ds3231_data_t data =
            { 0 };
        mjd_ds3231_get_datetime(&config, &data);

        // Response:  // year = implicitly 20XX (just the year, not the century)
        printf("RESPONSE\n");
        printf("  year 20%u | month %u | day %u | hours %u | minutes %u | seconds %u\n", data.year, data.month, data.day,
                data.hours, data.minutes, data.seconds);
        // Delay @ endOfLoop
        printf("\n");
        vTaskDelay(RTOS_DELAY_1SEC);
    }

    // ***** deinit
    //  Reuse the var config of the section "init"
    ESP_LOGI(TAG, "***** deinit");
    mjd_ds3231_deinit(&config);

    ESP_LOGI(TAG, "==> NEXT STEP: [ower down the ESP32 device briefly and power it up again. Check that the correct datetime was kept alive on the RTC board");

    /*
     * END
     */
    ESP_LOGI(TAG, "END %s()", __FUNCTION__);
}
