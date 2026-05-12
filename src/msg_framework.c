/**
 * @file msg_framework.c
 * @brief 消息框架核心实现
 *
 * 轻量级模块间通信框架，支持 RT-Thread 和 FreeRTOS。
 */
#include <stdint.h>
#include <string.h>
#include "include/msg_framework.h"
#include "include/mf_mempool.h"
#include "mf_port.h"

/* ============================================================
 * 版本信息
 * ============================================================ */
#define MF_VERSION_MAJOR   1
#define MF_VERSION_MINOR   1
#define MF_VERSION_STR     "1.1.0"

/** 模块队列映射节点 */
typedef struct {
    uint8_t       module_id;    /**< 模块 ID */
    mf_port_mq_t  mq;           /**< 消息队列句柄 */
    const char*    name;         /**< 模块名称 */
} mf_module_node_t;

/** 链表节点 */
typedef struct mf_entry_s {
    mf_module_node_t     node;
    struct mf_entry_s*   next;
    struct mf_entry_s*   prev;
} mf_entry_t;

/* ============================================================
 * 内部变量
 * ============================================================ */
static mf_entry_t       s_module_list;      /**< 模块链表头 */
static int              s_initialized = 0;   /**< 初始化标志 */
static uint8_t          s_debug_enable = 0; /**< 调试开关 */
static void           (*s_printf_cb)(const char* fmt, ...) = NULL;


static mf_entry_t* mf_find_module(uint8_t module_id);
static int         mf_internal_send(uint8_t dest_moduleid, mf_message_t* p_msg);

/**
 * @brief       初始化消息框架
 * @param[in]   p_config 配置参数，NULL 使用默认配置
 * @return      MF_OK 成功，其他失败
 */
int mf_init(mf_config_t* p_config)
{
    if (s_initialized) {
        return MF_OK;
    }

    /* 初始化链表头（哨兵节点） */
    s_module_list.next = &s_module_list;
    s_module_list.prev = &s_module_list;
    s_module_list.node.module_id = 0;
    s_module_list.node.mq = NULL;
    s_module_list.node.name = NULL;

    if (p_config != NULL) {
        s_debug_enable = p_config->enable_debug;
    }

#if MF_MEM_POOL_ENABLE
    mf_mem_pool_init();
#endif

    s_initialized = 1;
    return MF_OK;
}

/**
 * @brief       获取框架版本号
 * @return      版本字符串
 */
const char* mf_version(void)
{
    return MF_VERSION_STR;
}


/**
 * @brief       查找模块节点
 * @param[in]   module_id 模块ID
 * @return      模块节点指针，未找到返回NULL
 */
static mf_entry_t* mf_find_module(uint8_t module_id)
{
    mf_entry_t* p_entry = s_module_list.next;

    while (p_entry != &s_module_list) {
        if (p_entry->node.module_id == module_id) {
            return p_entry;
        }
        p_entry = p_entry->next;
    }

    return NULL;
}

/**
 * @brief       内部发送消息
 * @param[in]   dest_moduleid 目标模块ID
 * @param[in]   p_msg 消息指针
 * @return      MF_OK成功，MF_EEMPTY未找到模块，MF_EFULL队列满
 */
static int mf_internal_send(uint8_t dest_moduleid, mf_message_t* p_msg)
{
    mf_entry_t* p_entry = mf_find_module(dest_moduleid);
    if (p_entry == NULL || p_entry->node.mq == NULL) {
        return MF_EEMPTY;
    }

    int ret = mf_port_mq_send(p_entry->node.mq, p_msg, sizeof(mf_message_t));
    if (ret != MF_PORT_OK) {
        return MF_EFULL;
    }

    return MF_OK;
}


/**
 * @brief       绑定模块到消息队列
 * @param[in]   module_id 模块ID（应用层定义）
 * @param[in]   name 模块名称（用于队列命名）
 * @param[in]   msg_size 单条消息大小
 * @param[in]   max_msgs 队列最大消息数
 * @return      MF_OK成功，其他失败
 * @note        每个模块只需绑定一次
 */
int mf_module_bind(uint8_t module_id, const char* name,
                  uint32_t msg_size, uint32_t max_msgs)
{
    if (!s_initialized) {
        mf_init(NULL);
    }

    /* 检查是否已绑定 */
    if (mf_find_module(module_id) != NULL) {
        return MF_EEMPTY;
    }

    /* 创建消息队列 */
    mf_port_mq_cfg_t cfg;
    cfg.name = name;
    cfg.msg_size = msg_size;
    cfg.max_msgs = max_msgs;

    mf_port_mq_t mq = mf_port_mq_create(&cfg);
    if (mq == NULL) {
        return MF_ENOMEM;
    }

    /* 分配节点 */
    mf_entry_t* p_entry = (mf_entry_t*)mf_port_malloc(sizeof(mf_entry_t));
    if (p_entry == NULL) {
        mf_port_mq_delete(mq);
        return MF_ENOMEM;
    }

    /* 填充节点 */
    p_entry->node.module_id = module_id;
    p_entry->node.mq = mq;
    p_entry->node.name = name;

    /* 插入链表 */
    mf_port_enter_critical();
    mf_port_list_insert_before(&s_module_list.prev->next, p_entry);
    mf_port_exit_critical();

    return MF_OK;
}

/**
 * @brief       解绑模块
 * @param[in]   module_id 模块ID
 * @return      MF_OK成功，其他失败
 */
int mf_module_unbind(uint8_t module_id)
{
    mf_entry_t* p_entry = mf_find_module(module_id);
    if (p_entry == NULL) {
        return MF_EEMPTY;
    }

    /* 删除队列 */
    if (p_entry->node.mq != NULL) {
        mf_port_mq_delete(p_entry->node.mq);
    }

    /* 从链表移除 */
    mf_port_enter_critical();
    mf_port_list_remove(p_entry);
    mf_port_exit_critical();

    /* 释放节点 */
    mf_port_free(p_entry);

    return MF_OK;
}

/**
 * @brief       检查模块是否已绑定
 * @param[in]   module_id 模块ID
 * @return      1已绑定，0未绑定
 */
int mf_module_is_bound(uint8_t module_id)
{
    return (mf_find_module(module_id) != NULL) ? 1 : 0;
}


/**
 * @brief       发送消息
 * @param[in]   src_moduleid 源模块ID
 * @param[in]   dest_moduleid 目标模块ID
 * @param[in]   msg_code 消息码
 * @param[in]   msg_para 消息参数
 * @param[in]   p_data 数据指针（可为NULL）
 * @param[in]   data_len 数据长度（为0表示无数据）
 * @return      MF_OK成功，其他失败
 * @note        框架内部会复制数据，调用方可释放
 */
int mf_send_msg(uint8_t src_moduleid, uint8_t dest_moduleid,
                uint32_t msg_code, uint32_t msg_para,
                void* p_data, uint32_t data_len)
{
    uint8_t* p_copy = NULL;

    /* 复制数据 */
    if (p_data != NULL && data_len > 0) {
#if MF_MEM_POOL_ENABLE
        p_copy = (uint8_t*)mf_mempool_alloc(data_len);
#else
        p_copy = (uint8_t*)mf_port_malloc(data_len);
#endif
        if (p_copy == NULL) {
            return MF_ENOMEM;
        }
        memcpy(p_copy, p_data, data_len);
    }

    /* 构建消息 */
    mf_message_t msg;
    memset(&msg, 0, sizeof(msg));
    msg.src_moduleid = src_moduleid;
    msg.dest_moduleid = dest_moduleid;
    msg.msg_code = msg_code;
    msg.msg_para = msg_para;
    msg.pdata = p_copy;
    msg.datalen = (uint16_t)data_len;

    /* 发送到目标模块队列 */
    int ret = mf_internal_send(dest_moduleid, &msg);
    if (ret != MF_OK) {
        if (p_copy != NULL) {
#if MF_MEM_POOL_ENABLE
            mf_mempool_free(p_copy);
#else
            mf_port_free(p_copy);
#endif
        }
        return ret;
    }

    return MF_OK;
}

/**
 * @brief       接收消息
 * @param[in]   module_id 本模块ID
 * @param[out]  p_msg 消息缓冲区
 * @param[in]   msg_size 缓冲区大小
 * @param[in]   timeout_ms 超时时间（ms），-1永久等待，0非阻塞
 * @return      MF_OK成功，其他失败
 * @note        框架自动复制数据到缓冲区，需调用mf_free_msg释放内部内存
 */
int mf_recv_msg(uint8_t module_id, mf_message_t* p_msg,
                uint32_t msg_size, int32_t timeout_ms)
{
    if (p_msg == NULL || msg_size < sizeof(mf_message_t)) {
        return MF_EINVAL;
    }

    mf_entry_t* p_entry = mf_find_module(module_id);
    if (p_entry == NULL || p_entry->node.mq == NULL) {
        return MF_EEMPTY;
    }

    /* 接收消息 */
    int ret = mf_port_mq_recv(p_entry->node.mq, p_msg, sizeof(mf_message_t), timeout_ms);
    if (ret != MF_PORT_OK) {
        return MF_EINVAL;
    }

    return MF_OK;
}

/**
 * @brief       释放消息内部数据
 * @param[in]   p_msg 消息指针
 * @note        接收消息后必须调用此函数释放内部内存
 */
void mf_free_msg(mf_message_t* p_msg)
{
    if (p_msg == NULL) {
        return;
    }

    if (p_msg->pdata != NULL) {
#if MF_MEM_POOL_ENABLE
        mf_mempool_free(p_msg->pdata);
#else
        mf_port_free(p_msg->pdata);
#endif
        p_msg->pdata = NULL;
    }
    p_msg->datalen = 0;
}


/**
 * @brief       获取模块队列中的消息数量
 * @param[in]   module_id 模块ID
 * @return      消息数，<0表示失败
 */
int mf_get_msg_count(uint8_t module_id)
{
    (void)module_id;
    return -1;
}

/**
 * @brief       清空模块队列
 * @param[in]   module_id 模块ID
 * @return      MF_OK成功，其他失败
 */
int mf_flush(uint8_t module_id)
{
    mf_entry_t* p_entry = mf_find_module(module_id);
    if (p_entry == NULL) {
        return MF_EEMPTY;
    }

    /* 清空队列中所有消息 */
    mf_message_t msg;
    while (mf_port_mq_recv(p_entry->node.mq, &msg, sizeof(msg), 0) == MF_PORT_OK) {
        if (msg.pdata != NULL) {
#if MF_MEM_POOL_ENABLE
            mf_mempool_free(msg.pdata);
#else
            mf_port_free(msg.pdata);
#endif
        }
    }

    return MF_OK;
}

/**
 * @brief       设置调试输出回调
 * @param[in]   printf_cb 打印回调函数
 */
void mf_set_debug_cb(void (*printf_cb)(const char* fmt, ...))
{
    s_printf_cb = printf_cb;
}
