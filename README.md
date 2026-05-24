# Operating-System-Project
<img width="1241" height="566" alt="image" src="https://github.com/user-attachments/assets/3e286a19-1dec-4d67-9d30-f00da0d1778b" />

<img width="753" height="462" alt="image" src="https://github.com/user-attachments/assets/fc50517a-6802-43a5-88b8-5d0cfddcd397" />

# Mini RTOS on STM32F103C8T6

## 1. Mục tiêu
- Tạo task
- Context switch bằng PendSV
- Tick bằng SysTick
- Scheduler ưu tiên + round-robin
- Delay task
- Memory pool
- Mutex, Semaphore, Queue
- Demo LED + USB CDC

## 2. Phần cứng
- STM32F103C8T6 Blue Pill
- ST-Link V2
- Cáp micro USB
- LED, điện trở, nút nhấn
- Logic analyzer nếu cần debug

## 3. Cách build
- Mở MDK-ARM/myRTOS_Drivers.uvprojx bằng Keil uVision 5
- Chọn target STM32F103C8
- Build
- Flash bằng ST-Link

## 4. Cách demo
- LED task nháy PC13
- USB CDC gửi heartbeat
- Queue/Semaphore/Mutex chạy test

làm mini project 
<img width="723" height="623" alt="image" src="https://github.com/user-attachments/assets/9dd4ff96-fb62-4fb3-a71e-9ec3056c5974" />

