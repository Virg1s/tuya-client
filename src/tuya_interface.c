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

int send_msg_to_cloud(tuya_mqtt_context_t *context, char *key, char *value)
{
	char payload[MESSAGE_LEN_LIMIT];

	snprintf(payload, sizeof(payload), "{\"%s\": \"%s\"}", key, value);

	log_function(LOG_INFO, "sending message to the cloud: {\"%s\": \"%s\"}", key, value);

	tuyalink_thing_property_report_with_ack(context, NULL, payload);

	return 0;
}

void on_connected(tuya_mqtt_context_t *context, void *user_data)
{
	/*
	char *value = (char *)user_data;
	char *key = "funkcija";
	char payload[1000];

	log_function(LOG_INFO, "connected to cloud");

	snprintf(payload, sizeof(payload), "{\"%s\": \"%s\"}", key, value);
	tuyalink_thing_property_report_with_ack(context, NULL, payload);
	*/

	send_msg_to_cloud(context, "funkcija", (char *) user_data);
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
	case THING_TYPE_PROPERTY_SET:
		wrlen = fwrite(msg->data_string, sizeof(char),
			       strnlen(msg->data_string, MESSAGE_LEN_LIMIT),
			       cmessages);
		if (wrlen) {
			log_function(LOG_INFO, "property set:%s", msg->data_string);
		} else {
			log_function(LOG_INFO, "failed to set property:%s", msg->data_string);
		}

		break;
	default:
		break;
	}
}

int init_resources(tuya_mqtt_context_t *client_p, const char *deviceId, const char *deviceSecret, char *message)
{
	int ret;


	cmessages = fopen(CLOUD_MESSAGE_LOG, "a");

	if (cmessages == NULL)
		return -1;

	ret = tuya_mqtt_init(client_p,
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
		return -1;

	client_p->user_data = (void *) message;

	ret = tuya_mqtt_connect(client_p);

	if (ret != OPRT_OK)
		return -1;

	return 0;
}

void deinit_resources(tuya_mqtt_context_t *client_p)
{
	tuya_mqtt_deinit(client_p);
	fclose(cmessages);
}

int communicate_with_cloud(const char *deviceId, const char *deviceSecret, char *message)
{
	int ret = 0;
	tuya_mqtt_context_t client;

	ret = init_resources(&client, deviceId, deviceSecret, message); 

	if (ret) {
		deinit_resources(&client);
		return ret;
	}

	while (!exit_trigger && ret == OPRT_OK) {
		send_msg_to_cloud(&client, "funkcija", "labassssssssssss");
		ret = tuya_mqtt_loop(&client);
	}

	tuya_mqtt_disconnect(&client);

	deinit_resources(&client);

	return 0;
}
