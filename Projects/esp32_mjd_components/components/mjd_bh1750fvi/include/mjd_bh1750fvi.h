/*
 *
 */
#ifndef __MJD_BH1750FVI_H__
#define __MJD_BH1750FVI_H__

#ifdef __cplusplus
extern "C" {
#endif

// I2C features

// BH1750FVI Settings
#define BH1750FVI_I2C_ADDRESS_1 0x23
#define BH1750FVI_I2C_ADDRESS_2 0x5C

#define BH1750FVI_I2C_MASTER_FREQ_HZ         (400 * 1000)  /*!< Sensor supports: I2C master clock frequency 100K (Normal) XOR 400K (High) | ESP32 maxmimum is 1Mhz! */

#define BH1750FVI_I2C_MASTER_RX_BUF_DISABLE  (0)           /*!< For slaves. I2C master does not need RX buffer */
#define BH1750FVI_I2C_MASTER_TX_BUF_DISABLE  (0)           /*!< For slaves. I2C master does not need TX buffer */
#define BH1750FVI_I2C_MASTER_INTR_FLAG_NONE  (0)           /*!< For slaves. I2C disable interrupt */

// BH1750FVI Registers
#define BH1750FVI_REGISTER_POWER_DOWN 0x00                 /*!< No active state. */
#define BH1750FVI_REGISTER_POWER_ON 0x01                   /*!< Waiting for measurement command. */
#define BH1750FVI_REGISTER_RESET 0x07                      /*!< Reset Data register value. Reset command is not acceptable in Power Down mode. */
#define BH1750FVI_REGISTER_CONTINUOUS_HIGH_RES_MODE 0x10   /*!< Start measurement at 1lx resolution. Measurement Time is typically 120ms. */
#define BH1750FVI_REGISTER_CONTINUOUS_HIGH_RES_MODE_2 0x11 /*!< Start measurement at 0.5lx resolution. Measurement Time is typically 120ms. */
#define BH1750FVI_REGISTER_CONTINUOUS_LOW_RES_MODE 0x13    /*!< Start measurement at 4lx resolution. Measurement Time is typically 16ms. */
#define BH1750FVI_REGISTER_ONE_TIME_HIGH_RES_MODE 0x20     /*!< Start measurement at 1lx resolution. Measurement Time is typically 120ms. It is automatically set to Power Down mode after measurement. */
#define BH1750FVI_REGISTER_ONE_TIME_HIGH_RES_MODE_2 0x21   /*!< Start measurement at 0.5lx resolution. Measurement Time is typically 120ms. It is automatically set to Power Down mode after measurement. */
#define BH1750FVI_REGISTER_ONE_TIME_LOW_RES_MODE 0x23      /*!< Start measurement at 4lx resolution. Measurement Time is typically 16ms. It is automatically set to Power Down mode after measurement. */

/**
 * BH1750FVI Unknowns:
 * - Change Measurement time ( High bit ) 01000_MT[7,6,5] Change measurement time. ※ Please refer "adjust measurement result for influence of optical window."
 * - Change Measurement time ( Low bit ) 011_MT[4,3,2,1,0] Change measurement time. ※ Please refer "adjust measurement result for influence of optical window."
 */

/**
 * Data structs
 */
typedef struct {
    bool manage_i2c_driver;
    i2c_port_t i2c_port_num;
    uint8_t i2c_slave_addr;
    gpio_num_t i2c_scl_gpio_num;
    gpio_num_t i2c_sda_gpio_num;
    uint8_t bh1750fvi_mode;
} mjd_bh1750fvi_config_t;

#define MJD_BH1750FVI_CONFIG_DEFAULT() { \
    .manage_i2c_driver = true,  \
    .i2c_port_num = I2C_NUM_0,  \
    .i2c_slave_addr = BH1750FVI_I2C_ADDRESS_1,  \
    .bh1750fvi_mode = BH1750FVI_REGISTER_ONE_TIME_HIGH_RES_MODE_2 \
};

typedef struct {
    float light_intensity_lux;
} mjd_bh1750fvi_data_t;

/**
 * Function declarations
 */
esp_err_t mjd_bh1750fvi_init(const mjd_bh1750fvi_config_t* config);
esp_err_t mjd_bh1750fvi_deinit(const mjd_bh1750fvi_config_t* config);
esp_err_t mjd_bh1750fvi_read(const mjd_bh1750fvi_config_t* config, mjd_bh1750fvi_data_t* data);

#ifdef __cplusplus
}
#endif

#endif /* __MJD_BH1750FVI_H__ */
