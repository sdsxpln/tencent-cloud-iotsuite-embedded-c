#ifndef TC_IOT_DEVICE_CONFIG_H
#define TC_IOT_DEVICE_CONFIG_H

/* 服务端获取动态username和password接口*/
/* 地址格式为：<机房标识>.auth-device-iot.tencentcloudapi.com/device*/
/* 广州=gz*/
/* 北京=bj*/
/* ...*/
/* 以下为广州机房的接口地址：*/
#ifdef ENABLE_TLS
#define TC_IOT_CONFIG_AUTH_API_URL "https://gz.auth-device-iot.tencentcloudapi.com/device"
#else
#define TC_IOT_CONFIG_AUTH_API_URL "http://gz.auth-device-iot.tencentcloudapi.com/device"
#endif

/************************************************************************/
/**********************************必填项********************************/

#ifdef ENABLE_TLS
/* 是否启用TLS用于MQTT请求*/
#define TC_IOT_CONFIG_USE_TLS 1
#else
#define TC_IOT_CONFIG_USE_TLS 0
#endif

#if TC_IOT_CONFIG_USE_TLS
/* MQ服务的TLS端口一般为8883*/
#define TC_IOT_CONFIG_SERVER_PORT 8883
#else
/* MQ服务的默认端口一般为1883*/
#define TC_IOT_CONFIG_SERVER_PORT 1883
#endif


/* 以下配置需要先在官网创建产品和设备，然后获取相关信息更新*/
/* MQ服务地址，可以在产品“基本信息页”->“mqtt链接地址”位置找到。*/
#define TC_IOT_CONFIG_SERVER_HOST "mqtt-1e8w58ou4.ap-guangzhou.mqtt.tencentcloudmq.com"
/* 产品id，可以在产品“基本信息页”->“产品id”位置找到*/
#define TC_IOT_CONFIG_DEVICE_PRODUCT_ID "iot-fdxt0fie"

/* 设备密钥，可以在产品“设备管理”->“设备证书”->“Device Secret”位置找到*/
#define TC_IOT_CONFIG_DEVICE_SECRET "00000000000000000000000000000000"

/* 设备名称，可以在产品“设备管理”->“设备名称”位置找到*/
#define TC_IOT_CONFIG_DEVICE_NAME "device006"
/* client id，*/
/* 由两部分组成，组成形式为“Instanceid@DeviceID”，ClientID 的长度不超过 64个字符
 * */
/* ，请不要使用不可见字符。其中*/
/* Instanceid 为 IoT MQ 的实例 ID。*/
/* DeviceID 为每个设备独一无二的标识，由业务方自己指定，需保证全局唯一，例如每个
 * */
/* 传感器设备的序列号。*/
#define TC_IOT_CONFIG_DEVICE_CLIENT_ID "mqtt-1e8w58ou4@" TC_IOT_CONFIG_DEVICE_NAME

/************************************************************************/
/**********************************选填项********************************/
/* 关于username和password：*/
/* 1)如果是通过TC_IOT_CONFIG_AUTH_API_URL接口，动态获取的，以下两个参数可不用填写*/
/* 2)如果有预先申请好的固定username和password，可以把获取到的固定参数填写到如下位置*/
#define TC_IOT_CONFIG_DEVICE_USER_NAME ""
#define TC_IOT_CONFIG_DEVICE_PASSWORD ""
/************************************************************************/


/* connect、publish、subscribe、unsubscribe */
/* 等命令执行超时时长，单位是毫秒*/
#define TC_IOT_CONFIG_COMMAND_TIMEOUT_MS  10000

/* TLS 握手执行超时时长，单位是毫秒*/
#define TC_IOT_CONFIG_TLS_HANDSHAKE_TIMEOUT_MS  10000

/* keep alive 间隔时长，单位是秒*/
#define TC_IOT_CONFIG_KEEP_ALIVE_INTERVAL_SEC  60

/* 网络故障或服务端超时，是否自动重连*/
#define TC_IOT_CONFIG_AUTO_RECONNECT 1

#define TC_IOT_CONFIG_CLEAN_SESSION 0


/* shadow下行消息topic，mq服务端的响应和下行推送，*/
/* 都会发布到 "shadow/get/<product id>/<device name>" 这个topic*/
/* 客户端只需要订阅这个topic即可*/
#define TC_IOT_SUB_TOPIC_PREFIX "shadow/get/"
#define TC_IOT_SUB_TOPIC_FMT TC_IOT_SUB_TOPIC_PREFIX "%s/%s"
#define TC_IOT_SUB_TOPIC_DEF TC_IOT_SUB_TOPIC_PREFIX TC_IOT_CONFIG_DEVICE_PRODUCT_ID "/" TC_IOT_CONFIG_DEVICE_NAME
/* shadow上行消息topic，客户端请求服务端的消息，发到到这个topic即可*/
/* topic格式"shadow/update/<product id>/<device name>"*/
#define TC_IOT_PUB_TOPIC_PREFIX "shadow/update/"
#define TC_IOT_PUB_TOPIC_FMT TC_IOT_PUB_TOPIC_PREFIX "%s/%s"
#define TC_IOT_PUB_TOPIC_DEF TC_IOT_PUB_TOPIC_PREFIX TC_IOT_CONFIG_DEVICE_PRODUCT_ID "/" TC_IOT_CONFIG_DEVICE_NAME

/* tls 相关配置*/
/* 根证书路径*/
#define TC_IOT_CONFIG_ROOT_CA NULL
/* 客户端证书路径*/
#define TC_IOT_CONFIG_CLIENT_CRT NULL
/* 客户端私钥路径*/
#define TC_IOT_CONFIG_CLIENT_KEY NULL

#endif /* end of include guard */
