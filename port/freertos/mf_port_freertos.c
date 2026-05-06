/**
 * @file mf_port_freertos.c
 * @brief FreeRTOS 适配层实现
 *
 * 使用 FreeRTOS 的队列和静态内存实现适配层接口。
 */
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "portmacro.h"
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
    QueueHandle_t handle;
    uint32_t msg_size;
} mf_port_mq_impl_t;

mf_port_mq_t mf_port_mq_create(mf_port_mq_cfg_t* p_cfg)
{
    mf_port_mq_impl_t* p_impl = pvPortMalloc(sizeof(mf_port_mq_impl_t));
    if (p_impl == NULL) {
        return NULL;
    }

    p_impl->handle = xQueueCreate(p_cfg->max_msgs, p_cfg->msg_size);
    if (p_impl->handle == NULL) {
        vPortFree(p_impl);
        return NULL;
    }

    p_impl->msg_size = p_cfg->msg_size;
    return (mf_port_mq_t)p_impl;
}

void mf_port_mq_delete(mf_port_mq_t mq)
{
    if (mq) {
        mf_port_mq_impl_t* p_impl = (mf_port_mq_impl_t*)mq;
        vQueueDelete(p_impl->handle);
        vPortFree(p_impl);
    }
}

int mf_port_mq_send(mf_port_mq_t mq, void* p_msg, uint32_t size)
{
    mf_port_mq_impl_t* p_impl = (mf_port_mq_impl_t*)mq;
    (void)size;

    if (xQueueSend(p_impl->handle, p_msg, 0) != pdTRUE) {
        return MF_PORT_EFULL;
    }
    return MF_PORT_OK;
}

int mf_port_mq_recv(mf_port_mq_t mq, void* p_msg, uint32_t size, int32_t timeout_ms)
{
    mf_port_mq_impl_t* p_impl = (mf_port_mq_impl_t*)mq;
    TickType_t ticks = (timeout_ms < 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    (void)size;

    if (xQueueReceive(p_impl->handle, p_msg, ticks) == pdTRUE) {
        return MF_PORT_OK;
    }
    return MF_PORT_EINVAL;
}

/* ============================================================
 * 内存管理实现
 * ============================================================ */
void* mf_port_malloc(uint32_t size)
{
    return pvPortMalloc(size);
}

void mf_port_free(void* ptr)
{
    vPortFree(ptr);
}

/* ============================================================
 * 临界区实现
 * ============================================================ */
typedef struct {
    StaticSemaphore_t buffer;
    SemaphoreHandle_t handle;
} mf_port_critical_impl_t;

static mf_port_critical_impl_t s_critical;
static int s_critical_init = 0;

void mf_port_enter_critical(void)
{
    if (!s_critical_init) {
        s_critical.handle = xSemaphoreCreateMutexStatic(&s_critical.buffer);
        s_critical_init = 1;
    }
    xSemaphoreTake(s_critical.handle, portMAX_DELAY);
}

void mf_port_exit_critical(void)
{
    xSemaphoreGive(s_critical.handle);
}

/* ============================================================
 * 调试输出实现
 * ============================================================ */
void mf_port_printf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    /* 实现日志输出，例如通过 UART 或系统日志钩子 */
    (void)fmt;
    (void)args;
    va_end(args);
}
