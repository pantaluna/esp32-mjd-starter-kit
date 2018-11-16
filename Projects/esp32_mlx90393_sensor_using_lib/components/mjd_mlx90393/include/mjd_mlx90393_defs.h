/**
 * Contains the declarations for this device (especially the registers, NVRAM and commands).
 */
#ifndef __MJD_MLX90393_DEFS_H__
#define __MJD_MLX90393_DEFS_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Device: Registers
 *
 */
typedef enum {
    MJD_MLX90393_REG_00H = 0x00,
    MJD_MLX90393_REG_01H = 0x01,
    MJD_MLX90393_REG_02H = 0x02,
    MJD_MLX90393_REG_03H = 0x03,
    MJD_MLX90393_REG_04H = 0x04,
    MJD_MLX90393_REG_05H = 0x05,
    MJD_MLX90393_REG_06H = 0x06,
    MJD_MLX90393_REG_07H = 0x07,
    MJD_MLX90393_REG_08H = 0x08,
    MJD_MLX90393_REG_09H = 0x09,
    MJD_MLX90393_REG_MAX = 0xFF
} mjd_mlx90393_reg_t;

/**
 * Device: Commands
 *
 */
typedef enum {
    MJD_MLX90393_CMD_START_BURST_MODE = 0x10,
    MJD_MLX90393_CMD_WAKEUP_ON_CHANGE_MODE = 0x20,
    MJD_MLX90393_CMD_START_SINGLE_MEASUREMENT_MODE = 0x30,
    MJD_MLX90393_CMD_READ_MEASUREMENT = 0x40,
    MJD_MLX90393_CMD_READ_REGISTER = 0x50,
    MJD_MLX90393_CMD_WRITE_REGISTER = 0x60,
    MJD_MLX90393_CMD_EXIT_MODE = 0x80,
    MJD_MLX90393_CMD_MEMORY_RECALL = 0xD0,
    MJD_MLX90393_CMD_MEMORY_STORE = 0xE0,
    MJD_MLX90393_CMD_RESET = 0xF0,
    MJD_MLX90393_CMD_NOP_TODO = 0x00, // TODO
} mjd_mlx90393_command_t;

/**
 * Device: Metrics selector
 *
 */
typedef enum {
    MJD_MLX90393_METRIC_ALL_BITMASK = 0x8 | 0x4 | 0x2 | 0x1,
    MJD_MLX90393_METRIC_Z_AXIS_BITMASK = 0x8,
    MJD_MLX90393_METRIC_Y_AXIS_BITMASK = 0x4,
    MJD_MLX90393_METRIC_X_AXIS_BITMASK = 0x2,
    MJD_MLX90393_METRIC_TEMPERATURE_BITMASK = 0x1,
} mjd_mlx90393_metric_t;

/**
 * Device: Status Byte
 *
 */
typedef uint8_t mjd_mlx90393_status_byte_t;

typedef enum {
    /*The MODE bits define in which mode the MLX90393 is currently set. Whenever a mode transition
     * command is rejected, the first status byte after this command will have the expected mode bit
     * cleared, which serves as an indication that the command has been rejected, next to the ERROR bit.
     * The SM_MODE flag can be the result of an SM command or from raising the TRG pin when TRG mode
     * is enabled in the volatile memory of the MLX90393.
     */
    MJD_MLX90393_STATUS_BURST_MODE_BITMASK = 0x80,
    MJD_MLX90393_STATUS_WAKE_ON_CHANGE_MODE_BITMASK = 0x40,
    MJD_MLX90393_STATUS_SINGLE_MEASUREMENT_MODE_BITMASK = 0x20,

    /* This bit is set in case a command has been rejected or in case an uncorrectable error is detected in the memory,
     * a so called ECC_ERROR.
     * A single error in the memory can be corrected (see SED bit), two errors can be detected and will generate the ECC_ERROR. In such a case all commands
     * but the RT (Reset) command will be rejected.
     * @important The error bit is also set when the master is reading back data while the DRDY flag is low, when reading out the same measurement twice.
     *
     */
    MJD_MLX90393_STATUS_ERROR_BITMASK = 0x10,

    /*
     * The single error detection bit simply flags that a bit error in the non-volatile memory has been
     *  corrected. It is purely informative and has no impact on the operation of the MLX90393.
     */
    MJD_MLX90393_STATUS_SED_BITMASK = 0x08,

    /* Whenever the MLX90393 gets out of a reset situation – both hard and soft reset – the RS flag is set
     * to highlight this situation to the master in the first status byte that is read out. As soon as the first
     * status byte is read, the flag is cleared until the next reset occurs.
     * */
    MJD_MLX90393_STATUS_RESET_BITMASK = 0x04,

    /* The number of response bytes correspond to 2 + 2*D[1:0], so the expected byte counts are either 2, 4, 6 or 8.
     *
     * The D1 & D0 bits only have a meaning after the RR and RM commands, when data is expected as a response from the MLX90393.
     * For commands where no response is expected (example: RESET) , the content of D[1:0] should be ignored.
     */
    MJD_MLX90393_STATUS_D1_BITMASK = 0x02,
    MJD_MLX90393_STATUS_D0_BITMASK = 0x01,
} mjd_mlx90393_status_bit_t;

/**
 *  Device: Parameters- Register, Bit mask, Bit right shift
 *
 */

/*
 * HALLCONF
 *
 * @doc Modifies the hall plate spinning (2-phase vs 4-phase) which has an effect on the minimum sampling rate achievable.
 * @doc The default HALLCONF is 0xC.
 * @doc HALLCONF[3:2] defines the duration of one Hall plate spinning phase in clock cycles:
 *          #clocks/spinning_phase = 8 * 2 * HALLCONF[3:2]. Default this value is 3.
 *      HALLCONF[1:0] defines the number of amplifier chopping periods inside one spinning phase:
 *          #chopping_periods/spinning_phase = 2HALLCONF[1:0]. Default this value is 0.
 *
 * @default 0xc
 *
 */
enum {
    MJD_MLX90393_HALLCONF_REG = 0x00,
    MJD_MLX90393_HALLCONF_BITMASK = 0x000F,
    MJD_MLX90393_HALLCONF_BITSHIFT = 0
};
typedef enum {
    MJD_MLX90393_HALLCONF_0 = 0x0, /*!< 0x0 is not supported (it is never used in reality). Always use HallConf=0xC */
    MJD_MLX90393_HALLCONF_C = 0xC, /*!< DEFAULT */
} mjd_mlx90393_hallconf_t;

/*
 * GAIN_SEL
 *
 * @default 0x7
 *
 * @doc Sets the analog gain to the desired value.
 *      The sensitivity is dependent on the axis (X and Y have higher sensitivity) as well as the setting of the RES_XYZ[5:0] parameter.
 *
 */
enum {
    MJD_MLX90393_GAIN_SEL_REG = 0x00,
    MJD_MLX90393_GAIN_SEL_BITMASK = 0x0070,
    MJD_MLX90393_GAIN_SEL_BITSHIFT = 4
};
typedef enum {
    MJD_MLX90393_GAIN_SEL_0 = 0, /*!<  */
    MJD_MLX90393_GAIN_SEL_1 = 1, /*!< */
    MJD_MLX90393_GAIN_SEL_2, /*!< */
    MJD_MLX90393_GAIN_SEL_3, /*!< */
    MJD_MLX90393_GAIN_SEL_4, /*!< */
    MJD_MLX90393_GAIN_SEL_5, /*!< */
    MJD_MLX90393_GAIN_SEL_6, /*!< */
    MJD_MLX90393_GAIN_SEL_7, /*!< DEFAULT */
    MJD_MLX90393_GAIN_SEL_MAX, /*!< Used to create static arrays */
} mjd_mlx90393_gain_sel_t;

/*
 * Z_SERIES
 *
 * @doc Enables series connection of hall plates for Z axis measurement. In normal operation set to 0.
 *
 * @default 0x0
 *
 */
enum {
    MJD_MLX90393_Z_SERIES_REG = 0x00,
    MJD_MLX90393_Z_SERIES_BITMASK = 0x0080,
    MJD_MLX90393_Z_SERIES_BITSHIFT = 7
};
typedef enum {
    MJD_MLX90393_Z_SERIES_DISABLED = 0, /*!< */
    MJD_MLX90393_Z_SERIES_ENABLED = 1, /*!< */
} mjd_mlx90393_z_series_t;

/*
 * BIST
 *
 * @doc Enables (1) or disables (0) the built in self-test coil. In normal operation set to 0.
 *
 * @default 0x0
 *
 */
enum {
    MJD_MLX90393_BIST_REG = 0x00,
    MJD_MLX90393_BIST_BITMASK = 0x0100,
    MJD_MLX90393_BIST_BITSHIFT = 8
};
typedef enum {
    MJD_MLX90393_BIST_DISABLED = 0, /*!< */
    MJD_MLX90393_BIST_ENABLED = 1, /*!< */
} mjd_mlx90393_bist_t;

/*
 * BURST_DATA_RATE
 *
 * @doc Not used yet because it belongs to the Burst Mode which is not implemented in this component
 *
 */
enum {
    MJD_MLX90393_BURST_DATA_RATE_REG = 0x01,
    MJD_MLX90393_BURST_DATA_RATE_BITMASK = 0x003F,
    MJD_MLX90393_BURST_DATA_RATE_BITSHIFT = 0
};

enum {
    /* Not used yet because it belongs to the Burst Mode which is not implemented in this component */
    MJD_MLX90393_BURST_SEL_REG = 0x01,
    MJD_MLX90393_BURST_SEL_BITMASK = 0x03C0,
    MJD_MLX90393_BURST_SEL_BITSHIFT = 6
};

/*
 * TCMP_EN
 *
 * @doc Enables (1) or disables (0) the on-chip sensitivity drift compensation.
 * @doc Enabling the temperature compensation will influence the way the magnetic values are encoded and transmitted to the microcontroller.
 *
 * @doc Please refer to the application note on the temperature compensation for more info.
 *
 * @default 0x0
 *
 */
enum {
    MJD_MLX90393_TCMP_EN_REG = 0x01,
    MJD_MLX90393_TCMP_EN_BITMASK = 0x0400,
    MJD_MLX90393_TCMP_EN_BITSHIFT = 10
};
typedef enum {
    MJD_MLX90393_TCMP_EN_DISABLED = 0, /*!< DEFAULT */
    MJD_MLX90393_TCMP_EN_ENABLED = 1, /*!< */
} mjd_mlx90393_tcmp_en_t;

/*
 * EXT_TRIG
 *
 * @doc Allows for external trigger events when set to 1 and TRIG_INT_SEL = 0.
 *      When enabled an acquisition will start with the external trigger pin detects a high value.
 *      Acquisitions will continue to be triggered until the EST_TRIG pin is brought low.
 *
 * @default 0x0
 *
 * PS Register 0x01!
 *
 */
enum {
    MJD_MLX90393_EXT_TRIG_REG = 0x01,
    MJD_MLX90393_EXT_TRIG_BITMASK = 0x0800,
    MJD_MLX90393_EXT_TRIG_BITSHIFT = 11
};
typedef enum {
    MJD_MLX90393_EXT_TRIG_DISABLED = 0, /*!< */
    MJD_MLX90393_EXT_TRIG_ENABLED = 1, /*!< */
} mjd_mlx90393_ext_trig_t;

/*
 * WOC_DIFF
 *
 * @doc Not used yet because it belongs to the Wakeup On Change Mode which is not implemented in this component
 */
enum {
    MJD_MLX90393_WOC_DIFF_REG = 0x01,
    MJD_MLX90393_WOC_DIFF_BITMASK = 0x1000,
    MJD_MLX90393_WOC_DIFF_BITSHIFT = 12
};

typedef enum {
    MJD_MLX90393_WOC_DIFF_ABSOLUTE_MODE = 0, /*!< If 0, an absolute mode is used. Measurements are compared to the first measurements; */
    MJD_MLX90393_WOC_DIFF_RELATIVE_MODE = 1, /*!< If 1, a relative mode is used. Measurements are compared to the previous ones. */
} mjd_mlx90393_woc_diff_t;

/*
 * COMM_MODE
 *
 * @doc Defines the communication mode.
 *      When set to 0x0 or 0x1 both communication modes can be used but the selection is made by the CS pin.
 *      When set to 0x2 only SPI communication is allowed.
 *      When set to 0x3 only I2C communication is allowed.
 *
 * @default 0
 *
 */
enum {
    MJD_MLX90393_COMM_MODE_REG = 0x01,
    MJD_MLX90393_COMM_MODE_BITMASK = 0x6000,
    MJD_MLX90393_COMM_MODE_BITSHIFT = 13
};
typedef enum {
    MJD_MLX90393_COMM_MODE_I2C_AND_SPI = 0, /*!< DEFAULT */
    MJD_MLX90393_COMM_MODE_I2C_AND_SPI_1 = 1, /*!<  */
    MJD_MLX90393_COMM_MODE_SPI = 2, /*!<  */
    MJD_MLX90393_COMM_MODE_I2C = 3, /*!<  */
} mjd_mlx90393_comm_mode_t;

/*
 * TRIG_INT_SEL
 *
 * @doc Allows for external trigger events when set to 1 and TRIG_INT_SEL = 0.
 *      When enabled an acquisition will start with the external trigger pin detects a high value.
 *      Acquisitions will continue to be triggered until the EST_TRIG pin is brought low.
 *
 * @default 0x0
 *
 */enum {
    MJD_MLX90393_TRIG_INT_SEL_REG = 0x01,
    MJD_MLX90393_TRIG_INT_SEL_BITMASK = 0x8000,
    MJD_MLX90393_TRIG_INT_SEL_BITSHIFT = 15
};
typedef enum {
    MJD_MLX90393_TRIG_INT_SEL_DISABLED = 0, /*!< default */
    MJD_MLX90393_TRIG_INT_SEL_ENABLED = 1, /*!< */
} mjd_mlx90393_trig_int_sel_t;

/*
 * OSR
 *
 * @doc Oversampling ratio for the magnetic measurements.
 * @doc Defines the oversampling rate of the ADC decimation filter for magnetic measurements. This will directly impact the measurement time.
 *      OSR_ADC = 64 * 2^OSR.
 *
 * @default 0x0
 *
 */
enum {
    MJD_MLX90393_OSR_REG = 0x02,
    MJD_MLX90393_OSR_BITMASK = 0x0003,
    MJD_MLX90393_OSR_BITSHIFT = 0
};
typedef enum {
    MJD_MLX90393_OSR_0 = 0, /*!< DEFAULT */
    MJD_MLX90393_OSR_1 = 1, /*!< */
    MJD_MLX90393_OSR_2, /*!< */
    MJD_MLX90393_OSR_3, /*!< */
} mjd_mlx90393_osr_t;

/*
 * DIG_FILT
 *
 * @doc A control for the digital filter. An averaging over a certain amount (2^DIG_FILT) will be done. This will directly impact the measurement time.
 *
 * @default 0x0
 *
 */enum {
    MJD_MLX90393_DIG_FILT_REG = 0x02,
    MJD_MLX90393_DIG_FILT_BITMASK = 0x001C,
    MJD_MLX90393_DIG_FILT_BITSHIFT = 2
};
typedef enum {
    MJD_MLX90393_DIG_FILT_0 = 0, /*!< DEFAULT */
    MJD_MLX90393_DIG_FILT_1 = 1, /*!< */
    MJD_MLX90393_DIG_FILT_2, /*!< */
    MJD_MLX90393_DIG_FILT_3, /*!< */
    MJD_MLX90393_DIG_FILT_4, /*!< */
    MJD_MLX90393_DIG_FILT_5, /*!< */
    MJD_MLX90393_DIG_FILT_6, /*!< */
    MJD_MLX90393_DIG_FILT_7, /*!< */
} mjd_mlx90393_dig_filt_t;

/*
 * RES_XYZ
 *
 * @doc This 6 bit parameter consists out of three parts, one for each axis.
 *      The 2 left-most bits act on the Z-axis, the 2 right-most bits act on the X-axis and the 2 middle-bits act on the Y-axis.
 * @doc See section 15.4.1 GAIN_SEL for the relationship between the gain and resolution.
 *      See section 15.1.10 TCMP_EN for the relationship between RES_XYZ and the output data format.
 *
 * @default  X=0x0 Y=0x0 Z=0x0
 *
 */enum {
    MJD_MLX90393_RES_XYZ_REG = 0x02,
    MJD_MLX90393_RES_XYZ_BITMASK = 0x07E0,
    MJD_MLX90393_RES_XYZ_BITSHIFT = 5
};
typedef enum {
    MJD_MLX90393_RES_XYZ_0 = 0, /*!< DEFAULT for X and Y and Z */
    MJD_MLX90393_RES_XYZ_1 = 1, /*!< */
    MJD_MLX90393_RES_XYZ_2, /*!< */
    MJD_MLX90393_RES_XYZ_3, /*!< */
    MJD_MLX90393_RES_XYZ_MAX, /*!< Used to create static arrays */
} mjd_mlx90393_res_xyz_t;

/*
 * OSR2
 *
 * @doc Not used yet because it belongs to the Temperature Compensation Enabled Mode which is not implemented in this component
 *
 */
enum {
    MJD_MLX90393_OSR2_REG = 0x02,
    MJD_MLX90393_OSR2_BITMASK = 0x1800,
    MJD_MLX90393_OSR2_BITSHIFT = 11
};

/*
 * SENS_TC_LT
 *
 * @doc Sensitivity drift compensation factor for T < TREF. This is the parameter for temperatures below 35 degC.
 * @doc Please refer to the application note on the temperature compensation for more info.
 *
 * @doc My NVRAM values:
 *          SENS_TC_LT: 0x43 (67)
 *          SENS_TC_HT: 0x54 (84)
 * @important After packaging (QFN), an extra drift is introduced. This is experimentally found to be a constant offset:
 *              Sens_TC_LT_new = SENS_TC_LT –  6 = 61
 *              Sens_TC_HT_new = Sens_TC_HT – 15 = 69
 *
 * @default 0x43 (67) => Adjusted 0x3D (61)
 *
 */
enum {
    // These parameters are used for the temperature compensation for the sensitivity drift of the Hall elements.
    MJD_MLX90393_SENS_TC_LT_REG = 0x03,
    MJD_MLX90393_SENS_TC_LT_BITMASK = 0x00FF,
    MJD_MLX90393_SENS_TC_LT_BITSHIFT = 0
};

/*
 * SENS_TC_HT
 *
 * @doc Sensitivity drift compensation factor for T > TREF. This is the parameter for temperatures above 35 degC.
 * @doc Please refer to the application note on the temperature compensation for more info.
 *
 * @doc My NVRAM values:
 *          SENS_TC_LT: 0x43 (67)
 *          SENS_TC_HT: 0x54 (84)
 * @important After packaging (QFN), an extra drift is introduced. This is experimentally found to be a constant offset:
 *              Sens_TC_LT_new = SENS_TC_LT –  6 = 61
 *              Sens_TC_HT_new = Sens_TC_HT – 15 = 69
 *
 * @default 0x54 (84) => Adjusted 0x45 (69)
 *
 */
enum {
    // These parameters are used for the temperature compensation for the sensitivity drift of the Hall elements.
    MJD_MLX90393_SENS_TC_HT_REG = 0x03,
    MJD_MLX90393_SENS_TC_HT_BITMASK = 0xFF00,
    MJD_MLX90393_SENS_TC_HT_BITSHIFT = 8
};

/*
 * OFFSET_X OFFSET_Y OFFSET_Z
 *
 * @doc Constant offset correction, ***independent of temperature***, and programmable for each individual axis where i=X, Y, or Z.
 * @doc These parameters are used to compensate the offset in case the temperature compensation is enabled.
 *      These should be initialized with the values the part generates in no magnetic field (this is an expensive setup).
 *      These are NOT used with TCMP disabled. Please refer to the application note on the temperature compensation for more info.
 *
 * @default X=0 Y=0 Z=0
 *
 * @important The Offset values are relative to 0x8000
 *     "When enabling the TC, always adapt the offset to a value in unsigned format, centered around 0x8000.
 *      For example if the offset measured without the temperature compensation enabled equals 200LSB, the offset programmed will become 32768+200=32968=0x80C8."
 *
 */
enum {
    /* 1 WORD. These parameters are used to compensate the offset in case the temperature compensation is enabled. They are not used with TCMP disabled. */
    MJD_MLX90393_OFFSET_X_REG = 0x04,
    MJD_MLX90393_OFFSET_X_BITMASK = 0xFFFF,
    MJD_MLX90393_OFFSET_X_BITSHIFT = 0
};
enum {
    /* 1 WORD. These parameters are used to compensate the offset in case the temperature compensation is enabled. They are not used with TCMP disabled. */
    MJD_MLX90393_OFFSET_Y_REG = 0x05,
    MJD_MLX90393_OFFSET_Y_BITMASK = 0xFFFF,
    MJD_MLX90393_OFFSET_Y_BITSHIFT = 0
};
enum {
    /* 1 WORD. These parameters are used to compensate the offset in case the temperature compensation is enabled. They are not used with TCMP disabled. */
    MJD_MLX90393_OFFSET_Z_REG = 0x06,
    MJD_MLX90393_OFFSET_Z_BITMASK = 0xFFFF,
    MJD_MLX90393_OFFSET_Z_BITSHIFT = 0
};

/*
 * WOXY_THRESHOLD WOZ_THRESHOLD WOT_THRESHOLD
 *
 * @doc Not used yet because it belongs to the Wakeup On Change Mode which is not implemented in this component
 */
enum {
    MJD_MLX90393_WOXY_THRESHOLD_REG = 0x07,
    MJD_MLX90393_WOXY_THRESHOLD_BITMASK = 0xFFFF,
    MJD_MLX90393_WOXY_THRESHOLD_BITSHIFT = 0
};
enum {
    MJD_MLX90393_WOZ_THRESHOLD_REG = 0x08,
    MJD_MLX90393_WOZ_THRESHOLD_BITMASK = 0xFFFF,
    MJD_MLX90393_WOZ_THRESHOLD_BITSHIFT = 0
};
enum {
    MJD_MLX90393_WOT_THRESHOLD_REG = 0x09,
    MJD_MLX90393_WOT_THRESHOLD_BITMASK = 0xFFFF,
    MJD_MLX90393_WOT_THRESHOLD_BITSHIFT = 0
};
/*
 * TREF
 *
 * @doc The value that the sensor has at 35degC can be read back from the IC. It is stored in register 0x24 (in the Melexis area of the NVRAM).
 *      This digital value is called TREF in the document.
 *      Typically, it should be around 0xB668, but part to part variation this will vary. The value is trimmed during probing.
 *      This value will be the pivot point for the temperature compensation.
 *
 * @default *NONE (varies for each unit). Value for one of my units: TREF: 0xB668 (46696)
 *
 */
enum {
    /* The value that the sensor has at 35degC can be read back from the IC. It is stored in register 0x24.
     *
     */
    MJD_MLX90393_TREF_THRESHOLD_REG = 0x24,
    MJD_MLX90393_TREF_THRESHOLD_BITMASK = 0xFFFF,
    MJD_MLX90393_TREF_THRESHOLD_BITSHIFT = 0
};

#ifdef __cplusplus
}
#endif

#endif /* __MJD_MLX90393_DEFS_H__ */
