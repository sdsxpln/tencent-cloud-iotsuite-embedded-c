add_executable(demo_mqtt demo_mqtt.c tc_iot_client_config_opts.c)

if(ENABLE_TLS)
    target_link_libraries(demo_mqtt tc_iot_suite tc_iot_common tc_iot_hal mbedtls jsmn MQTTPacketClient)
else()
    target_link_libraries(demo_mqtt tc_iot_suite tc_iot_common tc_iot_hal jsmn MQTTPacketClient)
endif()

add_executable(demo_shadow demo_shadow.c tc_iot_client_config_opts.c)
if(ENABLE_TLS)
    target_link_libraries(demo_shadow tc_iot_suite tc_iot_common tc_iot_hal mbedtls jsmn MQTTPacketClient)
else()
    target_link_libraries(demo_shadow tc_iot_suite tc_iot_common tc_iot_hal jsmn MQTTPacketClient)
endif()

add_executable(demo_custom_topic demo_custom_topic.c tc_iot_client_config_opts.c)

if(ENABLE_TLS)
    target_link_libraries(demo_custom_topic tc_iot_suite tc_iot_common tc_iot_hal mbedtls jsmn MQTTPacketClient)
else()
    target_link_libraries(demo_custom_topic tc_iot_suite tc_iot_common tc_iot_hal jsmn MQTTPacketClient)
endif()

ADD_SUBDIRECTORY(light)

