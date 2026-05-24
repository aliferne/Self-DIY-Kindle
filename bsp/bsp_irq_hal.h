#pragma once

/*
 * interrupt handler for HAL lib
 */

#include "bsp_sys.h"
#include "bsp_gpio.h"

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
