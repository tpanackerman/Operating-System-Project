#include "os_queue.h"
#include <string.h>

/* ================================================================
   KH?I T?O
   ================================================================ */
void OS_Queue_Init(OS_Queue_t *queue, uint16_t msg_size,
                   uint16_t depth, const char *name) {
    if (queue == NULL) return;
    if (msg_size == 0 || msg_size > OS_QUEUE_MAX_MSG_SIZE) return;
    if (depth == 0 || depth > OS_QUEUE_MAX_DEPTH) return;

    OS_ENTER_CRITICAL();

    memset(queue->buf, 0, sizeof(queue->buf));
    queue->msg_size    = msg_size;
    queue->depth       = depth;
    queue->head        = 0;
    queue->tail        = 0;
    queue->count       = 0;
    queue->send_count  = 0;
    queue->recv_count  = 0;
    queue->drop_count  = 0;
    queue->initialized = 1;

    if (name != NULL) {
        strncpy(queue->name, name, sizeof(queue->name) - 1);
        queue->name[sizeof(queue->name) - 1] = '\0';
    }

    OS_EXIT_CRITICAL();
}

/* ================================================================
   G?I MESSAGE
   ================================================================ */
QueueStatus_t OS_Queue_Send(OS_Queue_t *queue, const void *msg,
                             uint32_t timeout_ms) {
    if (queue == NULL || !queue->initialized) return QUEUE_ERROR;
    if (msg == NULL) return QUEUE_ERROR;

    uint32_t start = HAL_GetTick();

    while (1) {
        OS_ENTER_CRITICAL();

        if (queue->count < queue->depth) {
            /* Còn ch? – copy message vào buffer */
            memcpy(queue->buf[queue->head], msg, queue->msg_size);
            queue->head = (queue->head + 1) % queue->depth;
            queue->count++;
            queue->send_count++;

            OS_EXIT_CRITICAL();
            return QUEUE_OK;
        }

        OS_EXIT_CRITICAL();

        /* Queue d?y – ki?m tra timeout */
        if (timeout_ms == OS_QUEUE_NO_WAIT) {
            queue->drop_count++;
            return QUEUE_FULL;
        }
        if (timeout_ms != OS_QUEUE_WAIT_FOREVER) {
            if ((HAL_GetTick() - start) >= timeout_ms) {
                queue->drop_count++;
                return QUEUE_TIMEOUT;
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
   NH?N MESSAGE
   ================================================================ */
QueueStatus_t OS_Queue_Receive(OS_Queue_t *queue, void *msg,
                                uint32_t timeout_ms) {
    if (queue == NULL || !queue->initialized) return QUEUE_ERROR;
    if (msg == NULL) return QUEUE_ERROR;

    uint32_t start = HAL_GetTick();

    while (1) {
        OS_ENTER_CRITICAL();

        if (queue->count > 0) {
            /* Có message – copy ra ngoài */
            memcpy(msg, queue->buf[queue->tail], queue->msg_size);
            /* Xóa slot v?a d?c */
            memset(queue->buf[queue->tail], 0, queue->msg_size);
            queue->tail = (queue->tail + 1) % queue->depth;
            queue->count--;
            queue->recv_count++;

            OS_EXIT_CRITICAL();
            return QUEUE_OK;
        }

        OS_EXIT_CRITICAL();

        /* Queue tr?ng – ki?m tra timeout */
        if (timeout_ms == OS_QUEUE_NO_WAIT) {
            return QUEUE_EMPTY;
        }
        if (timeout_ms != OS_QUEUE_WAIT_FOREVER) {
            if ((HAL_GetTick() - start) >= timeout_ms) {
                return QUEUE_TIMEOUT;
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
   TI?N ÍCH
   ================================================================ */
QueueStatus_t OS_Queue_Peek(OS_Queue_t *queue, void *msg) {
    if (queue == NULL || msg == NULL) return QUEUE_ERROR;

    OS_ENTER_CRITICAL();

    if (queue->count == 0) {
        OS_EXIT_CRITICAL();
        return QUEUE_EMPTY;
    }

    memcpy(msg, queue->buf[queue->tail], queue->msg_size);

    OS_EXIT_CRITICAL();
    return QUEUE_OK;
}

uint16_t OS_Queue_GetCount(OS_Queue_t *queue) {
    if (queue == NULL) return 0;
    return queue->count;
}

uint8_t OS_Queue_IsEmpty(OS_Queue_t *queue) {
    if (queue == NULL) return 1;
    return (queue->count == 0) ? 1 : 0;
}

uint8_t OS_Queue_IsFull(OS_Queue_t *queue) {
    if (queue == NULL) return 0;
    return (queue->count >= queue->depth) ? 1 : 0;
}

void OS_Queue_Flush(OS_Queue_t *queue) {
    if (queue == NULL) return;
    OS_ENTER_CRITICAL();
    memset(queue->buf, 0, sizeof(queue->buf));
    queue->head  = 0;
    queue->tail  = 0;
    queue->count = 0;
    OS_EXIT_CRITICAL();
}