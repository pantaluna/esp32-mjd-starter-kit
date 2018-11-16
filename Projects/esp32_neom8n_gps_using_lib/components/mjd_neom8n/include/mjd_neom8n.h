/*
 *
 */
#ifndef __MJD_NEOM8N_H__
#define __MJD_NEOM8N_H__

#ifdef __cplusplus
extern "C" {
#endif


// MINMEA Library
#include "minmea.h"

// MUTEX
#define MJD_NEOM8N_SERVICE_LOCK()     xSemaphoreTake(_neom8n_service_semaphore, portMAX_DELAY)
#define MJD_NEOM8N_SERVICE_UNLOCK()   xSemaphoreGive(_neom8n_service_semaphore)

// MONITOR TASK
#define MJD_NEOM8N_GPS_MONITOR_TASK_STACK_SIZE (4096)  // Stack size for the GPS monitor task

/**
 * Data structs
 */
typedef struct {
    uart_port_t uart_port;        /*!< The ESP32 UART interface being used (typically UART_NUM1 or UART_NUM2) */
    gpio_num_t uart_rx_gpio_num;  /*!< The ESP32 GPIO Number for the UART RX pin (pin 23 for an Adafruit Huzzah32) */
    gpio_num_t uart_tx_gpio_num;  /*!< The ESP32 GPIO Number for the UART TX pin (pin 22 for an Adafruit Huzzah32) */
    bool do_cold_start;           /*!< Do a cold start when initializing the device? */
} mjd_neom8n_config_t;

#define MJD_NEOM8N_CONFIG_DEFAULT() { \
    .uart_port = UART_NUM_MAX, \
    .uart_rx_gpio_num = GPIO_NUM_0, \
    .uart_tx_gpio_num = GPIO_NUM_0, \
    .do_cold_start = false \
}

typedef struct {
    bool data_received; /*!< Has data ever been received from the GPS device? Nice to know in case of a diagnosis */
    int fix_quality;    /*!< 0 = No fix, 1 = Autonomous (3D) GNSS fix, 2 = Differential GNSS fix, 4 = RTK fixed, 5 = RTK float, 6 = Estimated/Dead reckoning fix */
    float latitude;     /*!< The latest latitude reading */
    float longitude;    /*!< The latest longitude reading */
    int satellites_tracked; /*!< The number of satellites that are being tracked */
} mjd_neom8n_data_t;

/**
 * Function declarations
 */
esp_err_t mjd_neom8n_init(mjd_neom8n_config_t* ptr_param_config);
esp_err_t mjd_neom8n_deinit(mjd_neom8n_config_t* ptr_param_config);
esp_err_t mjd_neom8n_read(mjd_neom8n_config_t* ptr_param_config, mjd_neom8n_data_t* ptr_param_data);

esp_err_t mjd_neom8n_cold_start_forced(mjd_neom8n_config_t* ptr_param_config);
esp_err_t mjd_neom8n_power_down_for_15_seconds(mjd_neom8n_config_t* ptr_param_config);
esp_err_t mjd_neom8n_power_down(mjd_neom8n_config_t* ptr_param_config);
esp_err_t mjd_neom8n_power_up(mjd_neom8n_config_t* ptr_param_config);
esp_err_t mjd_neom8n_gnss_stop(mjd_neom8n_config_t* ptr_param_config);
esp_err_t mjd_neom8n_gnss_start(mjd_neom8n_config_t* ptr_param_config);
esp_err_t mjd_neom8n_set_measurement_rate_1000ms(mjd_neom8n_config_t* ptr_param_config);
esp_err_t mjd_neom8n_set_measurement_rate_100ms(mjd_neom8n_config_t* ptr_param_config);
esp_err_t mjd_neom8n_set_measurement_rate_5000ms(mjd_neom8n_config_t* ptr_param_config);


#ifdef __cplusplus
}
#endif

#endif /* __MJD_NEOM8N_H__ */
