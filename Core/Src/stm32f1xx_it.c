/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f1xx_it.c
  * @brief   Interrupt Service Routines.
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
#include "stm32f1xx_it.h"
#include "tiny_os.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

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
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern PCD_HandleTypeDef hpcd_USB_FS;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M3 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
   while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
  #if defined(__CC_ARM)
__asm void SVC_Handler(void)
{
    IMPORT OS_CurrentTCB

    LDR R0, =OS_CurrentTCB
    LDR R1, [R0]
    LDR R0, [R1]

    LDMIA R0!, {R4-R11}
    MSR PSP, R0

    MOVS R0, #2
    MSR CONTROL, R0
    ISB

    LDR LR, =0xFFFFFFFD
    BX LR
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
__asm void PendSV_Handler(void)
{
    IMPORT OS_CurrentTCB
    IMPORT OS_Schedule

    MRS R0, PSP

    LDR R3, =OS_CurrentTCB
    LDR R2, [R3]

    CBZ R2, PendSV_CallScheduler

    STMDB R0!, {R4-R11}
    STR R0, [R2]

PendSV_CallScheduler
    PUSH {R3, LR}
    BL OS_Schedule
    POP {R3, LR}

    STR R0, [R3]

    LDR R1, [R0]
    LDMIA R1!, {R4-R11}
    MSR PSP, R1

    BX LR
}

#else

/* Keil ARM Compiler 6 / GCC */

__attribute__((naked)) void SVC_Handler(void)
{
    __asm volatile (
        "ldr r0, =OS_CurrentTCB        \n"
        "ldr r1, [r0]                  \n"
        "ldr r0, [r1]                  \n"

        "ldmia r0!, {r4-r11}           \n"
        "msr psp, r0                   \n"

        "movs r0, #2                   \n"
        "msr control, r0               \n"
        "isb                           \n"

        "ldr lr, =0xFFFFFFFD           \n"
        "bx lr                         \n"
    );
}

__attribute__((naked)) void PendSV_Handler(void)
{
    __asm volatile (
        "mrs r0, psp                   \n"

        "ldr r3, =OS_CurrentTCB        \n"
        "ldr r2, [r3]                  \n"

        "cbz r2, 1f                    \n"

        "stmdb r0!, {r4-r11}           \n"
        "str r0, [r2]                  \n"

        "1:                            \n"
        "push {r3, lr}                 \n"
        "bl OS_Schedule                \n"
        "pop {r3, lr}                  \n"

        "str r0, [r3]                  \n"

        "ldr r1, [r0]                  \n"
        "ldmia r1!, {r4-r11}           \n"
        "msr psp, r1                   \n"

        "bx lr                         \n"
    );
}

#endif

void SysTick_Handler(void)
{
    HAL_IncTick();
    OS_Tick_Handler();
}

/******************************************************************************/
/* STM32F1xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f1xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles USB low priority or CAN RX0 interrupts.
  */
void USB_LP_CAN1_RX0_IRQHandler(void)
{
  /* USER CODE BEGIN USB_LP_CAN1_RX0_IRQn 0 */

  /* USER CODE END USB_LP_CAN1_RX0_IRQn 0 */
  HAL_PCD_IRQHandler(&hpcd_USB_FS);
  /* USER CODE BEGIN USB_LP_CAN1_RX0_IRQn 1 */

  /* USER CODE END USB_LP_CAN1_RX0_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
