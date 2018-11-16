/*
 * Goto README.md for instructions
 */
#ifndef __MJD_HUZZAH32_H__
#define __MJD_HUZZAH32_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "driver/adc.h"
#include "esp_adc_cal.h"

/**
 * Data Definitions
 */

/**
 * Function declarations
 */

/**
 * @brief Return the actual battery voltage level using ADC1. This func is specific for the Adafruit HUZZAH32 development board.
 *
 * @dependency Kconfig CONFIG_MJD_HUZZAH32_REFERENCE_VOLTAGE_MV (default 1100)
 *      The huzzah32_adc_voltage_reference (1100) is specific for this board when battery powered.
 *      It was measured using the func mjd_huzzah32_route_vref_to_gpio26() and stored in Kconfig.
 *
 * @dependency Kconfig CONFIG_MJD_HUZZAH32_VOLTAGE_REGULATOR_ENABLED [0,1](default 1=yes)
 *      Is the Voltage Regulator enabled (default)? Without extra wiring the VR is enabled.
 *      This impacts the calculation of the battery voltage.
 *          If enabled Then no extra calculation is needed.
 *          If disabled Then the func multiplies the computed battery voltage with a correction factor to obtain the correct battery voltage level.
 *      Disabling the VR can be done by wiring the pin "EN" to "GND" to conserve power when in LiFePO4 battery mode.
 *      Warning: only disable the VR when using 3.3V batteries such as LifePO4 ones
 *               that are connected to the pin "3.3V" (not to the LiPO battery connector).
 *
 *     Disabling the VR can be done by wiring the pin "EN" to "GND". The "EN" pin is located between the "BAT" and the "USB" pins.
 *     Warning: only disable the VR when using 3.3V batteries such as LifePO4 ones
 *              that are connected to the pin "3.3V" (not to the LiPO battery connector because that combination will not work -> error "Brownout detected").
 *
 * @important The battery voltage level reading is INVALID when powered by USB and no battery is connected (it will show a positive voltage of +-2*2.1V, not the expected 0V!).
 *
 * @logic The GPIO#35 (VBAT SENSE) outputs the battery voltage measurement (halved value). This pin is not exposed on the HUZZAH32 board.
 *
 * ADC_WIDTH_BIT_10 gives more accurate results, compared to ADC_WIDTH_BIT_11 and ADC_WIDTH_BIT_12. I have no idea why.
 *
 * Due to ADC characteristics, most accurate results are obtained within the following approximate voltage ranges:
 *      - 11dB attenuation (ADC_ATTEN_DB_11) voltages between 150 to 2450mV.
 *
 * Calibration Characterization is based on the features of the ESP32 chip. Features:
 *   a) The Reference voltage stored in eFuse.
 *   b) The Two Point values stored in eFuse.
 *   c) The default reference voltage that was supplied manually.
 * The HUZZAH32 I'm using has no eFuses for that.
 *
 * Soshine 18650 LiFePO4 3.2V Battery 1800mAh specs:
 * - Operating voltage         3.2V (halved = 1.6V).
 * - Minimum discharge voltage 2.8V (halved = 1.4V).
 * - Maximum charged voltage   3.6V (halved = 1.8V).
 *
 * @return float battery voltage level (unit=Volt)
 */
float mjd_huzzah32_get_battery_voltage();

/**
 * @brief Report the features of the current ESP32 board about the ADC Calibration Characterization. Figure out if the ESP32 eFuse contains specific information for that.
 *
 * The following information might be stored in eFuse:
 *   - "TP": characterization based on Two Point values.
 *   - "VREF": characterization based on reference voltage.
 *
 * https://esp-idf.readthedocs.io/en/latest/api-reference/peripherals/adc.html#
 *
 * @return
 */
void mjd_huzzah32_log_adc_efuses();

/**
 * @brief Route the actual VREF Voltage Reference of the ESP32 to GPIO#26 using ADC2. Then use a multimeter to determine the voltage. The value will be around 1100 (mV).
 *
 * @dependency Kconfig CONFIG_MJD_HUZZAH32_ROUTE_VREF_TO_GPIO_NUM
 *      int "Huzzah32 Route the Voltage Reference to this GPIO NUM (default 26)"
 *
 * GPIO#26 is the 5th pin down from the top left of the HUZZAH32 dev board.

 * @important This code is not needed to measuring the actual BATTERY voltage (we just need to measure the VREF value once). This logic will consume extra power (driving the output pin all the time).
 *
 * @important Do not enable Wifi or Bluetooth when running this func; conflict using ADC2.
 *
 * The VREF for the HUZZAH32 that I'm using right now is stored in Kconfig CONFIG_MJD_HUZZAH32_REFERENCE_VOLTAGE_MV and it is used in the func mjd_huzzah32_get_battery_voltage().
 *
 * @return
 *     - ESP_OK Success
 */
esp_err_t mjd_huzzah32_route_vref_to_gpio();

#ifdef __cplusplus
}
#endif

#endif /* __MJD_HUZZAH32_H__ */
