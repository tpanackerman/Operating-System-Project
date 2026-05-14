#ifndef OS_PROTOCOL_H
#define OS_PROTOCOL_H

#include "usb_cdc_driver.h"

/*
 * FORMAT GÓI TIN:
 * [0xAA][0x55][TYPE][LEN_H][LEN_L][DATA ...][CRC8]
 *
 * Ví dụ gửi heartbeat (4 byte data):
 * AA 55 FF 00 04 <4 bytes uptime> <CRC>
 */

#define PROTO_MAGIC_1   0xAA
#define PROTO_MAGIC_2   0x55

/* ===== LOẠI GÓI TIN ===== */
typedef enum {
    PKT_HEARTBEAT      = 0xFF,  /* Gửi mỗi 1 giây */
    PKT_TASK_STATUS    = 0x01,  /* Danh sách task + trạng thái */
    PKT_CPU_USAGE      = 0x02,  /* % CPU từng task */
    PKT_MEMORY_STATUS  = 0x03,  /* Memory pool còn bao nhiêu */
    PKT_MUTEX_STATUS   = 0x04,  /* Mutex đang bị lock hay không */
    PKT_QUEUE_STATUS   = 0x05,  /* Message queue còn bao nhiêu chỗ */
    PKT_LOG            = 0x10,  /* Chuỗi debug text */
} PacketType_t;

/* ===== CÁC STRUCT DATA ===== */
#pragma pack(push, 1)

/* PKT_HEARTBEAT */
typedef struct {
    uint32_t uptime_ms;
    uint8_t  num_tasks;
    uint8_t  os_ver_major;
    uint8_t  os_ver_minor;
} Proto_Heartbeat_t;

/* PKT_TASK_STATUS – gửi 1 struct này cho mỗi task */
typedef struct {
    uint8_t  task_id;
    char     name[12];
    uint8_t  state;      /* 0=Ready 1=Running 2=Blocked 3=Suspended */
    uint8_t  priority;
    uint32_t stack_free; /* Bytes stack còn trống */
} Proto_TaskStatus_t;

/* PKT_CPU_USAGE – gửi 1 struct cho mỗi task */
typedef struct {
    uint8_t  task_id;
    uint16_t percent_x100; /* 1234 = 12.34% */
} Proto_CpuUsage_t;

/* PKT_MEMORY_STATUS */
typedef struct {
    uint32_t total;
    uint32_t used;
    uint32_t free;
} Proto_MemStatus_t;

/* PKT_MUTEX_STATUS */
typedef struct {
    uint8_t mutex_id;
    uint8_t is_locked;
    uint8_t owner_task_id;
} Proto_MutexStatus_t;

#pragma pack(pop)

/* ===== CRC8 ===== */
static inline uint8_t Proto_CRC8(const uint8_t *data, uint16_t len) {
    uint8_t crc = 0x00;
    for (uint16_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++)
            crc = (crc & 0x80) ? ((crc << 1) ^ 0x07) : (crc << 1);
    }
    return crc;
}

/* ===== HÀM GỬI GÓI TIN ===== */
static inline void Proto_Send(PacketType_t type, const void *data, uint16_t data_len) {
    /* Header: AA 55 TYPE LEN_H LEN_L */
    uint8_t header[5];
    header[0] = PROTO_MAGIC_1;
    header[1] = PROTO_MAGIC_2;
    header[2] = (uint8_t)type;
    header[3] = (uint8_t)(data_len >> 8);
    header[4] = (uint8_t)(data_len & 0xFF);

    /* Tính CRC trên header + data */
    uint8_t crc = Proto_CRC8(header, 5);
    if (data && data_len > 0)
        crc ^= Proto_CRC8((const uint8_t *)data, data_len); /* CRC tiếp tục */

    /* Gửi từng phần */
    USB_CDC_Send(header, 5);
    if (data && data_len > 0)
        USB_CDC_Send((const uint8_t *)data, data_len);
    USB_CDC_Send(&crc, 1);
}

/* Tiện ích gửi log text */
static inline void Proto_Log(const char *msg) {
    Proto_Send(PKT_LOG, msg, (uint16_t)strlen(msg));
}

#endif /* OS_PROTOCOL_H */