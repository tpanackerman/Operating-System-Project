/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include "gpio_driver.h"
#include "usb_cdc_driver.h"
#include "os_protocol.h"
#include "tiny_os.h"
#include "os_memory.h"
#include "os_mutex.h"
#include "os_semaphore.h"
#include "os_queue.h"

/* =========================================
 * KHAI BÁO CÁC CHÂN (PORTA)
 * ========================================= */
 void SystemClock_Config(void);
#define BTN_PORT        GPIOA
#define BTN_PIN         GPIO_PIN_0

#define SENSOR_PORT     GPIOA
#define SENSOR_PIN      GPIO_PIN_2

#define RGB_R_PORT      GPIOA
#define RGB_R_PIN       GPIO_PIN_3
#define RGB_G_PORT      GPIOA
#define RGB_G_PIN       GPIO_PIN_4
#define RGB_B_PORT      GPIOA
#define RGB_B_PIN       GPIO_PIN_5

// KHAI BÁO CHÂN LED M?I C?M NGOÀI
#define EXT_LED_PORT    GPIOA
#define EXT_LED_PIN     GPIO_PIN_6

/* =========================================
 * BI?N TR?NG THÁI H? TH?NG
 * ========================================= */
typedef enum {
    MODE_OFF = 0,
    MODE_ON  = 1,
    MODE_AUTO = 2
} SystemMode_t;

volatile SystemMode_t current_mode = MODE_OFF;

// B? nh? Stack cho 2 Task
uint32_t stack_button[128];
uint32_t stack_sensor[128];

/* =========================================
 * CÁC HÀM H? TR? ÐI?U KHI?N
 * ========================================= */
void RGB_SetColor(uint8_t r, uint8_t g, uint8_t b) {
    GPIO_Write(RGB_R_PORT, RGB_R_PIN, r);
    GPIO_Write(RGB_G_PORT, RGB_G_PIN, g);
    GPIO_Write(RGB_B_PORT, RGB_B_PIN, b);
}

// Hàm di?u khi?n LED c?m ngoài (Sáng khi ? m?c 1)
void EXT_LED_On(void) {
    GPIO_Write(EXT_LED_PORT, EXT_LED_PIN, 1);
}

void EXT_LED_Off(void) {
    GPIO_Write(EXT_LED_PORT, EXT_LED_PIN, 0);
}

/* =========================================
 * TASK 1: QU?N LÝ NÚT NH?N & Ð?I TR?NG THÁI
 * ========================================= */
void Task_Button_Control(void *arg) {
    uint8_t last_btn_state = 1; // M?c d?nh Pull-up là 1

    // Kh?i t?o ban d?u: T?t h?t
    RGB_SetColor(0, 0, 0); 
    EXT_LED_Off();

    while (1) {
        uint8_t current_btn = GPIO_Read(BTN_PORT, BTN_PIN);

        // Phát hi?n su?n xu?ng (nh?n nút)
        if (current_btn == 0 && last_btn_state == 1) {
            
            OS_Delay(20); // Ch?ng d?i phím 20ms
            
            if (GPIO_Read(BTN_PORT, BTN_PIN) == 0) { // Ch?c ch?n dã nh?n
                
                // Ð?i tr?ng thái: OFF(0) -> ON(1) -> AUTO(2) -> vòng l?i OFF(0)
                if (current_mode == MODE_AUTO) {
                    current_mode = MODE_OFF;
                } else {
                    current_mode++;
                }

                // C?p nh?t LED RGB ch? báo và LED c?m ngoài
                switch (current_mode) {
                    case MODE_OFF:
                        RGB_SetColor(0, 0, 0); // RGB T?t
                        EXT_LED_Off();         // T?t c?ng LED ngoài
                        break;

                    case MODE_ON:
                        RGB_SetColor(0, 1, 0); // RGB Xanh lá
                        EXT_LED_On();          // B?t c?ng LED ngoài
                        break;

                    case MODE_AUTO:
                        RGB_SetColor(1, 1, 0); // RGB Vàng
                        // KHÔNG tác d?ng LED ngoài ? dây, d? Task 2 t? lo
                        break;
                }
            }
        }
        last_btn_state = current_btn;
        OS_Delay(50); // Nhu?ng CPU
    }
}

/* =========================================
 * TASK 2: Ð?C C?M BI?N (CH? CH?Y KHI AUTO)
 * ========================================= */
void Task_Auto_Light(void *arg) {
    while (1) {
        // Ch? quét c?m bi?n và b?t/t?t LED ngoài khi dang ? ch? d? AUTO
        if (current_mode == MODE_AUTO) {
            
            uint8_t sensor_val = GPIO_Read(SENSOR_PORT, SENSOR_PIN);
            
            // Logic module FC-51 LM393 (T?i = 1, Sáng = 0)
            if (sensor_val == 1) { 
                EXT_LED_On();   // Tr?i t?i -> B?t LED ngoài
            } else {        
                EXT_LED_Off();  // Tr?i sáng -> T?t LED ngoài
            }
        }
        
        OS_Delay(100); // Quét m?i 100ms
    }
}
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
  PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
/* =========================================
 * HÀM MAIN
 * ========================================= */
int main(void) {
    // 1. Kh?i t?o HAL và Clock
    HAL_Init();
    SystemClock_Config(); 
    MX_GPIO_Init(); // Kh?i t?o các chân do CubeMX sinh ra (có thêm PA6)
    
    // 2. Kh?i t?o Driver c?a b?n
    GPIO_Driver_Init();
    
    // 3. Kh?i t?o RTOS
    OS_Init();

    // 4. T?o các Task
    OS_CreateTask(Task_Button_Control, NULL, stack_button, 128, 2, 5, "Btn_Task");
    OS_CreateTask(Task_Auto_Light, NULL, stack_sensor, 128, 3, 5, "Sens_Task");

    // 5. B?t d?u b? l?p l?ch
    OS_Start();

    while (1) {
        // CPU luôn b?n r?n trong các Task, không bao gi? roi xu?ng dây
    }
}
