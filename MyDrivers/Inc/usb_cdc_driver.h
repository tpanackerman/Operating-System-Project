#ifndef USB_CDC_DRIVER_H
#define USB_CDC_DRIVER_H

#include "stm32f1xx_hal.h"
#include "usbd_cdc_if.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

/* ===== C?U HÌNH BUFFER ===== */
#define USB_TX_RING_SIZE    1024  /* B?t bu?c là luy th?a 2: 256, 512, 1024 */
#define USB_RX_RING_SIZE    256
#define USB_TX_PKT_MAX      64    /* USB FS t?i da 64 byte/packet */

/* ===== TR?NG THÁI TR? V? ===== */
typedef enum {
    USB_CDC_OK        = 0,
    USB_CDC_ERROR     = 1,
    USB_CDC_BUSY      = 2,
    USB_CDC_OVERFLOW  = 3,
} USB_CDC_Status;

/* ===== TH?NG KÊ (cho TV4 l?y hi?n th? dashboard) ===== */
typedef struct {
    uint32_t tx_total_bytes;
    uint32_t rx_total_bytes;
    uint32_t tx_overflow_count;
    uint8_t  tx_buffer_percent;   /* % TX buffer dang dùng */
    uint8_t  rx_buffer_percent;
    uint8_t  is_connected;
} USB_CDC_Stats_t;

/* ===== API CÔNG KHAI ===== */

/* G?i 1 l?n trong main() sau MX_USB_DEVICE_Init() */
void USB_CDC_Driver_Init(void);

/* Ki?m tra PC dã m? COM port chua */
uint8_t USB_CDC_IsConnected(void);

/* G?i m?ng byte – non-blocking */
USB_CDC_Status USB_CDC_Send(const uint8_t *buf, uint16_t len);

/* G?i chu?i ký t? */
USB_CDC_Status USB_CDC_SendString(const char *str);

/* G?i có d?nh d?ng ki?u printf */
USB_CDC_Status USB_CDC_Printf(const char *fmt, ...);

/* Ð?c d? li?u t? PC */
uint16_t USB_CDC_Receive(uint8_t *buf, uint16_t max_len);

/* S? byte dang ch? d?c */
uint16_t USB_CDC_RxAvailable(void);

/* Flush TX buffer – g?i d?nh k? t? task th?p uu tiên (TV1 t?o) */
void USB_CDC_FlushTX(void);

/* L?y th?ng kê */
void USB_CDC_GetStats(USB_CDC_Stats_t *out);

/* === Hàm n?i b? – KHÔNG g?i tr?c ti?p, ch? dùng trong usbd_cdc_if.c === */
void USB_CDC_RxCallback(uint8_t *buf, uint32_t len);
void USB_CDC_TxDoneCallback(void);
void USB_CDC_SetConnected(uint8_t connected);

#endif /* USB_CDC_DRIVER_H */
