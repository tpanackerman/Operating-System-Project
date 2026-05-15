#ifndef OS_SEMAPHORE_H
#define OS_SEMAPHORE_H

#include "os_types.h"

/* ================================================================
   OS_SEMAPHORE.H – Ð?ng b? hóa gi?a các task
   
   Khác v?i Mutex:
   - Mutex: "ch? 1 task du?c vào" (b?o v? tài nguyên)
   - Semaphore: "d?m s? lu?ng tài nguyên còn l?i"
   
   Ví d? th?c t? trong d? án:
   - TV5 d?c GPIO và "post" semaphore khi có d? li?u m?i
   - TV3 "wait" semaphore r?i g?i data lên USB
   ? Ð?m b?o TV3 ch? g?i khi th?t s? có data m?i
   
   Binary Semaphore (max=1): thay th? Mutex don gi?n
   Counting Semaphore (max>1): qu?n lý nhi?u tài nguyên
   ================================================================ */

#define OS_SEM_NO_WAIT      0
#define OS_SEM_WAIT_FOREVER 0xFFFFFFFF

/* ===== TR?NG THÁI ===== */
typedef enum {
    SEM_OK      = 0,
    SEM_TIMEOUT = 1,    /* H?t th?i gian ch?        */
    SEM_FULL    = 2,    /* Semaphore dã d?y (count = max) */
    SEM_ERROR   = 3,
} SemStatus_t;

/* ===== C?U TRÚC SEMAPHORE ===== */
typedef struct {
    volatile int32_t  count;        /* S? lu?ng tài nguyên hi?n có  */
    int32_t           max_count;    /* Gi?i h?n t?i da              */
    uint32_t          post_count;   /* T?ng s? l?n post (d? debug)  */
    uint32_t          wait_count;   /* T?ng s? l?n wait thành công  */
    char              name[12];
    uint8_t           initialized;
} OS_Sem_t;

/* ================================================================
   API CÔNG KHAI
   ================================================================ */

/**
 * @brief Kh?i t?o semaphore.
 * @param sem           Con tr? OS_Sem_t
 * @param initial_count Giá tr? d?m ban d?u (0 = b?t d?u b? block)
 * @param max_count     Giá tr? t?i da (1 = binary semaphore)
 * @param name          Tên d? debug
 *
 * Ví d? binary semaphore (b?t d?u locked):
 *   OS_Sem_t data_ready;
 *   OS_Sem_Init(&data_ready, 0, 1, "DataReady");
 *
 * Ví d? counting semaphore (3 tài nguyên):
 *   OS_Sem_t buf_slots;
 *   OS_Sem_Init(&buf_slots, 3, 3, "BufSlots");
 */
void OS_Sem_Init(OS_Sem_t *sem, int32_t initial_count,
                 int32_t max_count, const char *name);

/**
 * @brief Ch? semaphore (gi?m count). Block n?u count = 0.
 * @param sem        Con tr? semaphore
 * @param timeout_ms Th?i gian ch? t?i da (ms). OS_SEM_WAIT_FOREVER = mãi.
 * @retval SEM_OK n?u l?y du?c, SEM_TIMEOUT n?u h?t gi?.
 *
 * Ví d?: Task USB ch? có data m?i t? GPIO task:
 *   OS_Sem_Wait(&data_ready, OS_SEM_WAIT_FOREVER);
 *   // Ð?n dây ch?c ch?n có data m?i
 *   USB_CDC_Send(buffer, len);
 */
SemStatus_t OS_Sem_Wait(OS_Sem_t *sem, uint32_t timeout_ms);

/**
 * @brief Post semaphore (tang count). Không block.
 * @param sem Con tr? semaphore
 * @retval SEM_OK ho?c SEM_FULL n?u count dã b?ng max.
 *
 * Ví d?: GPIO task báo hi?u có data m?i:
 *   GPIO_ReadSensor(&buffer);
 *   OS_Sem_Post(&data_ready);   // Báo cho USB task bi?t
 */
SemStatus_t OS_Sem_Post(OS_Sem_t *sem);

/**
 * @brief L?y giá tr? count hi?n t?i (không block).
 */
int32_t OS_Sem_GetCount(OS_Sem_t *sem);

/**
 * @brief Reset semaphore v? giá tr? ban d?u.
 */
void OS_Sem_Reset(OS_Sem_t *sem);

#endif /* OS_SEMAPHORE_H */