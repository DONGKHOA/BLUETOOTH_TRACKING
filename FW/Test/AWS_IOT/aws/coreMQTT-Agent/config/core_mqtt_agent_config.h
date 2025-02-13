/* Wait only a short interval each iteration for faster
 * processing of incoming data. */
#define MQTT_AGENT_MAX_EVENT_QUEUE_WAIT_TIME       ( 10U )

/* We will be downloading the OTA image one block at a time.
 * Limit the number of outstanding ACK we need. */
#define MQTT_AGENT_MAX_OUTSTANDING_ACKS            ( 10U )