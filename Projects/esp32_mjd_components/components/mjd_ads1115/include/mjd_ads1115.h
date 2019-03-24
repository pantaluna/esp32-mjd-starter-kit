/*
 * Component header file.
 */
#ifndef __MJD_ADS1115_H__
#define __MJD_ADS1115_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Device specifics (registers, NVRAM, etc.)
 */
#include "mjd_ads1115_defs.h"

/**
 * I2C Settings
 */
#define MJD_ADS1115_I2C_ADDRESS_DEFAULT        (0x48)       /*!< This default 0x48 can be changed. */
#define MJD_ADS1115_I2C_MASTER_NUM_DEFAULT     (I2C_NUM_0)  /*!< This default I2C_NUM_0 can be changed. */
#define MJD_ADS1115_I2C_TIMEOUT_DEFAULT        (1000 / portTICK_PERIOD_MS) /*!< The default 1000ms can be changed. */
#define MJD_ADS1115_I2C_MASTER_FREQ_HZ         (100 * 1000) /*!< [ESP32 Max 1 MHz.] Use 10Khz for long wires (>25cm). I2C master clock freq: Normal (100 KHz), FastMode (400 Khz), FastModePlus (1 Mhz). */
#define MJD_ADS1115_I2C_MASTER_RX_BUF_DISABLE  (0)          /*!< I2C master does not need RX buffer. This param is for I2C slaves. */
#define MJD_ADS1115_I2C_MASTER_TX_BUF_DISABLE  (0)          /*!< I2C master does not need TX buffer. This param is for I2C slaves. */
#define MJD_ADS1115_I2C_MASTER_INTR_FLAG_NONE  (0)

/**
 * TIMER SETTINGS
 */
#define MJD_ADS1115_TIMER_GROUP_ID   (TIMER_GROUP_0) // TODO Move this to KConfig.
#define MJD_ADS1115_TIMER_ID         (TIMER_1)  // @important Use _1 for ADS1115 (_0 is used by mjd_mlx90393). TODO Move this to KConfig.

/**
 * CUSTOM default device settings - see mjd_ads1115_init()
 *
 *   These CUSTOM ones are the ones that are DIFFERENT from the device defaults. They have been determined by typical project requirements and from experience.
 *   These settings can be overwritten in your project during init().
 *
 * @doc The UART Output will lag behind significantly when changing the default to a higher Data Rate.
 *
 */
/////#define MJD_ADS1115_MUX_DEFAULT  (MJD_ADS1115_MUX_0_1)
#define MJD_ADS1115_MUX_DEFAULT  (MJD_ADS1115_MUX_0_GND)

/////#define MJD_ADS1115_PGA_DEFAULT  (MJD_ADS1115_PGA_2_048)
#define MJD_ADS1115_PGA_DEFAULT  (MJD_ADS1115_PGA_4_096)
/////#define MJD_ADS1115_PGA_DEFAULT  (MJD_ADS1115_PGA_6_144)

/////#define MJD_ADS1115_DATARATE_DEFAULT  (MJD_ADS1115_DATARATE_860_SPS)
/////#define MJD_ADS1115_DATARATE_DEFAULT  (MJD_ADS1115_DATARATE_128_SPS)
/////#define MJD_ADS1115_DATARATE_DEFAULT  (MJD_ADS1115_DATARATE_32_SPS)
#define MJD_ADS1115_DATARATE_DEFAULT  (MJD_ADS1115_DATARATE_8_SPS)


/**
 * Data structs
 *
 */

/*
 * mjd_ads1115_config_t
 *      int_gpio_num : INT DRDY Data Ready pin @rule -1 means not used to detect that a measurement is ready to be read.
 */
typedef struct {
        bool manage_i2c_driver;
        uint8_t i2c_slave_addr;
        int i2c_timeout; /*<! @param ticks_to_wait Maximum waiting ticks */
        i2c_port_t i2c_port_num;
        gpio_num_t i2c_scl_gpio_num;
        gpio_num_t i2c_sda_gpio_num;

        gpio_num_t alert_ready_gpio_num;

        mjd_ads1115_mux_t mux;
        mjd_ads1115_pga_t pga;
        mjd_ads1115_data_rate_t data_rate;
} mjd_ads1115_config_t;

#define MJD_ADS1115_CONFIG_DEFAULT() { \
    .manage_i2c_driver = true, \
    .i2c_slave_addr = MJD_ADS1115_I2C_ADDRESS_DEFAULT, \
    .i2c_timeout = MJD_ADS1115_I2C_TIMEOUT_DEFAULT, \
    .i2c_port_num = MJD_ADS1115_I2C_MASTER_NUM_DEFAULT, \
    .i2c_scl_gpio_num = -1, \
    .i2c_sda_gpio_num = -1, \
    .alert_ready_gpio_num = -1, \
    .mux = MJD_ADS1115_MUX_DEFAULT, \
    .pga = MJD_ADS1115_PGA_DEFAULT, \
    .data_rate = MJD_ADS1115_DATARATE_DEFAULT \
};

/*
 * mjd_ads1115_data_t
 */
typedef int16_t mjd_ads1115_data_raw_t; // @important In the past I used uint16_t which was wrong (and stripped the sign bit#15)!

typedef struct {
        mjd_ads1115_data_raw_t raw_value;
        mjd_ads1115_pga_t pga;
        float volt_value;
} mjd_ads1115_data_t;

/**
 * Function declarations
 */
esp_err_t mjd_ads1115_get_operational_status(const mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_operational_status_t* param_ptr_data);
esp_err_t mjd_ads1115_get_mux(mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_mux_t* param_ptr_data);
esp_err_t mjd_ads1115_get_pga(mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_pga_t* param_ptr_data);
esp_err_t mjd_ads1115_get_operating_mode(const mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_operating_mode_t* param_ptr_data);
esp_err_t mjd_ads1115_get_data_rate(mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_data_rate_t* param_ptr_data);
esp_err_t mjd_ads1115_get_comparator_mode(const mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_comparator_mode_t* param_ptr_data);
esp_err_t mjd_ads1115_get_comparator_polarity(const mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_comparator_polarity_t* param_ptr_data);
esp_err_t mjd_ads1115_get_latching_comparator(const mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_latching_comparator_t* param_ptr_data);
esp_err_t mjd_ads1115_get_comparator_queue(const mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_comparator_queue_t* param_ptr_data);
esp_err_t mjd_ads1115_get_low_threshold(const mjd_ads1115_config_t* param_ptr_config, uint16_t* param_ptr_data);
esp_err_t mjd_ads1115_get_high_threshold(const mjd_ads1115_config_t* param_ptr_config, uint16_t* param_ptr_data);
esp_err_t mjd_ads1115_get_conversion_ready_pin_in_low_reg(const mjd_ads1115_config_t* param_ptr_config,
                                                          mjd_ads1115_conversion_ready_pin_in_low_reg_t* param_ptr_data);
esp_err_t mjd_ads1115_get_conversion_ready_pin_in_high_reg(const mjd_ads1115_config_t* param_ptr_config,
                                                           mjd_ads1115_conversion_ready_pin_in_high_reg_t* param_ptr_data);

esp_err_t mjd_ads1115_set_operational_status(const mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_operational_status_t param_data);
esp_err_t mjd_ads1115_set_mux(mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_mux_t param_data);
esp_err_t mjd_ads1115_set_pga(mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_pga_t param_data);
esp_err_t mjd_ads1115_set_operating_mode(const mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_operating_mode_t param_data);
esp_err_t mjd_ads1115_set_data_rate(mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_data_rate_t param_data);
esp_err_t mjd_ads1115_set_comparator_mode(const mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_comparator_mode_t param_data);
esp_err_t mjd_ads1115_set_comparator_polarity(const mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_comparator_polarity_t param_data);
esp_err_t mjd_ads1115_set_latching_comparator(const mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_latching_comparator_t param_data);
esp_err_t mjd_ads1115_set_comparator_queue(const mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_comparator_queue_t param_data);
esp_err_t mjd_ads1115_set_low_threshold(const mjd_ads1115_config_t* param_ptr_config, uint16_t param_data);
esp_err_t mjd_ads1115_set_high_threshold(const mjd_ads1115_config_t* param_ptr_config, uint16_t param_data);
esp_err_t mjd_ads1115_set_conversion_ready_pin_in_low_reg(const mjd_ads1115_config_t* param_ptr_config,
                                                          mjd_ads1115_conversion_ready_pin_in_low_reg_t param_data);
esp_err_t mjd_ads1115_set_conversion_ready_pin_in_high_reg(const mjd_ads1115_config_t* param_ptr_config,
                                                           mjd_ads1115_conversion_ready_pin_in_high_reg_t param_data);

esp_err_t mjd_ads1115_log_device_parameters(mjd_ads1115_config_t* param_ptr_config);

esp_err_t mjd_ads1115_init(mjd_ads1115_config_t* param_ptr_config);
esp_err_t mjd_ads1115_deinit(const mjd_ads1115_config_t* param_ptr_config);

esp_err_t mjd_ads1115_cmd_get_single_conversion(mjd_ads1115_config_t* param_ptr_config, mjd_ads1115_data_t* param_ptr_data);

#ifdef __cplusplus
}
#endif

#endif /* __MJD_ADS1115_H__ */
