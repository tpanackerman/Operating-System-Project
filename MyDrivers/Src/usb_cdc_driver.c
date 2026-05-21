#include "usb_cdc_driver.h"

/* ========================================================
   RING BUFFER N?I B?
   ======================================================== */
extern uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len);
extern uint8_t CDC_IsTxBusy_FS(void);
/* Mask dùng thay modulo (nhanh hon, yêu c?u size là luy th?a 2) */
#define TX_MASK  (USB_TX_RING_SIZE - 1)
#define RX_MASK  (USB_RX_RING_SIZE - 1)

typedef struct {
    uint8_t  data[USB_TX_RING_SIZE];
    volatile uint16_t head;   /* Noi ghi vào ti?p theo */
    volatile uint16_t tail;   /* Noi d?c ra ti?p theo  */
} TxRing_t;

typedef struct {
    uint8_t  data[USB_RX_RING_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
} RxRing_t;

static TxRing_t  s_tx;
static RxRing_t  s_rx;

/* Buffer staging d? copy vào USB PMA */
static uint8_t   s_tx_pkt[USB_TX_PKT_MAX];

/* C? tr?ng thái */
static volatile uint8_t s_tx_busy     = 0;
static volatile uint8_t s_connected   = 0;

/* Th?ng kê */
static USB_CDC_Stats_t  s_stats;

/* --------- Helper --------- */
static uint16_t TxUsed(void) { return (s_tx.head - s_tx.tail) & TX_MASK; }
static uint16_t TxFree(void) { return USB_TX_RING_SIZE - 1 - TxUsed(); }
static uint16_t RxUsed(void) { return (s_rx.head - s_rx.tail) & RX_MASK; }

/* ========================================================
   KH?I T?O
   ======================================================== */
void USB_CDC_Driver_Init(void) {
    memset(&s_tx,    0, sizeof(s_tx));
    memset(&s_rx,    0, sizeof(s_rx));
    memset(&s_stats, 0, sizeof(s_stats));
    s_tx_busy   = 0;
    s_connected = 0;
}

/* ========================================================
   TR?NG THÁI K?T N?I
   ======================================================== */
uint8_t USB_CDC_IsConnected(void) {
    return s_connected;
}

/* G?i t? usbd_cdc_if.c */
void USB_CDC_SetConnected(uint8_t connected) {
    s_connected          = connected;
    s_stats.is_connected = connected;
    if (!connected) {
        /* Reset buffer khi PC ng?t k?t n?i */
        s_tx.head = s_tx.tail = 0;
        s_tx_busy = 0;
    }
}

/* ========================================================
   G?I D? LI?U
   ======================================================== */
USB_CDC_Status USB_CDC_Send(const uint8_t *buf, uint16_t len) {
    if (!s_connected) return USB_CDC_ERROR;
    if (len == 0)     return USB_CDC_OK;

    if (len > TxFree()) {
        s_stats.tx_overflow_count++;
        return USB_CDC_OVERFLOW;
    }

    /* Ghi tu?n t? vào ring buffer */
    for (uint16_t i = 0; i < len; i++) {
        s_tx.data[s_tx.head & TX_MASK] = buf[i];
        s_tx.head++;
    }
    s_stats.tx_total_bytes += len;
    return USB_CDC_OK;
}

USB_CDC_Status USB_CDC_SendString(const char *str) {
    return USB_CDC_Send((const uint8_t *)str, (uint16_t)strlen(str));
}

USB_CDC_Status USB_CDC_Printf(const char *fmt, ...)
{
    char tmp[128];
    va_list args;

    va_start(args, fmt);
    int n = vsnprintf(tmp, sizeof(tmp), fmt, args);
    va_end(args);

    if (n <= 0) {
        return USB_CDC_ERROR;
    }

    if (n >= (int)sizeof(tmp)) {
        n = sizeof(tmp) - 1;
    }

    return USB_CDC_Send((uint8_t *)tmp, (uint16_t)n);
}

/* ========================================================
   FLUSH TX – g?i d?nh k? t? task th?p uu tiên
   ======================================================== */
void USB_CDC_FlushTX(void)
{
    if (!s_connected) return;

    if (CDC_IsTxBusy_FS()) return;

    if (TxUsed() == 0) return;

    uint16_t send_len = TxUsed();

    if (send_len > USB_TX_PKT_MAX) {
        send_len = USB_TX_PKT_MAX;
    }

    for (uint16_t i = 0; i < send_len; i++) {
        s_tx_pkt[i] = s_tx.data[s_tx.tail & TX_MASK];
        s_tx.tail++;
    }

    if (CDC_Transmit_FS(s_tx_pkt, send_len) != USBD_OK) {
        s_tx.tail -= send_len;
    }
}

/* Callback khi USB g?i xong 1 packet – g?i t? usbd_cdc_if.c */
void USB_CDC_TxDoneCallback(void) {
    //s_tx_busy = 0;
    /* T? d?ng g?i packet ti?p n?u còn d? li?u trong buffer */
    //USB_CDC_FlushTX();
}

/* ========================================================
   NH?N D? LI?U
   ======================================================== */

/* Callback – du?c g?i khi PC g?i data xu?ng */
void USB_CDC_RxCallback(uint8_t *buf, uint32_t len) {
    for (uint32_t i = 0; i < len; i++) {
        /* Ch? ghi n?u còn ch? – b? byte m?i n?u d?y */
        if (RxUsed() < USB_RX_RING_SIZE - 1) {
            s_rx.data[s_rx.head & RX_MASK] = buf[i];
            s_rx.head++;
        }
    }
    s_stats.rx_total_bytes += len;
}

uint16_t USB_CDC_Receive(uint8_t *buf, uint16_t max_len) {
    uint16_t avail    = RxUsed();
    uint16_t read_len = (avail > max_len) ? max_len : avail;
    for (uint16_t i = 0; i < read_len; i++) {
        buf[i] = s_rx.data[s_rx.tail & RX_MASK];
        s_rx.tail++;
    }
    return read_len;
}

uint16_t USB_CDC_RxAvailable(void) {
    return RxUsed();
}

/* ========================================================
   TH?NG KÊ
   ======================================================== */
void USB_CDC_GetStats(USB_CDC_Stats_t *out) {
    out->tx_total_bytes    = s_stats.tx_total_bytes;
    out->rx_total_bytes    = s_stats.rx_total_bytes;
    out->tx_overflow_count = s_stats.tx_overflow_count;
    out->tx_buffer_percent = (uint8_t)((TxUsed() * 100) / USB_TX_RING_SIZE);
    out->rx_buffer_percent = (uint8_t)((RxUsed() * 100) / USB_RX_RING_SIZE);
    out->is_connected      = s_connected;
}

