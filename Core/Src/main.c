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

extern void Test_TV2_RunAll(void);
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
static uint32_t stack_led[128];
static uint32_t stack_heartbeat[192];
static uint32_t stack_usb_flush[128];
static uint32_t stack_usb_rx[192];
static uint32_t stack_idle[128];
static uint32_t stack_monitor[192];
static uint32_t stack_test[128];

static void Task_LED(void *arg)
{
    (void)arg;

    while (1) {
        LED_Toggle();

        if (USB_CDC_IsConnected()) {
            OS_Delay(500);
        } else {
            OS_Delay(200);
        }
    }
}

static void Task_Heartbeat(void *arg)
{
    (void)arg;

    while (1) {
        Proto_Heartbeat_t hb = {
            .uptime_ms = OS_GetTick(),
            .num_tasks = OS_GetTaskCount(),
            .os_ver_major = 0,
            .os_ver_minor = 1,
        };

        Proto_Send(PKT_HEARTBEAT, &hb, sizeof(hb));

        OS_Delay(1000);
    }
}

static void Task_USB_Flush(void *arg)
{
    (void)arg;

    while (1) {
        USB_CDC_FlushTX();
        OS_Delay(1);
    }
}

static void Task_USB_Rx(void *arg)
{
    (void)arg;

    uint8_t cmd[64];

    while (1) {
        if (USB_CDC_RxAvailable() > 0) {
            uint16_t n = USB_CDC_Receive(cmd, sizeof(cmd));
            (void)n;

            Proto_Log("USB command received\r\n");
        }

        OS_Delay(10);
    }
}

static void Task_Idle(void *arg)
{
    (void)arg;

    while (1) {
        __WFI();      // CPU ng? nh?, ch? interrupt ti?p theo
    }
}

static void Task_Monitor(void *arg)
{
    (void)arg;

    while (1) {
        uint8_t count = OS_GetTaskCount();

        for (uint8_t i = 0; i < count; i++) {
            OS_TCB_t *tcb = OS_GetTaskInfo(i);

            if (tcb != 0) {
                Proto_TaskStatus_t st;

                st.task_id = tcb->id;
                memset(st.name, 0, sizeof(st.name));

                if (tcb->name != 0) {
                    strncpy(st.name, tcb->name, sizeof(st.name) - 1);
                }

                st.state = (uint8_t)tcb->state;
                st.priority = tcb->priority;
                st.stack_free = OS_GetStackFreeBytes(i);

                Proto_Send(PKT_TASK_STATUS, &st, sizeof(st));
            }
        }

        OS_Delay(2000);
    }
}

static void Task_TV2_Test(void *arg)
{
    (void)arg;

    Test_TV2_RunAll();

    while (1)
    {
        OS_Delay(1000);
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  MX_GPIO_Init();
  MX_USB_DEVICE_Init();

  /* USER CODE BEGIN 2 */
GPIO_Driver_Init();
USB_CDC_Driver_Init();
LED_Off();

Proto_Log("System started\r\n");

OS_Init();

OS_CreateTask(Task_Heartbeat, 0, stack_heartbeat, 192, 1, 5, "heartbeat");
OS_CreateTask(Task_USB_Flush, 0, stack_usb_flush, 128, 2, 3, "usb_flush");
OS_CreateTask(Task_USB_Rx, 0, stack_usb_rx, 192, 2, 5, "usb_rx");
OS_CreateTask(Task_LED, 0, stack_led, 128, 3, 5, "led");

/* Idle task: uu tiên th?p nh?t */
OS_CreateTask(Task_Idle, 0, stack_idle, 128, 255, 1, "idle");
OS_CreateTask(Task_Monitor, 0, stack_monitor, 192, 4, 5, "monitor");
OS_CreateTask(Task_TV2_Test, NULL, stack_test, 128, 3);
OS_Start();
  /* USER CODE END 2 */

  /* USER CODE BEGIN WHILE */


  while (1)
  {
}

}
/**
  * @brief System Clock Configuration
  * @retval None
  */
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

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
