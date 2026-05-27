#include "bsp_sys.h"
#include "cmsis_os.h"
#include "stm32f4xx_hal.h"

void chip_delay_ms(uint32_t ms)
{
    HAL_Delay(ms);
}

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
