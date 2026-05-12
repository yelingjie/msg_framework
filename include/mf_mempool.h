/**
 * @file mf_mempool.h
 * @brief 消息框架内存池
 *
 * 提供固定大小块的内存池，减少频繁 malloc/free 导致的内存碎片。
 *
 * 使用方式：
 * @code
 * // 在 include msg_framework.h 之前定义宏
 * #define MF_MEM_POOL_ENABLE     1
 * #define MF_MEM_POOL_BUF_SIZE   4096
 * #define MF_MEM_POOL_BLOCK_SIZE 128
 *
 * #include "msg_framework.h"
 * @endcode
 */
#ifndef MF_MEMPOOL_H
#define MF_MEMPOOL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * 内存池 API
 * ============================================================ */
/**
 * @brief 初始化内存池
 * @return MF_OK 成功，其他失败
 */
int mf_mempool_init(void);

/**
 * @brief 从内存池分配内存
 * @param[in] size 请求大小
 * @return 内存指针，NULL 失败
 * @note 如果请求大小超过块大小，使用原生 malloc
 */
void* mf_mempool_alloc(uint32_t size);

/**
 * @brief 释放内存到内存池
 * @param[in] ptr 内存指针
 * @note 如果指针由内存池管理则回收，否则使用原生 free
 */
void mf_mempool_free(void* ptr);

/**
 * @brief 获取内存池空闲块数
 * @return 空闲块数
 */
uint32_t mf_mempool_get_free_count(void);

#ifdef __cplusplus
}
#endif

#endif /* MF_MEMPOOL_H */
