#include "mjd.h"

// I2C driver
#include "driver/i2c.h"

/*
 * Logging
 */
static const char TAG[] = "myapp";

/*
 * KConfig: LED, WIFI, I2C config
 *
 *
 */
static const int MY_SCANNER_I2C_MASTER_NUM = CONFIG_MY_SCANNER_I2C_MASTER_NUM; /* 0 1 !< I2C port number for master dev */
static const int MY_SCANNER_I2C_SCLK_GPIO_NUM = CONFIG_MY_SCANNER_I2C_SCLK_GPIO_NUM; /* 21 13 !< gpio number for I2C master clock */
static const int MY_SCANNER_I2C_SDA_GPIO_NUM = CONFIG_MY_SCANNER_I2C_SDA_GPIO_NUM; /* 17 15!< gpio number for I2C master data  */

/*
 * Project Globs
 */
#define SCANNER_I2C_MASTER_FREQ_HZ         (100 * 1000) /*!< I2C master clock frequency 100Khz (preferred) XOR 400Khz */
#define SCANNER_I2C_MASTER_RX_BUF_DISABLE  (0)          /*!< For slaves. I2C master does not need buffer */
#define SCANNER_I2C_MASTER_TX_BUF_DISABLE  (0)          /*!< For slaves. I2C master does not need buffer */
#define SCANNER_I2C_MASTER_INTR_FLAG_NONE  (0)

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
    ESP_LOGI(TAG, "@doc Wait 2 seconds after power-on (start logic analyzer, let I2C sensors become active!)");
    vTaskDelay(RTOS_DELAY_2SEC);

    /*
     * KConfig details
     */
    ESP_LOGI(TAG, "@info MY_SCANNER_I2C_MASTER_NUM:      %i", MY_SCANNER_I2C_MASTER_NUM);
    ESP_LOGI(TAG, "@info MY_SCANNER_I2C_SCLK_GPIO_NUM:   %i", MY_SCANNER_I2C_SCLK_GPIO_NUM);
    ESP_LOGI(TAG, "@info MY_SCANNER_I2C_SDA_GPIO_NUM:    %i", MY_SCANNER_I2C_SDA_GPIO_NUM);

    /*
     * MAIN
     */
    ESP_LOGI(TAG, "START SCANNING...");

    // config
    ESP_LOGI(TAG, "i2c_param_config()");
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.scl_io_num = MY_SCANNER_I2C_SCLK_GPIO_NUM;
    conf.sda_io_num = MY_SCANNER_I2C_SDA_GPIO_NUM;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = SCANNER_I2C_MASTER_FREQ_HZ;
    ESP_ERROR_CHECK(i2c_param_config(MY_SCANNER_I2C_MASTER_NUM, &conf));

    // install
    ESP_LOGI(TAG, "i2c_driver_install()");
    ESP_ERROR_CHECK(
            i2c_driver_install( MY_SCANNER_I2C_MASTER_NUM, I2C_MODE_MASTER,
                    SCANNER_I2C_MASTER_RX_BUF_DISABLE, SCANNER_I2C_MASTER_TX_BUF_DISABLE, SCANNER_I2C_MASTER_INTR_FLAG_NONE));

    // loop
    esp_err_t esp_retval;
    uint8_t num_devices_found = 0;
    uint8_t max_address = 0x7F; // 7 lower bits

    ESP_LOGI(TAG, "Checking devices 0x%X - 0x%X ...", 1, max_address);

    for (uint8_t slave_address = 1; slave_address <= max_address; ++slave_address) {
        ESP_LOGI(TAG, "0x%X ", slave_address);

        i2c_cmd_handle_t cmd;
        cmd = i2c_cmd_link_create();
        i2c_master_start(cmd);
        i2c_master_write_byte(cmd, (slave_address << 1) | I2C_MASTER_WRITE, true);
        i2c_master_stop(cmd);
        esp_retval = i2c_master_cmd_begin(MY_SCANNER_I2C_MASTER_NUM, cmd, RTOS_DELAY_1SEC);
        if (esp_retval == ESP_OK) {
            ++num_devices_found;
            ESP_LOGI(TAG, "\n    ***YES, found device with slave_address 0x%X***\n", slave_address);
            fflush(stdout);
        }
        i2c_cmd_link_delete(cmd);

        // Extra delay between scanning I2C slave devices so it is easier to see on the logic analyzer
        vTaskDelay(RTOS_DELAY_1MILLISEC);
    }

    printf("\n");
    ESP_LOGI(TAG, "SCAN REPORT:");
    if (num_devices_found != 0) {
        ESP_LOGI(TAG, "  Number of devices found: %u", num_devices_found);
    } else {
        ESP_LOGI(TAG, "  No devices found\n");
    }

    /*
     * END
     */
    ESP_LOGI(TAG, "END %s()", __FUNCTION__);
}
