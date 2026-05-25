#pragma once

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"

/*
stores the basic GPIO operations, models
*/

#define MAX_GPIO_PIN_NUM 16U
#define MAX_GPIO_CALLBACK 16U

typedef GPIO_TypeDef *GPIOx_Type_t;
typedef uint16_t Pinx_Type_t;
typedef GPIO_PinState GPIO_Pin_State_e;
typedef void (*gpio_irq_callback_t)(void);

typedef struct {
    uint32_t mode;
    uint32_t pull;
    uint32_t speed;
    uint32_t alternate;
} GPIOx_Config_t;

typedef struct {
    /* 
    shouldn't assign value, 
    automatically calculate when using `gpio_model.attach_interrupt`
    */
    IRQn_Type __irqn;
    /*
    e.g. GPIO_MODE_IT_FALLING
    */
    uint32_t trigger_edge;
    uint32_t preempt_priority;
    uint32_t sub_priority;
    /*
     * should be as fast as it can,
     * recommend that only set the flag.
     */
    gpio_irq_callback_t irq_callback;
} GPIOx_IRQ_Config_t;

/*
 * GPIO_Model, providing the basic interface for upstream to
 * call nesseary GPIO operations but feel free to different hardwares.
 */
typedef struct gpio_model {
    GPIOx_Type_t GPIOx;
    Pinx_Type_t Pinx;
    /* stores the configuration for this GPIO */
    GPIOx_Config_t config;
    GPIOx_IRQ_Config_t irq_config;
    
    /* automatically assigned when call `attach/detach_interrupt` */
    uint8_t __use_interrupt: 1;
} GPIO_Model_t;

extern gpio_irq_callback_t gpio_exti_callback[MAX_GPIO_CALLBACK];

void gpio_register(GPIO_Model_t *gpio_model, GPIOx_Type_t GPIOx, Pinx_Type_t Pinx);
void gpio_init(GPIO_Model_t *gpio_model, GPIOx_Config_t *init_conf);
void gpio_deinit(GPIO_Model_t *gpio_model);
void gpio_write_pin(GPIO_Model_t *gpio_model, GPIO_Pin_State_e stat);
GPIO_Pin_State_e gpio_read_pin(GPIO_Model_t *gpio_model);
void gpio_toggle_pin(GPIO_Model_t *gpio_model);
void gpio_attach_interrupt(GPIO_Model_t *gpio_model, GPIOx_IRQ_Config_t *irq_conf);
void gpio_detach_interrupt(GPIO_Model_t *gpio_model);
int gpio_get_pin_num(Pinx_Type_t pin);
