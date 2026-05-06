/**
 * @file test_msg_framework.c
 * @brief 消息框架单元测试
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "msg_framework.h"

/* 测试模块 ID */
#define MODULE_A  1
#define MODULE_B  2

/* 测试消息 */
typedef struct {
    uint32_t id;
    char     data[32];
} TestMsg;

static void test_basic(void)
{
    printf("\n=== 测试基本功能 ===\n");

    /* 初始化 */
    mf_init(NULL);
    printf("版本: %s\n", mf_version());

    /* 绑定模块 */
    int ret = mf_module_bind(MODULE_A, "mod_a", sizeof(TestMsg), 16);
    printf("绑定模块A: %s\n", ret == MF_OK ? "OK" : "FAIL");

    ret = mf_module_bind(MODULE_B, "mod_b", sizeof(TestMsg), 16);
    printf("绑定模块B: %s\n", ret == MF_OK ? "OK" : "FAIL");

    /* 检查绑定状态 */
    printf("模块A绑定状态: %d\n", mf_module_is_bound(MODULE_A));

    /* 解绑 */
    ret = mf_module_unbind(MODULE_A);
    printf("解绑模块A: %s\n", ret == MF_OK ? "OK" : "FAIL");
    printf("模块A绑定状态: %d\n", mf_module_is_bound(MODULE_A));
}

static void test_message(void)
{
    printf("\n=== 测试消息收发 ===\n");

    mf_module_bind(MODULE_A, "mod_a", sizeof(TestMsg), 16);
    mf_module_bind(MODULE_B, "mod_b", sizeof(TestMsg), 16);

    /* A 发送消息给 B */
    TestMsg msg = {.id = 100, .data = "Hello"};
    int ret = mf_send_msg(MODULE_A, MODULE_B, 1, 0, &msg, sizeof(msg));
    printf("A->B 发送消息: %s\n", ret == MF_OK ? "OK" : "FAIL");

    /* B 接收消息 */
    mf_message_t recv_msg;
    ret = mf_recv_msg(MODULE_B, &recv_msg, sizeof(recv_msg), 100);
    printf("B 接收消息: %s\n", ret == MF_OK ? "OK" : "FAIL");

    if (ret == MF_OK) {
        printf("  消息码: %d\n", recv_msg.msg_code);
        printf("  源模块: %d\n", recv_msg.src_moduleid);
        printf("  数据: %s\n", ((TestMsg*)recv_msg.pdata)->data);
        mf_free_msg(&recv_msg);
    }

    /* 清理 */
    mf_module_unbind(MODULE_A);
    mf_module_unbind(MODULE_B);
}

int main(void)
{
    printf("msg_framework 单元测试\n");
    printf("========================\n");

    test_basic();
    test_message();

    printf("\n=== 测试完成 ===\n");
    return 0;
}
