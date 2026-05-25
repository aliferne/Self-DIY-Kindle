/*
 * ============================================================
 * chip/stm32f4/gpio_chip.c
 *
 * BSP GPIO 抽象的 STM32F4 芯片实现。
 * ============================================================
 */

#include "bsp_gpio.h"

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_rcc.h"

/* ============================================================
 * 中断模型表
 *
 * 按 pin 号索引。ISR 查表找到 GPIO_Model_t 后置 irq_flag。
 * ============================================================ */

GPIO_Model_t *gpio_irq_models[GPIO_MAX_PIN] = {NULL};

/* ============================================================
 * 内部辅助：BSP 枚举 → HAL 枚举
 * ============================================================ */

static uint32_t mode_to_hal(GPIO_Mode_t m)
{
    switch (m) {
        case GPIO_Mode_Input:
            return GPIO_MODE_INPUT;
        case GPIO_Mode_Output_PP:
            return GPIO_MODE_OUTPUT_PP;
        case GPIO_Mode_Output_OD:
            return GPIO_MODE_OUTPUT_OD;
        case GPIO_Mode_AF_PP:
            return GPIO_MODE_AF_PP;
        case GPIO_Mode_AF_OD:
            return GPIO_MODE_AF_OD;
        case GPIO_Mode_Analog:
            return GPIO_MODE_ANALOG;
        case GPIO_Mode_IT_Rising:
            return GPIO_MODE_IT_RISING;
        case GPIO_Mode_IT_Falling:
            return GPIO_MODE_IT_FALLING;
        case GPIO_Mode_IT_Rising_Falling:
            return GPIO_MODE_IT_RISING_FALLING;
        default:
            return GPIO_MODE_INPUT;
    }
}

static uint32_t pull_to_hal(GPIO_Pull_t p)
{
    switch (p) {
        case GPIO_Pull_None:
            return GPIO_NOPULL;
        case GPIO_Pull_Up:
            return GPIO_PULLUP;
        case GPIO_Pull_Down:
            return GPIO_PULLDOWN;
        default:
            return GPIO_NOPULL;
    }
}

static uint32_t speed_to_hal(GPIO_Speed_t s)
{
    switch (s) {
        case GPIO_Speed_Low:
            return GPIO_SPEED_FREQ_LOW;
        case GPIO_Speed_Medium:
            return GPIO_SPEED_FREQ_MEDIUM;
        case GPIO_Speed_High:
            return GPIO_SPEED_FREQ_HIGH;
        case GPIO_Speed_Very_High:
            return GPIO_SPEED_FREQ_VERY_HIGH;
        default:
            return GPIO_SPEED_FREQ_LOW;
    }
}

static GPIO_PinState level_to_hal(GPIO_Level_t l)
{
    return (l == GPIO_Level_High) ? GPIO_PIN_SET : GPIO_PIN_RESET;
}

static GPIO_Level_t level_from_hal(GPIO_PinState s)
{
    return (s == GPIO_PIN_SET) ? GPIO_Level_High : GPIO_Level_Low;
}

/* ============================================================
 * 内部辅助：时钟初始化
 * ============================================================ */

static void clock_init(GPIO_TypeDef *gpiox)
{
    if (gpiox == GPIOA)
        __HAL_RCC_GPIOA_CLK_ENABLE();
#ifdef GPIOB
    else if (gpiox == GPIOB)
        __HAL_RCC_GPIOB_CLK_ENABLE();
#endif
#ifdef GPIOC
    else if (gpiox == GPIOC)
        __HAL_RCC_GPIOC_CLK_ENABLE();
#endif
#ifdef GPIOD
    else if (gpiox == GPIOD)
        __HAL_RCC_GPIOD_CLK_ENABLE();
#endif
#ifdef GPIOE
    else if (gpiox == GPIOE)
        __HAL_RCC_GPIOE_CLK_ENABLE();
#endif
#ifdef GPIOF
    else if (gpiox == GPIOF)
        __HAL_RCC_GPIOF_CLK_ENABLE();
#endif
#ifdef GPIOG
    else if (gpiox == GPIOG)
        __HAL_RCC_GPIOG_CLK_ENABLE();
#endif
#ifdef GPIOH
    else if (gpiox == GPIOH)
        __HAL_RCC_GPIOH_CLK_ENABLE();
#endif
#ifdef GPIOI
    else if (gpiox == GPIOI)
        __HAL_RCC_GPIOI_CLK_ENABLE();
#endif
}

/* ============================================================
 * 内部辅助：中断向量号获取
 * ============================================================ */

static IRQn_Type get_exti_irqn(uint16_t pin)
{
    int n = gpio_get_pin_num(pin);

    if (n < 0)
        return (IRQn_Type)0;

    if (n <= 4) {
        const IRQn_Type line[] = {
            EXTI0_IRQn, EXTI1_IRQn, EXTI2_IRQn, EXTI3_IRQn, EXTI4_IRQn};
        return line[n];
    } else if (n <= 9) {
        return EXTI9_5_IRQn;
    } else {
        return EXTI15_10_IRQn;
    }
}

/* ============================================================
 * API 实现
 * ============================================================ */

GPIO_Err_t gpio_register(GPIO_Model_t *m, GPIO_Port_t port, GPIO_Pin_t pin)
{
    m->port     = port;
    m->pin      = pin;
    m->use_irq  = 0;
    m->irq_flag = 0;

    return GPIO_Err_OK;
}

GPIO_Err_t gpio_init(GPIO_Model_t *m, const GPIO_Config_t *cfg)
{
    GPIO_TypeDef *gpiox = (GPIO_TypeDef *)m->port;

    clock_init(gpiox);

    GPIO_InitTypeDef init = {
        .Pin       = m->pin,
        .Mode      = mode_to_hal(cfg->mode),
        .Pull      = pull_to_hal(cfg->pull),
        .Speed     = speed_to_hal(cfg->speed),
        .Alternate = cfg->alternate,
    };
    m->config = *cfg;

    HAL_GPIO_Init(gpiox, &init);
    return GPIO_Err_OK;
}

GPIO_Err_t gpio_deinit(GPIO_Model_t *m)
{
    HAL_GPIO_DeInit((GPIO_TypeDef *)m->port, m->pin);
    return GPIO_Err_OK;
}

GPIO_Err_t gpio_write(GPIO_Model_t *m, GPIO_Level_t level)
{
    if (m->use_irq)
        return GPIO_Err_Call_Write_When_Using_IRQ;

    HAL_GPIO_WritePin((GPIO_TypeDef *)m->port, m->pin, level_to_hal(level));

    return GPIO_Err_OK;
}

GPIO_Level_t gpio_read(GPIO_Model_t *m)
{
    return level_from_hal(HAL_GPIO_ReadPin((GPIO_TypeDef *)m->port, m->pin));
}

GPIO_Err_t gpio_toggle(GPIO_Model_t *m)
{
    if (m->use_irq)
        return GPIO_Err_Call_Write_When_Using_IRQ;

    HAL_GPIO_TogglePin((GPIO_TypeDef *)m->port, m->pin);

    return GPIO_Err_OK;
}

GPIO_Err_t gpio_attach_irq(GPIO_Model_t *m, const GPIO_IRQ_Config_t *irq_cfg)
{
    GPIO_TypeDef *gpiox = (GPIO_TypeDef *)m->port;

    GPIO_InitTypeDef init = {
        .Pin       = m->pin,
        .Mode      = mode_to_hal(irq_cfg->trigger_edge),
        .Pull      = pull_to_hal(m->config.pull),
        .Speed     = speed_to_hal(m->config.speed),
        .Alternate = m->config.alternate,
    };

    HAL_GPIO_Init(gpiox, &init);

    m->irq_config = *irq_cfg;
    m->irq_flag   = 0;

    /* 将 model 注册到中断表，供 ISR 查找 */
    int pin_num = gpio_get_pin_num(m->pin);
    if (pin_num >= 0 && pin_num < GPIO_MAX_PIN) {
        if (gpio_irq_models[pin_num] != NULL) {
            return GPIO_Err_IRQ_Table_Was_Registered;
        }
        gpio_irq_models[pin_num] = m;
    } else {
        return GPIO_Err_Pin_Num_Out_Of_Index;
    }

    IRQn_Type irqn = get_exti_irqn(m->pin);
    HAL_NVIC_SetPriority(irqn, irq_cfg->preempt_priority, irq_cfg->sub_priority);
    HAL_NVIC_EnableIRQ(irqn);

    m->use_irq = 1;

    return GPIO_Err_OK;
}

GPIO_Err_t gpio_detach_irq(GPIO_Model_t *m)
{
    int pin_num = gpio_get_pin_num(m->pin);
    if (pin_num >= 0 && pin_num < GPIO_MAX_PIN) {
        gpio_irq_models[pin_num] = NULL;
    } else {
        return GPIO_Err_Pin_Num_Out_Of_Index;
    }

    IRQn_Type irqn = get_exti_irqn(m->pin);
    HAL_NVIC_DisableIRQ(irqn);

    m->use_irq  = 0;
    m->irq_flag = 0;

    GPIO_InitTypeDef init = {
        .Pin       = m->pin,
        .Mode      = mode_to_hal(m->config.mode),
        .Pull      = pull_to_hal(m->config.pull),
        .Speed     = speed_to_hal(m->config.speed),
        .Alternate = m->config.alternate,
    };

    HAL_GPIO_Init((GPIO_TypeDef *)m->port, &init);

    return GPIO_Err_OK;
}

void gpio_set_irq_flag(GPIO_Model_t *m)
{
    m->irq_flag = 1;
}

void gpio_clear_irq_flag(GPIO_Model_t *m)
{
    m->irq_flag = 0;
}

int gpio_get_pin_num(GPIO_Pin_t pin)
{
    if (pin == 0)
        return -1;

    return __builtin_ctz(pin);
}
