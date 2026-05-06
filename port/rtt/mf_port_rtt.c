/**
 * @file mf_port_rtt.c
 * @brief RT-Thread 适配层实现
 *
 * 使用 RT-Thread 的消息队列和链表实现适配层接口。
 */
#include <rtthread.h>
#include <stdarg.h>
#include "mf_port.h"

/* ============================================================
 * 链表实现
 * ============================================================ */
void mf_port_list_init(mf_port_list_t p_list)
{
    rt_list_init((rt_list_t)p_list);
}

int mf_port_list_isempty(mf_port_list_t p_list)
{
    return rt_list_isempty((rt_list_t)p_list) == RT_FALSE ? 0 : 1;
}

void mf_port_list_insert_before(void* p_before, void* p_node)
{
    rt_list_insert_before((struct rt_list_node*)p_before,
                          (struct rt_list_node*)p_node);
}

void mf_port_list_remove(void* p_node)
{
    rt_list_remove((struct rt_list_node*)p_node);
}

/* ============================================================
 * 消息队列实现
 * ============================================================ */
mf_port_mq_t mf_port_mq_create(mf_port_mq_cfg_t* p_cfg)
{
    return (mf_port_mq_t)rt_mq_create(p_cfg->name,
                                      p_cfg->msg_size,
                                      p_cfg->max_msgs,
                                      RT_IPC_FLAG_FIFO);
}

void mf_port_mq_delete(mf_port_mq_t mq)
{
    if (mq) {
        rt_mq_delete((rt_mq_t)mq);
    }
}

int mf_port_mq_send(mf_port_mq_t mq, void* p_msg, uint32_t size)
{
    rt_err_t err = rt_mq_send((rt_mq_t)mq, p_msg, size);
    if (err != RT_EOK) {
        return MF_PORT_EFULL;
    }
    return MF_PORT_OK;
}

int mf_port_mq_recv(mf_port_mq_t mq, void* p_msg, uint32_t size, int32_t timeout_ms)
{
    rt_err_t err = rt_mq_recv((rt_mq_t)mq, p_msg, size, timeout_ms);
    if (err == RT_EOK) {
        return MF_PORT_OK;
    }
    return MF_PORT_EINVAL;
}

/* ============================================================
 * 内存管理实现
 * ============================================================ */
void* mf_port_malloc(uint32_t size)
{
    return rt_malloc(size);
}

void mf_port_free(void* ptr)
{
    rt_free(ptr);
}

/* ============================================================
 * 临界区实现
 * ============================================================ */
void mf_port_enter_critical(void)
{
    rt_enter_critical();
}

void mf_port_exit_critical(void)
{
    rt_exit_critical();
}

/* ============================================================
 * 调试输出实现
 * ============================================================ */
void mf_port_printf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    rt_kprintf(fmt, args);
    va_end(args);
}