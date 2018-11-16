/*
 *
 */
#ifndef __MJD_DS3231_H__
#define __MJD_DS3231_H__

#ifdef __cplusplus
extern "C" {
#endif

// I2C features

// DS3231 Settings
#define DS3231_I2C_ADDR                   0x68        /*!< FIXED slave address for the I2C slave device/sensor */
#define DS3231_I2C_MASTER_FREQ_HZ         100 * 1000  /*!< I2C master clock frequency 100K (Normal) XOR 400K (High) */
#define DS3231_I2C_MASTER_RX_BUF_DISABLE  0           /*!< For slaves. I2C master does not need RX buffer */
#define DS3231_I2C_MASTER_TX_BUF_DISABLE  0           /*!< For slaves. I2C master does not need TX buffer */
#define DS3231_I2C_MASTER_INTR_FLAG_NONE  0

// DS3231 Registers
#define DS3231_REGISTER_SECONDS 0x00  /*!< 00–59 BCD */
#define DS3231_REGISTER_MINUTES 0x01  /*!< 00–59 BCD */
#define DS3231_REGISTER_HOUR    0x02  /*!< 00–23 BCD */
#define DS3231_REGISTER_WEEKDAY 0x05  /*!< 1-7 1=Sunday 7=Saturday */
#define DS3231_REGISTER_DAY     0x04  /*!< 01-31 */
#define DS3231_REGISTER_MONTH   0x05  /*!< 01–12 */
#define DS3231_REGISTER_YEAR    0x06  /*!< 0-99 */

#define DS3231_REGISTER_CONTROL         0x0E  /*!< [notusedyet] EOSC BBSQW CONV RS2 RS1 INTCN A2IE A1IE */
#define DS3231_REGISTER_CONTROL_STATUS  0x0F  /*!< [notusedyet] OSF 0 0 0 EN32kHz BSY A2F A1F */
#define DS3231_REGISTER_AGING_OFFSET    0x10  /*!< [notusedyet] SIGN DATA DATA DATA DATA DATA DATA DATA */
#define DS3231_REGISTER_TEMPERATURE_MSB 0x11  /*!< [notusedyet] SIGN DATA DATA DATA DATA DATA DATA DATA*/
#define DS3231_REGISTER_TEMPERATURE_LSB 0x12  /*!< [notusedyet] DATA DATA 0 0 0 0 0 0 */

/**
 * Data structs
 */
typedef struct {
        bool manage_i2c_driver;
        i2c_port_t i2c_port_num;
        uint8_t i2c_slave_addr;
        gpio_num_t scl_io_num;
        gpio_num_t sda_io_num;
} mjd_ds3231_config_t;

#define MJD_DS3231_CONFIG_DEFAULT() { \
    .manage_i2c_driver = true,  \
    .i2c_port_num = I2C_NUM_0,  \
    .i2c_slave_addr = DS3231_I2C_ADDR  \
};

typedef struct {
        uint8_t seconds;
        uint8_t minutes;
        uint8_t hours;
        uint8_t weekday;
        uint8_t day;
        uint8_t month;
        uint8_t year;
} mjd_ds3231_data_t;

/**
 * Function declarations
 */
esp_err_t mjd_ds3231_init(const mjd_ds3231_config_t* config);
esp_err_t mjd_ds3231_deinit(const mjd_ds3231_config_t* config);
esp_err_t mjd_ds3231_get_datetime(const mjd_ds3231_config_t* config, mjd_ds3231_data_t* data);
esp_err_t mjd_ds3231_set_datetime(const mjd_ds3231_config_t* config, const mjd_ds3231_data_t* data);

#ifdef __cplusplus
}
#endif

#endif /* __MJD_DS3231_H__ */
