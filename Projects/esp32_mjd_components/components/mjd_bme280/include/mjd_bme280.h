/*
 *
 */
#ifndef __MJD_BME280_H__
#define __MJD_BME280_H__

#ifdef __cplusplus
extern "C" {
#endif

// Bosch driver
#include "bme280.h"

// I2C features

// BME280 Settings
#define BME280_I2C_MASTER_FREQ_HZ         (100 * 1000)  /*!< [ESP32 Max 1MHz] I2C master clock frequency Normal (100Kbit) XOR Fast Mode (1 Mbit/s) */
#define BME280_I2C_MASTER_RX_BUF_DISABLE  (0)           /*!< For slaves. I2C master does not need RX buffer */
#define BME280_I2C_MASTER_TX_BUF_DISABLE  (0)           /*!< For slaves. I2C master does not need TX buffer */
#define BME280_I2C_MASTER_INTR_FLAG_NONE  (0)

// Bosch driver settings (not used anymore)
/////#define BME280_I2C_BUFFER_LEN (8)
/////#define BME280_DATA_INDEX     (1)
/////#define BME280_ADDRESS_INDEX  (2)

/**
 * Data structs
 */
typedef struct {
    bool manage_i2c_driver;
    i2c_port_t i2c_port_num;
    uint8_t i2c_slave_addr;
    gpio_num_t i2c_scl_gpio_num;
    gpio_num_t i2c_sda_gpio_num;
    struct bme280_dev bme280_device;
} mjd_bme280_config_t;

/*
 * @doc bme280_work_mode WORKING MODE DEFINITION DOES NOT EXIST ANYMORE!
 *
 *  osr_h:  humidity oversampling ratio
 *  osr_p:  pressure oversampling ratio
 *  osr_t:  temperature oversampling ratio
 *  filter: filter coefficient
 *
 * @doc Oversampling macros
 *  #define BME280_NO_OVERSAMPLING      UINT8_C(0x00)
 *  #define BME280_OVERSAMPLING_1X      UINT8_C(0x01)
 *  #define BME280_OVERSAMPLING_2X      UINT8_C(0x02)
 *  #define BME280_OVERSAMPLING_4X      UINT8_C(0x03)
 *  #define BME280_OVERSAMPLING_8X      UINT8_C(0x04)
 *  #define BME280_OVERSAMPLING_16X     UINT8_C(0x05)
 *
 * @doc Filter coefficient macros
 *  #define BME280_FILTER_COEFF_OFF               (0x00)
 *  #define BME280_FILTER_COEFF_2                 (0x01)
 *  #define BME280_FILTER_COEFF_4                 (0x02)
 *  #define BME280_FILTER_COEFF_8                 (0x03)
 *  #define BME280_FILTER_COEFF_16                (0x04)
 *
 *  TODO Standby time???
 */
#define MJD_BME280_CONFIG_DEFAULT() { \
    .manage_i2c_driver = true,  \
    .i2c_port_num = I2C_NUM_0,  \
    .i2c_slave_addr = 0x76  \
};

typedef struct {
    double humidity_percent;
    double pressure_hpascal;
    double temperature_celsius;
} mjd_bme280_data_t;

/**
 * Function declarations
 */
esp_err_t mjd_bme280_init(mjd_bme280_config_t* ptr_param_config);
esp_err_t mjd_bme280_deinit(mjd_bme280_config_t* ptr_param_config);
esp_err_t mjd_bme280_read_forced(mjd_bme280_config_t* ptr_param_config, mjd_bme280_data_t* ptr_param_data);

#ifdef __cplusplus
}
#endif

#endif /* __MJD_BME280_H__ */
