#include "irq_chip.h"
#include "stm32f4xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"

/* 优先使用 HAL 库提供的系统中断文件 */
#define __weak __attribute__((weak))

__weak void NMI_Handler(void)
{
    while (1) {}
}

__weak void HardFault_Handler(void)
{
    while (1) {}
}

__weak void MemManage_Handler(void)
{
    while (1) {}
}

__weak void BusFault_Handler(void)
{
    while (1) {}
}

__weak void UsageFault_Handler(void)
{
    while (1) {}
}

__weak void DebugMon_Handler(void)
{
}

__weak void SysTick_Handler(void)
{
    HAL_IncTick();
#if (INCLUDE_xTaskGetSchedulerState == 1)
    if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
#endif /* INCLUDE_xTaskGetSchedulerState */
        xPortSysTickHandler();
#if (INCLUDE_xTaskGetSchedulerState == 1)
    }
#endif /* INCLUDE_xTaskGetSchedulerState */
}
