#ifndef OS_MEMORY_H
#define OS_MEMORY_H

#include "os_types.h"

/* ================================================================
   OS_MEMORY.H – Qu?n lý b? nh? d?ng b?ng Memory Pool
   Thay th? malloc/free – không b? phân m?nh b? nh?
   
   Cách ho?t d?ng:
   +------------------------------------------+
   ¦         MEMORY POOL (1024 bytes)         ¦
   +------------------------------------------¦
   ¦ Block0 ¦ Block1 ¦ Block2 ¦  ...  Block31 ¦
   ¦ 32B    ¦ 32B    ¦ 32B    ¦       32B     ¦
   +------------------------------------------+
   M?i block: c?p phát ho?c gi?i phóng nguyên kh?i
   ? Không bao gi? b? phân m?nh
   ================================================================ */

/* ===== C?U HÌNH ===== */
#define MEM_POOL_TOTAL_SIZE     1024    /* T?ng 1KB                     */
#define MEM_BLOCK_SIZE          32      /* M?i block 32 bytes           */
#define MEM_NUM_BLOCKS          (MEM_POOL_TOTAL_SIZE / MEM_BLOCK_SIZE)  /* = 32 blocks */

/* ===== TR?NG THÁI TR? V? ===== */
typedef enum {
    MEM_OK          = 0,
    MEM_ERR_FULL    = 1,    /* H?t block tr?ng       */
    MEM_ERR_INVALID = 2,    /* Con tr? không h?p l?  */
} MemStatus_t;

/* ===== TH?NG KÊ (TV4 l?y d? v? dashboard) ===== */
typedef struct {
    uint16_t total_blocks;      /* T?ng s? block: 32        */
    uint16_t used_blocks;       /* S? block dang dùng       */
    uint16_t free_blocks;       /* S? block còn tr?ng       */
    uint16_t peak_used;         /* Cao nh?t t?ng dùng       */
    uint32_t alloc_count;       /* T?ng s? l?n c?p phát     */
    uint32_t free_count;        /* T?ng s? l?n gi?i phóng   */
    uint32_t fail_count;        /* S? l?n c?p phát th?t b?i */
} MemStats_t;

/* ================================================================
   API CÔNG KHAI
   ================================================================ */

/**
 * @brief Kh?i t?o Memory Pool. G?i 1 l?n trong main() tru?c OS_Start().
 */
void OS_Mem_Init(void);

/**
 * @brief C?p phát 1 block b? nh? (32 bytes).
 * @retval Con tr? d?n block, ho?c NULL n?u h?t b? nh?.
 *
 * Ví d?:
 *   uint8_t *buf = (uint8_t *)OS_Mem_Alloc();
 *   if (buf == NULL) { // x? lý l?i h?t RAM }
 */
void* OS_Mem_Alloc(void);

/**
 * @brief Gi?i phóng block b? nh? dã c?p phát.
 * @param ptr Con tr? tr? v? t? OS_Mem_Alloc().
 * @retval MEM_OK ho?c MEM_ERR_INVALID n?u con tr? sai.
 *
 * Ví d?:
 *   OS_Mem_Free(buf);
 *   buf = NULL;  // Nh? set NULL sau khi free
 */
MemStatus_t OS_Mem_Free(void *ptr);

/**
 * @brief L?y th?ng kê b? nh? hi?n t?i.
 */
void OS_Mem_GetStats(MemStats_t *out);

/**
 * @brief Ki?m tra con tr? có thu?c memory pool không.
 * @retval 1 = h?p l?, 0 = không h?p l?
 */
uint8_t OS_Mem_IsValid(void *ptr);

#endif /* OS_MEMORY_H */