/*
 * Goto README.md for instructions
 */

// Component header file(s)
#include "mjd.h"
#include "mjd_huzzah32.h"

/*
 * Logging
 */
static const char TAG[] = "mjd_huzzah32";

/*
 * Data definitions
 */

/**************************************
 * PRIVATE.
 *
 */
static void _log_adc_calibration_value_type(esp_adc_cal_value_t val_type) {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        ESP_LOGD(TAG, "  Calibration value characterized using Two Point Value");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        ESP_LOGD(TAG, "  Calibration value characterized using eFuse Vref");
    } else {
        ESP_LOGD(TAG, "  Calibration value characterized using manually supplied Default Vref");
    }
}

/**************************************
 * PUBLIC.
 *
 */
float mjd_huzzah32_get_battery_voltage() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    /**************************************************************************
     * Reuseable variables
     */
    float battery_voltage_float = 0;

    // FUNC
    const uint32_t huzzah32_adc_voltage_reference = CONFIG_MJD_HUZZAH32_REFERENCE_VOLTAGE_MV; // @unit millivolt. Reference Voltage = +-1100mV. This value is specific for this board; it was measured using the func mjd_huzzah32_route_vref_to_gpio26() [see documentation for instructions]
    const adc_bits_width_t adc_width = ADC_WIDTH_BIT_10;    // BIT_10=0..1023
    const adc1_channel_t adc_channel = ADC1_GPIO35_CHANNEL; // ADC1_GPIO35_CHANNEL ADC1_CHANNEL_7_GPIO_NUM
    const adc_atten_t adc_atten = ADC_ATTEN_DB_11;

    const float factor_correction_for_disabled_voltage_regulator = 1.20;

    // Configure ADC1
    adc1_config_width(adc_width);
    adc1_config_channel_atten(adc_channel, adc_atten);

    // Characterize ADC1
    esp_adc_cal_characteristics_t adc_cal_characteristics =
        { 0 };
    esp_adc_cal_value_t calibration_value_type = esp_adc_cal_characterize(ADC_UNIT_1, adc_atten, adc_width,
            huzzah32_adc_voltage_reference, &adc_cal_characteristics);
    _log_adc_calibration_value_type(calibration_value_type);

    // Logic:
    // - ADC read 64 samples (an attempt to reduce the impact of noise).
    // - Deduct actual battery voltage from the ADC reading.
    // - Double the mV value per the circuit design for getting the HUZZAH32 battery voltage.
    // - If the Voltage Regulator is disabled Then apply a correction factor.
    uint32_t voltage_mv = 0;
    uint32_t one_voltage_read = 0;
    const uint32_t NBR_OF_SAMPLES = 64;
    for (int i = 0; i < NBR_OF_SAMPLES; i++) {
        if (esp_adc_cal_get_voltage(adc_channel, &adc_cal_characteristics, &one_voltage_read) != ESP_OK) {
            ESP_LOGE(TAG, "error returned from esp_adc_cal_get_voltage()");
            battery_voltage_float = -1;
            // GOTO
            goto cleanup;
        }
        voltage_mv += one_voltage_read;
    }
    voltage_mv /= NBR_OF_SAMPLES;
    voltage_mv *= 2;
    ESP_LOGD(TAG, "voltage_mv (*2): %u", voltage_mv);

    battery_voltage_float = voltage_mv / 1000.0;
    if (CONFIG_MJD_HUZZAH32_VOLTAGE_REGULATOR_ENABLED == 0) {
        battery_voltage_float *= factor_correction_for_disabled_voltage_regulator;
        ESP_LOGD(TAG, "battery_voltage_float corrected for the disabled Voltage Regulator (*%f)", factor_correction_for_disabled_voltage_regulator);
    }
    ESP_LOGD(TAG, "battery_voltage_float: %f", battery_voltage_float);

    // LABEL
    cleanup: ;

    // RETURN
    return battery_voltage_float;
}

void mjd_huzzah32_log_adc_efuses() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    ESP_LOGI(TAG, "REPORT: ADC eFuses availability");
    ESP_LOGI(TAG,
            "  @doc If both 'eFuse Two Point' and 'eFuse Vref' are not supported then the ADC Calibration logic will use the Voltage Reference that you provided manually.");

    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        ESP_LOGI(TAG, "  eFuse Two Point: supported");
    } else {
        ESP_LOGI(TAG, "  eFuse Two Point: NOT supported");
    }

    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
        ESP_LOGI(TAG, "  eFuse Vref:      supported");
    } else {
        ESP_LOGI(TAG, "  eFuse Vref:      NOT supported");
    }
}

esp_err_t mjd_huzzah32_route_vref_to_gpio() {
    ESP_LOGD(TAG, "%s()", __FUNCTION__);

    /**************************************************************************
     * Reuseable variables
     */
    esp_err_t f_retval = ESP_OK;

    /**************************************************************************
     * Main
     */
    uint8_t gpio_ref = CONFIG_MJD_HUZZAH32_ROUTE_VREF_TO_GPIO_NUM;

    f_retval = adc2_vref_to_gpio(gpio_ref);
    if (f_retval != ESP_OK) {
        ESP_LOGE(TAG, "ABORT. Cannot route vref to GPIO %u", gpio_ref);
        // GOTO
        goto cleanup;
    }

    /**************************************************************************
     * LABEL cleanup
     */
    cleanup: ;

    // RETURN
    return f_retval;
}
