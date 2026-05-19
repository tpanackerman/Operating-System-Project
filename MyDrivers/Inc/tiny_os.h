#ifndef TINY_OS_H
#define TINY_OS_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

#define OS_MAX_TASKS 8
#define OS_TICK_HZ 1000
#define OS_DEFAULT_SLICE 5

typedef void (*OS_TaskFunc_t)(void *arg);

typedef enum
{
    OS_TASK_UNUSED = 0,
    OS_TASK_READY,
    OS_TASK_RUNNING,
    OS_TASK_BLOCKED,
    OS_TASK_SUSPENDED
} OS_TaskState_t;

/*
 * TCB: Task Control Block
 * Lưu ý: sp phải là field đầu tiên vì PendSV assembly truy cập trực tiếp.
 */
typedef struct
{
    uint32_t *sp;

    uint32_t *stack_start;
    uint32_t stack_words;

    uint8_t id;
    uint8_t priority; /* Số càng nhỏ ưu tiên càng cao */
    OS_TaskState_t state;

    uint32_t delay_ticks;
    uint32_t time_slice_ticks;
    uint32_t slice_left;

    OS_TaskFunc_t task_func;
    void *arg;
    const char *name;
} OS_TCB_t;

extern OS_TCB_t *volatile OS_CurrentTCB;

void OS_Init(void);

int OS_CreateTask(OS_TaskFunc_t task_func,
                  void *arg,
                  uint32_t *stack_mem,
                  uint32_t stack_words,
                  uint8_t priority,
                  uint32_t time_slice_ticks,
                  const char *name);

void OS_Start(void);
void OS_Tick_Handler(void);
void OS_Delay(uint32_t ticks);
void OS_Yield(void);


uint32_t OS_GetTick(void);
uint8_t OS_GetTaskCount(void);

OS_TCB_t* OS_GetCurrentTask(void);
OS_TCB_t *OS_GetTaskInfo(uint8_t index);
uint32_t OS_GetStackFreeBytes(uint8_t index);

/* Hàm được PendSV gọi */
OS_TCB_t *OS_Schedule(void);

#endif
