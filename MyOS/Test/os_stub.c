#include "os_types.h"

/* ================================================================
   OS_STUB.C – Hàm gi? d? TV2 test d?c l?p khi TV1 chua xong
   
   Khi TV1 hoàn thành RTOS, XÓA file này di và dùng
   hàm th?t c?a TV1 thay th?.
   ================================================================ */

/* Task gi? – d?i di?n cho "task dang ch?y" khi test */
static TCB_t s_dummy_task = {
    .task_id  = 0,
    .state    = TASK_RUNNING,
    .priority = 4,
    .name     = "MainTask",
};

/* TV1 s? implement hàm này th?t s? */
TCB_t* OS_GetCurrentTask(void) {
    /* Khi chua có RTOS: tr? v? task gi? */
    return &s_dummy_task;
}

/* TV1 s? implement hàm này th?t s? */
void OS_Delay(uint32_t ticks) {
    /* Khi chua có RTOS: dùng HAL_Delay */
    HAL_Delay(ticks);
}

/* TV1 s? implement hàm này th?t s? */
void OS_Yield(void) {
    /* Khi chua có RTOS: không làm gì */
}