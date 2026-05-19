#include "tiny_os.h"

static OS_TCB_t os_tasks[OS_MAX_TASKS];

OS_TCB_t *volatile OS_CurrentTCB = 0;

static volatile uint32_t os_tick = 0;
static uint8_t os_task_count = 0;
static int32_t os_current_index = -1;
static uint8_t os_started = 0;

static void OS_TriggerPendSV(void)
{
    SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;
    __DSB();
    __ISB();
}

static void OS_TaskExit_Error(void)
{
    __disable_irq();

    if (OS_CurrentTCB != 0)
    {
        OS_CurrentTCB->state = OS_TASK_SUSPENDED;
    }

    __enable_irq();
    OS_Yield();

    while (1)
    {
    }
}

void OS_Init(void)
{
    __disable_irq();

    os_tick = 0;
    os_task_count = 0;
    os_current_index = -1;
    os_started = 0;
    OS_CurrentTCB = 0;

    for (uint8_t i = 0; i < OS_MAX_TASKS; i++)
    {
        os_tasks[i].state = OS_TASK_UNUSED;
    }

    uint32_t lowest_prio = (1UL << __NVIC_PRIO_BITS) - 1UL;

    NVIC_SetPriority(PendSV_IRQn, lowest_prio);
    NVIC_SetPriority(SysTick_IRQn, lowest_prio - 1UL);

    SysTick_Config(SystemCoreClock / OS_TICK_HZ);

    __enable_irq();
}

OS_TCB_t *OS_GetCurrentTask(void)
{
    return (OS_TCB_t *)OS_CurrentTCB;
}

uint8_t OS_GetCurrentTaskId(void)
{
    if (OS_CurrentTCB == 0)
        return 0xFF;

    return OS_CurrentTCB->id;
}

uint8_t OS_IsRunning(void)
{
    return os_started;
}

int OS_CreateTask(OS_TaskFunc_t task_func,
                  void *arg,
                  uint32_t *stack_mem,
                  uint32_t stack_words,
                  uint8_t priority,
                  uint32_t time_slice_ticks,
                  const char *name)
{
    if (task_func == 0 || stack_mem == 0 || stack_words < 64) {
        return -1;
    }

    if (os_task_count >= OS_MAX_TASKS) {
        return -2;
    }

    __disable_irq();

    for (uint32_t i = 0; i < stack_words; i++) {
        stack_mem[i] = 0xA5A5A5A5UL;
    }

    uint8_t id = os_task_count;
    OS_TCB_t *tcb = &os_tasks[id];

    uint32_t *sp = stack_mem + stack_words;

    /* Căn stack 8 byte */
    sp = (uint32_t *)((uint32_t)sp & ~0x7UL);

    /*
     * Stack frame giả lập như CPU tự push khi vào exception:
     * xPSR, PC, LR, R12, R3, R2, R1, R0
     */
    *(--sp) = 0x01000000UL;                /* xPSR: Thumb bit */
    *(--sp) = (uint32_t)task_func;         /* PC */
    *(--sp) = (uint32_t)OS_TaskExit_Error; /* LR */
    *(--sp) = 0x12121212UL;                /* R12 */
    *(--sp) = 0x03030303UL;                /* R3 */
    *(--sp) = 0x02020202UL;                /* R2 */
    *(--sp) = 0x01010101UL;                /* R1 */
    *(--sp) = (uint32_t)arg;               /* R0 */

    /*
     * R4-R11 do PendSV tự lưu/phục hồi.
     * Thứ tự này khớp với LDMIA sp!, {r4-r11}
     */
    *(--sp) = 0x11111111UL; /* R11 */
    *(--sp) = 0x10101010UL; /* R10 */
    *(--sp) = 0x09090909UL; /* R9 */
    *(--sp) = 0x08080808UL; /* R8 */
    *(--sp) = 0x07070707UL; /* R7 */
    *(--sp) = 0x06060606UL; /* R6 */
    *(--sp) = 0x05050505UL; /* R5 */
    *(--sp) = 0x04040404UL; /* R4 */

    tcb->sp = sp;
    tcb->stack_start = stack_mem;
    tcb->stack_words = stack_words;
    tcb->id = id;
    tcb->priority = priority;
    tcb->state = OS_TASK_READY;
    tcb->delay_ticks = 0;
    tcb->time_slice_ticks = (time_slice_ticks == 0) ? OS_DEFAULT_SLICE : time_slice_ticks;
    tcb->slice_left = tcb->time_slice_ticks;
    tcb->task_func = task_func;
    tcb->arg = arg;
    tcb->name = name;

    os_task_count++;

    __enable_irq();

    return id;
}

static OS_TCB_t *OS_SelectNextTask(void)
{
    uint8_t best_prio = 255;
    OS_TCB_t *selected = 0;

    for (uint8_t i = 0; i < os_task_count; i++)
    {
        if (os_tasks[i].state == OS_TASK_READY ||
            os_tasks[i].state == OS_TASK_RUNNING)
        {
            if (os_tasks[i].priority < best_prio)
            {
                best_prio = os_tasks[i].priority;
            }
        }
    }

    if (best_prio == 255)
    {
        return OS_CurrentTCB;
    }

    /*
     * Round-robin trong nhóm cùng priority.
     * Bắt đầu tìm từ task sau task hiện tại.
     */
    for (uint8_t step = 1; step <= os_task_count; step++)
    {
        uint8_t idx = (uint8_t)((os_current_index + step) % os_task_count);

        if ((os_tasks[idx].state == OS_TASK_READY ||
             os_tasks[idx].state == OS_TASK_RUNNING) &&
            os_tasks[idx].priority == best_prio)
        {
            selected = &os_tasks[idx];
            os_current_index = idx;
            break;
        }
    }

    return selected;
}

OS_TCB_t *OS_Schedule(void)
{
    if (OS_CurrentTCB != 0 &&
        OS_CurrentTCB->state == OS_TASK_RUNNING)
    {
        OS_CurrentTCB->state = OS_TASK_READY;
    }

    OS_TCB_t *next = OS_SelectNextTask();

    if (next != 0)
    {
        next->state = OS_TASK_RUNNING;
        next->slice_left = next->time_slice_ticks;
        OS_CurrentTCB = next;
    }

    return (OS_TCB_t *)OS_CurrentTCB;
}

void OS_Tick_Handler(void)
{
    if (!os_started)
    {
        return;
    }

    os_tick++;

    uint8_t need_switch = 0;

    for (uint8_t i = 0; i < os_task_count; i++)
    {
        if (os_tasks[i].state == OS_TASK_BLOCKED)
        {
            if (os_tasks[i].delay_ticks > 0)
            {
                os_tasks[i].delay_ticks--;
            }

            if (os_tasks[i].delay_ticks == 0)
            {
                os_tasks[i].state = OS_TASK_READY;

                if (OS_CurrentTCB != 0 &&
                    os_tasks[i].priority < OS_CurrentTCB->priority)
                {
                    need_switch = 1;
                }
            }
        }
    }

    if (OS_CurrentTCB != 0)
    {
        if (OS_CurrentTCB->slice_left > 0)
        {
            OS_CurrentTCB->slice_left--;
        }

        if (OS_CurrentTCB->slice_left == 0)
        {
            need_switch = 1;
        }
    }

    if (need_switch)
    {
        OS_TriggerPendSV();
    }
}

void OS_Delay(uint32_t ticks)
{
    if (ticks == 0)
    {
        OS_Yield();
        return;
    }

    __disable_irq();

    if (OS_CurrentTCB != 0)
    {
        OS_CurrentTCB->delay_ticks = ticks;
        OS_CurrentTCB->state = OS_TASK_BLOCKED;
        OS_TriggerPendSV();
    }

    __enable_irq();
}

void OS_Yield(void)
{
    OS_TriggerPendSV();
}

uint32_t OS_GetTick(void)
{
    return os_tick;
}

uint8_t OS_GetTaskCount(void)
{
    return os_task_count;
}

static void OS_CallSVC(void)
{
#if defined(__CC_ARM)
    __asm {
        SVC 0
    }
#else
    __asm volatile("svc 0");
#endif
}
OS_TCB_t *OS_GetTaskInfo(uint8_t index)
{
    if (index >= os_task_count) {
        return 0;
    }

    return &os_tasks[index];
}

uint32_t OS_GetStackFreeBytes(uint8_t index)
{
    if (index >= os_task_count) {
        return 0;
    }

    OS_TCB_t *tcb = &os_tasks[index];

    uint32_t free_words = 0;

    for (uint32_t i = 0; i < tcb->stack_words; i++) {
        if (tcb->stack_start[i] == 0xA5A5A5A5UL) {
            free_words++;
        } else {
            break;
        }
    }

    return free_words * 4;
}
void OS_Start(void)
{
    if (os_task_count == 0)
    {
        return;
    }

    __disable_irq();

    os_started = 1;
    OS_CurrentTCB = OS_Schedule();

    __enable_irq();

    OS_CallSVC();

    while (1)
    {
    }
}
void OS_BlockCurrent(uint32_t timeout_ticks)
{
    __disable_irq();

    if (OS_CurrentTCB != 0)
    {
        OS_CurrentTCB->state = OS_TASK_BLOCKED;
        OS_CurrentTCB->delay_ticks = timeout_ticks;
    }

    __enable_irq();

    OS_Yield();
}

void OS_WakeTask(OS_TCB_t *task)
{
    __disable_irq();

    if (task != 0 && task->state == OS_TASK_BLOCKED)
    {
        task->state = OS_TASK_READY;
        task->delay_ticks = 0;
    }

    __enable_irq();
}
