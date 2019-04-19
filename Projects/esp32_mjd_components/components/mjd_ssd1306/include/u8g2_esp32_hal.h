/*
 * Modified u8g2_esp32_hal.h
 *
 * Origin: kolban Feb 12, 2017
 */

#ifndef U8G2_ESP32_HAL_H_
#define U8G2_ESP32_HAL_H_

#ifdef __cplusplus
extern "C" {
#endif

// Includes
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/i2c.h"

#include "u8g2.h"

#define U8G2_ESP32_HAL_UNDEFINED (-1)

#define I2C_MASTER_TX_BUF_DISABLE   (0)      //  I2C master do not need buffer
#define I2C_MASTER_RX_BUF_DISABLE   (0)      //  I2C master do not need buffer
#define I2C_MASTER_FREQ_HZ          (100000) //  I2C master clock frequency 10-100Khz
#define ACK_CHECK_EN   (0x1)                 //  I2C master will check ack from slave
#define ACK_CHECK_DIS  (0x0)                 //  I2C master will not check ack from slave

typedef struct {
        bool manage_i2c_driver; // ***Added new property***
        i2c_port_t i2c_port_num; // ***Added new property*** I2C_NUM_0 I2C_NUM_1
        gpio_num_t clk;
        gpio_num_t mosi;
        gpio_num_t sda; // data for I2C
        gpio_num_t scl; // clock for I2C
        gpio_num_t cs;
        gpio_num_t reset;
        gpio_num_t dc;
} u8g2_esp32_hal_t;

#define U8G2_ESP32_HAL_DEFAULT { \
    U8G2_ESP32_HAL_UNDEFINED, \
    U8G2_ESP32_HAL_UNDEFINED, \
    U8G2_ESP32_HAL_UNDEFINED, \
    U8G2_ESP32_HAL_UNDEFINED, \
    U8G2_ESP32_HAL_UNDEFINED, \
    U8G2_ESP32_HAL_UNDEFINED, \
    U8G2_ESP32_HAL_UNDEFINED, \
    U8G2_ESP32_HAL_UNDEFINED, \
    U8G2_ESP32_HAL_UNDEFINED \
    }

void u8g2_esp32_hal_init(u8g2_esp32_hal_t u8g2_esp32_hal_param);
uint8_t u8g2_esp32_spi_byte_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8g2_esp32_i2c_byte_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8g2_esp32_gpio_and_delay_cb(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
#ifdef __cplusplus
}
#endif

#endif /* U8G2_ESP32_HAL_H_ */
