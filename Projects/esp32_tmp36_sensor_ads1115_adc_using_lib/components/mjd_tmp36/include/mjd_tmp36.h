/*
 * Component header file.
 */
#ifndef __MJD_TMP36_H__
#define __MJD_TMP36_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Data structs
 *
 */

/*
 * mjd_tmp36_data_t
 */
typedef struct {
        float in_volts;
        float out_degrees_celsius;
} mjd_tmp36_data_t;

/**
 * Function declarations
 */
esp_err_t mjd_tmp36_log_config();

esp_err_t mjd_tmp36_convert_volts_to_degrees_celsius(mjd_tmp36_data_t* param_ptr_data);

#ifdef __cplusplus
}
#endif

#endif /* __MJD_TMP36_H__ */
