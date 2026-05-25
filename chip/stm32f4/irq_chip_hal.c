/*
 * ============================================================
 * bsp_irq_hal.c  —  中断回调桥接实现
 * ============================================================
 */

#include "bsp_gpio.h"
#include "bsp_irq_hal.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    int pin_num = gpio_get_pin_num(GPIO_Pin);

    if (pin_num >= 0 && pin_num < GPIO_MAX_PIN) {
        GPIO_Model_t *m = gpio_irq_models[pin_num];
        if (m != NULL) {
            gpio_set_irq_flag(m);
        }
    }
}
