#ifndef GPIO_DRIVER_H
#define GPIO_DRIVER_H

#include "stm32f1xx_hal.h"

/* ===== Ð?NH NGHIA CHÂN ===== */
/* LED onboard Blue Pill – PC13, active LOW */
#define LED_PORT        GPIOC
#define LED_PIN         GPIO_PIN_13

/* Debug pin – PA1, dùng d? do b?ng oscilloscope */
#define DBG_PORT        GPIOA
#define DBG_PIN_NUM     GPIO_PIN_1

/* ===== API ===== */
void    GPIO_Driver_Init(void);

/* LED */
void    LED_On(void);
void    LED_Off(void);
void    LED_Toggle(void);

/* Debug pin – TV1 g?i ? d?u/cu?i Context Switch d? do th?i gian */
void    DBG_Set(void);
void    DBG_Reset(void);
void    DBG_Toggle(void);

/* Ða nang */
void    GPIO_Write(GPIO_TypeDef *port, uint16_t pin, uint8_t state);
uint8_t GPIO_Read(GPIO_TypeDef *port, uint16_t pin);

#endif

