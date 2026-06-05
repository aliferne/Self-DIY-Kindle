/*
 * chip/stm32f4/uart_chip.c
 */

#include "bsp_uart.h"

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_uart.h"
#include "stm32f4xx_hal_usart.h"
#include <stdint.h>

#define UART_MAX_TIMEOUT 5000U

UART_Err_t uart_init(UART_Model_t *m, const UART_Config_t *cfg)
{
}

UART_Err_t uart_deinit(UART_Model_t *m)
{
}

UART_Err_t uart_send(UART_Model_t *m, const uint8_t *data, uint16_t len)
{
    UART_Err_t ret        = UART_OK;
    UART_HandleTypeDef *h = (UART_HandleTypeDef *)m->handle;

    switch (m->config.mode) {
        case UART_Mode_Polling:
            HAL_UART_Transmit(h, data, len, UART_MAX_TIMEOUT);
            break;
        case UART_Mode_Interrupt:
            HAL_UART_Transmit_IT(h, data, len);
            break;
        case UART_Mode_DMA:
            HAL_UART_Transmit_DMA(h, data, len);
            break;
    }

    return ret;
}

UART_Err_t uart_recv(UART_Model_t *m, uint8_t *data, uint16_t len)
{
    UART_Err_t ret        = UART_OK;
    UART_HandleTypeDef *h = (UART_HandleTypeDef *)m->handle;

    switch (m->config.mode) {
        case UART_Mode_Polling:
            HAL_UART_Receive(h, data, len, UART_MAX_TIMEOUT);
            break;
        case UART_Mode_Interrupt:
            HAL_UART_Receive_IT(h, data, len);
            break;
        case UART_Mode_DMA:
            HAL_UART_Receive_DMA(h, data, len);
            break;
    }

    return ret;
}

UART_Err_t uart_attach_irq(UART_Model_t *m, void (*callback)(void *))
{
}

UART_Err_t uart_detach_irq(UART_Model_t *m)
{
}
