/*
 *
 */
#ifndef __MJD_MQTT_H__
#define __MJD_MQTT_H__

#ifdef __cplusplus
extern "C" {
#endif



// Includes
#include "esp_mqtt.h"

// Defines
#define MJD_MQTT_LOG_MQTT_PUBLISH (false)
#define MJD_MQTT_MAX_PUBLISH_ATTEMPTS (2)

// @doc https://github.com/mqttjs/MQTT.js#qos
// Here is how QoS works:
//      QoS 0 : received at most once : The packet is sent, and that's it. There is no validation about whether it has been received.
//      QoS 1 : received at least once : The packet is sent and stored as long as the client has not received a confirmation from the server.
//              MQTT ensures that it will be received, but there can be duplicates.
//      QoS 2 : received exactly once : Same as QoS 1 and guaranteed no duplicates.
#define MJD_MQTT_QOS_0 (0)
#define MJD_MQTT_QOS_1 (1)

// TBD Constants

// Typedefs

// Function Declarations
esp_err_t mjd_mqtt_init(size_t buffer_size, int command_timeout);
esp_err_t mjd_mqtt_start(const char *host, const char *port, const char *client_id, const char *username, const char *password);
esp_err_t mjd_mqtt_publish(const char *topic, uint8_t *payload, size_t len, int qos, bool retained);
esp_err_t mjd_mqtt_stop();



#ifdef __cplusplus
}
#endif

#endif /* __MJD_MQTT_H__ */
