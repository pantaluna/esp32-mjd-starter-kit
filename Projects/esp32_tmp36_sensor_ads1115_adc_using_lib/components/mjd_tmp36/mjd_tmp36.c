/*
 * Component main file.
 */
#include "driver/timer.h"

// Component header file(s)
#include "mjd.h"
#include "mjd_tmp36.h"

/*
 * Logging
 */
static const char TAG[] = "mjd_tmp36";

/*
 * TMP36 Static Device Params (private for this component)
 *   Offset: 500 milliVolts
 *   Scale:   10 milliVolts / °C
 *   => Formula: Temperature in °C = [(Vout in mV) - 500mV] / 10mV
 *
 */
static const float _offset_volts = 0.5;
static const float _scale_degrees_celsius_per_volt = 0.01;



/*
 * MAIN
 */

/*********************************************************************************
 * mjd_tmp36_log_config()
 *
 * @important Use ESP_LOGI() in this public func - not ESP_LOGD()!
 *
 *********************************************************************************/
esp_err_t mjd_tmp36_log_config() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    ESP_LOGI(TAG, "TMP36 Analog Temperature Sensor: config:");
    ESP_LOGI(TAG, "  _offset_volts:                   %f", _offset_volts);
    ESP_LOGI(TAG, "  _scale_degrees_celsius_per_volt: %f", _scale_degrees_celsius_per_volt);

    return f_retval;
}


/*********************************************************************************
 * Convert the voltage to Degrees Celsius
 *
 * TMP36 Offset: 500 milliVolts
 * TMP36 Scale:   10 milliVolts / °C
 * => Formula: Temperature in in °C = [(Vout in mV) - 500mV] / 10mV
 *
 *********************************************************************************/
esp_err_t mjd_tmp36_convert_volts_to_degrees_celsius(mjd_tmp36_data_t* param_ptr_data) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    esp_err_t f_retval = ESP_OK;

    /*****
     * Calculation
     */
    param_ptr_data->out_degrees_celsius = (param_ptr_data->in_volts - _offset_volts)
            / _scale_degrees_celsius_per_volt;

    return f_retval;
}
