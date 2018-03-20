#include "tc_iot_device_config.h"
#include "tc_iot_device_logic.h"
#include "tc_iot_export.h"


/* anis color control codes */
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_256_FORMAT   "\x1b[38;5;%dm"

int run_shadow(tc_iot_shadow_config * p_client_config);
void parse_command(tc_iot_mqtt_client_config * config, int argc, char ** argv) ;
void get_message_ack_callback(tc_iot_command_ack_status_e ack_status, tc_iot_message_data * md , void * session_context);
extern tc_iot_shadow_config g_client_config;

#define TC_IOT_TROUBLE_SHOOTING_URL "https://git.io/vN9le"

/* 循环退出标识 */
volatile int stop = 0;
void sig_handler(int sig) {
    if (sig == SIGINT) {
        tc_iot_hal_printf("SIGINT received, going down.\n");
        stop ++;
    } else if (sig == SIGTERM) {
        tc_iot_hal_printf("SIGTERM received, going down.\n");
        stop ++;
    } else {
        tc_iot_hal_printf("signal received:%d\n", sig);
    }
    if (stop >= 3) {
        tc_iot_hal_printf("SIGINT/SIGTERM received over %d times, force shutdown now.\n", stop);
        exit(0);
    }
}


/**
 * @brief operate_light 操作灯光控制开关
 *
 * @param light 灯状态数据
 */
void operate_device(tc_iot_shadow_local_data * light) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    int color = light->color % 3;
    const char * ansi_color = NULL;
    const char * ansi_color_name = NULL;
    static const char * brightness_bar      = "||||||||||||||||||||";
    static const char * brightness_bar_left = "--------------------";
    int brightness_bar_len = strlen(brightness_bar);
    int brightness_bar_left_len = 0;

    switch(light->color) {
        case TC_IOT_PROP_color_red:
            ansi_color = ANSI_COLOR_RED;
            ansi_color_name = "RED";
            break;
        case TC_IOT_PROP_color_green:
            ansi_color = ANSI_COLOR_GREEN;
            ansi_color_name = "GREEN";
            break;
        case TC_IOT_PROP_color_blue:
            ansi_color = ANSI_COLOR_BLUE;
            ansi_color_name = "BLUE";
            break;
        default:
            ansi_color = ANSI_COLOR_YELLOW;
            ansi_color_name = "UNKNOWN";
            break;
    }

    /* 灯光亮度显示条 */
    brightness_bar_len = light->brightness >= 100?brightness_bar_len:(int)((light->brightness * brightness_bar_len)/100);
    brightness_bar_left_len = strlen(brightness_bar) - brightness_bar_len;

    if (light->device_switch) {
        /* 灯光开启式，按照控制参数展示 */
        tc_iot_hal_printf( "%s%04d-%02d-%02d %02d:%02d:%02d " ANSI_COLOR_RESET, 
                ansi_color,
                tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        tc_iot_hal_printf(
                "%s[  lighting  ]|[color:%s]|[brightness:%.*s%.*s]\n" 
                ANSI_COLOR_RESET, 
                ansi_color,
                ansi_color_name,
                brightness_bar_len, brightness_bar,
                brightness_bar_left_len,brightness_bar_left
                );
    } else {
        /* 灯处于关闭状态时的展示 */
        tc_iot_hal_printf( ANSI_COLOR_YELLOW "%04d-%02d-%02d %02d:%02d:%02d " ANSI_COLOR_RESET, 
                tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        tc_iot_hal_printf(
                ANSI_COLOR_YELLOW "[" "light is off" "]|[color:%s]|[brightness:%.*s%.*s]\n" ANSI_COLOR_RESET , 
                ansi_color_name,
                brightness_bar_len, brightness_bar,
                brightness_bar_left_len, brightness_bar_left
                );
    }
}

int main(int argc, char** argv) {
    tc_iot_mqtt_client_config * p_client_config;
    bool token_defined;
    int ret;
    long timestamp = tc_iot_hal_timestamp(NULL);
    tc_iot_hal_srandom(timestamp);
    long nonce = tc_iot_hal_random();

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    p_client_config = &(g_client_config.mqtt_client_config);

    /* 解析命令行参数 */
    parse_command(p_client_config, argc, argv);

    /* 根据 product id 和device name 定义，生成发布和订阅的 Topic 名称。 */
    snprintf(g_client_config.sub_topic,TC_IOT_MAX_MQTT_TOPIC_LEN, TC_IOT_SUB_TOPIC_FMT, 
            p_client_config->device_info.product_id,p_client_config->device_info.device_name);
    snprintf(g_client_config.pub_topic,TC_IOT_MAX_MQTT_TOPIC_LEN, TC_IOT_PUB_TOPIC_FMT, 
            p_client_config->device_info.product_id,p_client_config->device_info.device_name);

    /* 判断是否需要获取动态 token */
    token_defined = strlen(p_client_config->device_info.username) && strlen(p_client_config->device_info.password);

    if (!token_defined) {
        /* 获取动态 token */
        tc_iot_hal_printf("requesting username and password for mqtt.\n");
        ret = http_refresh_auth_token_with_expire(
                TC_IOT_CONFIG_AUTH_API_URL, NULL,
                timestamp, nonce,
                &p_client_config->device_info, TC_IOT_TOKEN_MAX_EXPIRE_SECOND);
        if (ret != TC_IOT_SUCCESS) {
            tc_iot_hal_printf("refresh token failed, trouble shooting guide: " "%s#%d\n", TC_IOT_TROUBLE_SHOOTING_URL, ret);
            return 0;
        }
        tc_iot_hal_printf("request username and password for mqtt success.\n");
    } else {
        tc_iot_hal_printf("username & password using: %s %s\n", p_client_config->device_info.username, p_client_config->device_info.password);
    }

    run_shadow(&g_client_config);

    return 0;
}

/* 影子数据 Client  */
extern tc_iot_shadow_client g_tc_iot_shadow_client;

int run_shadow(tc_iot_shadow_config * p_client_config) {
    int timeout = 500;
    int ret = 0;
    char buffer[512];
    int buffer_len = sizeof(buffer);
    tc_iot_shadow_client* p_shadow_client = &g_tc_iot_shadow_client;

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
    ret = tc_iot_shadow_get(p_shadow_client, buffer, buffer_len, get_message_ack_callback, 
            TC_IOT_CONFIG_COMMAND_TIMEOUT_MS, p_shadow_client);
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
