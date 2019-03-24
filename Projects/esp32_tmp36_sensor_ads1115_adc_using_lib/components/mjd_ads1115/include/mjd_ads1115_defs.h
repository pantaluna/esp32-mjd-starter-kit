/**
 * Contains the declarations for this device (especially the registers, NVRAM and commands).
 */
#ifndef __MJD_ADS1115_DEFS_H__
#define __MJD_ADS1115_DEFS_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Device: Registers
 *  Register address pointer
 *      00 : Conversion register 16 bit
 *      01 : Config register 16 bit
 *      10 : Lo_thresh register 16 bit
 *      11 : Hi_thresh register 16 bit
 *
 */
typedef enum {
    MJD_ADS1115_REG_CONVERSION = 0,
    MJD_ADS1115_REG_CONFIG = 1,
    MJD_ADS1115_REG_LOWTHRESHOLD = 2,
    MJD_ADS1115_REG_HIGHTHRESHOLD = 3,
} mjd_ads1115_reg_t;

/**
 * Device: Commands (not relevant for this device)
 *
 */

/**
 *  Device: Parameters- Register, Bit mask, Bit right shift
 *
 */

/*
 * 15 OS R/W 1h Operational status or single-shot conversion start
 *
 * This bit determines the operational status of the device. OS can only be written
 * when in power-down state and has no effect when a conversion is ongoing.
 * When writing:
 * 0 : No effect
 * 1 : Start a single conversion (when in power-down state)
 * When reading:
 * 0 : Device is currently performing a conversion
 * 1 : Device is not currently performing a conversion

 * @default 0b0
 *
 */
enum {
    MJD_ADS1115_OPSTATUS_REG = MJD_ADS1115_REG_CONFIG,
    MJD_ADS1115_OPSTATUS_BITMASK = 0x8000,
    MJD_ADS1115_OPSTATUS_BITSHIFT = 15
};
typedef enum {
    MJD_ADS1115_OPSTATUS_NO_EFFECT = 0, /*!< @case When writing! */
    MJD_ADS1115_OPSTATUS_START_SINGLE_CONVERSION = 1, /*!< @case When writing! */
    MJD_ADS1115_OPSTATUS_IS_PERFORMING_CONVERSION = 0, /*!< @case When reading! */
    MJD_ADS1115_OPSTATUS_IS_NOT_PERFORMING_CONVERSION = 1, /*!<@ case When reading! */
} mjd_ads1115_operational_status_t;

/*
 * 14:12 MUX[2:0] R/W 0h Input multiplexer configuration
 *
 * @legend AINp and AINn denote the selected Positive and Negative inputs. AINx denotes one of the four available analog inputs.
 * These bits configure the input multiplexer. These bits serve no function on the ADS1113 and ADS1114.
 *  000 : AINp = AIN0  and AINn = AIN1 (default)
 *  001 : AINp = AIN0  and AINn = AIN3
 *  010 : AINp = AIN1  and AINn = AIN3
 *  011 : AINp = AIN2  and AINn = AIN3
 *  100 : AINp = AIN0  and AINn = GND  => The setting if you want to measure voltage on the AIN0 input pin (compared to GND).
 *  101 : AINp = AIN1  and AINn = GND
 *  110 : AINp = AIN2  and AINn = GND
 *  111 : AINp = AIN3  and AINn = GND
 *
 * @default 0b0
 *
 */
enum {
    MJD_ADS1115_MUX_REG = MJD_ADS1115_REG_CONFIG,
    MJD_ADS1115_MUX_BITMASK = 0x7000,
    MJD_ADS1115_MUX_BITSHIFT = 12
};
typedef enum {
    MJD_ADS1115_MUX_0_1 = 0,/*<! Device default; AINp = AIN0 and AINn = AIN1 */
    MJD_ADS1115_MUX_0_3,
    MJD_ADS1115_MUX_1_3,
    MJD_ADS1115_MUX_2_3,
    MJD_ADS1115_MUX_0_GND, /*<! My custom default; AINp = AIN0 and AINn = GND */
    MJD_ADS1115_MUX_1_GND,
    MJD_ADS1115_MUX_2_GND,
    MJD_ADS1115_MUX_3_GND,
} mjd_ads1115_mux_t;

/*
 * 11:9 PGA[2:0] R/W 2h Programmable gain amplifier configuration
 *
 * These bits set the FSR of the programmable gain amplifier. These bits serve no function on the ADS1113.
 *  000 : FSR = ±6.144 V            => 1 bit = 3mV
 *  001 : FSR = ±4.096 V            => 1 bit = 2mV
 *  010 : FSR = ±2.048 V (default)  => 1 bit = 1mV
 *  011 : FSR = ±1.024 V            => 1 bit = 0.5mV
 *  100 : FSR = ±0.512 V            => 1 bit = 0.25mV
 *  101 : FSR = ±0.256 V            => 1 bit = 0.125mV
 *  110 : FSR = ±0.256 V (not used)
 *  111 : FSR = ±0.256 V (not used)
 *
 * @default 0b010
 *
 */
enum {
    MJD_ADS1115_PGA_REG = MJD_ADS1115_REG_CONFIG,
    MJD_ADS1115_PGA_BITMASK = 0x0E00,
    MJD_ADS1115_PGA_BITSHIFT = 9
};
typedef enum {
    MJD_ADS1115_PGA_6_144 = 0,
    MJD_ADS1115_PGA_4_096,
    MJD_ADS1115_PGA_2_048,
    MJD_ADS1115_PGA_1_024,
    MJD_ADS1115_PGA_0_512,
    MJD_ADS1115_PGA_0_256,
    MJD_ADS1115_PGA_MAX,
} mjd_ads1115_pga_t;

/*
 * 8 MODE R/W 1h Device operating mode
 *
 * This bit controls the operating mode.
 *  0 : Continuous-conversion mode
 *  1 : Single-shot mode or power-down state (default)
 *
 * @default 0b1
 *
 */
enum {
    MJD_ADS1115_OPMODE_REG = MJD_ADS1115_REG_CONFIG,
    MJD_ADS1115_OPMODE_BITMASK = 0x0100,
    MJD_ADS1115_OPMODE_BITSHIFT = 8
};
typedef enum {
    MJD_ADS1115_OPMODE_CONTINUOUS = 0,
    MJD_ADS1115_OPMODE_SINGLE_SHOT

} mjd_ads1115_operating_mode_t;

/*
 * 7:5 DR[2:0] R/W 4h Data rate
 *
 * Conversions in the ADS111x settle within a single cycle; thus, the conversion time is equal to 1 / DR.
 * @tip The noise performance improves when lowering the output data rate because more samples of the internal modulator are averaged to yield one conversion result.
 *
 * These bits control the data rate setting.
 * 000 : 8 SPS
 * 001 : 16 SPS
 * 010 : 32 SPS
 * 011 : 64 SPS
 * 100 : 128 SPS (default)
 * 101 : 250 SPS
 * 110 : 475 SPS
 * 111 : 860 SPS
 *
 * @default 0b100
 *
 */
enum {
    MJD_ADS1115_DATARATE_REG = MJD_ADS1115_REG_CONFIG,
    MJD_ADS1115_DATARATE_BITMASK = 0x00E0,
    MJD_ADS1115_DATARATE_BITSHIFT = 5
};
typedef enum {
    MJD_ADS1115_DATARATE_8_SPS = 0,
    MJD_ADS1115_DATARATE_16_SPS,
    MJD_ADS1115_DATARATE_32_SPS,
    MJD_ADS1115_DATARATE_64_SPS,
    MJD_ADS1115_DATARATE_128_SPS,
    MJD_ADS1115_DATARATE_250_SPS,
    MJD_ADS1115_DATARATE_475_SPS,
    MJD_ADS1115_DATARATE_860_SPS,
    MJD_ADS1115_DATARATE_MAX,
} mjd_ads1115_data_rate_t;

/*
 * 4 COMP_MODE R/W 0h Comparator mode (ADS1114 and ADS1115 only)
 *
 * This bit configures the comparator operating mode. This bit serves no function on the ADS1113.
 *  0 : Traditional comparator (default)
 *  1 : Window comparator
 *
 * @default 0b0
 *
 */
enum {
    MJD_ADS1115_COMPARATORMODE_REG = MJD_ADS1115_REG_CONFIG,
    MJD_ADS1115_COMPARATORMODE_BITMASK = 0x0010,
    MJD_ADS1115_COMPARATORMODE_BITSHIFT = 4
};
typedef enum {
    MJD_ADS1115_COMPARATORMODE_TRADITIONAL = 0,
    MJD_ADS1115_COMPARATORMODE_WINDOW,
} mjd_ads1115_comparator_mode_t;

/*
 * 3 COMP_POL R/W 0h Comparator polarity (ADS1114 and ADS1115 only)
 *
 * This bit controls the polarity of the ALERT/RDY pin. This bit serves no function on the ADS1113.
 *  0 : Active low (default)
 *  1 : Active high
 *
 * @default 0b0
 *
 */
enum {
    MJD_ADS1115_COMPARATORPOLARITY_REG = MJD_ADS1115_REG_CONFIG,
    MJD_ADS1115_COMPARATORPOLARITY_BITMASK = 0x0008,
    MJD_ADS1115_COMPARATORPOLARITY_BITSHIFT = 3
};
typedef enum {
    MJD_ADS1115_COMPARATORPOLARITY_ACTIVE_LOW = 0,
    MJD_ADS1115_COMPARATORPOLARITY_ACTIVE_HIGH,
} mjd_ads1115_comparator_polarity_t;

/*
 * 2 COMP_LAT R/W 0h Latching comparator (ADS1114 and ADS1115 only)
 *
 * This bit controls whether the ALERT/RDY pin latches after being asserted or clears after conversions are within the margin
 * of the upper and lower threshold values. This bit serves no function on the ADS1113.
 *  0 : Nonlatching comparator . The ALERT/RDY pin does not latch when asserted (default).
 *  1 : Latching comparator. The asserted ALERT/RDY pin remains latched until conversion data are read by the master
 *      or an appropriate SMBus alert response is sent by the master.
 *      The device responds with its address, and it is the lowest address currently asserting the ALERT/RDY bus line.
 *
 * @default 0b0
 *
 */
enum {
    MJD_ADS1115_LATCHINGCOMPARATOR_REG = MJD_ADS1115_REG_CONFIG,
    MJD_ADS1115_LATCHINGCOMPARATOR_BITMASK = 0x0004,
    MJD_ADS1115_LATCHINGCOMPARATOR_BITSHIFT = 2
};
typedef enum {
    MJD_ADS1115_LATCHINGCOMPARATOR_NON_LATCHING = 0,
    MJD_ADS1115_LATCHINGCOMPARATOR_LATCHING,
} mjd_ads1115_latching_comparator_t;

/*
 * 1:0 COMP_QUE[1:0] R/W 3h Comparator queue and disable (ADS1114 and ADS1115 only)
 *
 * These bits perform two functions. When set to 11, the comparator is disabled and the ALERT/RDY pin is set to a high-impedance state.
 * When set to any other value, the ALERT/RDY pin and the comparator function are enabled, and the set value determines
 * the number of successive conversions exceeding the upper or lower threshold required before asserting the ALERT/RDY pin.
 * These bits serve no function on the ADS1113.
 *  00 : Assert after one conversion
 *  01 : Assert after two conversions
 *  10 : Assert after four conversions
 *  11 : Disable comparator and set ALERT/RDY pin to high-impedance (default)
 *
 * @default 0b11
 *
 */
enum {
    MJD_ADS1115_COMPARATORQUEUE_REG = MJD_ADS1115_REG_CONFIG,
    MJD_ADS1115_COMPARATORQUEUE_BITMASK = 0x0003,
    MJD_ADS1115_COMPARATORQUEUE_BITSHIFT = 0
};
typedef enum {
    MJD_ADS1115_COMPARATORQUEUE_ASSERT_AFTER_ONE_CONVERSION = 0,
    MJD_ADS1115_COMPARATORQUEUE_ASSERT_AFTER_TWO_CONVERSIONS,
    MJD_ADS1115_COMPARATORQUEUE_ASSERT_AFTER_FOUR_CONVERSIONS,
    MJD_ADS1115_COMPARATORQUEUE_DISABLE_COMPARATOR,
} mjd_ads1115_comparator_queue_t;

/*
 * 15:0 Lo_thresh[15:0] R/W Low threshold value
 *
 * @default 0x8000 (32768)
 *
 * TODO Check if necessary to exclude the MSB (used for conversionreadypin)?
 *
 */
enum {
    MJD_ADS1115_LOWTHRESHOLD_REG = MJD_ADS1115_REG_LOWTHRESHOLD, /*!< special register */
    MJD_ADS1115_LOWTHRESHOLD_BITMASK = 0xFFFF,
    MJD_ADS1115_LOWTHRESHOLD_BITSHIFT = 0
};

/*
 * 15:0 Hi_thresh[15:0] R/W High threshold value
 *
 * @default  0x7FFF (32767)
 *
 * TODO Check if necessary to exclude the MSB (used for conversionreadypin)?
 *
 */
enum {
    MJD_ADS1115_HIGHTHRESHOLD_REG = MJD_ADS1115_REG_HIGHTHRESHOLD, /*!< special register */
    MJD_ADS1115_HIGHTHRESHOLD_BITMASK = 0xFFFF,
    MJD_ADS1115_HIGHTHRESHOLD_BITSHIFT = 0
};

/*
 * 15 Lo_thresh[15:15] W  Conversion Ready Pin LOW Register
 *
 * @important This is a special function of the Low threshold Register! The ALERT/RDY pin can also be configured as a conversion ready pin. Set the most-significant bit of the
 *            Hi_thresh register to 1 and the most-significant bit of Lo_thresh register to 0 to enable the pin as a conversion ready pin.
 *
 * @default ???
 *
 */
enum {
    MJD_ADS1115_CONVERSIONREADYPININLOWREG_REG = MJD_ADS1115_REG_LOWTHRESHOLD, /*!< special register */
    MJD_ADS1115_CONVERSIONREADYPININLOWREG_BITMASK = 0x8000,
    MJD_ADS1115_CONVERSIONREADYPININLOWREG_BITSHIFT = 15
};
typedef enum {
    MJD_ADS1115_CONVERSIONREADYPININLOWREG_ENABLED = 0, /*!< LOW enabled=0! */
    MJD_ADS1115_CONVERSIONREADYPININLOWREG_DISABLED = 1
} mjd_ads1115_conversion_ready_pin_in_low_reg_t;

/*
 * 15 Hi_thresh[15:15] W  Conversion Ready Pin HIGH Register
 *
 * @important This is a special function of the High threshold Register! The ALERT/RDY pin can also be configured as a conversion ready pin. Set the most-significant bit of the
 *            Hi_thresh register to 1 and the most-significant bit of Lo_thresh register to 0 to enable the pin as a conversion ready pin.
 *
 * @default ???
 *
 */
enum {
    MJD_ADS1115_CONVERSIONREADYPININHIGHREG_REG = MJD_ADS1115_REG_HIGHTHRESHOLD, /*!< special register */
    MJD_ADS1115_CONVERSIONREADYPININHIGHREG_BITMASK = 0x8000,
    MJD_ADS1115_CONVERSIONREADYPININHIGHREG_BITSHIFT = 15
};
typedef enum {
    MJD_ADS1115_CONVERSIONREADYPININHIGHREG_DISABLED = 0,
    MJD_ADS1115_CONVERSIONREADYPININHIGHREG_ENABLED = 1 /*!< HIGH enabled=1! */
} mjd_ads1115_conversion_ready_pin_in_high_reg_t;

#ifdef __cplusplus
}
#endif

#endif /* __MJD_ADS1115_DEFS_H__ */
