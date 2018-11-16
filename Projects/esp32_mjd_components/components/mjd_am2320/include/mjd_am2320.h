/*
 *
 */
#ifndef __MJD_AM2320_H__
#define __MJD_AM2320_H__

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Data structs
 */
typedef struct {
    gpio_num_t gpio_pin;
    rmt_channel_t rmt_channel;
} mjd_am2320_config_t;

typedef struct {
    float humidity_percent;
    float temperature_celsius;
} mjd_am2320_data_t;

/**
 * Function declarations
 */
esp_err_t mjd_am2320_init(const mjd_am2320_config_t* config);
esp_err_t mjd_am2320_read(const mjd_am2320_config_t* config, mjd_am2320_data_t* data);


#ifdef __cplusplus
}
#endif

#endif /* __MJD_AM2320_H__ */
