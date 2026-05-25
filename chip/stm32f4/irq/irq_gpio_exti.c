#include "irq_chip.h"
#include "bsp_gpio.h"
#include "stm32f4xx_hal.h"

/*
 * 负责处理各类中断
 */

/* ============================================================
 * 局部变量/函数
 * ============================================================ */
static void handle_exti_irq(uint16_t GPIO_Pin);

/* ============================================================
 * IRQ 接口实现
 * ============================================================ */

void EXTI0_IRQHandler(void)
{
    handle_exti_irq(1 << 0);
}

void EXTI1_IRQHandler(void)
{
    handle_exti_irq(1 << 1);
}

void EXTI2_IRQHandler(void)
{
    handle_exti_irq(1 << 2);
}

void EXTI3_IRQHandler(void)
{
    handle_exti_irq(1 << 3);
}

void EXTI4_IRQHandler(void)
{
    handle_exti_irq(1 << 4);
}

void EXTI9_5_IRQHandler(void)
{
    for (int i = 5; i <= 9; i++) {
        handle_exti_irq(1 << i);
    }
}

void EXTI15_10_IRQHandler(void)
{
    for (int i = 10; i <= 15; i++) {
        handle_exti_irq(1 << i);
    }
}

/*
 * 处理EXTI中断
 * 本质是通过注册好的 GPIO Model 表找到对应的 irq_flag 并置位。
 */
static void handle_exti_irq(uint16_t GPIO_Pin)
{
    if (__HAL_GPIO_EXTI_GET_IT(GPIO_Pin) != RESET) {
        __HAL_GPIO_EXTI_CLEAR_IT(GPIO_Pin);

        int pin_num     = gpio_get_pin_num(GPIO_Pin);
        GPIO_Model_t *m = gpio_irq_models[pin_num];

        if (m != NULL)
            gpio_set_irq_flag(m);
    }
}
