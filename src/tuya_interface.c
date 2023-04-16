#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "cJSON.h"
#include "tuya_cacert.h"
#include "tuya_error_code.h"
#include "system_interface.h"
#include "mqtt_client_interface.h"
#include "tuyalink_core.h"
#include "tuya_interface.h"
#include "tuyarepd.h"

#define MESSAGE_LEN_LIMIT 1000
#define CLOUD_MESSAGE_LOG "/home/virgis/dev/teltonika/part4/src/cloud_messages"

FILE *cmessages;

void on_connected(tuya_mqtt_context_t *context, void *user_data)
{
	char *value = (char *)user_data;
	char *key = "funkcija";
	char payload[1000];

	snprintf(payload, sizeof(payload), "{\"%s\": \"%s\"}", key, value);
	log_function(LOG_INFO, "connected to cloud");
	tuyalink_thing_property_report_with_ack(context, NULL, payload);
}

void on_messages(tuya_mqtt_context_t *context, void *user_data,
		 const tuyalink_message_t *msg)
{
	(void)context;
	(void)user_data;

	int wrlen;

	log_function(LOG_INFO, "on message id:%s, type:%d, code:%d", msg->msgid,
	       msg->type, msg->code);
	switch (msg->type) {
	case THING_TYPE_MODEL_RSP:
		log_function(LOG_INFO, "Model data:%s", msg->data_string);
		break;

	case THING_TYPE_PROPERTY_SET:
		wrlen = fwrite(msg->data_string, sizeof(char),
			       strnlen(msg->data_string, MESSAGE_LEN_LIMIT),
			       cmessages);
		log_function(LOG_INFO, "property set:%s, fp: %ld, wrlen: %d",
		       msg->data_string, (long)cmessages, wrlen);
		break;

	case THING_TYPE_PROPERTY_REPORT_RSP:
		break;

	default:
		break;
	}
}

int communicate_with_cloud(const char *deviceId, const char *deviceSecret,
			   char *message)
{
	int ret = OPRT_OK;

	cmessages = fopen(CLOUD_MESSAGE_LOG, "a");

	tuya_mqtt_context_t client;

	ret = tuya_mqtt_init(&client,
			     &(const tuya_mqtt_config_t){
				     .host = "m1.tuyacn.com",
				     .port = 8883,
				     .cacert = tuya_cacert_pem,
				     .cacert_len = sizeof(tuya_cacert_pem),
				     .device_id = deviceId,
				     .device_secret = deviceSecret,
				     .keepalive = 100,
				     .timeout_ms = 2000,
				     .on_connected = on_connected,
				     .on_messages = on_messages,
			     });
	if (ret != OPRT_OK)
		goto cleanup;

	client.user_data = (void *)message;

	ret = tuya_mqtt_connect(&client);

	if (ret != OPRT_OK)
		goto cleanup;

	while (!exit_trigger) {
		tuya_mqtt_loop(&client);
	}

	tuya_mqtt_disconnect(&client);

	cleanup: 
	tuya_mqtt_deinit(&client);
	fclose(cmessages);

	return ret;
}
