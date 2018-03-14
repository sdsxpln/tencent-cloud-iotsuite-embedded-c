#include "tc_iot_device_config.h"
#include "tc_iot_shadow_local_data.h"
#include "tc_iot_export.h"


/* anis color control codes */
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_256_FORMAT   "\x1b[38;5;%dm"


void _light_on_message_received(tc_iot_message_data* md);
int _tc_iot_sync_shadow_property(tc_iot_shadow_property properties[], const char * doc_start, jsmntok_t * json_token, int tok_count);


volatile int stop;

/* 设备初始配置 */
tc_iot_shadow_config g_client_config = {
    {
        {
            /* device info*/
            TC_IOT_CONFIG_DEVICE_SECRET, TC_IOT_CONFIG_DEVICE_PRODUCT_ID,
            TC_IOT_CONFIG_DEVICE_NAME, TC_IOT_CONFIG_DEVICE_CLIENT_ID,
            TC_IOT_CONFIG_DEVICE_USER_NAME, TC_IOT_CONFIG_DEVICE_PASSWORD, 0,
        },
        TC_IOT_CONFIG_SERVER_HOST,
        TC_IOT_CONFIG_SERVER_PORT,
        TC_IOT_CONFIG_COMMAND_TIMEOUT_MS,
        TC_IOT_CONFIG_TLS_HANDSHAKE_TIMEOUT_MS,
        TC_IOT_CONFIG_KEEP_ALIVE_INTERVAL_SEC,
        TC_IOT_CONFIG_CLEAN_SESSION,
        TC_IOT_CONFIG_USE_TLS,
        TC_IOT_CONFIG_AUTO_RECONNECT,
        TC_IOT_CONFIG_ROOT_CA,
        TC_IOT_CONFIG_CLIENT_CRT,
        TC_IOT_CONFIG_CLIENT_KEY,
        NULL,
        NULL,
        0,  /* send will */
        { 
            {'M', 'Q', 'T', 'W'}, 0, {NULL, {0, NULL}}, {NULL, {0, NULL}}, 0, 0, 
        }
    },
    TC_IOT_SUB_TOPIC_DEF,
    TC_IOT_PUB_TOPIC_DEF,
    _light_on_message_received,
};

/* 设备状态数据 */
static tc_iot_shadow_control g_device_vars = 
{
    /* current  */
    {
        false,  /* 开关状态 */
        "colorful light", /* 设备标识 */
        2, /* 设备光颜色控制 */
        100, /* 亮度 */
    },
    /* reported data */
    {
        false,  /* 开关状态 */
        "", /* 设备标识 */
        0, /* 设备光颜色控制 */
        0, /* 亮度 */
    }
}
;

typedef enum _tc_iot_shadow_property_index_e {
    TC_IOT_PROP_light_switch = 0,
    TC_IOT_PROP_name = 1,
    TC_IOT_PROP_color = 2,
    TC_IOT_PROP_brightness = 3,
    TC_IOT_PROP_MAX,
} tc_iot_shadow_property_index_e;

void _tc_iot_shadow_property_control_callback(void * p_prop);

#define DECLARE_PROPERTY_DEF(var, name, shadow_type, callback) {#name, TC_IOT_PROP_ ## name, sizeof(var), shadow_type, &var, callback}

static tc_iot_shadow_property g_device_property_defs[] = {
    DECLARE_PROPERTY_DEF(g_device_vars.current.light_switch, light_switch, TC_IOT_SHADOW_TYPE_BOOL, _tc_iot_shadow_property_control_callback),
    DECLARE_PROPERTY_DEF(g_device_vars.current.name, name, TC_IOT_SHADOW_TYPE_STRING, _tc_iot_shadow_property_control_callback),
    DECLARE_PROPERTY_DEF(g_device_vars.current.color, color, TC_IOT_SHADOW_TYPE_NUMBER, _tc_iot_shadow_property_control_callback),
    DECLARE_PROPERTY_DEF(g_device_vars.current.brightness, brightness, TC_IOT_SHADOW_TYPE_NUMBER, _tc_iot_shadow_property_control_callback),
};

static tc_iot_shadow_property g_device_reported_property_defs[] = {
    DECLARE_PROPERTY_DEF(g_device_vars.reported.light_switch, light_switch, TC_IOT_SHADOW_TYPE_BOOL, NULL),
    DECLARE_PROPERTY_DEF(g_device_vars.reported.name, name, TC_IOT_SHADOW_TYPE_STRING, NULL),
    DECLARE_PROPERTY_DEF(g_device_vars.reported.color, color, TC_IOT_SHADOW_TYPE_NUMBER, NULL),
    DECLARE_PROPERTY_DEF(g_device_vars.reported.brightness, brightness, TC_IOT_SHADOW_TYPE_NUMBER, NULL),
};

/* 影子数据 Client  */
tc_iot_shadow_client client;

void _tc_iot_shadow_property_control_callback(void * p_prop) {
    tc_iot_shadow_property * p_property = (tc_iot_shadow_property *) p_prop;
    if (!p_property) {
        return;
    }
    switch (p_property->id) {
        case TC_IOT_PROP_light_switch:
            LOG_TRACE("do something for light_switch change");
            break;
        case TC_IOT_PROP_name:
            LOG_TRACE("do something for name change");
            break;
        case TC_IOT_PROP_color:
            LOG_TRACE("do something for color change");
            break;
        case TC_IOT_PROP_brightness:
            LOG_TRACE("do something for brightness change");
            break;
        default:
            LOG_WARN("unkown property id = %d", p_property->id);
            break;
    }
}

/**
 * @brief get_message_ack_callback shadow_get 回调函数
 *
 * @param ack_status 回调状态，标识消息是否正常收到响应，还是已经超时等。
 * @param md 回调状态为 TC_IOT_ACK_SUCCESS 时，用来传递影子数据请求响应消息。
 * @param session_context 回调 context。
 */
void get_message_ack_callback(tc_iot_command_ack_status_e ack_status, 
        tc_iot_message_data * md , void * session_context) {

    tc_iot_mqtt_message* message = NULL;

    if (ack_status != TC_IOT_ACK_SUCCESS) {
        if (ack_status == TC_IOT_ACK_TIMEOUT) {
            LOG_ERROR("request timeout");
        }
        return;
    }

    message = md->message;
    _light_on_message_received(md);
}

/**
 * @brief report_message_ack_callback shadow_update 上报消息回调
 *
 * @param ack_status 回调状态，标识消息是否正常收到响应，还是已经超时等。
 * @param md 回调状态为 TC_IOT_ACK_SUCCESS 时，用来传递影子数据请求响应消息。
 * @param session_context 回调 context。
 */
void report_message_ack_callback(tc_iot_command_ack_status_e ack_status, 
        tc_iot_message_data * md , void * session_context) {
    tc_iot_mqtt_message* message = NULL;

    if (ack_status != TC_IOT_ACK_SUCCESS) {
        if (ack_status == TC_IOT_ACK_TIMEOUT) {
            LOG_ERROR("request timeout");
        }
        return;
    }

    message = md->message;
    LOG_TRACE("[s->c] %.*s", (int)message->payloadlen, (char*)message->payload);
}


/**
 * @brief operate_light 操作设备光控制开关
 *
 * @param light 设备状态数据
 */
static void operate_light(tc_iot_shadow_local_data * light) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    int color = light->color & 0xFF;
    static const char * brightness_bar      = "||||||||||||||||||||";
    static const char * brightness_bar_left = "--------------------";
    int brightness_bar_len = strlen(brightness_bar);
    int brightness_bar_left_len = 0;

    /* 设备光亮度显示条 */
    brightness_bar_len = light->brightness >= 100?brightness_bar_len:(int)((light->brightness/100) * brightness_bar_len);
    brightness_bar_left_len = strlen(brightness_bar) - brightness_bar_len;

    if (light->light_switch) {
        /* 设备光开启式，按照控制参数展示 */
        tc_iot_hal_printf( ANSI_COLOR_256_FORMAT "%04d-%02d-%02d %02d:%02d:%02d " ANSI_COLOR_RESET, 
                color,
                tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        tc_iot_hal_printf(
                ANSI_COLOR_256_FORMAT 
                "[%s]|[  lighting  ]|[color:%03d]|[brightness:%.*s%.*s]\n" 
                ANSI_COLOR_RESET, 
                color,
                light->name,
                color,
                brightness_bar_len, brightness_bar,
                brightness_bar_left_len,brightness_bar_left
                );
    } else {
        /* 设备处于关闭状态时的展示 */
        tc_iot_hal_printf( ANSI_COLOR_RED "%04d-%02d-%02d %02d:%02d:%02d " ANSI_COLOR_RESET, 
                tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        tc_iot_hal_printf(
                ANSI_COLOR_RED "[%s]|[" "light is off" "]|[color:%03d]|[brightness:%.*s%.*s]\n" ANSI_COLOR_RESET , 
                light->name,
                color,
                brightness_bar_len, brightness_bar,
                brightness_bar_left_len, brightness_bar_left
                );
    }
}


/**
 * @brief report_light 上报设备数据，用于初始状态上报
 *
 * @param p_shadow_client shadow 控制句柄
 * @param light 设备状态数据
 */
static void report_light(tc_iot_shadow_client * p_shadow_client, tc_iot_shadow_local_data * light) {
    char buffer[512];
    int buffer_len = sizeof(buffer);
    char reported[256];

    snprintf(reported, sizeof(reported), 
            "{\"name\":\"%s\",\"color\":%d,\"brightness\":%d,\"light_switch\":%s}",
            tc_iot_json_inline_escape(buffer, buffer_len, g_device_vars.current.name),
            g_device_vars.current.color,g_device_vars.current.brightness,
            g_device_vars.current.light_switch?TC_IOT_JSON_TRUE:TC_IOT_JSON_FALSE);
    /* 此时由于 desired 状态未知，仅上报 reported ，不处理 desired 。 */
    tc_iot_shadow_update(p_shadow_client, buffer, buffer_len, reported, NULL, report_message_ack_callback, TC_IOT_CONFIG_COMMAND_TIMEOUT_MS, NULL);
    LOG_TRACE("[c->s] shadow_update_reported\n%s", buffer);
}


/**
 * @brief desired_light 接收 control 指令，对设备进行状态变更后，上报数据。
 * reported 状态此时已经和 desired 状态一直，同时上报 "desired":null，
 * 要求服务端清除 desired 状态数据，避免重复下发指令。
 *
 * @param p_shadow_client shadow 控制句柄
 * @param light 设备状态数据
 */
static void desired_light(tc_iot_shadow_client * p_shadow_client, tc_iot_shadow_local_data * light) {
    char buffer[512];
    int buffer_len = sizeof(buffer);
    char reported[256];

    snprintf(reported, sizeof(reported), 
            "{\"name\":\"%s\",\"color\":%d,\"brightness\":%d,\"light_switch\":%s}",
            tc_iot_json_inline_escape(buffer, buffer_len, g_device_vars.current.name),
            g_device_vars.current.color,g_device_vars.current.brightness,
            g_device_vars.current.light_switch?TC_IOT_JSON_TRUE:TC_IOT_JSON_FALSE);

    tc_iot_shadow_update(p_shadow_client, buffer, buffer_len, reported, TC_IOT_JSON_NULL, report_message_ack_callback, TC_IOT_CONFIG_COMMAND_TIMEOUT_MS, NULL);
    LOG_TRACE("[c->s] shadow_report_and_clean_desired\n%s", buffer);
}


/**
 * @brief _tc_iot_sync_shadow_property 根据服务端下发的影子数据，同步到本地设备状态数据，并进
 * 行上报。
 *
 * @param light_state 设备状态数据
 * @param doc_start reported or desired 数据起始位置。
 * @param json_token json token 数组起始位置
 * @param tok_count 有效 json token 数量
 *
 * @return 
 */
int _tc_iot_sync_shadow_property(tc_iot_shadow_property properties[], const char * doc_start, jsmntok_t * json_token, int tok_count) {
    int i,j;
    jsmntok_t  * key_tok = NULL;
    jsmntok_t  * val_tok = NULL;
    char field_buf[TC_IOT_LIGHT_NAME_LEN+1];
    int field_len = sizeof(field_buf);
    int new_number = 0;
    bool new_bool = 0;
    int  key_len = 0, val_len = 0;
    const char * key_start;
    const char * val_start;
    tc_iot_shadow_property * p_prop = NULL;

    if (!properties) {
        LOG_ERROR("light_state is null");
        return TC_IOT_NULL_POINTER;
    }

    if (!doc_start) {
        LOG_ERROR("doc_start is null");
        return TC_IOT_NULL_POINTER;
    }

    if (!json_token) {
        LOG_ERROR("json_token is null");
        return TC_IOT_NULL_POINTER;
    }

    if (!tok_count) {
        LOG_ERROR("tok_count is invalid");
        return TC_IOT_INVALID_PARAMETER;
    }

    memset(field_buf, 0, sizeof(field_buf));

    for (i = 0; i < tok_count/2; i++) {
        /* 位置 0 是object对象，所以要从位置 1 开始取数据*/
        /* 2*i+1 为 key 字段，2*i + 2 为 value 字段*/
        key_tok = &(json_token[2*i + 1]);
        key_start = doc_start + key_tok->start;
        key_len = key_tok->end - key_tok->start;

        val_tok = &(json_token[2*i + 2]);
        val_start = doc_start + val_tok->start;
        val_len = val_tok->end - val_tok->start;
        for(j = 0; j < TC_IOT_PROP_MAX; j++) {
            p_prop = &properties[j];
            if (!p_prop->ptr) {
                LOG_ERROR("No.%d property ptr invalid.", j);
                continue;
            }

            if (strncmp(p_prop->name, key_start, key_len) == 0)  {
                if (p_prop->type == TC_IOT_SHADOW_TYPE_STRING) {
                    if (val_len > p_prop->len) {
                        LOG_ERROR("%s[%.*s] to long, skip this settings", 
                                p_prop->name,val_len, val_start);
                        continue;
                    }
                    strncpy(field_buf, val_start, val_len);
                    field_buf[val_len] = '\0';
                    if (strncmp(p_prop->ptr, field_buf, val_len) != 0) {
                        LOG_TRACE("state change:[%s|%s -> %s]", p_prop->name, (const char *) p_prop->ptr, field_buf);
                        strncpy(p_prop->ptr, field_buf, val_len + 1);
                    }
                } else if (p_prop->type == TC_IOT_SHADOW_TYPE_BOOL) {
                    strncpy(field_buf, val_start, val_len);
                    field_buf[val_len] = '\0';
                    new_bool = (strncmp(TC_IOT_JSON_TRUE, field_buf, val_len) == 0);
                    if (new_bool != (*(bool *)p_prop->ptr)) {
                        LOG_TRACE("state change:[%s|%s -> %s]", p_prop->name, (*(bool *) p_prop->ptr)?"true":"false", field_buf);
                        *(bool *)p_prop->ptr = new_bool;
                    }
                } else if (p_prop->type == TC_IOT_SHADOW_TYPE_NUMBER) {
                    strncpy(field_buf, val_start, val_len);
                    field_buf[val_len] = '\0';
                    new_number = atoi(field_buf);
                    if (new_number != (*(unsigned int *)p_prop->ptr)) {
                        LOG_TRACE("state change:[%s|%d -> %s]", p_prop->name, (*( unsigned int *) p_prop->ptr), field_buf);
                        *(unsigned int *)p_prop->ptr = new_number;
                    }
                } else {
                    LOG_ERROR("%s type=%d invalid.", p_prop->name, p_prop->type);
                    continue;
                }
                if (p_prop->fn_change_notify) {
                    p_prop->fn_change_notify(p_prop);
                }
            }
        }
    }
    return 0;
}



/**
 * @brief _light_on_message_received 数据回调，处理 shadow_get 获取最新状态，或
 * 者影子服务推送的最新控制指令数据。
 *
 * @param md 影子数据消息
 */
void _light_on_message_received(tc_iot_message_data* md) {
    jsmntok_t  json_token[TC_IOT_MAX_JSON_TOKEN_COUNT];
    char field_buf[TC_IOT_LIGHT_NAME_LEN+1];
    int field_index = 0;
    const char * reported_start = NULL;
    int reported_len = 0;
    const char * desired_start = NULL;
    int desired_len = 0;
    int ret = 0;

    memset(field_buf, 0, sizeof(field_buf));

    tc_iot_mqtt_message* message = md->message;
    LOG_TRACE("[s->c] %.*s", (int)message->payloadlen, (char*)message->payload);

    /* 有效性检查 */
    ret = tc_iot_json_parse(message->payload, message->payloadlen, json_token, TC_IOT_ARRAY_LENGTH(json_token));
    if (ret <= 0) {
        return ;
    }

    field_index = tc_iot_json_find_token((char*)message->payload, json_token, ret, "method", field_buf, sizeof(field_buf));
    if (field_index <= 0 ) {
        LOG_ERROR("field method not found in JSON: %.*s", (int)message->payloadlen, (char*)message->payload);
        return ;
    }

    if (strncmp("control", field_buf, strlen(field_buf)) == 0) {
        LOG_TRACE("Control data receved.");
    } else if (strncmp("reply", field_buf, strlen(field_buf)) == 0) {
        LOG_TRACE("Reply pack recevied.");
    }

    /* 检查 reported 字段是否存在 */
    field_index = tc_iot_json_find_token((char*)message->payload, json_token, ret, 
            "payload.state.reported", NULL, 0);
    if (field_index <= 0 ) {
        /* LOG_TRACE("payload.state.reported not found"); */
    } else {
        reported_start = message->payload + json_token[field_index].start;
        reported_len = json_token[field_index].end - json_token[field_index].start;
        LOG_TRACE("payload.state.reported found:%.*s", reported_len, reported_start);
    }

    /* 检查 desired 字段是否存在 */
    field_index = tc_iot_json_find_token((char*)message->payload, json_token, ret, 
            "payload.state.desired", NULL, 0);
    if (field_index <= 0 ) {
        /* LOG_TRACE("payload.state.desired not found"); */
    } else {
        desired_start = message->payload + json_token[field_index].start;
        desired_len = json_token[field_index].end - json_token[field_index].start;
        LOG_TRACE("payload.state.desired found:%.*s", desired_len, desired_start);
    }

    /* 如果设备无本地存储，则设备重启后可先同步之前上
     * 报的状态。
     *
     * 如果设备有本地存储，一般情况下，重启后本地状态还会
     * 和服务端一致。不一致时，以本地设备状态优先，还是
     * 以服务端优先，可根据实际业务情况进行区别处理。
     * */
    if (reported_start) {
        ret = tc_iot_json_parse(reported_start,reported_len, json_token, TC_IOT_ARRAY_LENGTH(json_token));
        if (ret <= 0) {
            return ;
        }
        LOG_TRACE("---synchronizing reported status---");
        _tc_iot_sync_shadow_property(g_device_reported_property_defs, reported_start, json_token, ret);
        LOG_TRACE("---synchronizing reported status success---");
    }

    /* 根据控制台或者 APP 端的指令，设定设备状态 */
    if (desired_start) {
        ret = tc_iot_json_parse(desired_start,desired_len, json_token, TC_IOT_ARRAY_LENGTH(json_token));
        if (ret <= 0) {
            return ;
        }
        LOG_TRACE("---synchronizing desired status---");
        _tc_iot_sync_shadow_property(g_device_property_defs, desired_start, json_token, ret);
        LOG_TRACE("---synchronizing desired status success---");
    }

    /* 根据最新状态数据，对设备进行控制：开关、改变颜色
     * 亮度等。*/
    operate_light(&g_device_vars.current);

    /* 如果状态发生变化，则更新设备状态 */
    if (reported_start || desired_start) {
        /* 上报最新的设备状态，并通知服务端去掉 
         * desired 数据，避免指令重复下发。 
         * */
        if (desired_start) {
            desired_light(&client, &g_device_vars.current);
        }
    } else {
        /* 服务端无 reported 和 desired 状态，
         * 说明设备首次启动或数据被重置，重新上报初始状态 */
        report_light(&client, &g_device_vars.current);
    }
}

/* 影子数据 Client  */
tc_iot_shadow_client client;

int run_shadow(tc_iot_shadow_config * p_client_config) {
    int timeout = 2000;
    int ret = 0;
    char buffer[512];
    int buffer_len = sizeof(buffer);
    tc_iot_shadow_client* p_shadow_client = &client;

    /* 初始化 shadow client */
    tc_iot_hal_printf("constructing mqtt shadow client.\n");
    ret = tc_iot_shadow_construct(p_shadow_client, p_client_config);
    if (ret != TC_IOT_SUCCESS) {
        tc_iot_hal_printf("construct shadow client failed, trouble shooting guide: " "%s#%d\n", TC_IOT_TROUBLE_SHOOTING_URL, ret);
        return 0;
    }

    tc_iot_hal_printf("construct mqtt shadow client success.\n");
    tc_iot_hal_printf("yield waiting for server push.\n");
    /* 执行 yield 收取影子服务端前序指令消息，清理历史状态。 */
    tc_iot_shadow_yield(p_shadow_client, timeout);
    tc_iot_hal_printf("yield waiting for server finished.\n");

    /* 通过get操作主动获取服务端影子设备状态，以便设备端同步更新至最新状态*/
    ret = tc_iot_shadow_get(p_shadow_client, buffer, buffer_len, get_message_ack_callback, TC_IOT_CONFIG_COMMAND_TIMEOUT_MS, NULL);
    LOG_TRACE("[c->s] shadow_get\n%s", buffer);

    /* 循环等待控制指令 */
    while (!stop) {
        tc_iot_shadow_yield(p_shadow_client, timeout);
    }

    tc_iot_hal_printf("Stopping\n");
    tc_iot_shadow_destroy(p_shadow_client);
    tc_iot_hal_printf("Exit success.\n");
    return 0;
}
