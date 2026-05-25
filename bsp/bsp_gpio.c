#include "bsp_gpio.h"
#include "stm32f407xx.h"
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_rcc.h"

/*
 * usage:
 *
 * you should always call `gpio_register` at first,
 * it will set up the neccesary variables and functions for the gpio.
 * then call `gpio_model.gpio_init` to initialize your gpio.
 *
 * ```c
 * GPIO_Model_t usr_led;
 * GPIOx_Config_t init_conf = {0};
 * init_conf.mode           = GPIO_MODE_OUTPUT_PP;
 * init_conf.pull           = GPIO_NOPULL;
 * gpio_init(&usr_led, &init_conf);
 * gpio_write_pin(&usr_led, GPIO_PIN_SET);
 * ```
 */

/* callback functions of EXTI */
gpio_irq_callback_t gpio_exti_callback[MAX_GPIO_CALLBACK] = {NULL};

static void gpio_clock_init(GPIOx_Type_t gpiox);
static IRQn_Type get_gpio_irqn(Pinx_Type_t pin);

/*
 * register gpio port and pin,
 * it will assign port, pin and functions to `gpio_model`
 */
void gpio_register(
    GPIO_Model_t *gpio_model, GPIOx_Type_t gpiox, Pinx_Type_t pinx)
{
    gpio_model->GPIOx           = gpiox;
    gpio_model->Pinx            = pinx;
    gpio_model->__use_interrupt = 0;
    /*
    since all gpio models shares the same functions,
    there's no need to assign functions for each model.
    */
}

/*
 * initialize gpio
 */
void gpio_init(GPIO_Model_t *gpio_model, GPIOx_Config_t *init_conf)
{
    gpio_clock_init(gpio_model->GPIOx);

    GPIO_InitTypeDef gpio_init = {
        .Pin       = gpio_model->Pinx,
        .Mode      = init_conf->mode,
        .Pull      = init_conf->pull,
        .Speed     = init_conf->speed,
        .Alternate = init_conf->alternate,
    };
    gpio_model->config = *init_conf;

    HAL_GPIO_Init(gpio_model->GPIOx, &gpio_init);
}

/*
 * deinitialize gpio
 */
void gpio_deinit(GPIO_Model_t *gpio_model)
{
    HAL_GPIO_DeInit(gpio_model->GPIOx, gpio_model->Pinx);
}

/*
 * write high/low level to pin
 */
void gpio_write_pin(GPIO_Model_t *gpio_model, GPIO_Pin_State_e stat)
{
    /* EXTI means that GPIO is in input mode */
    if (gpio_model->__use_interrupt == 1)
        return;

    HAL_GPIO_WritePin(gpio_model->GPIOx, gpio_model->Pinx, stat);
}

/*
 * read the voltage level from gpio
 */
GPIO_Pin_State_e gpio_read_pin(GPIO_Model_t *gpio_model)
{
    return HAL_GPIO_ReadPin(gpio_model->GPIOx, gpio_model->Pinx);
}

/*
 * toggle voltage level
 */
void gpio_toggle_pin(GPIO_Model_t *gpio_model)
{
    /* EXTI means that GPIO is in input mode */
    if (gpio_model->__use_interrupt == 1)
        return;
    
    HAL_GPIO_TogglePin(gpio_model->GPIOx, gpio_model->Pinx);
}

/*
 * attach interrupt for gpio pin
 */
void gpio_attach_interrupt(GPIO_Model_t *gpio_model, GPIOx_IRQ_Config_t *irq_conf)
{
    const GPIOx_Config_t *config = &gpio_model->config;
    GPIO_InitTypeDef gpio_init   = {
          .Pin       = gpio_model->Pinx,
          .Mode      = irq_conf->trigger_edge,
          .Pull      = config->pull,
          .Speed     = config->speed,
          .Alternate = config->alternate,
    };

    HAL_GPIO_Init(gpio_model->GPIOx, &gpio_init);

    gpio_model->irq_config = *irq_conf;

    /* register isr in the isr_table */
    int pin_num                 = gpio_get_pin_num(gpio_model->Pinx);
    gpio_exti_callback[pin_num] = irq_conf->irq_callback;

    IRQn_Type irqn = get_gpio_irqn(gpio_model->Pinx);
    HAL_NVIC_SetPriority(irqn, irq_conf->preempt_priority, irq_conf->sub_priority);
    HAL_NVIC_EnableIRQ(irqn);
    gpio_model->__use_interrupt = 1;
}

void gpio_detach_interrupt(GPIO_Model_t *gpio_model)
{
    int pin_num                 = gpio_get_pin_num(gpio_model->Pinx);
    gpio_exti_callback[pin_num] = NULL;

    IRQn_Type irqn = get_gpio_irqn(gpio_model->Pinx);
    HAL_NVIC_DisableIRQ(irqn);
    gpio_model->__use_interrupt = 1;

    const GPIOx_Config_t *config = &gpio_model->config;
    GPIO_InitTypeDef gpio_init   = {
          .Pin       = gpio_model->Pinx,
          .Mode      = config->mode,
          .Pull      = config->pull,
          .Speed     = config->speed,
          .Alternate = config->alternate,
    };
    HAL_GPIO_Init(gpio_model->GPIOx, &gpio_init);
}

/*
 * automatically get irqn by gpio pin number
 * should read the datasheets to ensure which pin leads to which EXTI_IRQn
 */
static IRQn_Type get_gpio_irqn(Pinx_Type_t pin)
{
    int pin_num = gpio_get_pin_num(pin);

    if (pin_num <= 4) {
        const IRQn_Type line[] =
            {EXTI0_IRQn, EXTI1_IRQn, EXTI2_IRQn, EXTI3_IRQn, EXTI4_IRQn};
        return line[pin_num];
    } else if (pin_num <= 9) {
        return EXTI9_5_IRQn;
    } else {
        return EXTI15_10_IRQn;
    }
}

/*
 * automatically get pin num when program is running
 *
 * warning: may only support gcc
 */
int gpio_get_pin_num(Pinx_Type_t pin)
{
    if (pin == 0)
        return -1;
    /* should ensure that numbers in binary has only one 1-bit, otherwise 0 is returned */
    return __builtin_ctz(pin);
}

/*
 * initialize clock for gpio,
 * this function assumes that GPIOA should be exists
 * and other ports depends on if is defined
 */
static void gpio_clock_init(GPIOx_Type_t gpiox)
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
