#include "irq_chip.h"
#include "bsp_handle.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_uart.h"

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == NULL) return;

    GIVEUP(huart);
    // if (__HAL_UART_GET_FLAG(huart, UART_FLAG_TC) != RESET)
    //     __HAL_UART_CLEAR_FLAG(huart, UART_FLAG_TC);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == NULL) return;

    GIVEUP(huart);
    // if (__HAL_UART_GET_FLAG(huart, UART_FLAG_RXNE) != RESET)
    //     __HAL_UART_CLEAR_FLAG(huart, UART_FLAG_RXNE);
}