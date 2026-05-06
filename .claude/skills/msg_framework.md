# msg_framework Skill

触发关键词：`msg_framework`、`消息框架`、`模块间通信`、`添加模块`、`创建模块`、`删除模块`、`发送消息`、`接收消息`

## 描述

当用户需要使用 msg_framework 进行模块间消息通信时触发此 skill。专注于帮助用户调用 API 函数实现模块通信。

## 快速参考

### 模块管理 API

| 功能 | API | 示例 |
|------|-----|------|
| 创建模块 | `mf_module_bind(id, name, msg_size, max_msgs)` | `mf_module_bind(1, "sensor", 64, 16);` |
| 删除模块 | `mf_module_unbind(id)` | `mf_module_unbind(1);` |
| 检查模块 | `mf_module_is_bound(id)` | `if(mf_module_is_bound(1))` |

### 消息通信 API

| 功能 | API | 示例 |
|------|-----|------|
| 发送消息 | `mf_send_msg(src, dest, code, para, data, len)` | `mf_send_msg(1, 2, 100, 0, &msg, sizeof(msg));` |
| 接收消息 | `mf_recv_msg(id, &msg, size, timeout)` | `mf_recv_msg(2, &msg, sizeof(msg), 1000);` |
| 释放消息 | `mf_free_msg(&msg)` | `mf_free_msg(&msg);` |
| 清空队列 | `mf_flush(id)` | `mf_flush(2);` |

## 典型场景

### 场景1: 添加新模块

**用户说**: "添加一个传感器模块"

```c
// 1. 定义模块ID
#define MODULE_SENSOR   10

// 2. 在模块初始化函数中绑定
int sensor_init(void) {
    return mf_module_bind(MODULE_SENSOR, "sensor", sizeof(SensorMsg), 16);
}
```

### 场景2: 给指定模块发送消息

**用户说**: "给显示模块发送温度数据"

```c
// 定义消息结构
typedef struct {
    float temp;
    float humidity;
} SensorDataMsg;

// 发送消息给显示模块(MODULE_DISPLAY = 2)
SensorDataMsg data = {.temp = 25.5, .humidity = 60.0};
mf_send_msg(MODULE_SENSOR,     // 源模块：传感器
            MODULE_DISPLAY,     // 目标模块：显示
            MSG_ID_TEMP_DATA,   // 消息码
            0,                   // 参数
            &data,               // 数据
            sizeof(data));       // 数据长度
```

### 场景3: 接收消息

**用户说**: "接收传感器发来的消息"

```c
void display_thread_entry(void* param) {
    mf_message_t msg;

    while (1) {
        // 接收消息，超时1000ms
        int ret = mf_recv_msg(MODULE_DISPLAY, &msg, sizeof(msg), 1000);

        if (ret == MF_OK) {
            switch (msg.msg_code) {
                case MSG_ID_TEMP_DATA: {
                    SensorDataMsg* p = (SensorDataMsg*)msg.pdata;
                    printf("温度: %.1f, 湿度: %.1f\n", p->temp, p->humidity);
                    break;
                }
                case MSG_ID_ALARM:
                    // 处理报警
                    break;
            }
            mf_free_msg(&msg);  // 必须调用释放内存
        }
    }
}
```

### 场景4: 删除模块

**用户说**: "删除传感器模块"

```c
mf_module_unbind(MODULE_SENSOR);
```

## 代码模板

### 完整模块通信示例

```c
#include "msg_framework.h"

// ========== 定义区 ==========
#define MODULE_A   1
#define MODULE_B   2

#define MSG_HELLO  1001

typedef struct {
    char content[64];
} HelloMsg;

// ========== 模块A: 发送消息 ==========
void module_a_thread(void* param) {
    mf_module_bind(MODULE_A, "mod_a", sizeof(HelloMsg), 16);

    while (1) {
        HelloMsg msg = {.content = "Hello"};
        mf_send_msg(MODULE_A, MODULE_B, MSG_HELLO, 0, &msg, sizeof(msg));
        rt_thread_mdelay(1000);
    }
}

// ========== 模块B: 接收消息 ==========
void module_b_thread(void* param) {
    mf_module_bind(MODULE_B, "mod_b", sizeof(HelloMsg), 16);

    while (1) {
        mf_message_t msg;
        if (mf_recv_msg(MODULE_B, &msg, sizeof(msg), 1000) == MF_OK) {
            if (msg.msg_code == MSG_HELLO) {
                HelloMsg* p = (HelloMsg*)msg.pdata;
                printf("收到: %s\n", p->content);
            }
            mf_free_msg(&msg);
        }
    }
}
```

## 错误码

| 错误码 | 含义 |
|--------|------|
| MF_OK | 成功 |
| MF_ERR | 通用错误 |
| MF_EEMPTY | 资源未绑定 |
| MF_ENOMEM | 内存分配失败 |
| MF_EFULL | 队列满 |
| MF_EINVAL | 参数无效 |
| MF_ETIMEOUT | 超时 |

## 常见问题

| 问题 | 解决方案 |
|------|----------|
| 消息发送失败 | 检查目标模块是否已 bind |
| 接收不到消息 | 确认消息目标ID正确，队列未满 |
| 程序卡住 | 检查超时设置，消息是否被正确释放 |
| 内存泄漏 | 确保每次 `mf_recv_msg` 后调用 `mf_free_msg` |

## 触发检查

当用户提到以下内容时触发：
- "添加模块"、"创建模块"、"注册模块"
- "发送消息"、"发消息给"、"发送指令"
- "接收消息"、"收取消息"、"消息回调"
- "删除模块"、"解绑模块"
- "msg_framework 怎么用"、"消息框架 API"
