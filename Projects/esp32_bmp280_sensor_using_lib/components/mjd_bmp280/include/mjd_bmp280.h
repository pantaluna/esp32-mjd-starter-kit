/*
 *
 */
#ifndef __MJD_BMP280_H__
#define __MJD_BMP280_H__

#ifdef __cplusplus
extern "C" {
#endif

// Bosch driver
#include "bmp280.h"

// I2C features

// BMP280 Settings
#define BMP280_I2C_MASTER_FREQ_HZ         (100 * 1000)  /*!< [ESP32 Max 1MHz] I2C master clock frequency Normal (100Kbit) XOR Fast Mode (1 Mbit/s) */
#define BMP280_I2C_MASTER_RX_BUF_DISABLE  (0)           /*!< For slaves. I2C master does not need RX buffer */
#define BMP280_I2C_MASTER_TX_BUF_DISABLE  (0)           /*!< For slaves. I2C master does not need TX buffer */
#define BMP280_I2C_MASTER_INTR_FLAG_NONE  (0)

// Bosch driver settings
#define BMP280_I2C_BUFFER_LEN (8)
#define BMP280_DATA_INDEX     (1)
#define BMP280_ADDRESS_INDEX  (2)

/**
 * Data structs
 */
typedef struct {
    bool manage_i2c_driver;
    i2c_port_t i2c_port_num;
    uint8_t i2c_slave_addr;
    gpio_num_t i2c_scl_gpio_num;
    gpio_num_t i2c_sda_gpio_num;
    u8 bmp280_work_mode;
    u8 bmp280_filter_coefficient;
} mjd_bmp280_config_t;

/*
 * @doc bmp280_work_mode WORKING MODE DEFINITION
 *   #define BMP280_ULTRA_LOW_POWER_MODE          (0x00)
 *   #define BMP280_LOW_POWER_MODE                (0x01)
 *   #define BMP280_STANDARD_RESOLUTION_MODE      (0x02)
 *   #define BMP280_HIGH_RESOLUTION_MODE          (0x03)
 *   #define BMP280_ULTRA_HIGH_RESOLUTION_MODE    (0x04)
 *
 * @doc bmp280_filter_coefficient FILTER DEFINITION
 *   #define BMP280_FILTER_COEFF_OFF               (0x00)
 *   #define BMP280_FILTER_COEFF_2                 (0x01)
 *   #define BMP280_FILTER_COEFF_4                 (0x02)
 *   #define BMP280_FILTER_COEFF_8                 (0x03)
 *   #define BMP280_FILTER_COEFF_16                (0x04)
 */
#define MJD_BMP280_CONFIG_DEFAULT() { \
    .manage_i2c_driver = true,  \
    .i2c_port_num = I2C_NUM_0,  \
    .i2c_slave_addr = 0x76,  \
    .bmp280_work_mode = BMP280_ULTRA_LOW_POWER_MODE,  \
    .bmp280_filter_coefficient = BMP280_FILTER_COEFF_OFF  \
};

typedef struct {
    double pressure_hpascal;
    double temperature_celsius;
} mjd_bmp280_data_t;

/**
 * Function declarations
 */
esp_err_t mjd_bmp280_init(const mjd_bmp280_config_t* config);
esp_err_t mjd_bmp280_deinit(const mjd_bmp280_config_t* config);
esp_err_t mjd_bmp280_read_forced(const mjd_bmp280_config_t* config, mjd_bmp280_data_t* data);

#ifdef __cplusplus
}
#endif

#endif /* __MJD_BMP280_H__ */
