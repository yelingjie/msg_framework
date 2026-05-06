/**
 * @file mf_port_zephyr.c
 * @brief Zephyr 适配层实现
 *
 * 使用 Zephyr 的 k_msgq 和系统内存管理实现适配层接口。
 */
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <sys/util.h>
#include <kernel.h>
#include <sys/mempool.h>
#include <init.h>
#include "mf_port.h"

/* ============================================================
 * 链表实现
 * ============================================================ */
typedef struct mf_port_list_node {
    struct mf_port_list_node* next;
    struct mf_port_list_node* prev;
} mf_port_list_node_t;

void mf_port_list_init(mf_port_list_t p_list)
{
    mf_port_list_node_t* head = (mf_port_list_node_t*)p_list;
    head->next = head->prev = head;
}

int mf_port_list_isempty(mf_port_list_t p_list)
{
    mf_port_list_node_t* head = (mf_port_list_node_t*)p_list;
    return (head->next == head) ? 1 : 0;
}

void mf_port_list_insert_before(void* p_before, void* p_node)
{
    mf_port_list_node_t* before = (mf_port_list_node_t*)p_before;
    mf_port_list_node_t* node = (mf_port_list_node_t*)p_node;
    mf_port_list_node_t* prev = before->prev;

    node->next = before;
    node->prev = prev;
    prev->next = node;
    before->prev = node;
}

void mf_port_list_remove(void* p_node)
{
    mf_port_list_node_t* node = (mf_port_list_node_t*)p_node;
    node->prev->next = node->next;
    node->next->prev = node->prev;
}

/* ============================================================
 * 消息队列实现
 * ============================================================ */
typedef struct {
    struct k_msgq handle;
    char msgq_buffer[CONFIG_MSGQ_MAX_MSG_SIZE];
} mf_port_mq_impl_t;

mf_port_mq_t mf_port_mq_create(mf_port_mq_cfg_t* p_cfg)
{
    mf_port_mq_impl_t* p_impl = k_malloc(sizeof(mf_port_mq_impl_t));
    if (p_impl == NULL) {
        return NULL;
    }

    /* 计算消息对齐大小 */
    uint32_t msg_size = ROUND_UP(p_cfg->msg_size, sizeof(void*));

    /* 创建消息队列 */
    k_msgq_init(&p_impl->handle,
                p_impl->msgq_buffer,
                msg_size,
                p_cfg->max_msgs);

    return (mf_port_mq_t)p_impl;
}

void mf_port_mq_delete(mf_port_mq_t mq)
{
    if (mq) {
        mf_port_mq_impl_t* p_impl = (mf_port_mq_impl_t*)mq;
        k_msgq_purge(&p_impl->handle);
        k_free(p_impl);
    }
}

int mf_port_mq_send(mf_port_mq_t mq, void* p_msg, uint32_t size)
{
    mf_port_mq_impl_t* p_impl = (mf_port_mq_impl_t*)mq;
    (void)size;

    int ret = k_msgq_put(&p_impl->handle, p_msg, K_NO_WAIT);
    if (ret == 0) {
        return MF_PORT_OK;
    }
    return MF_PORT_EFULL;
}

int mf_port_mq_recv(mf_port_mq_t mq, void* p_msg, uint32_t size, int32_t timeout_ms)
{
    mf_port_mq_impl_t* p_impl = (mf_port_mq_impl_t*)mq;
    (void)size;

    k_timeout_t timeout;
    if (timeout_ms < 0) {
        timeout = K_FOREVER;
    } else if (timeout_ms == 0) {
        timeout = K_NO_WAIT;
    } else {
        timeout = K_MSEC(timeout_ms);
    }

    int ret = k_msgq_get(&p_impl->handle, p_msg, timeout);
    if (ret == 0) {
        return MF_PORT_OK;
    }
    return MF_PORT_EINVAL;
}

/* ============================================================
 * 内存管理实现
 * ============================================================ */
void* mf_port_malloc(uint32_t size)
{
    return k_malloc(size);
}

void mf_port_free(void* ptr)
{
    k_free(ptr);
}

/* ============================================================
 * 临界区实现
 * ============================================================ */
static struct k_spinlock s_critical_lock;

void mf_port_enter_critical(void)
{
    k_spinlock_key_t key = k_spin_lock(&s_critical_lock);
    (void)key;
}

void mf_port_exit_critical(void)
{
    k_spin_unlock(&s_critical_lock);
}

/* ============================================================
 * 调试输出实现
 * ============================================================ */
void mf_port_printf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    vprintk(fmt, args);
    va_end(args);
}
