/**
 * @file mf_mempool.c
 * @brief 消息框架内存池实现
 */
#include <stdint.h>
#include <string.h>
#include "mf_mempool.h"
#include "mf_port.h"

#if MF_MEM_POOL_ENABLE

#define MF_MEM_POOL_MAGIC   0xABCD1234U

/** 内存块头信息（占用块头部空间） */
typedef struct {
    uint32_t magic;      /**< 魔数，用于校验 */
    uint32_t size;       /**< 块大小 */
} mf_mem_block_hdr_t;

/** 内存池状态 */
typedef struct {
    uint8_t*  pool_buf;     /**< 池缓冲区 */
    uint32_t  pool_size;    /**< 池大小 */
    uint32_t  block_size;   /**< 块大小（含头部） */
    uint32_t  block_count;  /**< 总块数 */
    uint32_t  free_count;   /**< 空闲块数 */
} mf_mem_pool_t;

static mf_mem_pool_t s_mem_pool;
static int s_mem_pool_init = 0;

/**
 * @brief       初始化内存池
 * @return      MF_OK 成功，其他失败
 */
int mf_mempool_init(void)
{
    if (s_mem_pool_init) {
        return MF_OK;
    }

    uint32_t block_total_size = MF_MEM_POOL_BLOCK_SIZE + sizeof(mf_mem_block_hdr_t);
    uint32_t block_count = MF_MEM_POOL_BUF_SIZE / block_total_size;

    if (block_count == 0) {
        return MF_EINVAL;
    }

    s_mem_pool.pool_buf = (uint8_t*)mf_port_malloc(MF_MEM_POOL_BUF_SIZE);
    if (s_mem_pool.pool_buf == NULL) {
        return MF_ENOMEM;
    }

    s_mem_pool.pool_size = MF_MEM_POOL_BUF_SIZE;
    s_mem_pool.block_size = block_total_size;
    s_mem_pool.block_count = block_count;
    s_mem_pool.free_count = block_count;

    s_mem_pool_init = 1;
    return MF_OK;
}

/**
 * @brief       从内存池分配内存
 * @param[in]   size 请求大小
 * @return      内存指针，NULL 失败
 * @note        如果请求大小超过块大小，使用原生 malloc
 */
void* mf_mempool_alloc(uint32_t size)
{
    if (!s_mem_pool_init) {
        mf_mempool_init();
    }

    uint32_t block_total_size = MF_MEM_POOL_BLOCK_SIZE + sizeof(mf_mem_block_hdr_t);

    if (size > MF_MEM_POOL_BLOCK_SIZE) {
        return mf_port_malloc(size);
    }

    if (s_mem_pool.free_count == 0) {
        return mf_port_malloc(size);
    }

    uint8_t* ptr = s_mem_pool.pool_buf;
    uint32_t block_count = s_mem_pool.block_count;

    for (uint32_t i = 0; i < block_count; i++) {
        uint8_t* block_ptr = ptr + (i * s_mem_pool.block_size);
        mf_mem_block_hdr_t* p_hdr = (mf_mem_block_hdr_t*)block_ptr;

        if (p_hdr->magic == 0) {
            p_hdr->magic = MF_MEM_POOL_MAGIC;
            p_hdr->size = size;
            s_mem_pool.free_count--;
            return (void*)(block_ptr + sizeof(mf_mem_block_hdr_t));
        }
    }

    return mf_port_malloc(size);
}

/**
 * @brief       释放内存到内存池
 * @param[in]   ptr 内存指针
 * @note        如果指针由内存池管理则回收，否则使用原生 free
 */
void mf_mempool_free(void* ptr)
{
    if (ptr == NULL) {
        return;
    }

    uint8_t* block_ptr = (uint8_t*)ptr - sizeof(mf_mem_block_hdr_t);
    mf_mem_block_hdr_t* p_hdr = (mf_mem_block_hdr_t*)block_ptr;

    if (p_hdr->magic == MF_MEM_POOL_MAGIC) {
        if (block_ptr >= s_mem_pool.pool_buf &&
            block_ptr < (s_mem_pool.pool_buf + s_mem_pool.pool_size)) {
            p_hdr->magic = 0;
            p_hdr->size = 0;
            s_mem_pool.free_count++;
            return;
        }
    }

    mf_port_free(ptr);
}

/**
 * @brief       获取内存池空闲块数
 * @return      空闲块数
 */
uint32_t mf_mempool_get_free_count(void)
{
    if (!s_mem_pool_init) {
        return 0;
    }
    return s_mem_pool.free_count;
}

#endif /* MF_MEM_POOL_ENABLE */
