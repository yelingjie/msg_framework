# msg_framework

轻量级模块间消息通信框架，支持 RT-Thread、FreeRTOS 和 Zephyr。

## 特性

- **跨平台**：支持 RT-Thread、FreeRTOS、Zephyr
- **轻量级**：核心代码约 300 行，零依赖
- **异步通信**：基于消息队列的模块间通信
- **易于使用**：简单的 API 设计

## 架构图

```
┌─────────────────────────────────────────────────────────────────┐
│                         msg_framework                            │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│   ┌──────────┐    ┌──────────┐    ┌──────────┐    ┌──────────┐  │
│   │ Module A │    │ Module B │    │ Module C │    │ Module D │  │
│   └────┬─────┘    └────┬─────┘    └────┬─────┘    └────┬─────┘  │
│        │               │               │               │         │
│        │  mf_send_msg  │               │               │         │
│        │   ───────────► │               │               │         │
│        │               │               │               │         │
│        │    ┌──────────────────────────────┐          │         │
│        │    │        消息队列中心             │          │         │
│        │    │  ┌─────────────────────────┐ │          │         │
│        │    │  │  MSG Queue for Module B │ │          │         │
│        │    │  │  [msg1] [msg2] [msg3]   │ │          │         │
│        │    │  └─────────────────────────┘ │          │         │
│        │    └──────────────────────────────┘          │         │
│        │               │               │               │         │
│        │               │  mf_recv_msg   │               │         │
│        │               │  ◄──────────── │               │         │
│        │               │               │               │         │
└────────┴───────────────┴───────────────┴───────────────┴─────────┘
                              │
                              ▼
                    ┌─────────────────┐
                    │   OS 适配层      │
                    ├─────────────────┤
                    │ rtt/            │
                    │ freertos/       │
                    │ zephyr/         │
                    └─────────────────┘
```

## 优点

### 1. 低耦合
- 模块间通过消息通信，无需相互引用
- 模块可独立编译、测试和重用
- 新增模块不影响现有模块

### 2. 异步通信
- 发送消息后立即返回，无需等待处理
- 接收模块按自身节奏处理消息
- 提高系统响应性和吞吐量

### 3. 可追溯性
- 消息带源/目标模块 ID，便于追踪
- 消息码统一管理，行为可预期
- 适合调试和日志分析

### 4. 易于集成
- 只需实现 `mf_port.h` 接口即可支持新 OS
- 核心代码与 OS 解耦
- 适用于资源受限的嵌入式系统

### 5. 内存安全
- 框架自动管理消息内存复制
- 调用方无需关心数据生命周期
- 避免内存泄漏和野指针

## 快速开始

### 初始化框架

```c
mf_config_t config = {
    .enable_debug = 1,
};
mf_init(&config);
```

### 绑定模块

```c
// 每个模块只需绑定一次
mf_module_bind(MODULE_ID, "module_a", sizeof(AppMsg), 16);
```

### 发送消息

```c
AppMsg msg = {...};
mf_send_msg(SRC_MODULE, DEST_MODULE, MSG_ID, 0, &msg, sizeof(msg));
```

### 接收消息

```c
mf_message_t msg;
int ret = mf_recv_msg(MY_MODULE_ID, &msg, sizeof(msg), 1000);
if (ret == MF_OK) {
    // 处理消息
    AppMsg* p = (AppMsg*)msg.pdata;
    mf_free_msg(&msg);  // 必须释放
}
```

## 构建

### 使用 CMake

```bash
# RT-Thread 版本
mkdir build && cd build
cmake ..
make

# FreeRTOS 版本
cmake -DUSE_FREERTOS=ON ..
make

# Zephyr 版本
cmake -DUSE_ZEPHYR=ON ..
make
```

### 嵌入到现有项目

将 `include/` 和 `src/` 目录添加到你的项目，然后根据目标 OS 选择对应的 port 文件。

## API 列表

| API | 说明 |
|-----|------|
| `mf_init()` | 初始化框架 |
| `mf_version()` | 获取版本 |
| `mf_module_bind()` | 绑定模块到消息队列 |
| `mf_module_unbind()` | 解绑模块 |
| `mf_module_is_bound()` | 检查模块是否已绑定 |
| `mf_send_msg()` | 发送消息 |
| `mf_recv_msg()` | 接收消息 |
| `mf_free_msg()` | 释放消息内存 |
| `mf_flush()` | 清空队列 |
| `mf_set_debug_cb()` | 设置调试输出回调 |

## 消息结构

```c
typedef struct {
    uint8_t  src_moduleid;   // 源模块 ID
    uint8_t  dest_moduleid;  // 目标模块 ID
    uint32_t msg_code;       // 消息码（应用层定义）
    uint32_t msg_para;       // 消息参数
    void*    pdata;          // 数据指针
    uint16_t datalen;        // 数据长度
} mf_message_t;
```

## 目录结构

```
msg_framework/
├── include/                 # 公共头文件
│   └── msg_framework.h
├── src/                     # 核心实现
│   └── msg_framework.c
├── port/                    # OS 适配层
│   ├── mf_port.h           # 适配层接口
│   ├── rtt/                # RT-Thread 适配
│   │   └── mf_port_rtt.c
│   ├── freertos/           # FreeRTOS 适配
│   │   └── mf_port_freertos.c
│   └── zephyr/             # Zephyr 适配
│       └── mf_port_zephyr.c
├── tests/                   # 单元测试
└── CMakeLists.txt
```

## 添加新的 OS 适配层

如需支持其他 RTOS，可按以下步骤添加：

1. 在 `port/` 下创建新目录，如 `myos/`
2. 实现 `mf_port.h` 中定义的接口
3. 在 CMakeLists.txt 中添加对应的选项和源文件
