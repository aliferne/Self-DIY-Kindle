#include "bsp_handle.h"
#include "bsp_sys.h"
#include "bsp_uart.h"
#include "cmsis_os.h"
#include "stm32f4xx_hal.h"
#include "usart.h"
#include <stdint.h>

#define SYS_USE_UART &huart1
static UART_Model_t sys_com;

void sys_chip_init(void)
{
    UART_Config_t cfg = {
        /* TODO: */
        .mode = UART_Mode_Polling,
    };

    uart_init(&sys_com, SYS_USE_UART, &cfg);
}

/*
 * 此函数依赖于 SYS_USE_UART
 */
int _write(int file, char *ptr, int len)
{
    GIVEUP(file);

    /* 对于 printf, 应当使用这种阻塞的方式 */
    HAL_UART_Transmit(SYS_USE_UART, (uint8_t *)ptr, len, 2500);
    while (__HAL_UART_GET_FLAG(SYS_USE_UART, UART_FLAG_TC) == RESET);

    return len;
}

/* ============== 系统延时相关 ============== */

uint32_t chip_get_tick(void)
{
    return HAL_GetTick();
}

/* -------------- 阻塞式 -------------- */

void chip_delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}

/*
 * 函数是否到达最大延时，返回 1 则表示到达
 * 此函数仅负责判断 SysTick 是否到达预设的目标值，实际使用方法为：
 *
 * ```c
 * while ( <cond> )
 * {
 *      if (chip_till_max_delay(start_time, max_delay))
 *          // timeout, do something then
 * }
 * ```
 *
 * 其本身是非阻塞的，但需要通过阻塞的方式来实现延时效果
 */
uint8_t chip_till_max_delay(uint32_t start_time, uint32_t max_delay)
{
    return (chip_get_tick() - start_time) >= max_delay;
}

/* -------------- 非阻塞式 -------------- */

void os_delay_ms(uint32_t ms)
{
    osDelay(ms);
}

void os_delay_until(uint32_t *prv_wake_time, uint32_t ms)
{
    osDelayUntil(prv_wake_time, ms);
}

#ifdef USE_DWT_DELAY

void dwt_init(void)
{
    if (!(CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk)) {
        // 使能 DWT 跟踪
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        // 使能 CYCCNT 计数器
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
        // 清空计数值
        DWT->CYCCNT = 0;
    }
}

void dwt_delay_us(uint32_t us)
{
    uint32_t start        = DWT->CYCCNT;
    uint32_t delay_cycles = us * (SystemCoreClock / 1000000);
    while ((DWT->CYCCNT - start) < delay_cycles);
}

void dwt_delay_ms(uint32_t ms)
{
    dwt_delay_us(ms * 1000);
}

#endif
