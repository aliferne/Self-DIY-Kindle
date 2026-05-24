#include "bsp_irq_hal.h"

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    int pin_num = gpio_get_pin_num(GPIO_Pin);

    if (gpio_exti_callback[pin_num] != NULL)
        gpio_exti_callback[pin_num]();
}
