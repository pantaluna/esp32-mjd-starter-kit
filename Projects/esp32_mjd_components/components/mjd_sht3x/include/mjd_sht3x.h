/*
 * Component header file.
 */
#ifndef __MJD_SHT3X_H__
#define __MJD_SHT3X_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Device specifics (registers, NVRAM, etc.)
 */
#include "mjd_sht3x_defs.h"

/**
 * I2C Settings
 *
 * - I2C_ADDRESS:
 *      I2C Address     Condition
 *      -----------     ---------
 *      0x44 (default)  ADDR (pin 2) connected to logic low
 *      0x45            ADDR (pin 2) connected to logic high
 *
 */
#define MJD_SHT3X_I2C_ADDRESS_DEFAULT        (0x44)       /*!< The default 0x44 can be changed. */
#define MJD_SHT3X_I2C_MASTER_NUM_DEFAULT     (I2C_NUM_0)  /*!< The default I2C_NUM_0 can be changed. */
#define MJD_SHT3X_I2C_TIMEOUT_DEFAULT        (2000 / portTICK_PERIOD_MS) /*!< The default 2000ms can be changed. */
#define MJD_SHT3X_I2C_MASTER_FREQ_HZ         (100 * 1000) /*!< [ESP32 Max 1 MHz.] Use 10Khz for long wires (>25cm). I2C master clock freq: Normal (100 KHz), FastMode (400 Khz), FastModePlus (1 Mhz). */
#define MJD_SHT3X_I2C_MASTER_RX_BUF_DISABLE  (0)          /*!< I2C master does not need RX buffer. This param is for I2C slaves. */
#define MJD_SHT3X_I2C_MASTER_TX_BUF_DISABLE  (0)          /*!< I2C master does not need TX buffer. This param is for I2C slaves. */
#define MJD_SHT3X_I2C_MASTER_INTR_FLAG_NONE  (0)

/**
 * TIMER SETTINGS
 *
 * @doc Not needed for SHT3X because it has no MEASUREMENT_READY pin.
 */

/**
 * CUSTOM default device settings - see mjd_sht3x_init()
 *
 *   These CUSTOM ones are the ones that are DIFFERENT from the device defaults. They have been determined by typical project requirements and from experience.
 *   These settings can be overwritten in your project during init().
 *
 * @doc The UART Output will lag behind significantly when changing the default to a higher Data Rate.
 *
 */
#define MJD_SHT3X_REPEATABILITY_DEFAULT  (MJD_SHT3X_REPEATABILITY_HIGH)

/**
 * Data structs
 *
 */

/*
 * mjd_sht3x_config_t
 *      int_gpio_num : INT DRDY Data Ready pin @rule -1 means not used to detect that a measurement is ready to be read.
 */
typedef struct {
        bool manage_i2c_driver;
        uint8_t i2c_slave_addr;
        int i2c_timeout; /*<! @param ticks_to_wait Maximum waiting ticks */
        i2c_port_t i2c_port_num;
        gpio_num_t i2c_scl_gpio_num;
        gpio_num_t i2c_sda_gpio_num;

        mjd_sht3x_repeatability_t repeatability;
} mjd_sht3x_config_t;

#define MJD_SHT3X_CONFIG_DEFAULT() { \
    .manage_i2c_driver = true, \
    .i2c_slave_addr = MJD_SHT3X_I2C_ADDRESS_DEFAULT, \
    .i2c_timeout = MJD_SHT3X_I2C_TIMEOUT_DEFAULT, \
    .i2c_port_num = MJD_SHT3X_I2C_MASTER_NUM_DEFAULT, \
    .i2c_scl_gpio_num = -1, \
    .i2c_sda_gpio_num = -1, \
    .repeatability = MJD_SHT3X_REPEATABILITY_DEFAULT \
};

/*
 * mjd_sht3x_data_t
 */
typedef int16_t mjd_sht3x_data_raw_t; // @important In the past I used uint16_t which was wrong (and stripped the sign bit#15)!

typedef struct {
        mjd_sht3x_data_raw_t raw_value;
        mjd_sht3x_repeatability_t repeatability;
        float temperature_celsius;
        float temperature_fahrenheit;
        float relative_humidity;
        float dew_point_celsius;
        float dew_point_fahrenheit;
} mjd_sht3x_data_t;

/**
 * Function declarations
 */
esp_err_t mjd_sht3x_get_alert_pending_status(const mjd_sht3x_config_t* param_ptr_config, mjd_sht3x_alert_pending_status_t* param_ptr_data);
esp_err_t mjd_sht3x_get_heater_status(mjd_sht3x_config_t* param_ptr_config, mjd_sht3x_heater_status_t* param_ptr_data);
esp_err_t mjd_sht3x_get_rh_tracking_alert(mjd_sht3x_config_t* param_ptr_config, mjd_sht3x_rh_tracking_alert_t* param_ptr_data);
esp_err_t mjd_sht3x_get_t_tracking_alert(const mjd_sht3x_config_t* param_ptr_config, mjd_sht3x_t_tracking_alert_t* param_ptr_data);
esp_err_t mjd_sht3x_get_system_reset_detected(mjd_sht3x_config_t* param_ptr_config, mjd_sht3x_system_reset_detected_t* param_ptr_data);
esp_err_t mjd_sht3x_get_last_command_status(const mjd_sht3x_config_t* param_ptr_config, mjd_sht3x_last_command_status_t* param_ptr_data);
esp_err_t mjd_sht3x_get_write_data_checksum_status(const mjd_sht3x_config_t* param_ptr_config,
                                                   mjd_sht3x_write_data_checksum_status_t* param_ptr_data);

esp_err_t mjd_sht3x_log_device_parameters(mjd_sht3x_config_t* param_ptr_config);

esp_err_t mjd_sht3x_cmd_break(const mjd_sht3x_config_t* param_ptr_config);
esp_err_t mjd_sht3x_cmd_clear_status_register(const mjd_sht3x_config_t* param_ptr_config);
esp_err_t mjd_sht3x_cmd_soft_reset(const mjd_sht3x_config_t* param_ptr_config);

esp_err_t mjd_sht3x_cmd_get_single_measurement(const mjd_sht3x_config_t* param_ptr_config, mjd_sht3x_data_t* param_ptr_data);

esp_err_t mjd_sht3x_init(mjd_sht3x_config_t* param_ptr_config);
esp_err_t mjd_sht3x_deinit(const mjd_sht3x_config_t* param_ptr_config);

#ifdef __cplusplus
}
#endif

#endif /* __MJD_SHT3X_H__ */
