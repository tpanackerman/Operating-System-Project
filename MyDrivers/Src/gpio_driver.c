#include "gpio_driver.h"

/* CubeMX dã t?o MX_GPIO_Init() trong Core/Src/gpio.c
   Hàm dó du?c g?i trong main() r?i.
   File này ch? cung c?p API d? dùng cho c? nhóm. */

void GPIO_Driver_Init(void) {
    /* Không c?n làm gì thêm – CubeMX dã init */
    /* Ð? dây phòng khi c?n m? r?ng */
}

/* ===== LED ===== */
void LED_On(void) {
    /* PC13 active LOW: kéo xu?ng 0 = dèn sáng */
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_RESET);
}

void LED_Off(void) {
    HAL_GPIO_WritePin(LED_PORT, LED_PIN, GPIO_PIN_SET);
}

void LED_Toggle(void) {
    HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
}

/* ===== DEBUG PIN ===== */
void DBG_Set(void) {
    HAL_GPIO_WritePin(DBG_PORT, DBG_PIN_NUM, GPIO_PIN_SET);
}

void DBG_Reset(void) {
    HAL_GPIO_WritePin(DBG_PORT, DBG_PIN_NUM, GPIO_PIN_RESET);
}

void DBG_Toggle(void) {
    HAL_GPIO_TogglePin(DBG_PORT, DBG_PIN_NUM);
}

/* ===== ÐA NANG ===== */
void GPIO_Write(GPIO_TypeDef *port, uint16_t pin, uint8_t state) {
    HAL_GPIO_WritePin(port, pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

uint8_t GPIO_Read(GPIO_TypeDef *port, uint16_t pin) {
    return (uint8_t)HAL_GPIO_ReadPin(port, pin);
}
