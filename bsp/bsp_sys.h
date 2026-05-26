#pragma once

#include "cmsis_os.h"

#define __nop() __asm volatile("nop")

/*
 * system handler, handling system-level operations, such as delay, etc.
 */

#define Delay(x) osDelay(x)

/*
 * 借助 __nop 实现延时
 */
static inline void DelayUs(uint32_t us)
{
    while (us--)
        __nop();
}
