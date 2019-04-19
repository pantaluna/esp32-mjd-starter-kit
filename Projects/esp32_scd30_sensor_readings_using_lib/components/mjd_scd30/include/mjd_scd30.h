/*
 * Component header file.
 */
#ifndef __MJD_SCD30_H__
#define __MJD_SCD30_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Device specifics (registers, commands, etc.)
 */
#include "mjd_scd30_defs.h"

/**
 * I2C Settings
 *
 * - Support I2C slaves that use clock-stretching
 *     @doc APB_CLK 80Mhz 80 MHz = 12.5 nanosec per tick
 *            default:   32000  =>  0.4 millisec
 *            test:     128000      1.6 millisec
 *            maxval:  1048575     13.1 millisec
 *
 * @doc Maximal I2C speed is 100 kHz.
 * @doc SCD30 does not support I2C repeated starts!
 * @doc The I2C master must support clock stretching.
 *      Clock stretching period in write- and read frames is 12 ms, however,
 *      due to internal calibration processes a maximal clock stretching of 150 ms may occur once per day!
 */
#define MJD_SCD30_I2C_ADDRESS_DEFAULT           (0x61)       /*!< Cannot be changed... */
#define MJD_SCD30_I2C_MASTER_NUM_DEFAULT        (I2C_NUM_0)  /*!< The default I2C_NUM_0 can be changed. */
#define MJD_SCD30_I2C_MASTER_FREQ_HZ            (100 * 1000) /*!< Default 100 * 1000 [ESP32 Max 1 MHz.] Use 10Khz for long wires (>25cm). I2C master clock freq: Normal (100 KHz), FastMode (400 Khz), FastModePlus (1 Mhz). */
#define MJD_SCD30_I2C_MASTER_RX_BUF_DISABLE     (0)          /*!< I2C master does not need RX buffer. This param is for I2C slaves. */
#define MJD_SCD30_I2C_MASTER_TX_BUF_DISABLE     (0)          /*!< I2C master does not need TX buffer. This param is for I2C slaves. */
#define MJD_SCD30_I2C_MASTER_INTR_FLAG_NONE     (0)
#define MJD_SCD30_I2C_MAX_TICKS_TO_WAIT_DEFAULT (1000 / portTICK_PERIOD_MS) /*!< 1 sec */
#define MJD_SCD30_I2C_SLAVE_TIMEOUT_MAXVAL      (1048575)    /*!< I2C clock-stretching. See docs above. */

#define MJD_SCD30_DELAY_MS_AFTER_I2C_WRITE (30) /*!< See _i2c_write_bytes(). All I2C Write commands must be suffixed with a delay of MINIMUM *30* millisec (Sensirion sources). */

#define MJD_SCD30_CMD_TX_BUF_SIZE (2) /*!< I2C Commands Write: the number of bytes to send for a command (without extra data*/

/**
 * CUSTOM default device settings - see mjd_scd30_init()
 *
 */
#define MJD_SCD30_MEASUREMENT_INTERVAL_DEFAULT  (5) /*!< Seconds. MJD_SCD30_MEASUREMENT_INTERVAL_MIN = 2, MJD_SCD30_MEASUREMENT_INTERVAL_MAX = 1800 */
#define MJD_SCD30_TEMPERATURE_OFFSET_DEFAULT    (100) /*!< MJD_SCD30_TEMPERATURE_OFFSET_MIN = 0 */
#define MJD_SCD30_ALTITUDE_COMPENSATION_DEFAULT (10) /*!< MJD_SCD30_TEMPERATURE_OFFSET_MIN = 0 */

/**
 * TODO Use the RDY pin of the SCD30 (optional).
 *
 */

/**
 * Data structs
 *
 */

/*****
 * Classification: EU IDA Levels for indoor air quality
 *
 * ...
 *
 */
typedef enum {
    MJD_SCD30_EU_IDA_CATEGORY_1 = 1,
    MJD_SCD30_EU_IDA_CATEGORY_2 = 2,
    MJD_SCD30_EU_IDA_CATEGORY_3 = 3,
    MJD_SCD30_EU_IDA_CATEGORY_4 = 4,
} mjd_scd30_eu_ida_category_t;

enum {
    MJD_SCD30_EU_IDA_CATEGORY_CODE_MAXLEN = 8,
    MJD_SCD30_EU_IDA_CATEGORY_DESC_MAXLEN = 32,
};

mjd_scd30_eu_ida_category_t mjd_scd30_compute_eu_ida_category(float param_co2_ppm);
esp_err_t mjd_scd30_get_ida_category_details(mjd_scd30_eu_ida_category_t param_category, char param_ptr_code[], char param_ptr_desc[]);

/*****
 * mjd_scd30_config_t
 *   param int_gpio_num         : INT DRDY Data Ready pin @rule -1 means not used to detect that a measurement is ready to be read.
 *   param i2c_max_ticks_to_wait: Max ticks to wait for for i2c_master_cmd_begin() *Nothing to do with I2C clockstretching.*
 */
typedef struct {
        bool manage_i2c_driver;
        i2c_port_t i2c_port_num;
        uint8_t i2c_slave_addr;
        gpio_num_t i2c_scl_gpio_num;
        gpio_num_t i2c_sda_gpio_num;
        int i2c_max_ticks_to_wait;

        uint16_t measurement_interval;
        uint16_t temperature_offset;
        uint16_t altitude_compensation;
} mjd_scd30_config_t;

#define MJD_SCD30_CONFIG_DEFAULT() { \
    .manage_i2c_driver = true, \
    .i2c_port_num = MJD_SCD30_I2C_MASTER_NUM_DEFAULT, \
    .i2c_slave_addr = MJD_SCD30_I2C_ADDRESS_DEFAULT, \
    .i2c_scl_gpio_num = -1, \
    .i2c_sda_gpio_num = -1, \
    .i2c_max_ticks_to_wait = MJD_SCD30_I2C_MAX_TICKS_TO_WAIT_DEFAULT, \
    .measurement_interval = MJD_SCD30_MEASUREMENT_INTERVAL_DEFAULT, \
    .temperature_offset = MJD_SCD30_TEMPERATURE_OFFSET_DEFAULT, \
    .altitude_compensation = MJD_SCD30_ALTITUDE_COMPENSATION_DEFAULT \
};

/*****
 * mjd_scd30_data_t
 */
typedef struct {
        uint16_t measurement_interval;
        uint16_t temperature_offset;

        uint16_t raw_data[6]; // @important unsigned!
        float co2_ppm;
        float temperature_celsius;
        float temperature_fahrenheit;
        float relative_humidity;
        float dew_point_celsius;
        float dew_point_fahrenheit;

        mjd_scd30_eu_ida_category_t eu_ida_category;
        char eu_ida_category_code[MJD_SCD30_EU_IDA_CATEGORY_CODE_MAXLEN];
        char eu_ida_category_desc[MJD_SCD30_EU_IDA_CATEGORY_DESC_MAXLEN];
} mjd_scd30_data_t;

/*****
 * Function declarations
 */
esp_err_t mjd_scd30_cmd_get_measurement_interval(const mjd_scd30_config_t* param_ptr_config, uint16_t* param_data); // uint16!
esp_err_t mjd_scd30_cmd_get_data_ready_status(const mjd_scd30_config_t* param_ptr_config,
                                              mjd_scd30_data_ready_status_t* param_data);
esp_err_t mjd_scd30_cmd_read_measurement(const mjd_scd30_config_t* param_ptr_config, mjd_scd30_data_t* param_ptr_data);
esp_err_t mjd_scd30_cmd_set_automatic_self_calibration(const mjd_scd30_config_t* param_ptr_config,
                                                       const mjd_scd30_asc_automatic_self_calibration_t param_data);
esp_err_t mjd_scd30_cmd_get_automatic_self_calibration(const mjd_scd30_config_t* param_ptr_config,
                                                       mjd_scd30_asc_automatic_self_calibration_t* param_data);
esp_err_t mjd_scd30_cmd_set_forced_recalibration_value(const mjd_scd30_config_t* param_ptr_config, int16_t param_data); // int16!
esp_err_t mjd_scd30_cmd_get_forced_recalibration_value(const mjd_scd30_config_t* param_ptr_config, uint16_t* param_data);
esp_err_t mjd_scd30_cmd_set_temperature_offset(const mjd_scd30_config_t* param_ptr_config, int16_t param_data); // int16!
esp_err_t mjd_scd30_cmd_get_temperature_offset(const mjd_scd30_config_t* param_ptr_config, uint16_t* param_data); // uint16!
esp_err_t mjd_scd30_cmd_set_altitude_compensation(const mjd_scd30_config_t* param_ptr_config, int16_t param_data); // int16!
esp_err_t mjd_scd30_cmd_get_altitude_compensation(const mjd_scd30_config_t* param_ptr_config, uint16_t* param_data); // uint16!
esp_err_t mjd_scd30_cmd_get_firmware_version(const mjd_scd30_config_t* param_ptr_config, char * param_ptr_data);
esp_err_t mjd_scd30_cmd_soft_reset(const mjd_scd30_config_t* param_ptr_config);

esp_err_t mjd_scd30_log_device_parameters(mjd_scd30_config_t* param_ptr_config);

esp_err_t mjd_scd30_cmd_trigger_continuous_measurement(const mjd_scd30_config_t* param_ptr_config, int16_t param_data); // int16!
esp_err_t mjd_scd30_cmd_stop_continuous_measurement(const mjd_scd30_config_t* param_ptr_config);
esp_err_t mjd_scd30_cmd_set_measurement_interval(const mjd_scd30_config_t* param_ptr_config, int16_t param_data); // int16!

esp_err_t mjd_scd30_init(mjd_scd30_config_t* param_ptr_config);
esp_err_t mjd_scd30_deinit(const mjd_scd30_config_t* param_ptr_config);

#ifdef __cplusplus
}
#endif

#endif /* __MJD_SCD30_H__ */
