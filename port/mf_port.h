/**
 * @file mf_port.h
 * @brief 消息框架 OS 适配层接口
 *
 * 定义框架与底层 OS 之间的适配接口。
 * 使用时根据目标 OS 选择对应的实现文件。
 */
#ifndef MF_PORT_H
#define MF_PORT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * 错误码（与框架保持一致）
 * ============================================================ */
#define MF_PORT_OK      0
#define MF_PORT_ERR    -1
#define MF_PORT_EEMPTY -2
#define MF_PORT_ENOMEM -3
#define MF_PORT_EFULL  -4
#define MF_PORT_EINVAL -5

/* ============================================================
 * 类型抽象
 * ============================================================ */
typedef void* mf_port_mq_t;      /**< 消息队列句柄 */
typedef void* mf_port_list_t;    /**< 链表头指针 */
typedef void* mf_port_mutex_t;    /**< 互斥锁句柄 */

/* ============================================================
 * 配置结构
 * ============================================================ */
typedef struct {
    const char* name;       /**< 名称 */
    uint32_t    msg_size;    /**< 单条消息大小 */
    uint32_t    max_msgs;    /**< 最大消息数 */
} mf_port_mq_cfg_t;

/* ============================================================
 * 链表操作
 * ============================================================ */
/**
 * @brief 初始化链表
 * @param[in] p_list 链表头指针
 */
void mf_port_list_init(mf_port_list_t p_list);

/**
 * @brief 链表是否为空
 * @param[in] p_list 链表头指针
 * @return 1 为空，0 非空
 */
int mf_port_list_isempty(mf_port_list_t p_list);

/**
 * @brief 在节点前插入
 * @param[in] p_before 参考节点
 * @param[in] p_node  待插入节点
 */
void mf_port_list_insert_before(void* p_before, void* p_node);

/**
 * @brief 移除节点
 * @param[in] p_node 待移除节点
 */
void mf_port_list_remove(void* p_node);

/* ============================================================
 * 消息队列操作
 * ============================================================ */
/**
 * @brief 创建消息队列
 * @param[in] p_cfg 配置参数
 * @return 队列句柄，NULL 失败
 */
mf_port_mq_t mf_port_mq_create(mf_port_mq_cfg_t* p_cfg);

/**
 * @brief 删除消息队列
 * @param[in] mq 队列句柄
 */
void mf_port_mq_delete(mf_port_mq_t mq);

/**
 * @brief 发送消息
 * @param[in] mq      队列句柄
 * @param[in] p_msg   消息指针
 * @param[in] size    消息大小
 * @return MF_PORT_OK 成功，其他失败
 */
int mf_port_mq_send(mf_port_mq_t mq, void* p_msg, uint32_t size);

/**
 * @brief 接收消息
 * @param[in] mq         队列句柄
 * @param[in] p_msg      消息缓冲区
 * @param[in] size       缓冲区大小
 * @param[in] timeout_ms 超时时间（ms），-1 永久等待
 * @return MF_PORT_OK 成功，其他失败
 */
int mf_port_mq_recv(mf_port_mq_t mq, void* p_msg, uint32_t size, int32_t timeout_ms);

/* ============================================================
 * 内存管理
 * ============================================================ */
/**
 * @brief 分配内存
 * @param[in] size 字节数
 * @return 内存指针，NULL 失败
 */
void* mf_port_malloc(uint32_t size);

/**
 * @brief 释放内存
 * @param[in] ptr 内存指针
 */
void mf_port_free(void* ptr);

/* ============================================================
 * 临界区
 * ============================================================ */
/**
 * @brief 进入临界区
 */
void mf_port_enter_critical(void);

/**
 * @brief 退出临界区
 */
void mf_port_exit_critical(void);

/* ============================================================
 * 调试输出
 * ============================================================ */
/**
 * @brief 打印日志
 * @param[in] fmt 格式化字符串
 * @param[in] ... 可变参数
 */
void mf_port_printf(const char* fmt, ...);

#ifdef __cplusplus
}
#endif

#endif /* MF_PORT_H */
