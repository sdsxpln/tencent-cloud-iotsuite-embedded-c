#ifndef IOT_CONST_01091047_H
#define IOT_CONST_01091047_H

typedef enum _tc_iot_sys_code_e {
    TC_IOT_SUCCESS = 0,
    TC_IOT_FAILURE = -0x1,
    TC_IOT_BUFFER_OVERFLOW = -0x2,
    TC_IOT_INVALID_PARAMETER = -0x10,
    TC_IOT_NULL_POINTER = -0x11,
    TC_IOT_TLS_NOT_SUPPORTED = -0x12,

    TC_IOT_NETWORK_ERROR_BASE = -0x100,
    TC_IOT_NET_CONNECT_FAILED = -0x101,
    TC_IOT_NET_UNKNOWN_HOST = 0x102,
    TC_IOT_NET_SOCKET_FAILED = 0x103,
    TC_IOT_SEND_PACK_FAILED = 0x104,

    TC_IOT_MQTT_RECONNECT_TIMEOUT = 0x150,
    TC_IOT_MQTT_RECONNECT_IN_PROGRESS = 0x151,
    TC_IOT_MQTT_RECONNECT_FAILED = 0x152,
    TC_IOT_MQTT_NETWORK_UNAVAILABLE = 0x153,
    TC_IOT_MQTT_WAIT_ACT_TIMEOUT = 0x154,

    TC_IOT_MBED_TLS_ERROR_BASE = -0x200,
    TC_IOT_CTR_DRBG_SEED_FAILED = -0x201,
    TC_IOT_X509_CRT_PARSE_FILE_FAILED = -0x202,
    TC_IOT_PK_PRIVATE_KEY_PARSE_ERROR = -0x203,
    TC_IOT_PK_PARSE_KEYFILE_FAILED = -0x204,
    TC_IOT_TLS_NET_SET_BLOCK_FAILED = -0X205,
    TC_IOT_TLS_SSL_CONFIG_DEFAULTS_FAILED = -0X206,
    TC_IOT_TLS_SSL_CONF_OWN_CERT_FAILED = -0X207,
    TC_IOT_TLS_SSL_SETUP_FAILED = 0x208,
    TC_IOT_TLS_SSL_SET_HOSTNAME_FAILED = -0x209,
    TC_IOT_TLS_SSL_HANDSHAKE_FAILED = -0x210,
    TC_IOT_TLS_X509_CRT_VERIFY_FAILED = -0x211,
    TC_IOT_TLS_SSL_WRITE_FAILED = -0x212,
    TC_IOT_TLS_SSL_READ_FAILED = -0x213,
    TC_IOT_TLS_SSL_READ_TIMEOUT = -0x214,
    TC_IOT_TLS_MD_SETUP_FAILED = -0x215,

    TC_IOT_LOGIC_ERROR_BASE = -0x1000,
    TC_IOT_NETWORK_PTR_NULL = -0x1001,
    TC_IOT_NETCONTEXT_PTR_NULL = -0x1002,
    TC_IOT_JSON_PATH_NO_MATCH = -0x1003,
    TC_IOT_JSON_PARSE_FAILED = -0x1004,
    ERROR_HTTP_REQUEST_FAILED = -0x1005,

} tc_iot_sys_code_e;

#endif /* end of include guard */