/*
 * Goto the README.md for instructions
 *
 */
#ifndef __MJD_JSNSR04T_H__
#define __MJD_JSNSR04T_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Includes: system, own
 */
#include "mjd.h"

/*
 * Sensor settings
 */

/**
 * Data structs
 *
 */

/*
 * CONFIG
 *
 *   @doc https://statisticsbyjim.com/basics/variability-range-interquartile-variance-standard-deviation/
 *
 */
typedef struct {
        bool is_init; /*!< Has this component instance been initialized? */
        gpio_num_t trigger_gpio_num; /*!< The GPIO# that is wired up to the TRIGGER pin of the sensor */
        gpio_num_t echo_gpio_num; /*!< The GPIO# that is wired up to the ECHO pin of the sensor */
        rmt_channel_t rmt_channel; /*!< Which RMT Channel number to use (handy when using multiple components that use the RMT peripheral */
        double distance_sensor_to_artifact_cm; /*!< The distance (cm) between the sensor and the artifact to be monitored. Typical for a stilling well setup. This distance is subtracted from the actual measurement. Also used to circumvent the dead measurement zone of the sensor (+-25cm). */
        uint32_t nbr_of_errors; /*!< Runtime Statistics: the total number of errors when interacting with the sensor */
        uint32_t nbr_of_samples; /*!< How many samples to read to come to one weighted measurement? */
        double max_range_allowed_in_samples_cm; /*<! Reject a set of measurements if the range is higher than this prop. Statistics Dispersion Range outlier detection method */
} mjd_jsnsr04t_config_t;

#define MJD_JSNSR04T_CONFIG_DEFAULT() { \
    .is_init = false, \
    .trigger_gpio_num = GPIO_NUM_MAX, \
    .echo_gpio_num = GPIO_NUM_MAX, \
    .rmt_channel = RMT_CHANNEL_MAX, \
    \
    .distance_sensor_to_artifact_cm = 0.0, \
    .nbr_of_errors = 0, \
    .nbr_of_samples = 5, \
    .max_range_allowed_in_samples_cm = 10.0, \
}

/*
 * DATA
 */
typedef struct {
        bool data_received; /*!< Has data been received from the device? */
        bool is_an_error; /*!< Is the data an error? */
        double distance_cm; /*!< The measured distance. The distance_sensor_to_artifact_cm is already subtracted from the original measured distance. */
} mjd_jsnsr04t_data_t;

#define MJD_JSNSR04T_DATA_DEFAULT() { \
    .data_received = false, \
    .is_an_error = false, \
    .distance_cm = 0.0, \
}

/**
 * Function declarations
 */
esp_err_t mjd_jsnsr04t_log_config(mjd_jsnsr04t_config_t param_config);
esp_err_t mjd_jsnsr04t_log_data(mjd_jsnsr04t_data_t param_data);
esp_err_t mjd_jsnsr04t_init(mjd_jsnsr04t_config_t* param_ptr_config);
esp_err_t mjd_jsnsr04t_deinit(mjd_jsnsr04t_config_t* param_ptr_config);
esp_err_t mjd_jsnsr04t_get_measurement(mjd_jsnsr04t_config_t* param_ptr_config, mjd_jsnsr04t_data_t* param_ptr_data);

#ifdef __cplusplus
}
#endif

#endif /* __MJD_JSNSR04T_H__ */
