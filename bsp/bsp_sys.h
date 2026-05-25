#pragma once

#include "cmsis_os.h"

/*
 * system handler, handling system-level operations, such as delay, etc.
 */

#define Delay(x) osDelay(x)

/*
 * 微秒级忙等延时，无内联汇编，跨 Cortex-M 通用。
 * 精度依赖于芯片主频，如有精确延时需求可在 chip 层替换。
 */
static inline void DelayUs(uint32_t us)
{
    while (us--)
        __NOP();
}
