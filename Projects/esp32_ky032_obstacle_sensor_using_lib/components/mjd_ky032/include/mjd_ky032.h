/*
 *
 */
#ifndef __MJD_KY032_H__
#define __MJD_KY032_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Data structs
 */
typedef struct {
    gpio_num_t data_gpio_num;
    SemaphoreHandle_t gpio_isr_mux;
    bool is_init;
} mjd_ky032_config_t;

#define MJD_KY032_CONFIG_DEFAULT() { \
    .data_gpio_num = GPIO_NUM_0, \
    .gpio_isr_mux = NULL, \
    .is_init = false \
}

/**
 * Function declarations
 */
esp_err_t mjd_ky032_init(mjd_ky032_config_t* ptr_param_config);
esp_err_t mjd_ky032_deinit(mjd_ky032_config_t* ptr_param_config);


#ifdef __cplusplus
}
#endif

#endif /* __MJD_KY032_H__ */
