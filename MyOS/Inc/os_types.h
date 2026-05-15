#ifndef OS_TYPES_H
#define OS_TYPES_H

#include "stm32f1xx_hal.h"
#include <stdint.h>
#include <stddef.h>

/* ================================================================
   OS_TYPES.H – Ð?nh nghia chung cho c? TV1 và TV2
   TV1 dùng: TaskState, TCB, OS_CreateTask, OS_Delay
   TV2 dùng: TaskState d? check owner c?a Mutex/Semaphore
   ================================================================ */

/* ===== C?U HÌNH H? TH?NG ===== */
#define OS_MAX_TASKS        8       /* T?i da 8 task d?ng th?i      */
#define OS_MAX_PRIORITIES   8       /* M?c uu tiên 0 (th?p) – 7 (cao) */
#define OS_TICK_RATE_HZ     1000    /* SysTick 1ms                   */

/* ===== TR?NG THÁI TASK ===== */
typedef enum {
    TASK_READY      = 0,    /* S?n sàng ch?y                */
    TASK_RUNNING    = 1,    /* Ðang ch?y                     */
    TASK_BLOCKED    = 2,    /* Ðang ch? (delay, mutex, queue)*/
    TASK_SUSPENDED  = 3,    /* B? t?m d?ng                   */
    TASK_DELETED    = 4,    /* Ðã b? xóa                     */
} TaskState_t;

/* ===== TASK CONTROL BLOCK – TV1 s? di?n d?y d? ===== */
/* TV2 ch? c?n task_id và state d? qu?n lý owner Mutex  */
typedef struct TCB {
    uint32_t    *stack_ptr;             /* Con tr? stack (TV1 qu?n lý)  */
    uint32_t    stack[128];             /* Stack 512 bytes m?i task      */
    TaskState_t  state;                 /* Tr?ng thái hi?n t?i           */
    uint8_t      priority;              /* M?c uu tiên 0–7               */
    uint8_t      task_id;               /* ID d?nh danh task             */
    char         name[12];             /* Tên task d? debug             */
    uint32_t     delay_ticks;           /* S? tick còn delay             */
    struct TCB  *next;                  /* Con tr? linked list (TV1 dùng)*/
} TCB_t;

/* ===== CRITICAL SECTION – T?t/b?t ng?t d? b?o v? d? li?u dùng chung ===== */
#define OS_ENTER_CRITICAL()     __disable_irq()
#define OS_EXIT_CRITICAL()      __enable_irq()

/* ===== API TV1 S? IMPLEMENT – TV2 g?i các hàm này ===== */
/* TV2 c?n bi?t task hi?n t?i dang ch?y d? gán owner Mutex */
extern TCB_t *OS_GetCurrentTask(void);
extern void   OS_Delay(uint32_t ticks);
extern void   OS_Yield(void);

#endif /* OS_TYPES_H */