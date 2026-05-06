/**
 * @file example.c
 * @brief 消息框架使用示例
 *
 * 展示如何在两个模块之间进行消息通信。
 */
#include <stdio.h>
#include <stdlib.h>
#include "msg_framework.h"

/* 模块 ID 定义 */
#define MODULE_SENSOR   1
#define MODULE_DISPLAY  2
#define MODULE_CONTROL  3

/* 消息码定义 */
#define MSG_ID_SENSOR_DATA    1001
#define MSG_ID_DISPLAY_UPDATE 1002
#define MSG_ID_CONTROL_CMD    1003

/* 传感器数据消息 */
typedef struct {
    float temperature;
    float humidity;
    uint32_t timestamp;
} SensorDataMsg;

/* 显示更新消息 */
typedef struct {
    char text[64];
} DisplayMsg;

/* 控制命令消息 */
typedef struct {
    uint8_t cmd;      // 1: 启动, 2: 停止
    uint32_t param;
} ControlCmdMsg;

/* 模拟传感器线程 */
static void* sensor_thread(void* arg)
{
    (void)arg;
    int count = 0;

    while (1) {
        /* 模拟读取传感器数据 */
        SensorDataMsg data = {
            .temperature = 25.0f + (count % 10),
            .humidity = 60.0f + (count % 20),
            .timestamp = count
        };

        /* 发送数据到显示模块 */
        mf_send_msg(MODULE_SENSOR, MODULE_DISPLAY,
                    MSG_ID_SENSOR_DATA, 0,
                    &data, sizeof(data));

        count++;
        /* 模拟 1 秒采集一次 */
    }

    return NULL;
}

/* 模拟显示线程 */
static void* display_thread(void* arg)
{
    (void)arg;

    while (1) {
        mf_message_t msg;
        int ret = mf_recv_msg(MODULE_DISPLAY, &msg, sizeof(msg), 1000);

        if (ret == MF_OK) {
            switch (msg.msg_code) {
                case MSG_ID_SENSOR_DATA: {
                    SensorDataMsg* pdata = (SensorDataMsg*)msg.pdata;
                    printf("显示收到传感器数据: 温度=%.1f, 湿度=%.1f\n",
                           pdata->temperature, pdata->humidity);
                    break;
                }
                case MSG_ID_CONTROL_CMD: {
                    ControlCmdMsg* pdata = (ControlCmdMsg*)msg.pdata;
                    printf("显示收到控制命令: cmd=%d\n", pdata->cmd);
                    break;
                }
            }
            mf_free_msg(&msg);
        }
    }

    return NULL;
}

/* 模拟控制线程 */
static void* control_thread(void* arg)
{
    (void)arg;

    /* 模拟发送控制命令 */
    ControlCmdMsg cmd = {.cmd = 1, .param = 100};
    mf_send_msg(MODULE_CONTROL, MODULE_DISPLAY,
                MSG_ID_CONTROL_CMD, 0, &cmd, sizeof(cmd));

    return NULL;
}

int main(void)
{
    printf("msg_framework 示例\n");
    printf("====================\n\n");

    /* 初始化框架 */
    mf_init(NULL);
    printf("框架版本: %s\n", mf_version());

    /* 绑定模块 */
    mf_module_bind(MODULE_SENSOR, "sensor", sizeof(SensorDataMsg), 16);
    mf_module_bind(MODULE_DISPLAY, "display", sizeof(DisplayMsg), 16);
    mf_module_bind(MODULE_CONTROL, "control", sizeof(ControlCmdMsg), 16);

    printf("\n模块绑定完成\n");

    /* 模拟传感器线程 */
    printf("\n模拟传感器数据...\n");
    sensor_thread(NULL);

    /* 模拟控制线程 */
    printf("\n模拟控制命令...\n");
    control_thread(NULL);

    /* 模拟显示线程（非阻塞演示） */
    printf("\n模拟显示线程（接收 2 条消息）...\n");
    for (int i = 0; i < 2; i++) {
        display_thread(NULL);
    }

    printf("\n示例完成\n");

    /* 清理 */
    mf_module_unbind(MODULE_SENSOR);
    mf_module_unbind(MODULE_DISPLAY);
    mf_module_unbind(MODULE_CONTROL);

    return 0;
}
