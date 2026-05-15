#include "os_memory.h"
#include "os_mutex.h"
#include "os_semaphore.h"
#include "os_queue.h"
#include "usb_cdc_driver.h"
#include "os_protocol.h"

/* ================================================================
   TEST_TV2.C – Bài test d?c l?p cho TV2 (không c?n TV1)
   
   Copy hàm Test_TV2_RunAll() vào while(1) c?a main.c d? ch?y.
   K?t qu? in ra PuTTY qua USB.
   ================================================================ */

/* ===== BI?N DÙNG CHUNG TRONG CÁC TEST ===== */
static OS_Mutex_t test_mutex;
static OS_Sem_t   test_sem;
static OS_Queue_t test_queue;

/* Struct test g?i qua queue */
typedef struct {
    uint32_t timestamp;
    uint16_t value;
    uint8_t  sensor_id;
} TestMsg_t;

/* ================================================================
   TEST 1 – MEMORY POOL
   ================================================================ */
static void Test_Memory(void) {
    USB_CDC_SendString("\r\n=== TEST 1: MEMORY POOL ===\r\n");

    OS_Mem_Init();

    MemStats_t stats;
    OS_Mem_GetStats(&stats);
    USB_CDC_Printf("Ban dau: %d blocks tu do / %d tong\r\n",
                   stats.free_blocks, stats.total_blocks);

    /* C?p phát 3 block */
    uint8_t *p1 = (uint8_t *)OS_Mem_Alloc();
    uint8_t *p2 = (uint8_t *)OS_Mem_Alloc();
    uint8_t *p3 = (uint8_t *)OS_Mem_Alloc();

    if (p1 && p2 && p3) {
        USB_CDC_SendString("[OK] Cap phat 3 block thanh cong\r\n");
    } else {
        USB_CDC_SendString("[FAIL] Cap phat that bai!\r\n");
        return;
    }

    /* Ghi d? li?u vào block */
    for (int i = 0; i < 32; i++) p1[i] = (uint8_t)i;
    for (int i = 0; i < 32; i++) p2[i] = (uint8_t)(i + 100);

    /* Ki?m tra d? li?u không b? dè lên nhau */
    uint8_t ok = 1;
    for (int i = 0; i < 32; i++) {
        if (p1[i] != (uint8_t)i) { ok = 0; break; }
    }
    USB_CDC_Printf("[%s] Block doc lap nhau (khong bi ghi de)\r\n",
                   ok ? "OK" : "FAIL");

    /* Th?ng kê sau c?p phát */
    OS_Mem_GetStats(&stats);
    USB_CDC_Printf("Sau alloc: dang dung %d blocks, con %d blocks\r\n",
                   stats.used_blocks, stats.free_blocks);

    /* Gi?i phóng */
    OS_Mem_Free(p1);
    OS_Mem_Free(p2);
    OS_Mem_Free(p3);

    OS_Mem_GetStats(&stats);
    USB_CDC_Printf("Sau free: con lai %d blocks tu do\r\n", stats.free_blocks);

    /* Test pointer sai */
    uint8_t dummy;
    MemStatus_t st = OS_Mem_Free(&dummy);
    USB_CDC_Printf("[%s] Free pointer sai tra ve loi\r\n",
                   (st == MEM_ERR_INVALID) ? "OK" : "FAIL");

    /* Test c?p phát h?t */
    void *ptrs[32];
    uint8_t alloc_count = 0;
    for (int i = 0; i < 32; i++) {
        ptrs[i] = OS_Mem_Alloc();
        if (ptrs[i]) alloc_count++;
    }
    void *extra = OS_Mem_Alloc();  /* Ph?i fail */
    USB_CDC_Printf("[%s] Cap phat het 32 blocks, them 1 phai fail\r\n",
                   (alloc_count == 32 && extra == NULL) ? "OK" : "FAIL");

    for (int i = 0; i < 32; i++) OS_Mem_Free(ptrs[i]);

    OS_Mem_GetStats(&stats);
    USB_CDC_Printf("Stress test done. Peak used: %d blocks. Fail count: %lu\r\n",
                   stats.peak_used, stats.fail_count);
    USB_CDC_SendString("=== MEMORY POOL: DONE ===\r\n");
}

/* ================================================================
   TEST 2 – MUTEX
   ================================================================ */
static void Test_Mutex(void) {
    USB_CDC_SendString("\r\n=== TEST 2: MUTEX ===\r\n");

    OS_Mutex_Init(&test_mutex, "TestMutex");
    USB_CDC_Printf("Khoi tao mutex: locked=%d\r\n",
                   OS_Mutex_IsLocked(&test_mutex));

    /* Lock */
    MutexStatus_t st = OS_Mutex_Lock(&test_mutex, 100);
    USB_CDC_Printf("[%s] Lock lan 1: status=%d, locked=%d\r\n",
                   (st == MUTEX_OK) ? "OK" : "FAIL",
                   st, OS_Mutex_IsLocked(&test_mutex));

    /* TryLock khi dang b? khóa */
    st = OS_Mutex_TryLock(&test_mutex);
    USB_CDC_Printf("[%s] TryLock khi dang locked phai tra LOCKED (=%d)\r\n",
                   (st == MUTEX_LOCKED) ? "OK" : "FAIL", st);

    /* Lock v?i timeout ng?n – ph?i timeout */
    st = OS_Mutex_Lock(&test_mutex, 50);
    USB_CDC_Printf("[%s] Lock timeout 50ms phai tra TIMEOUT (=%d)\r\n",
                   (st == MUTEX_TIMEOUT) ? "OK" : "FAIL", st);

    /* Unlock */
    st = OS_Mutex_Unlock(&test_mutex);
    USB_CDC_Printf("[%s] Unlock: status=%d, locked=%d\r\n",
                   (st == MUTEX_OK) ? "OK" : "FAIL",
                   st, OS_Mutex_IsLocked(&test_mutex));

    /* Lock l?i sau khi unlock */
    st = OS_Mutex_Lock(&test_mutex, 0);
    USB_CDC_Printf("[%s] TryLock sau unlock phai OK (=%d)\r\n",
                   (st == MUTEX_OK) ? "OK" : "FAIL", st);
    OS_Mutex_Unlock(&test_mutex);

    USB_CDC_SendString("=== MUTEX: DONE ===\r\n");
}

/* ================================================================
   TEST 3 – SEMAPHORE
   ================================================================ */
static void Test_Semaphore(void) {
    USB_CDC_SendString("\r\n=== TEST 3: SEMAPHORE ===\r\n");

    /* Binary semaphore b?t d?u = 0 (locked) */
    OS_Sem_Init(&test_sem, 0, 1, "TestSem");
    USB_CDC_Printf("Khoi tao sem count=%d\r\n", OS_Sem_GetCount(&test_sem));

    /* Wait ngay l?p t?c ph?i fail (count=0) */
    SemStatus_t st = OS_Sem_Wait(&test_sem, OS_SEM_NO_WAIT);
    USB_CDC_Printf("[%s] Wait khi count=0 phai TIMEOUT (=%d)\r\n",
                   (st == SEM_TIMEOUT) ? "OK" : "FAIL", st);

    /* Post */
    st = OS_Sem_Post(&test_sem);
    USB_CDC_Printf("[%s] Post: status=%d, count=%d\r\n",
                   (st == SEM_OK) ? "OK" : "FAIL",
                   st, OS_Sem_GetCount(&test_sem));

    /* Post l?i – vu?t max ph?i fail */
    st = OS_Sem_Post(&test_sem);
    USB_CDC_Printf("[%s] Post khi count=max phai SEM_FULL (=%d)\r\n",
                   (st == SEM_FULL) ? "OK" : "FAIL", st);

    /* Wait thành công */
    st = OS_Sem_Wait(&test_sem, 100);
    USB_CDC_Printf("[%s] Wait khi count=1 phai OK (=%d), count sau=%d\r\n",
                   (st == SEM_OK) ? "OK" : "FAIL",
                   st, OS_Sem_GetCount(&test_sem));

    /* Test counting semaphore */
    OS_Sem_Init(&test_sem, 3, 3, "CountSem");
    USB_CDC_Printf("Counting sem (max=3) count=%d\r\n",
                   OS_Sem_GetCount(&test_sem));

    OS_Sem_Wait(&test_sem, 0);
    OS_Sem_Wait(&test_sem, 0);
    OS_Sem_Wait(&test_sem, 0);
    USB_CDC_Printf("Sau 3 wait: count=%d\r\n", OS_Sem_GetCount(&test_sem));

    st = OS_Sem_Wait(&test_sem, 0);
    USB_CDC_Printf("[%s] Wait thu 4 phai TIMEOUT (=%d)\r\n",
                   (st == SEM_TIMEOUT) ? "OK" : "FAIL", st);

    USB_CDC_SendString("=== SEMAPHORE: DONE ===\r\n");
}

/* ================================================================
   TEST 4 – MESSAGE QUEUE
   ================================================================ */
static void Test_Queue(void) {
    USB_CDC_SendString("\r\n=== TEST 4: MESSAGE QUEUE ===\r\n");

    OS_Queue_Init(&test_queue, sizeof(TestMsg_t), 4, "TestQueue");
    USB_CDC_Printf("Queue: msg_size=%d depth=%d count=%d\r\n",
                   (int)sizeof(TestMsg_t),
                   test_queue.depth,
                   OS_Queue_GetCount(&test_queue));

    /* G?i 3 message */
    for (int i = 0; i < 3; i++) {
        TestMsg_t msg = {
            .timestamp = HAL_GetTick(),
            .value     = (uint16_t)(100 + i * 10),
            .sensor_id = (uint8_t)i,
        };
        QueueStatus_t st = OS_Queue_Send(&test_queue, &msg, 0);
        USB_CDC_Printf("[%s] Send msg[%d] value=%d\r\n",
                       (st == QUEUE_OK) ? "OK" : "FAIL", i, msg.value);
    }

    USB_CDC_Printf("Queue count sau 3 send: %d\r\n",
                   OS_Queue_GetCount(&test_queue));

    /* Peek – xem không l?y */
    TestMsg_t peek_msg;
    OS_Queue_Peek(&test_queue, &peek_msg);
    USB_CDC_Printf("Peek: value=%d (queue van con %d msg)\r\n",
                   peek_msg.value, OS_Queue_GetCount(&test_queue));

    /* Nh?n t?ng message */
    for (int i = 0; i < 3; i++) {
        TestMsg_t rx;
        QueueStatus_t st = OS_Queue_Receive(&test_queue, &rx, 100);
        USB_CDC_Printf("[%s] Recv msg[%d]: sensor=%d value=%d\r\n",
                       (st == QUEUE_OK) ? "OK" : "FAIL",
                       i, rx.sensor_id, rx.value);
    }

    /* Nh?n khi queue tr?ng ph?i fail */
    TestMsg_t rx;
    QueueStatus_t st = OS_Queue_Receive(&test_queue, &rx, OS_QUEUE_NO_WAIT);
    USB_CDC_Printf("[%s] Recv khi trong phai EMPTY (=%d)\r\n",
                   (st == QUEUE_EMPTY) ? "OK" : "FAIL", st);

    /* Test queue d?y */
    TestMsg_t dummy = {0};
    int sent = 0;
    while (OS_Queue_Send(&test_queue, &dummy, 0) == QUEUE_OK) sent++;
    USB_CDC_Printf("Day queue: gui duoc %d msg, IsFull=%d\r\n",
                   sent, OS_Queue_IsFull(&test_queue));

    OS_Queue_Flush(&test_queue);
    USB_CDC_Printf("Sau flush: count=%d\r\n", OS_Queue_GetCount(&test_queue));

    USB_CDC_SendString("=== QUEUE: DONE ===\r\n");
}

/* ================================================================
   TEST 5 – G?I TH?NG KÊ LÊN PC QUA PROTOCOL (TV4 xem)
   ================================================================ */
static void Test_SendStats(void) {
    USB_CDC_SendString("\r\n=== TEST 5: GUI STATS QUA PROTOCOL ===\r\n");

    /* G?i Memory Status */
    MemStats_t mem;
    OS_Mem_GetStats(&mem);

    Proto_MemStatus_t pmem = {
        .total = MEM_POOL_TOTAL_SIZE,
        .used  = mem.used_blocks * MEM_BLOCK_SIZE,
        .free  = mem.free_blocks * MEM_BLOCK_SIZE,
    };
    Proto_Send(PKT_MEMORY_STATUS, &pmem, sizeof(pmem));
    USB_CDC_SendString("[OK] Memory status packet da gui\r\n");

    /* G?i Mutex Status */
    Proto_MutexStatus_t pmutex = {
        .mutex_id      = 0,
        .is_locked     = OS_Mutex_IsLocked(&test_mutex),
        .owner_task_id = OS_Mutex_GetOwner(&test_mutex),
    };
    Proto_Send(PKT_MUTEX_STATUS, &pmutex, sizeof(pmutex));
    USB_CDC_SendString("[OK] Mutex status packet da gui\r\n");

    USB_CDC_SendString("=== STATS: DONE ===\r\n");
}

/* ================================================================
   HÀM G?I T? MAIN – Ch?y toàn b? test 1 l?n
   ================================================================ */
void Test_TV2_RunAll(void) {
    static uint8_t done = 0;
    if (done) return;   /* Ch? ch?y 1 l?n */
    done = 1;

    HAL_Delay(2000);    /* Ch? USB k?t n?i */

    USB_CDC_SendString("\r\n");
    USB_CDC_SendString("========================================\r\n");
    USB_CDC_SendString("  TV2 - MEMORY & IPC TEST\r\n");
    USB_CDC_SendString("========================================\r\n");

    Test_Memory();
    HAL_Delay(100);

    Test_Mutex();
    HAL_Delay(100);

    Test_Semaphore();
    HAL_Delay(100);

    Test_Queue();
    HAL_Delay(100);

    Test_SendStats();

    USB_CDC_SendString("\r\n========================================\r\n");
    USB_CDC_SendString("  ALL TESTS DONE\r\n");
    USB_CDC_SendString("========================================\r\n");
}