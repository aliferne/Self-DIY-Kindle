#pragma once

/* =================================================
 * 板级系统层，需要在 chip 层中实现一些 API
 * ================================================= */

#include <stdint.h>
#include <stddef.h>

#define GIVEUP(x) (void)(x)

/* 以下的延时为阻塞式的 */

void chip_delay_ms(uint32_t ms);

/* 以下的延时为非阻塞式的，通常由 RTOS 层提供 */

void os_delay_ms(uint32_t ms);
void os_delay_until(uint32_t *prv_wake_time, uint32_t ms);

/* 以下为特定内核的单片机才有的外设，如果没有则移除宏定义 */
#define USE_DWT_DELAY

#ifdef USE_DWT_DELAY

/* 使用 dwt 模块之前一定需要先初始化 */

void dwt_init(void);
void dwt_delay_us(uint32_t us);
void dwt_delay_ms(uint32_t ms);

#endif
