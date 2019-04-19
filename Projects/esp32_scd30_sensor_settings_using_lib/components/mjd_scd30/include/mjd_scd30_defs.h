/**
 * Contains the declarations for this device (especially the registers, NVRAM and commands).
 */
#ifndef __MJD_SCD30_DEFS_H__
#define __MJD_SCD30_DEFS_H__

#ifdef __cplusplus
extern "C" {
#endif

/*****
 * Device: Registers
 *
 * N.R. The SCD30 has no generic status register. You can get various status information using the Commands.
 *
 */

/*****
 * Device: Commands
 *
 * @important After sending a command to the sensor a minimal waiting time of 1ms is needed.
 *
 */
typedef enum {
    MJD_SCD30_CMD_GET_DATA_READY_STATUS = 0x0202,
    MJD_SCD30_CMD_READ_MEASUREMENT = 0x0300,
    MJD_SCD30_CMD_AUTOMATIC_SELF_CALIBRATION = 0x5306,
    MJD_SCD30_CMD_FORCED_RECALIBRATION_VALUE = 0x5204,
    MJD_SCD30_CMD_TEMPERATURE_OFFSET = 0x5403,
    MJD_SCD30_CMD_ALTITUDE_COMPENSATION = 0x5102,
    MJD_SCD30_CMD_GET_FIRMWARE_VERSION = 0xD100,
    MJD_SCD30_CMD_SOFT_RESET = 0xD304,
    MJD_SCD30_CMD_TRIGGER_CONTINUOUS_MEASUREMENT = 0x0010,
    MJD_SCD30_CMD_STOP_CONTINUOUS_MEASUREMENT = 0x0104,
    MJD_SCD30_CMD_MEASUREMENT_INTERVAL = 0x4600,
} mjd_scd30_command_t;

/*****
 * Classification: known CO2 ppm levels
 *
 * For setting the FRC whiolst the device is in fresh air, outdoor.
 *
 */
enum {
    MJD_SCD30_CO2_PPM_OUTDOOR_FRESH_AIR = 400,
};

/*****
 * Classification: ambient pressure compensation range (optional)
 *
 * for CMD Trigger continuous measurement
 *
 * PS The older docs mentions max 1400 mBar. The latest docs mention max 1200 mBar.
 *
 */
enum {
    MJD_SCD30_AMBIENT_PRESSURE_DISABLED = 0,
    MJD_SCD30_AMBIENT_PRESSURE_MIN = 700,
    MJD_SCD30_AMBIENT_PRESSURE_MAX = 1400,
};

/*****
 * Classification: measurement interval range
 *
 * ...
 *
 */
enum {
    MJD_SCD30_MEASUREMENT_INTERVAL_MIN = 2,
    MJD_SCD30_MEASUREMENT_INTERVAL_MAX = 1800,
};

/*****
 * Classification: data ready status
 *
 * ...
 *
 */
typedef enum {
    MJD_SCD30_DATA_READY_STATUS_NO = 0,
    MJD_SCD30_DATA_READY_STATUS_YES = 1,
} mjd_scd30_data_ready_status_t;

/*****
 * Classification: ASC Automatic Self Calibration
 *
 * ...
 *
 */
typedef enum {
    MJD_SCD30_ASC_AUTOMATIC_SELF_CALIBRATION_NO = 0,
    MJD_SCD30_ASC_AUTOMATIC_SELF_CALIBRATION_YES = 1,
} mjd_scd30_asc_automatic_self_calibration_t;

/*****
 * Classification: Forced Recalibration range
 *
 * The reference CO2 concentration has to be within this range.
 *
 */
enum {
    MJD_SCD30_FORCED_RECALIBRATION_MIN = 400,
    MJD_SCD30_FORCED_RECALIBRATION_MAX = 2000,
};

/*****
 * Classification: temperature_offset minimum
 *
 */
enum {
    MJD_SCD30_TEMPERATURE_OFFSET_MIN = 0
};

/*****
 * Classification: altitude minimum
 *
 */
enum {
    MJD_SCD30_ALTITUDE_MIN = 0
};

/*****
 * Device: Parameters - Register, Bit mask, Bit right shift
 *
 * N.R. The SCD30 has no generic status register. You can get various status information using the Commands.
 *
 */

#ifdef __cplusplus
}
#endif

#endif /* __MJD_SCD30_DEFS_H__ */
