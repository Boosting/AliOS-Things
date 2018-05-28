/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "iot_import.h"
#include "iot_export.h"
#include "aos/log.h"
#include "aos/yloop.h"
#include "aos/network.h"
#include <netmgr.h>
#include <aos/kernel.h>
#include <netmgr.h>
#include <aos/cli.h>
#include <aos/cloud.h>
#include "iot_export_mqtt.h"
#include "mqtt_instance.h"
#include "cJSON.h"
#include "hal/soc/uart.h"
#include "mqttSensor.h"
#include "Temp_Humi.h"

#ifdef AOS_ATCMD
#include <atparser.h>
#endif

#if defined(MQTT_ID2_AUTH) && defined(TEST_ID2_DAILY)
/*
    #define PRODUCT_KEY             "OvNmiEYRDSY"
    #define DEVICE_NAME             "sh_online_sample_mqtt"
    #define DEVICE_SECRET           "v9mqGzepKEphLhXmAoiaUIR2HZ7XwTky"
*/
#else
#if 0
    #define PRODUCT_KEY             "a1HuBDQoCYT"
    #define DEVICE_NAME             "firstdevice"
    #define DEVICE_SECRET           "rCPR1QWoTfjX2SMTrrkZIXtE8OiXOIX1"
#endif
#endif

typedef struct {
    char productKey[16];
    char deviceName[32];
    char deviceSecret[48];

    int max_msg_size;
    int max_msgq_size;
    int connected;
    int (*event_handler)(int event_type, void *ctx);
    int (*delete_subdev)(char *productKey, char *deviceName, void *ctx);
    void *ctx;
} MqttContext;

// These are pre-defined topics
#define TOPIC_UPDATE            "/"PRODUCT_KEY"/"DEVICE_NAME"/update"
#define TOPIC_ERROR             "/"PRODUCT_KEY"/"DEVICE_NAME"/update/error"
#define TOPIC_GET               "/"PRODUCT_KEY"/"DEVICE_NAME"/get"
#define TOPIC_POST              "/sys/"PRODUCT_KEY"/"DEVICE_NAME"/thing/event/property/post"

#define MSG_LEN_MAX             (2048)

int cnt = 0;
extern HCHO_t HCHO;
static int is_subscribed = 0;

#ifdef MQTT_PRESS_TEST 
static int sub_counter = 0;
static int pub_counter = 0;
#endif
char msg_pub[128];

static void ota_init(void *pclient);
int mqtt_client_example(void);
static void wifi_service_event(input_event_t *event, void *priv_data) 
{
    // 判断Wi-Fi事件
    if (event->type != EV_WIFI) {
        return;
    }

    // 判断联网成功
    if (event->code != CODE_WIFI_ON_GOT_IP) {
        return;
    }

    // 执行MQTT例程
    LOG("\r\n---------------------------------------Wi-Fi 联网成功，执行MQTT程序!\r\n");
    mqtt_client_example();
}

static void mqtt_sub_callback(char *topic, int topic_len, void *payload, int payload_len, void *ctx)
{
    LOG("----");
    LOG("Topic: '%s' (Length: %d)",
                  topic,
                  topic_len);
    LOG("Payload: '%s' (Length: %d)",
                  (char*)payload,
                  payload_len);
    LOG("----");

#ifdef MQTT_PRESS_TEST 
    sub_counter++;
    int rc = mqtt_publish(TOPIC_UPDATE, IOTX_MQTT_QOS1, payload, payload_len);
    if(rc < 0) {
        LOG("IOT_MQTT_Publish fail, ret=%d", rc);
    }
    else {
        pub_counter++; 
    }
    LOG("RECV=%d, SEND=%d", sub_counter, pub_counter);
#endif MQTT_PRESS_TEST 

}



/*
 * Subscribe the topic: IOT_MQTT_Subscribe(pclient, TOPIC_DATA, IOTX_MQTT_QOS1, _demo_message_arrive, NULL);
 * Publish the topic: IOT_MQTT_Publish(pclient, TOPIC_DATA, &topic_msg);
 */
static void mqtt_work(void *parms) 
{

    int rc = -1;
    float ftemp;

    char *out = NULL;

    // 有数据更新
    if (HCHO.update == 1)
    {
        HCHO.update = 0;   
    }
    else
    {
        goto Delay;
    }
    cJSON * root =  cJSON_CreateObject();
    cJSON_AddItemToObject(root, "id", cJSON_CreateString("12345678"));
    cJSON_AddItemToObject(root, "version", cJSON_CreateString("1.0"));
    cJSON * params =  cJSON_CreateObject();
    cJSON_AddItemToObject(root, "params", params);
    ftemp = cnt * 0.1;
    cJSON_AddNumberToObject(params, "HCHOValue", HCHO.value);
    cJSON_AddItemToObject(root, "method", cJSON_CreateString("thing.event.property.post"));
    out = cJSON_Print(root);
    // LOG("\r\n json字符:\r\n%s\r\n", out);
    
    /* Generate topic message */
    rc = mqtt_publish(TOPIC_POST, IOTX_MQTT_QOS1, out, strlen(out));
    if (rc < 0) 
    {
        LOG("error occur when publish");
    }

    // LOG("packet-id=%u, topic=%s msg=%s", (uint32_t)rc, TOPIC_POST, out);
    cJSON_Delete(root);

Delay:
    aos_post_delayed_action(1000, mqtt_work, NULL);

    cnt++;

    // // 循环发送2000次
    // if(cnt < 2000) 
    // {
        
    // } 
    // else 
    // {
    //     LOG("\r\n-------------------------- 发送200次后断开连接。\r\n");
    //     aos_cancel_delayed_action(3000, mqtt_work, NULL);
    //     mqtt_unsubscribe(TOPIC_GET);
    //     aos_msleep(200);
    //     mqtt_deinit_instance();
    //     is_subscribed = 0;
    //     cnt = 0;
    // }
  
}



static void mqtt_service_event(input_event_t *event, void *priv_data) {

    if (event->type != EV_SYS) {
        return;
    }

    if (event->code != CODE_SYS_ON_MQTT_READ) {
        return;
    }
    LOG("mqtt_service_event!");
    mqtt_work(NULL);
}

static int smartled_event_handler(int event_type, void *ctx)
{
    LOG("event_type %d\n", event_type);
    switch (event_type) {
    default:
        break;
    }

    return 0;
}

static MqttContext mqtt;

int mqtt_client_example(void)
{
    memset(&mqtt, 0, sizeof(MqttContext));

    LOG("\r\n 准备productKey deviceName 数据。\r\n");
    strncpy(mqtt.productKey,   PRODUCT_KEY,   sizeof(mqtt.productKey)   - 1);
    strncpy(mqtt.deviceName,   DEVICE_NAME,   sizeof(mqtt.deviceName)   - 1);
    strncpy(mqtt.deviceSecret, DEVICE_SECRET, sizeof(mqtt.deviceSecret) - 1);

    mqtt.max_msg_size = MSG_LEN_MAX;
    mqtt.max_msgq_size = 8;

    mqtt.event_handler = smartled_event_handler;
    mqtt.delete_subdev = NULL;

    LOG("\r\n MQTT 使用productKey deviceName 初始化实例 & 缓存。\r\n");
    if (mqtt_init_instance(mqtt.productKey, mqtt.deviceName, mqtt.deviceSecret, mqtt.max_msg_size) < 0) 
    {
        LOG("mqtt_init_instance failed\n");
        return -1;
    }
    else
    {
        LOG("MQTT init Success!\r\n");        
    }
    aos_register_event_filter(EV_SYS,  mqtt_service_event, NULL);

    return 0;

}

static void handle_mqtt(char *pwbuf, int blen, int argc, char **argv)
{
    mqtt_client_example();
}

static struct cli_command mqttcmd = {
    .name = "mqtt",
    .help = "factory mqtt",
    .function = handle_mqtt
};

#ifdef AOS_ATCMD
static void at_uart_configure(uart_dev_t *u)
{
    u->port                = AT_UART_PORT;
    u->config.baud_rate    = AT_UART_BAUDRATE;
    u->config.data_width   = AT_UART_DATA_WIDTH;
    u->config.parity       = AT_UART_PARITY;
    u->config.stop_bits    = AT_UART_STOP_BITS;
    u->config.flow_control = AT_UART_FLOW_CONTROL;
}
#endif


int application_start(int argc, char *argv[])
{
#if AOS_ATCMD
    at.set_mode(ASYN);
    at.init(AT_RECV_PREFIX, AT_RECV_SUCCESS_POSTFIX,
            AT_RECV_FAIL_POSTFIX, AT_SEND_DELIMITER, 1000);
#endif


#ifdef WITH_SAL
    sal_init();
#endif
    aos_set_log_level(AOS_LL_DEBUG);

    // 注册事件，等待Wi-Fi联网成功
    aos_register_event_filter(EV_WIFI, wifi_service_event, NULL);

    // 新建任务 
    aos_task_new(
                    "HCHO_Task",            // 任务名称
                    HCHO_task,              // 执行函数
                    NULL,                   // 传参
                    3072                    // 堆栈字节
                );
    
    // 创建温湿度任务
    temp_humi_task_create();
    
    netmgr_init();
    LOG("\r\nnetmgr 初始化!\r\n");  
    netmgr_start(false);
    LOG("\r\n启动netmgr & 注册mqttcmd!\r\n"); 
    aos_cli_register_command(&mqttcmd);

    aos_loop_run();
    return 0;
}

static void ota_init(void *P)
{
    aos_post_event(EV_SYS, CODE_SYS_ON_START_FOTA, 0u);
}
