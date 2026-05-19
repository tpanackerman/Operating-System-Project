#ifndef OS_MUTEX_H
#define OS_MUTEX_H

#include "os_types.h"

/* ================================================================
   OS_MUTEX.H – Khóa tài nguyên dùng chung gi?a các task
   
   V?n d? gi?i quy?t:
   Task A và Task B cùng mu?n ghi vào USB buffer ? n?u không khóa
   thì 2 task ghi dè lên nhau ? data b? h?ng.
   
   Cách dùng:
   OS_Mutex_Lock(&usb_mutex);      // Task A chi?m khóa
       ghi vào USB buffer...       // Vùng an toàn
   OS_Mutex_Unlock(&usb_mutex);    // Task A nh? khóa
                                   // Task B m?i du?c vào
   ================================================================ */

#define OS_MAX_MUTEXES      4       /* T?i da 4 mutex d?ng th?i */
#define OS_MUTEX_NO_WAIT    0       /* Không ch?, tr? v? ngay   */
#define OS_MUTEX_WAIT_FOREVER 0xFFFFFFFF

/* ===== TR?NG THÁI ===== */
typedef enum {
    MUTEX_OK        = 0,
    MUTEX_LOCKED    = 1,    /* Mutex dang b? lock b?i task khác */
    MUTEX_TIMEOUT   = 2,    /* H?t th?i gian ch?                */
    MUTEX_ERROR     = 3,    /* L?i (NULL, chua init...)         */
} MutexStatus_t;

/* ===== C?U TRÚC MUTEX ===== */
typedef struct {
    volatile uint8_t  is_locked;        /* 0 = m?, 1 = dang khóa       */
    volatile uint8_t  owner_id;         /* ID task dang gi? khóa        */
    volatile uint32_t lock_count;       /* S? l?n lock (d? debug)       */
    char              name[12];         /* Tên mutex d? debug           */
    uint8_t           initialized;      /* 1 = dã init                  */
} OS_Mutex_t;

/* ================================================================
   API CÔNG KHAI
   ================================================================ */

/**
 * @brief Kh?i t?o mutex. G?i tru?c khi dùng.
 * @param mutex Con tr? d?n bi?n OS_Mutex_t
 * @param name  Tên mutex (t?i da 11 ký t?)
 *
 * Ví d?:
 *   OS_Mutex_t usb_mutex;
 *   OS_Mutex_Init(&usb_mutex, "USB_Mutex");
 */
void OS_Mutex_Init(OS_Mutex_t *mutex, const char *name);

/**
 * @brief Chi?m khóa mutex.
 *        N?u mutex dang b? task khác gi? ? ch? d?n khi du?c.
 * @param mutex   Con tr? mutex
 * @param timeout S? ms t?i da ch?. OS_MUTEX_WAIT_FOREVER = ch? mãi.
 * @retval MUTEX_OK n?u l?y du?c khóa, MUTEX_TIMEOUT n?u h?t gi?.
 *
 * Ví d?:
 *   if (OS_Mutex_Lock(&usb_mutex, 100) == MUTEX_OK) {
 *       // Ghi data an toàn
 *       OS_Mutex_Unlock(&usb_mutex);
 *   }
 */
MutexStatus_t OS_Mutex_Lock(OS_Mutex_t *mutex, uint32_t timeout_ms);

/**
 * @brief Nh? khóa mutex. Ch? task dang gi? m?i du?c nh?.
 * @param mutex Con tr? mutex
 * @retval MUTEX_OK ho?c MUTEX_ERROR n?u task hi?n t?i không ph?i owner.
 */
MutexStatus_t OS_Mutex_Unlock(OS_Mutex_t *mutex);

/**
 * @brief Th? l?y khóa nhung KHÔNG ch? (non-blocking).
 * @retval MUTEX_OK n?u l?y du?c ngay, MUTEX_LOCKED n?u dang b?n.
 *
 * Ví d?: Dùng khi không mu?n task b? block:
 *   if (OS_Mutex_TryLock(&usb_mutex) == MUTEX_OK) {
 *       // Làm gì dó
 *       OS_Mutex_Unlock(&usb_mutex);
 *   } else {
 *       // Mutex b?n, làm vi?c khác
 *   }
 */
MutexStatus_t OS_Mutex_TryLock(OS_Mutex_t *mutex);

/**
 * @brief Ki?m tra mutex có dang b? khóa không.
 */
uint8_t OS_Mutex_IsLocked(OS_Mutex_t *mutex);

/**
 * @brief L?y ID task dang gi? mutex (d? debug).
 */
uint8_t OS_Mutex_GetOwner(OS_Mutex_t *mutex);

#endif /* OS_MUTEX_H */
