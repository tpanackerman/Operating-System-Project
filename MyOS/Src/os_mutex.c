#include "os_mutex.h"
#include <string.h>

/* ================================================================
   KH?I T?O
   ================================================================ */
void OS_Mutex_Init(OS_Mutex_t *mutex, const char *name) {
    if (mutex == NULL) return;

    OS_ENTER_CRITICAL();

    mutex->is_locked   = 0;
uint8_t current_id = OS_GetCurrentTaskId();

if (current_id == 0xFF)
{
    return OS_ERROR;
}

mutex->owner_id = current_id;
    mutex->lock_count  = 0;
    mutex->initialized = 1;

    if (name != NULL) {
        strncpy(mutex->name, name, sizeof(mutex->name) - 1);
        mutex->name[sizeof(mutex->name) - 1] = '\0';
    } else {
        strncpy(mutex->name, "Unnamed", sizeof(mutex->name) - 1);
    }

    OS_EXIT_CRITICAL();
}

/* ================================================================
   LOCK – Chi?m khóa
   ================================================================ */
MutexStatus_t OS_Mutex_Lock(OS_Mutex_t *mutex, uint32_t timeout_ms) {
    if (mutex == NULL || !mutex->initialized) return MUTEX_ERROR;

    uint32_t start = HAL_GetTick();

    while (1) {
        OS_ENTER_CRITICAL();

        /* Ki?m tra mutex còn tr?ng */
        if (mutex->is_locked == 0) {
            /* L?y du?c – ghi owner và khóa l?i */
            mutex->is_locked  = 1;
            mutex->lock_count++;

            /* L?y ID task hi?n t?i n?u RTOS dã ch?y */
            TCB_t *cur = OS_GetCurrentTask();
            mutex->owner_id = (cur != NULL) ? cur->task_id : 0;

            OS_EXIT_CRITICAL();
            return MUTEX_OK;
        }

        OS_EXIT_CRITICAL();

        /* Ki?m tra timeout */
        if (timeout_ms == OS_MUTEX_NO_WAIT) {
            return MUTEX_LOCKED;
        }
        if (timeout_ms != OS_MUTEX_WAIT_FOREVER) {
            if ((HAL_GetTick() - start) >= timeout_ms) {
                return MUTEX_TIMEOUT;
            }
        }

        /* Nhu?ng CPU cho task khác – khi RTOS chua có thì ch? busy-wait */
        TCB_t *cur = OS_GetCurrentTask();
        if (cur != NULL) {
            OS_Yield();
        }
        /* N?u chua có RTOS, vòng l?p ti?p t?c (busy-wait) */
    }
}

/* ================================================================
   UNLOCK – Nh? khóa
   ================================================================ */
MutexStatus_t OS_Mutex_Unlock(OS_Mutex_t *mutex) {
    if (mutex == NULL || !mutex->initialized) return MUTEX_ERROR;

    OS_ENTER_CRITICAL();

    if (mutex->is_locked == 0) {
        /* Mutex dã m? r?i – l?i logic */
        OS_EXIT_CRITICAL();
        return MUTEX_ERROR;
    }

    /* Ki?m tra task hi?n t?i có ph?i owner không */
    TCB_t *cur = OS_GetCurrentTask();
    if (cur != NULL && cur->task_id != mutex->owner_id) {
        /* Task khác không du?c phép unlock */
        OS_EXIT_CRITICAL();
        return MUTEX_ERROR;
    }

    mutex->is_locked = 0;
    mutex->owner_id  = 0xFF;

    OS_EXIT_CRITICAL();
    return MUTEX_OK;
}

/* ================================================================
   TRY LOCK – Không ch?
   ================================================================ */
MutexStatus_t OS_Mutex_TryLock(OS_Mutex_t *mutex) {
    if (mutex == NULL || !mutex->initialized) return MUTEX_ERROR;

    OS_ENTER_CRITICAL();

    if (mutex->is_locked != 0) {
        OS_EXIT_CRITICAL();
        return MUTEX_LOCKED;
    }

    mutex->is_locked = 1;
    mutex->lock_count++;

    TCB_t *cur = OS_GetCurrentTask();
    mutex->owner_id = (cur != NULL) ? cur->task_id : 0;

    OS_EXIT_CRITICAL();
    return MUTEX_OK;
}

/* ================================================================
   TI?N ÍCH
   ================================================================ */
uint8_t OS_Mutex_IsLocked(OS_Mutex_t *mutex) {
    if (mutex == NULL) return 0;
    return mutex->is_locked;
}

uint8_t OS_Mutex_GetOwner(OS_Mutex_t *mutex) {
    if (mutex == NULL) return 0xFF;
    return mutex->owner_id;
}
