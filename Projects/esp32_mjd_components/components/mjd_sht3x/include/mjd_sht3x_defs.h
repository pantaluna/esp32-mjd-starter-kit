/**
 * Contains the declarations for this device (especially the registers, NVRAM and commands).
 */
#ifndef __MJD_SHT3X_DEFS_H__
#define __MJD_SHT3X_DEFS_H__

#ifdef __cplusplus
extern "C" {
#endif

/*****
 * Device: Registers
 *
 *
 */
typedef enum {
    MJD_SHT3X_REG_STATUS = 0xF32D,
} mjd_sht3x_reg_t;

/*****
 * Classification: Repeatability
 *
 * The repeatability property influences the measurement duration, noise levels and the overall energy consumption of the sensor.
 *
 * - Measurement duration:
 *     High repeatability:   15 ms
 *     Medium repeatability:  6 ms
 *     Low repeatability:     4 ms
 * - I2C Clock Stretch
 *     This feature is not supported properly in the ESP32 I2C Driver so we will not use the Single Shot Modes with I2C Clock Stretch enabled...
 *
 * @default 0
 *
 */
typedef enum {
    MJD_SHT3X_REPEATABILITY_HIGH,
    MJD_SHT3X_REPEATABILITY_MEDIUM,
    MJD_SHT3X_REPEATABILITY_LOW,
    MJD_SHT3X_REPEATABILITY_MAX,
} mjd_sht3x_repeatability_t;

/*****
 * Device: Commands
 *
 * @important After sending a command to the sensor a minimal waiting time of 1ms is needed.
 *
 */
typedef enum {
    MJD_SHT3X_CMD_START_SINGLE_SHOT_MODE_REPEATABILITY_HIGH = 0x2C06,
    MJD_SHT3X_CMD_START_SINGLE_SHOT_MODE_REPEATABILITY_MEDIUM = 0x2C0D,
    MJD_SHT3X_CMD_START_SINGLE_SHOT_MODE_REPEATABILITY_LOW = 0x2C10,
    MJD_SHT3X_CMD_SOFT_RESET = 0x30A2,
    MJD_SHT3X_CMD_BREAK = 0x3093,
    MJD_SHT3X_CMD_CLEAR_STATUS_REGISTER = 0x3041,
} mjd_sht3x_command_t;

/*****
 * Device: Parameters - Register, Bit mask, Bit right shift
 *
 * Table 18 Description of the status register.
 *
 */

/*
 * b15 Alert pending status
 *  0: no pending alerts
 *  1: at least one pending alert
 *
 * @default 1
 *
 */
enum {
    MJD_SHT3X_ALERT_PENDING_STATUS_REG = MJD_SHT3X_REG_STATUS,
    MJD_SHT3X_ALERT_PENDING_STATUS_BITMASK = 0x8000,
    MJD_SHT3X_ALERT_PENDING_STATUS_BITSHIFT = 15
};
typedef enum {
    MJD_SHT3X_ALERT_PENDING_STATUS_NO = 0,
    MJD_SHT3X_ALERT_PENDING_STATUS_AT_LEAST_ONE = 1,
} mjd_sht3x_alert_pending_status_t;

/*
 * b14 Reserved
 *
 * @default 0
 *
 */

/*
 * b13 Heater status
 *  0 : Heater OFF
 *  1 : Heater ON
 *
 * @default 0
 *
 */
enum {
    MJD_SHT3X_HEATER_STATUS_REG = MJD_SHT3X_REG_STATUS,
    MJD_SHT3X_HEATER_STATUS_BITMASK = 0x2000,
    MJD_SHT3X_HEATER_STATUS_BITSHIFT = 13
};
typedef enum {
    MJD_SHT3X_HEATER_STATUS_OFF = 0,
    MJD_SHT3X_HEATER_STATUS_ON = 1,
} mjd_sht3x_heater_status_t;

/*
 * b12 Reserved
 *
 * @default 0
 *
 */

/*
 * b11 RH tracking alert
 *  0 : no alert
 *  1 : alert
 *
 * @default 0
 *
 */
enum {
    MJD_SHT3X_RH_TRACKING_ALERT_REG = MJD_SHT3X_REG_STATUS,
    MJD_SHT3X_RH_TRACKING_ALERT_BITMASK = 0x0800,
    MJD_SHT3X_RH_TRACKING_ALERT_BITSHIFT = 11
};
typedef enum {
    MJD_SHT3X_RH_TRACKING_ALERT_NO = 0,
    MJD_SHT3X_RH_TRACKING_ALERT_YES = 1,
} mjd_sht3x_rh_tracking_alert_t;

/*
 * b10 T tracking alert
 *  0 : no alert
 *  1 : alert
 *
 * @default 0
 *
 */
enum {
    MJD_SHT3X_T_TRACKING_ALERT_REG = MJD_SHT3X_REG_STATUS,
    MJD_SHT3X_T_TRACKING_ALERT_BITMASK = 0x0400,
    MJD_SHT3X_T_TRACKING_ALERT_BITSHIFT = 10
};
typedef enum {
    MJD_SHT3X_T_TRACKING_ALERT_NO = 0,
    MJD_SHT3X_T_TRACKING_ALERT_YES = 1,
} mjd_sht3x_t_tracking_alert_t;

/*
 * b9:5 Reserved
 *
 * @default 'xxxxx'
 *
 */

/*
 * b4 System reset detected
 *  0: no reset detected since last ‘clear status register’ command
 *  1: reset detected (hard reset, soft reset command or supply fail)
 *
 * @default 1
 *
 */
enum {
    MJD_SHT3X_SYSTEM_RESET_DETECTED_REG = MJD_SHT3X_REG_STATUS,
    MJD_SHT3X_SYSTEM_RESET_DETECTED_BITMASK = 0x0010,
    MJD_SHT3X_SYSTEM_RESET_DETECTED_BITSHIFT = 4
};
typedef enum {
    MJD_SHT3X_SYSTEM_RESET_DETECTED_NO = 0,
    MJD_SHT3X_SYSTEM_RESET_DETECTED_YES = 1,
} mjd_sht3x_system_reset_detected_t;

/*
 * b3:2 Reserved
 *
 * @default '00'
 *
 */

/*
 * b1 Last Command status
 *  0: last command executed successfully
 *  1: last command not processed. It was either invalid, failed the integrated command checksum
 *
 * @default 0
 *
 */
enum {
    MJD_SHT3X_LAST_COMMAND_STATUS_REG = MJD_SHT3X_REG_STATUS,
    MJD_SHT3X_LAST_COMMAND_STATUS_BITMASK = 0x0002,
    MJD_SHT3X_LAST_COMMAND_STATUS_BITSHIFT = 1
};
typedef enum {
    MJD_SHT3X_LAST_COMMAND_STATUS_OK = 0,
    MJD_SHT3X_LAST_COMMAND_STATUS_ERROR = 1,
} mjd_sht3x_last_command_status_t;

/*
 * b0 Write data checksum status
 *  0: checksum of last write transfer was correct
 *  1: checksum of last write transfer failed
 *
 * @default 0
 *
 */
enum {
    MJD_SHT3X_WRITE_DATA_CHECKSUM_STATUS_REG = MJD_SHT3X_REG_STATUS,
    MJD_SHT3X_WRITE_DATA_CHECKSUM_STATUS_BITMASK = 0x0001,
    MJD_SHT3X_WRITE_DATA_CHECKSUM_STATUS_BITSHIFT = 0
};
typedef enum {
    MJD_SHT3X_WRITE_DATA_CHECKSUM_STATUS_OK = 0,
    MJD_SHT3X_WRITE_DATA_CHECKSUM_STATUS_ERROR = 1,
} mjd_sht3x_write_data_checksum_status_t;

#ifdef __cplusplus
}
#endif

#endif /* __MJD_SHT3X_DEFS_H__ */
