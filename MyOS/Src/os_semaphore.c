#include "os_semaphore.h"
#include <string.h>

/* ================================================================
   KH?I T?O
   ================================================================ */
void OS_Sem_Init(OS_Sem_t *sem, int32_t initial_count,
                 int32_t max_count, const char *name) {
    if (sem == NULL || max_count <= 0) return;
    if (initial_count > max_count) initial_count = max_count;

    OS_ENTER_CRITICAL();

    sem->count       = initial_count;
    sem->max_count   = max_count;
    sem->post_count  = 0;
    sem->wait_count  = 0;
    sem->initialized = 1;

    if (name != NULL) {
        strncpy(sem->name, name, sizeof(sem->name) - 1);
        sem->name[sizeof(sem->name) - 1] = '\0';
    }

    OS_EXIT_CRITICAL();
}

/* ================================================================
   WAIT – Ch? semaphore (gi?m count)
   ================================================================ */
SemStatus_t OS_Sem_Wait(OS_Sem_t *sem, uint32_t timeout_ms) {
    if (sem == NULL || !sem->initialized) return SEM_ERROR;

    uint32_t start = HAL_GetTick();

    while (1) {
        OS_ENTER_CRITICAL();

        if (sem->count > 0) {
            sem->count--;
            sem->wait_count++;
            OS_EXIT_CRITICAL();
            return SEM_OK;
        }

        OS_EXIT_CRITICAL();

        /* Ki?m tra timeout */
        if (timeout_ms == OS_SEM_NO_WAIT) {
            return SEM_TIMEOUT;
        }
        if (timeout_ms != OS_SEM_WAIT_FOREVER) {
            if ((HAL_GetTick() - start) >= timeout_ms) {
                return SEM_TIMEOUT;
            }
        }

        /* Nhu?ng CPU */
        TCB_t *cur = OS_GetCurrentTask();
        if (cur != NULL) {
            OS_Yield();
        }
    }
}

/* ================================================================
   POST – Tang count (signal)
   ================================================================ */
SemStatus_t OS_Sem_Post(OS_Sem_t *sem) {
    if (sem == NULL || !sem->initialized) return SEM_ERROR;

    OS_ENTER_CRITICAL();

    if (sem->count >= sem->max_count) {
        OS_EXIT_CRITICAL();
        return SEM_FULL;    /* Không th? tang quá max */
    }

    sem->count++;
    sem->post_count++;

    OS_EXIT_CRITICAL();
    return SEM_OK;
}

/* ================================================================
   TI?N ÍCH
   ================================================================ */
int32_t OS_Sem_GetCount(OS_Sem_t *sem) {
    if (sem == NULL) return -1;
    return sem->count;
}

void OS_Sem_Reset(OS_Sem_t *sem) {
    if (sem == NULL) return;
    OS_ENTER_CRITICAL();
    sem->count = 0;
    OS_EXIT_CRITICAL();
}