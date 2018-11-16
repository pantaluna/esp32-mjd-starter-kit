/*
 *
 */
#ifndef __MJD_DHT11_H__
#define __MJD_DHT11_H__

#ifdef __cplusplus
extern "C" {
#endif


/**
 * Data structs
 */
typedef struct {
    gpio_num_t gpio_pin;
    rmt_channel_t rmt_channel;
} mjd_dht11_config_t;

typedef struct {
    float humidity_percent;
    float temperature_celsius;
} mjd_dht11_data_t;

/**
 * Function declarations
 */
esp_err_t mjd_dht11_init(const mjd_dht11_config_t* config);
esp_err_t mjd_dht11_read(const mjd_dht11_config_t* config, mjd_dht11_data_t* data);


#ifdef __cplusplus
}
#endif

#endif /* __MJD_DHT11_H__ */
