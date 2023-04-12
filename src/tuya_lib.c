#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>

#include "cJSON.h"
#include "tuya_cacert.h"
#include "tuya_error_code.h"
#include "system_interface.h"
#include "mqtt_client_interface.h"
#include "tuyalink_core.h"
#include "tuya_lib.h"
#include "tuyarepd.h"

#define MESSAGE_LEN_LIMIT 1000

FILE *cmessages;

void on_connected(tuya_mqtt_context_t* context, void* user_data)
{
    char *value = (char *) user_data;
    char *key = "funkcija";
    char payload[1000];

	snprintf(payload, sizeof(payload), "{\"%s\": \"%s\"}", key, value);
    syslog(LOG_INFO, "connected to cloud");
    tuyalink_thing_property_report_with_ack(context, NULL, payload);
}

void on_disconnect(tuya_mqtt_context_t* context, void* user_data)
{
    syslog(LOG_INFO, "disconnected from the cloud");
}

void on_messages(tuya_mqtt_context_t* context, void* user_data, const tuyalink_message_t* msg)
{
	int wrlen;

    syslog(LOG_INFO, "on message id:%s, type:%d, code:%d", msg->msgid, msg->type, msg->code);
    switch (msg->type) {
        case THING_TYPE_MODEL_RSP:
            syslog(LOG_INFO, "Model data:%s", msg->data_string);
            break;

        case THING_TYPE_PROPERTY_SET:
		wrlen = fwrite(msg->data_string, sizeof(char), strnlen(msg->data_string, MESSAGE_LEN_LIMIT), cmessages);
            syslog(LOG_INFO, "property set:%s, fp: %ld, wrlen: %d", msg->data_string, (long) cmessages, wrlen);
            break;

        case THING_TYPE_PROPERTY_REPORT_RSP:
            break;

        default:
            break;
    }
}

int communicate_with_cloud(const char *deviceId, const char *deviceSecret, char *message)
{
    int ret = OPRT_OK;
	cmessages = fopen("/home/virgis/dev/teltonika/part4/src/cloud_messages", "a");
	syslog(LOG_INFO, "cmessages: %ld", (long) cmessages);
   
	tuya_mqtt_context_t client;

    ret = tuya_mqtt_init(&client, &(const tuya_mqtt_config_t) {
        .host = "m1.tuyacn.com",
        .port = 8883,
        .cacert = tuya_cacert_pem,
        .cacert_len = sizeof(tuya_cacert_pem),
        .device_id = deviceId,
        .device_secret = deviceSecret,
        .keepalive = 100,
        .timeout_ms = 2000,
        .on_connected = on_connected,
        .on_disconnect = on_disconnect,
        .on_messages = on_messages,
    });
    assert(ret == OPRT_OK);

	client.user_data = (void *) message;

    ret = tuya_mqtt_connect(&client);
    assert(ret == OPRT_OK);

    while(!exit_trigger) {
        tuya_mqtt_loop(&client);
    }

    fclose(cmessages);
    tuya_mqtt_disconnect(&client);
    tuya_mqtt_deinit(&client);

    return ret;
}

