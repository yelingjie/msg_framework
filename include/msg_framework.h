/**
 * @file msg_framework.h
 * @brief 消息框架 - 轻量级模块间通信库
 *
 * 支持 RT-Thread 和 FreeRTOS，基于消息队列的异步通信框架。
 *
 * @example
 * // 模块初始化
 * int ret = mf_module_bind(MODULE_A_ID, "mod_a", sizeof(AppMsg), 16);
 *
 * // 发送消息
 * AppMsg msg = {...};
 * mf_send_msg(MODULE_A_ID, MODULE_B_ID, MSG_ID_TEST, 0, &msg, sizeof(msg));
 *
 * // 接收消息
 * AppMsg msg;
 * mf_recv_msg(MODULE_B_ID, &msg, sizeof(msg), 1000);
 */
#ifndef MSG_FRAMEWORK_H
#define MSG_FRAMEWORK_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * 内存池配置（可选）
 * ============================================================ */
/**
 * @brief 开启内存池
 * @note 开启后可减少频繁 malloc/free 导致的内存碎片
 *       使用 MF_MEM_POOL_BUF_SIZE 设置池大小
 * @note 需在 include msg_framework.h 之前定义
 */
#ifndef MF_MEM_POOL_ENABLE
#define MF_MEM_POOL_ENABLE      0
#endif

/**
 * @brief 内存池总大小（字节）
 * @note 仅在 MF_MEM_POOL_ENABLE 为 1 时生效
 *       需要根据实际消息大小和并发量调整
 */
#ifndef MF_MEM_POOL_BUF_SIZE
#define MF_MEM_POOL_BUF_SIZE    4096
#endif

/**
 * @brief 单个内存块大小（字节）
 * @note 仅在 MF_MEM_POOL_ENABLE 为 1 时生效
 *       应大于等于应用层最大消息大小
 */
#ifndef MF_MEM_POOL_BLOCK_SIZE
#define MF_MEM_POOL_BLOCK_SIZE  256
#endif

/* ============================================================
 * 错误码定义
 * ============================================================ */
#define MF_OK       0   /**< 成功 */
#define MF_ERR     -1   /**< 通用错误 */
#define MF_EEMPTY  -2   /**< 资源未绑定 */
#define MF_ENOMEM  -3   /**< 内存分配失败 */
#define MF_EFULL   -4   /**< 队列满 */
#define MF_EINVAL  -5   /**< 参数无效 */
#define MF_ETIMEOUT -6  /**< 超时 */

/* ============================================================
 * 模块 ID 和消息码
 * ============================================================ */
/* 模块 ID 由应用层定义，框架不做限制 */
/* 消息码由应用层定义，框架不做限制 */

/* ============================================================
 * 消息结构体
 * ============================================================ */
/** 消息头结构 */
typedef struct {
    uint8_t  src_moduleid;  /**< 源模块 ID */
    uint8_t  dest_moduleid; /**< 目标模块 ID */
    uint32_t msg_code;      /**< 消息码 */
    uint32_t msg_para;      /**< 消息参数 */
    void*    pdata;         /**< 数据指针 */
    uint16_t datalen;       /**< 数据长度 */
} mf_message_t;

/* ============================================================
 * 框架配置
 * ============================================================ */
/** 框架配置 */
typedef struct {
    uint8_t enable_debug;   /**< 开启调试输出 */
} mf_config_t;

/* ============================================================
 * 核心 API
 * ============================================================ */
/**
 * @brief 初始化框架
 * @param[in] p_config 配置参数，NULL 使用默认配置
 * @return MF_OK 成功，其他失败
 */
int mf_init(mf_config_t* p_config);

/**
 * @brief 获取框架版本
 * @return 版本字符串
 */
const char* mf_version(void);

/* ============================================================
 * 模块管理
 * ============================================================ */
/**
 * @brief 绑定模块到消息队列
 * @param[in] module_id   模块 ID（应用层定义）
 * @param[in] name        模块名称（用于队列命名）
 * @param[in] msg_size    单条消息大小
 * @param[in] max_msgs    队列最大消息数
 * @return MF_OK 成功，其他失败
 * @note 每个模块只需绑定一次
 */
int mf_module_bind(uint8_t module_id, const char* name,
                  uint32_t msg_size, uint32_t max_msgs);

/**
 * @brief 解绑模块
 * @param[in] module_id 模块 ID
 * @return MF_OK 成功，其他失败
 */
int mf_module_unbind(uint8_t module_id);

/**
 * @brief 检查模块是否已绑定
 * @param[in] module_id 模块 ID
 * @return 1 已绑定，0 未绑定
 */
int mf_module_is_bound(uint8_t module_id);

/* ============================================================
 * 消息发送/接收
 * ============================================================ */
/**
 * @brief 发送消息
 * @param[in] src_moduleid  源模块 ID
 * @param[in] dest_moduleid 目标模块 ID
 * @param[in] msg_code     消息码
 * @param[in] msg_para     消息参数
 * @param[in] p_data       数据指针（可为 NULL）
 * @param[in] data_len     数据长度（为 0 表示无数据）
 * @return MF_OK 成功，其他失败
 * @note 框架内部会复制数据，调用方可释放
 */
int mf_send_msg(uint8_t src_moduleid, uint8_t dest_moduleid,
                uint32_t msg_code, uint32_t msg_para,
                void* p_data, uint32_t data_len);

/**
 * @brief 接收消息
 * @param[in]  module_id    本模块 ID
 * @param[out] p_msg        消息缓冲区
 * @param[in]  msg_size     缓冲区大小
 * @param[in]  timeout_ms  超时时间（ms），-1 永久等待，0 非阻塞
 * @return MF_OK 成功，其他失败
 * @note 框架自动复制数据到缓冲区，需调用 mf_free_msg 释放内部内存
 */
int mf_recv_msg(uint8_t module_id, mf_message_t* p_msg,
                uint32_t msg_size, int32_t timeout_ms);

/**
 * @brief 释放消息内部数据
 * @param[in] p_msg 消息指针
 * @note 接收消息后必须调用此函数释放内部内存
 */
void mf_free_msg(mf_message_t* p_msg);

/* ============================================================
 * 辅助 API
 * ============================================================ */
/**
 * @brief 获取模块队列中的消息数量
 * @param[in] module_id 模块 ID
 * @return 消息数，<0 表示失败
 */
int mf_get_msg_count(uint8_t module_id);

/**
 * @brief 清空模块队列
 * @param[in] module_id 模块 ID
 * @return MF_OK 成功，其他失败
 */
int mf_flush(uint8_t module_id);

/**
 * @brief 设置调试输出回调
 * @param[in] printf_cb 打印回调函数
 */
void mf_set_debug_cb(void (*printf_cb)(const char* fmt, ...));

#ifdef __cplusplus
}
#endif

#endif /* MSG_FRAMEWORK_H */
