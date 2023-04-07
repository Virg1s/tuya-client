#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "cJSON.h"
#include "tuya_cacert.h"
#include "tuya_log.h"
#include "tuya_error_code.h"
#include "system_interface.h"
#include "mqtt_client_interface.h"
#include "tuyalink_core.h"
//#include "tuya_lib.c"

volatile sig_atomic_t exit_signal = 0;

void set_exit_signal(int signo)
{
	exit_signal;
}

void on_connected(tuya_mqtt_context_t* context, void* user_data)
{
    char *value = (char *) user_data;
    char *key = "funkcija";
    char payload[1000];

	snprintf(payload, sizeof(payload) - 50, "{\"%s\": \"%s\"}", key, value);
    TY_LOGI("on connected");
    tuyalink_thing_property_report_with_ack(context, NULL, payload);
}

void on_disconnect(tuya_mqtt_context_t* context, void* user_data)
{
    TY_LOGI("on disconnect");
}

void on_messages(tuya_mqtt_context_t* context, void* user_data, const tuyalink_message_t* msg)
{
    TY_LOGI("on message id:%s, type:%d, code:%d", msg->msgid, msg->type, msg->code);
    switch (msg->type) {
        case THING_TYPE_MODEL_RSP:
            TY_LOGI("Model data:%s", msg->data_string);
            break;

        case THING_TYPE_PROPERTY_SET:
            TY_LOGI("property set:%s", msg->data_string);
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

    for (;;) {
        tuya_mqtt_loop(&client);
    }

    return ret;
}
/*
int main(void)
{
	const char productId[] = "xa5ecaywubiym1bq";
	const char deviceId[] = "26bf9c459833b88e53mgqj";
	const char deviceSecret[] = "5ZUcwOQRDm3rzNUQ";

	communicate_with_cloud(deviceId, deviceSecret, "debug message");
}
*/
