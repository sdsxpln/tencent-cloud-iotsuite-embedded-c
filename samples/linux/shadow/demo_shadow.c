#include "tc_iot_device_config.h"
#include "tc_iot_export.h"

static volatile int stop = 0;

void sig_handler(int sig) {
    if (sig == SIGINT) {
        tc_iot_hal_printf("SIGINT received, going down.\n");
        stop = 1;
    } else if (sig == SIGTERM) {
        tc_iot_hal_printf("SIGTERM received, going down.\n");
        stop = 1;
    } else {
        tc_iot_hal_printf("signal received:%d\n", sig);
    }
}

void _on_message_received(tc_iot_message_data* md) {
    tc_iot_mqtt_message* message = md->message;
    /* tc_iot_hal_printf("->%.*s\t", md->topicName->lenstring.len, */
           /* md->topicName->lenstring.data); */
    tc_iot_hal_printf("[s->c] %.*s\n", (int)message->payloadlen, (char*)message->payload);
}

tc_iot_shadow_config g_client_config = {
    {
        {
            // device info
            TC_IOT_CONFIG_DEVICE_SECRET, TC_IOT_CONFIG_DEVICE_PRODUCT_ID,
            TC_IOT_CONFIG_DEVICE_NAME, TC_IOT_CONFIG_DEVICE_CLIENT_ID,
            TC_IOT_CONFIG_DEVICE_USER_NAME, TC_IOT_CONFIG_DEVICE_PASSWORD, 0,
        },
        TC_IOT_CONFIG_SERVER_HOST,
        TC_IOT_CONFIG_SERVER_PORT,
        TC_IOT_CONFIG_COMMAND_TIMEOUT_MS,
        TC_IOT_CONFIG_KEEP_ALIVE_INTERVAL_SEC,
        TC_IOT_CONFIG_CLEAN_SESSION,
        TC_IOT_CONFIG_USE_TLS,
        TC_IOT_CONFIG_AUTO_RECONNECT,
        TC_IOT_CONFIG_ROOT_CA,
        TC_IOT_CONFIG_CLIENT_CRT,
        TC_IOT_CONFIG_CLIENT_KEY,
        NULL,
        NULL,
    },
    SUB_TOPIC,
    PUB_TOPIC,
    _on_message_received,
};

tc_iot_shadow_client client;

int main(int argc, char** argv) {
    tc_iot_shadow_client* p_shadow_client = &client;
    int ret = 0;
    char buffer[512];
    int buffer_len = sizeof(buffer);
    char reported[256];
    char desired[256];

    tc_iot_hal_printf("requesting username and password for mqtt.\n");
    ret = http_refresh_auth_token(
        TC_IOT_CONFIG_AUTH_API_URL, NULL,
        &g_client_config.mqtt_client_config.device_info);
    if (ret != TC_IOT_SUCCESS) {
        tc_iot_hal_printf("refresh token failed, visit: https://github.com/tencentyun/tencent-cloud-iotsuite-embedded-c/wiki/trouble_shooting#%d\n.", ret);
        return 0;
    }
    tc_iot_hal_printf("request username and password for mqtt success.\n");

    tc_iot_hal_printf("constructing mqtt shadow client.\n");
    ret = tc_iot_shadow_construct(p_shadow_client, &g_client_config);
    if (ret != TC_IOT_SUCCESS) {
        tc_iot_hal_printf("construct shadow failed, visit: https://github.com/tencentyun/tencent-cloud-iotsuite-embedded-c/wiki/trouble_shooting#%d\n.", ret);
        return 0;
    }

    tc_iot_hal_printf("construct mqtt shadow client success.\n");
    int timeout = TC_IOT_CONFIG_COMMAND_TIMEOUT_MS;
    tc_iot_hal_printf("yield waiting for server push.\n");
    tc_iot_shadow_yield(p_shadow_client, timeout);
    tc_iot_hal_printf("yield waiting for server finished.\n");

    tc_iot_hal_printf("[c->s] shadow_get\n");
    tc_iot_shadow_get(p_shadow_client);
    tc_iot_shadow_yield(p_shadow_client, timeout);

    /* snprintf(reported, sizeof(reported),  */
            /* "{\"string\":\"%s\",\"number\":%d,\"double\":%f,\"bool\":%s,\"obj\":%s}", */
            /* tc_iot_json_inline_escape(buffer, buffer_len, "A string \"\r\n"), */
            /* 12345,3.14159, TC_IOT_JSON_TRUE, TC_IOT_JSON_NULL); */
    /* snprintf(desired, sizeof(desired),  */
            /* "{\"string\":\"%s\",\"number\":%d,\"double\":%f,\"bool\":%s,\"obj\":%s}", */
            /* tc_iot_json_inline_escape(buffer, buffer_len, "Hello, world!"), */
            /* 700,10000.1234, TC_IOT_JSON_FALSE, TC_IOT_JSON_NULL); */

    snprintf(reported, sizeof(reported), 
            "{\"string\":\"%s\"}",
            tc_iot_json_inline_escape(buffer, buffer_len, "A string")
            );
    snprintf(desired, sizeof(desired), 
            "{\"string\":\"%s\"}",
            tc_iot_json_inline_escape(buffer, buffer_len, "A string")
            );

    
    ret =  tc_iot_shadow_doc_pack_for_update(buffer, buffer_len, p_shadow_client, reported, desired);
    tc_iot_hal_printf("[c->s] shadow_update_all\n%s\n", buffer);
    tc_iot_shadow_update(p_shadow_client, buffer);
    tc_iot_shadow_yield(p_shadow_client, timeout);

    ret =  tc_iot_shadow_doc_pack_for_update(buffer, buffer_len, p_shadow_client, reported, NULL);
    tc_iot_hal_printf("[c->s] shadow_update_reported\n%s\n", buffer);
    tc_iot_shadow_update(p_shadow_client, buffer);
    tc_iot_shadow_yield(p_shadow_client, timeout);

    ret =  tc_iot_shadow_doc_pack_for_update(buffer, buffer_len, p_shadow_client, NULL, desired);
    tc_iot_hal_printf("[c->s] shadow_update_desired\n%s\n", buffer);
    tc_iot_shadow_update(p_shadow_client, buffer);
    tc_iot_shadow_yield(p_shadow_client, timeout);

    ret =  tc_iot_shadow_doc_pack_for_delete(buffer, buffer_len, p_shadow_client, TC_IOT_JSON_NULL, TC_IOT_JSON_NULL);
    tc_iot_hal_printf("[c->s] shadow_delete\n%s\n",buffer);
    tc_iot_shadow_delete(p_shadow_client, buffer);
    tc_iot_shadow_yield(p_shadow_client, timeout);

    tc_iot_hal_printf("Stopping\n");
    tc_iot_shadow_destroy(p_shadow_client);
    tc_iot_hal_printf("Exit success.\n");
    return 0;
}
