#include "os_memory.h"
#include <string.h>

/* ================================================================
   BI?N N?I B?
   ================================================================ */

/* Vùng nh? th?t s? – khai báo static d? n?m trong BSS segment */
static uint8_t  s_pool[MEM_NUM_BLOCKS][MEM_BLOCK_SIZE];

/* Bitmap tr?ng thái: 0 = block tr?ng, 1 = block dang dùng */
static uint8_t  s_used[MEM_NUM_BLOCKS];

/* Th?ng kê */
static MemStats_t s_stats;

/* ================================================================
   KH?I T?O
   ================================================================ */
void OS_Mem_Init(void) {
    OS_ENTER_CRITICAL();

    memset(s_pool, 0,    sizeof(s_pool));
    memset(s_used, 0,    sizeof(s_used));
    memset(&s_stats, 0,  sizeof(s_stats));

    s_stats.total_blocks = MEM_NUM_BLOCKS;
    s_stats.free_blocks  = MEM_NUM_BLOCKS;

    OS_EXIT_CRITICAL();
}

/* ================================================================
   C?P PHÁT BLOCK
   ================================================================ */
void* OS_Mem_Alloc(void) {
    void *ptr = NULL;

    OS_ENTER_CRITICAL();

    /* Tìm block d?u tiên còn tr?ng */
    for (uint16_t i = 0; i < MEM_NUM_BLOCKS; i++) {
        if (s_used[i] == 0) {
            s_used[i] = 1;                  /* Ðánh d?u dã dùng */
            ptr = (void *)s_pool[i];        /* Tr? v? con tr? block */

            /* C?p nh?t th?ng kê */
            s_stats.used_blocks++;
            s_stats.free_blocks--;
            s_stats.alloc_count++;
            if (s_stats.used_blocks > s_stats.peak_used) {
                s_stats.peak_used = s_stats.used_blocks;
            }
            break;
        }
    }

    if (ptr == NULL) {
        s_stats.fail_count++;   /* Ð?m s? l?n c?p phát th?t b?i */
    }

    OS_EXIT_CRITICAL();
    return ptr;
}

/* ================================================================
   GI?I PHÓNG BLOCK
   ================================================================ */
MemStatus_t OS_Mem_Free(void *ptr) {
    if (ptr == NULL) return MEM_ERR_INVALID;

    /* Tính index c?a block t? d?a ch? con tr? */
    uint8_t *base = (uint8_t *)s_pool;
    uint8_t *p    = (uint8_t *)ptr;

    /* Ki?m tra con tr? có n?m trong pool không */
    if (p < base || p >= base + sizeof(s_pool)) {
        return MEM_ERR_INVALID;
    }

    /* Ki?m tra alignment – ph?i là d?u block */
    uint32_t offset = (uint32_t)(p - base);
    if (offset % MEM_BLOCK_SIZE != 0) {
        return MEM_ERR_INVALID;
    }

    uint16_t idx = (uint16_t)(offset / MEM_BLOCK_SIZE);

    OS_ENTER_CRITICAL();

    if (s_used[idx] == 0) {
        /* Block dã free r?i – double free, b? qua */
        OS_EXIT_CRITICAL();
        return MEM_ERR_INVALID;
    }

    /* Xóa block và dánh d?u tr?ng */
    memset(s_pool[idx], 0, MEM_BLOCK_SIZE);
    s_used[idx] = 0;

    /* C?p nh?t th?ng kê */
    s_stats.used_blocks--;
    s_stats.free_blocks++;
    s_stats.free_count++;

    OS_EXIT_CRITICAL();
    return MEM_OK;
}

/* ================================================================
   TH?NG KÊ & TI?N ÍCH
   ================================================================ */
void OS_Mem_GetStats(MemStats_t *out) {
    OS_ENTER_CRITICAL();
    *out = s_stats;
    OS_EXIT_CRITICAL();
}

uint8_t OS_Mem_IsValid(void *ptr) {
    if (ptr == NULL) return 0;
    uint8_t *base = (uint8_t *)s_pool;
    uint8_t *p    = (uint8_t *)ptr;
    if (p < base || p >= base + sizeof(s_pool)) return 0;
    uint32_t offset = (uint32_t)(p - base);
    if (offset % MEM_BLOCK_SIZE != 0) return 0;
    return 1;
}