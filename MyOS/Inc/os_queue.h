#ifndef OS_QUEUE_H
#define OS_QUEUE_H

#include "os_types.h"

/* ================================================================
   OS_QUEUE.H – Hàng d?i tin nh?n gi?a các task
   
   Ví d? th?c t? trong d? án:
   
   TV5 (GPIO task) d?c sensor     TV3 (USB task) g?i lên PC
        ¦                               ¦
        ¦  OS_Queue_Send(&q, &data)     ¦  OS_Queue_Receive(&q, &data)
        +----------? [Queue] ----------?+
   
   Queue ho?t d?ng nhu ?ng nu?c:
   - Task A b? data vào d?u này
   - Task B l?y data ra d?u kia
   - N?u queue d?y ? Task A ch?
   - N?u queue tr?ng ? Task B ch?
   
   C?U HÌNH:
   - M?i message t?i da 32 bytes (= 1 block memory pool)
   - M?i queue t?i da 8 message
   ================================================================ */

#define OS_QUEUE_MAX_MSG_SIZE   32  /* Bytes t?i da m?i message     */
#define OS_QUEUE_MAX_DEPTH      8   /* S? message t?i da trong queue */
#define OS_QUEUE_NO_WAIT        0
#define OS_QUEUE_WAIT_FOREVER   0xFFFFFFFF

/* ===== TR?NG THÁI ===== */
typedef enum {
    QUEUE_OK        = 0,
    QUEUE_FULL      = 1,    /* Queue d?y, không th? g?i thêm   */
    QUEUE_EMPTY     = 2,    /* Queue tr?ng, không có gì d?c     */
    QUEUE_TIMEOUT   = 3,    /* H?t th?i gian ch?                */
    QUEUE_ERROR     = 4,    /* L?i tham s?                      */
} QueueStatus_t;

/* ===== C?U TRÚC QUEUE ===== */
typedef struct {
    uint8_t  buf[OS_QUEUE_MAX_DEPTH][OS_QUEUE_MAX_MSG_SIZE]; /* Buffer */
    uint16_t msg_size;      /* Kích thu?c m?i message (bytes)   */
    uint16_t depth;         /* S? slot t?i da                   */
    volatile uint16_t head; /* V? trí ghi ti?p theo             */
    volatile uint16_t tail; /* V? trí d?c ti?p theo             */
    volatile uint16_t count;/* S? message hi?n có               */
    uint32_t send_count;    /* T?ng s? message dã g?i           */
    uint32_t recv_count;    /* T?ng s? message dã nh?n          */
    uint32_t drop_count;    /* S? message b? drop do queue d?y  */
    char     name[12];
    uint8_t  initialized;
} OS_Queue_t;

/* ================================================================
   API CÔNG KHAI
   ================================================================ */

/**
 * @brief Kh?i t?o queue.
 * @param queue    Con tr? OS_Queue_t
 * @param msg_size Kích thu?c m?i message (bytes, t?i da 32)
 * @param depth    S? message t?i da (t?i da 8)
 * @param name     Tên d? debug
 *
 * Ví d?: Queue g?i struct SensorData t? TV5 sang TV3:
 *   OS_Queue_t sensor_queue;
 *   OS_Queue_Init(&sensor_queue, sizeof(SensorData_t), 8, "SensorQ");
 */
void OS_Queue_Init(OS_Queue_t *queue, uint16_t msg_size,
                   uint16_t depth, const char *name);

/**
 * @brief G?i message vào queue.
 * @param queue      Con tr? queue
 * @param msg        Con tr? data c?n g?i
 * @param timeout_ms Th?i gian ch? n?u queue d?y
 * @retval QUEUE_OK ho?c QUEUE_FULL/QUEUE_TIMEOUT
 *
 * Ví d?:
 *   SensorData_t data = { .value = 1234, .timestamp = HAL_GetTick() };
 *   OS_Queue_Send(&sensor_queue, &data, 100);  // Ch? t?i da 100ms
 */
QueueStatus_t OS_Queue_Send(OS_Queue_t *queue, const void *msg,
                             uint32_t timeout_ms);

/**
 * @brief Nh?n message t? queue.
 * @param queue      Con tr? queue
 * @param msg        Buffer nh?n data ra
 * @param timeout_ms Th?i gian ch? n?u queue tr?ng
 * @retval QUEUE_OK ho?c QUEUE_EMPTY/QUEUE_TIMEOUT
 *
 * Ví d?:
 *   SensorData_t data;
 *   if (OS_Queue_Receive(&sensor_queue, &data, 500) == QUEUE_OK) {
 *       USB_CDC_Printf("Sensor: %d\r\n", data.value);
 *   }
 */
QueueStatus_t OS_Queue_Receive(OS_Queue_t *queue, void *msg,
                                uint32_t timeout_ms);

/**
 * @brief Xem message d?u tiên nhung KHÔNG l?y ra kh?i queue.
 */
QueueStatus_t OS_Queue_Peek(OS_Queue_t *queue, void *msg);

/**
 * @brief S? message dang có trong queue.
 */
uint16_t OS_Queue_GetCount(OS_Queue_t *queue);

/**
 * @brief Ki?m tra queue có tr?ng không.
 */
uint8_t OS_Queue_IsEmpty(OS_Queue_t *queue);

/**
 * @brief Ki?m tra queue có d?y không.
 */
uint8_t OS_Queue_IsFull(OS_Queue_t *queue);

/**
 * @brief Xóa toàn b? message trong queue.
 */
void OS_Queue_Flush(OS_Queue_t *queue);

#endif /* OS_QUEUE_H */