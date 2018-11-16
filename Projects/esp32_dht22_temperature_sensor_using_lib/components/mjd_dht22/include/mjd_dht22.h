/*
 *
 */
#ifndef __MJD_DHT22_H__
#define __MJD_DHT22_H__

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Data structs
 */
typedef struct {
    gpio_num_t gpio_pin;
    rmt_channel_t rmt_channel;
} mjd_dht22_config_t;

typedef struct {
    float humidity_percent;
    float temperature_celsius;
} mjd_dht22_data_t;

/**
 * Function declarations
 */
esp_err_t mjd_dht22_init(const mjd_dht22_config_t* config);
esp_err_t mjd_dht22_read(const mjd_dht22_config_t* config, mjd_dht22_data_t* data);


#ifdef __cplusplus
}
#endif

#endif /* __MJD_DHT22_H__ */
