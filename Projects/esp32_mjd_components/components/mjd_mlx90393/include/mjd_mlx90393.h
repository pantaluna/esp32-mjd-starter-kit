/**
 * Component header file.
 */
#ifndef __MJD_MLX90393_H__
#define __MJD_MLX90393_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Device specifics (registers, NVRAM, etc.)
 */
#include "mjd_mlx90393_defs.h"

/**
 * I2C Settings
 */
#define MJD_MLX90393_I2C_MASTER_FREQ_HZ         (400 * 1000)  /*!< [ESP32 Max 1MHz] I2C master clock frequency Normal (100KHz) XOR Fast Mode (400 Khz) */
#define MJD_MLX90393_I2C_MASTER_RX_BUF_DISABLE  (0)           /*!< For slaves. I2C master does not need RX buffer */
#define MJD_MLX90393_I2C_MASTER_TX_BUF_DISABLE  (0)           /*!< For slaves. I2C master does not need TX buffer */
#define MJD_MLX90393_I2C_MASTER_INTR_FLAG_NONE  (0)
#define MJD_MLX90393_I2C_TIMEOUT_DEFAULT        (1000 / portTICK_PERIOD_MS)

/**
 * TIMER SETTINGS
 */
#define MJD_MLX90393_TIMER_GROUP_ID   (TIMER_GROUP_0)
#define MJD_MLX90393_TIMER_ID     (TIMER_0)

/**
 * My default MLX device settings - see mjd_mlx90393_init() - recommended settings from the DATA SHEET.
 */
#define MJD_MLX90393_COMM_MODE_DEFAULT  (MJD_MLX90393_COMM_MODE_I2C)
#define MJD_MLX90393_TCMP_EN_DEFAULT    (MJD_MLX90393_TCMP_EN_DISABLED)
#define MJD_MLX90393_HALLCONF_DEFAULT   (MJD_MLX90393_HALLCONF_C)

/////#define MJD_MLX90393_GAIN_SEL_DEFAULT   (MJD_MLX90393_GAIN_SEL_0)
#define MJD_MLX90393_GAIN_SEL_DEFAULT   (MJD_MLX90393_GAIN_SEL_7)

/////#define MJD_MLX90393_RES_XYZ_DEFAULT    (MJD_MLX90393_RES_XYZ_3)
#define MJD_MLX90393_RES_XYZ_DEFAULT    (MJD_MLX90393_RES_XYZ_0)

/////#define MJD_MLX90393_DIG_FILT_DEFAULT   (MJD_MLX90393_DIG_FILT_1)
#define MJD_MLX90393_DIG_FILT_DEFAULT   (MJD_MLX90393_DIG_FILT_7)

/////#define MJD_MLX90393_OSR_DEFAULT        (MJD_MLX90393_OSR_0)
#define MJD_MLX90393_OSR_DEFAULT        (MJD_MLX90393_OSR_3)

#define MJD_MLX90393_OFFSET_X_DEFAULT  (0)
#define MJD_MLX90393_OFFSET_Y_DEFAULT  (0)
#define MJD_MLX90393_OFFSET_Z_DEFAULT  (0)

/**
 * Data structs
 *
 */
typedef struct {
        bool temperature;
        bool x_axis;
        bool y_axis;
        bool z_axis;
} mjd_mlx90393_metrics_selector_t;

/*
 * mjd_mlx90393_config_t
 *      int_gpio_num : Melexis INT DRDY Data Ready pin @rule -1 means not used to detect that a measurement is ready to be read.
 */
typedef struct {
        bool manage_i2c_driver;
        i2c_port_t i2c_port_num;
        uint8_t i2c_slave_addr;
        gpio_num_t i2c_scl_gpio_num;
        gpio_num_t i2c_sda_gpio_num;
        gpio_num_t int_gpio_num;

        mjd_mlx90393_metrics_selector_t mlx_metrics_selector;

        uint16_t mlx_sens_tc_lt;
        uint16_t mlx_sens_tc_ht;
        uint16_t mlx_tref;

        mjd_mlx90393_comm_mode_t mlx_comm_mode;
        mjd_mlx90393_tcmp_en_t mlx_tcmp_en;
        mjd_mlx90393_hallconf_t mlx_hallconf;
        mjd_mlx90393_gain_sel_t mlx_gain_sel;
        mjd_mlx90393_osr_t mlx_osr;
        mjd_mlx90393_dig_filt_t mlx_dig_filt;
        mjd_mlx90393_res_xyz_t mlx_res_x, mlx_res_y, mlx_res_z;
        uint16_t mlx_offset_x, mlx_offset_y, mlx_offset_z;
} mjd_mlx90393_config_t;

/*
 */
#define MJD_MLX90393_CONFIG_DEFAULT() { \
    .manage_i2c_driver = true, \
    .i2c_port_num = I2C_NUM_0, \
    .i2c_slave_addr = 0x0C, \
    .i2c_scl_gpio_num = -1, \
    .i2c_sda_gpio_num = -1, \
    .int_gpio_num = -1, \
    .mlx_metrics_selector = { \
            .x_axis = true, \
            .y_axis = true, \
            .z_axis = true, \
            .temperature = true \
    } \
};

typedef struct {
        uint16_t t;
        uint16_t x;
        uint16_t y;
        uint16_t z;
} mjd_mlx90393_data_raw_t;

typedef struct {
        uint16_t t_raw;
        uint16_t x_raw;
        uint16_t y_raw;
        uint16_t z_raw;
        float t;
        float x;
        float y;
        float z;
} mjd_mlx90393_data_t;

/**
 * Function declarations
 */
esp_err_t mjd_mlx90393_init(mjd_mlx90393_config_t* param_ptr_config);
esp_err_t mjd_mlx90393_deinit(const mjd_mlx90393_config_t* param_ptr_config);

esp_err_t mjd_mlx90393_log_device_parameters(const mjd_mlx90393_config_t* param_ptr_config);

esp_err_t mjd_mlx90393_cmd_reset(const mjd_mlx90393_config_t* param_ptr_config);
esp_err_t mjd_mlx90393_cmd_exit(const mjd_mlx90393_config_t* param_ptr_config);

esp_err_t mjd_mlx90393_get_comm_mode(const mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_comm_mode_t* param_ptr_data);
esp_err_t mjd_mlx90393_get_tcmp_en(const mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_tcmp_en_t* param_ptr_data);
esp_err_t mjd_mlx90393_get_hallconf(const mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_hallconf_t* param_ptr_data);
esp_err_t mjd_mlx90393_get_gain_sel(const mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_gain_sel_t* param_ptr_data);
esp_err_t mjd_mlx90393_get_z_series(const mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_z_series_t* param_ptr_data);
esp_err_t mjd_mlx90393_get_bist(const mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_bist_t* param_ptr_data);
esp_err_t mjd_mlx90393_get_ext_trig(const mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_ext_trig_t* param_ptr_data);
esp_err_t mjd_mlx90393_get_trig_int_sel(const mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_trig_int_sel_t* param_ptr_data);
esp_err_t mjd_mlx90393_get_osr(const mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_osr_t* param_ptr_data);
esp_err_t mjd_mlx90393_get_dig_filt(const mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_dig_filt_t* param_ptr_data);
esp_err_t mjd_mlx90393_get_res_xyz(const mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_res_xyz_t* param_x, mjd_mlx90393_res_xyz_t* param_y,
                                   mjd_mlx90393_res_xyz_t* param_z);
esp_err_t mjd_mlx90393_get_sens_tc_lt(const mjd_mlx90393_config_t* param_ptr_config, uint8_t* param_ptr_data);
esp_err_t mjd_mlx90393_get_sens_tc_ht(const mjd_mlx90393_config_t* param_ptr_config, uint8_t* param_ptr_data);
esp_err_t mjd_mlx90393_get_offset_xyz(const mjd_mlx90393_config_t* param_ptr_config, uint16_t* param_x, uint16_t* param_y, uint16_t* param_z);
esp_err_t mjd_mlx90393_get_tref(const mjd_mlx90393_config_t* param_ptr_config, uint16_t* param_ptr_data);

esp_err_t mjd_mlx90393_set_comm_mode(mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_comm_mode_t param_data);
esp_err_t mjd_mlx90393_set_tcmp_en(mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_tcmp_en_t param_data);
esp_err_t mjd_mlx90393_set_hallconf(mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_hallconf_t param_data);
esp_err_t mjd_mlx90393_set_gain_sel(mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_gain_sel_t param_data);
esp_err_t mjd_mlx90393_set_osr(mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_osr_t param_data);
esp_err_t mjd_mlx90393_set_dig_filt(mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_dig_filt_t param_data);
esp_err_t mjd_mlx90393_set_res_xyz(mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_res_xyz_t param_res_x, mjd_mlx90393_res_xyz_t param_res_y,
                                   mjd_mlx90393_res_xyz_t param_res_z);
esp_err_t mjd_mlx90393_set_offset_xyz(mjd_mlx90393_config_t* param_ptr_config, uint16_t param_offset_x, uint16_t param_offset_y,
                                      uint16_t param_offset_z);
esp_err_t mjd_mlx90393_cmd_start_measurement(const mjd_mlx90393_config_t* param_ptr_config);
esp_err_t mjd_mlx90393_cmd_read_measurement(const mjd_mlx90393_config_t* param_ptr_config, mjd_mlx90393_data_t* param_ptr_data);

#ifdef __cplusplus
}
#endif

#endif /* __MJD_MLX90393_H__ */
