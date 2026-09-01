/*
 * chip/stm32f4/uart_chip.c
 */

#include "bsp_sys.h"
#include "bsp_uart.h"

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_uart.h"
#include <stdint.h>
#include <string.h>

#define UART_MAX_TIMEOUT 5000U

UART_Err_t uart_init(UART_Model_t *m, UART_Handle_t handle, const UART_Config_t *cfg)
{
    if (m == NULL || handle == NULL || cfg == NULL)
        return UART_Err_Invalid_Args;

    /* 初始化操作交由 CubeMX 生成的代码 */
    m->config = *cfg;
    m->handle = handle;

    return UART_Err_Ok;
}

UART_Err_t uart_deinit(UART_Model_t *m)
{
    if (m == NULL || m->handle == NULL)
        return UART_Err_Invalid_Args;

    /* 总觉得结构体应该加一个已经 deinit 的标志位，对于所有 bsp 的来说都是 */
    HAL_UART_DeInit((UART_HandleTypeDef *)m->handle);

    return UART_Err_Ok;
}

UART_Err_t uart_send(UART_Model_t *m, const uint8_t *data, uint16_t len)
{
    UART_Err_t ret            = UART_Err_Ok;
    UART_HandleTypeDef *h     = (UART_HandleTypeDef *)m->handle;
    HAL_StatusTypeDef hal_ret = HAL_OK;

    switch (m->config.mode) {
        case UART_Mode_Polling:
            hal_ret = HAL_UART_Transmit(h, data, len, UART_MAX_TIMEOUT);
            break;
        /* 
        TODO: 下面这些模式应当借助环形缓冲区实现，
              中断发送会在一段时间内返回 HAL_BUSY，
              此时如果紧接着是另一个中断发送，则该发送可能无法被发送
        */
        case UART_Mode_Interrupt:
            hal_ret = HAL_UART_Transmit_IT(h, data, len);
            break;
        case UART_Mode_DMA:
            hal_ret = HAL_UART_Transmit_DMA(h, data, len);
            break;
    }

    if (hal_ret == HAL_TIMEOUT)
        ret = UART_Err_Timeout;
    else if (hal_ret != HAL_OK)
        ret = UART_Err_Generic;

    return ret;
}

UART_Err_t uart_recv(UART_Model_t *m, uint8_t *data, uint16_t len)
{
    UART_Err_t ret            = UART_Err_Ok;
    UART_HandleTypeDef *h     = (UART_HandleTypeDef *)m->handle;
    HAL_StatusTypeDef hal_ret = HAL_OK;

    switch (m->config.mode) {
        case UART_Mode_Polling:
            hal_ret = HAL_UART_Receive(h, data, len, UART_MAX_TIMEOUT);
            break;
        case UART_Mode_Interrupt:
            hal_ret = HAL_UART_Receive_IT(h, data, len);
            break;
        case UART_Mode_DMA:
            hal_ret = HAL_UART_Receive_DMA(h, data, len);
            break;
    }

    if (hal_ret == HAL_TIMEOUT)
        ret = UART_Err_Timeout;
    else if (hal_ret != HAL_OK)
        ret = UART_Err_Generic;

    return ret;
}

/* TODO: */
UART_Err_t uart_attach_irq(UART_Model_t *m, void (*callback)(void *))
{
}

UART_Err_t uart_detach_irq(UART_Model_t *m)
{
}
