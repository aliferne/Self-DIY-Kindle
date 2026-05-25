#pragma once

/*
 * ============================================================
 * bsp_irq_hal.h  —  STM32 HAL 中断回调桥接
 * ============================================================
 *
 * 本文件是 HAL EXTI 回调与 GPIO 中断表之间的桥。
 * 迁移芯片时需要同步替换。
 */

#include <stdint.h>

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
