#include "bsp_gpio.h"
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
 * gpio_register(&usr_led, USER_LED_PORT, USER_LED_PIN);
 * GPIOx_Config_t init_conf = {0};
 * init_conf.mode           = GPIO_MODE_OUTPUT_PP;
 * init_conf.pull           = GPIO_NOPULL;
 * usr_led.init(&usr_led, &init_conf);
 * usr_led.write_pin(&usr_led, GPIO_PIN_SET);
 * ```
 */

static void gpio_clock_init(GPIOx_Type_t gpiox);

/*
 * register gpio port and pin,
 * it will assign port, pin and functions to `gpio_model`
 */
void gpio_register(
    GPIO_Model_t *gpio_model, GPIOx_Type_t gpiox, Pinx_Type_t pinx)
{
    gpio_model->GPIOx            = gpiox;
    gpio_model->Pinx             = pinx;
    gpio_model->init             = gpio_init;
    gpio_model->read_pin         = gpio_read_pin;
    gpio_model->write_pin        = gpio_write_pin;
    gpio_model->toggle_pin       = gpio_toggle_pin;
    gpio_model->attach_interrupt = gpio_attach_interrupt;
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

    if (gpio_model->config.use_interrupt == Interrupt_Enable)
    {
        gpio_model->attach_interrupt(gpio_model);
    }
}

/*
 * write high/low level to pin
 */
void gpio_write_pin(GPIO_Model_t *gpio_model, GPIO_Pin_State_e stat)
{
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
    HAL_GPIO_TogglePin(gpio_model->GPIOx, gpio_model->Pinx);
}

/*
 * attach interrupt for gpio pin
 */
void gpio_attach_interrupt(GPIO_Model_t *gpio_model)
{
    /* Not Implemented */
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
