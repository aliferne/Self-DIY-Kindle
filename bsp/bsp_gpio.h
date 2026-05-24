#pragma once

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"

/*
stores the basic GPIO operations, models
*/

typedef GPIO_TypeDef *GPIOx_Type_t;
typedef uint16_t Pinx_Type_t;
typedef GPIO_PinState GPIO_Pin_State_e;
typedef void (*gpio_irq_callback_t)(void);

typedef enum {
    Interrupt_Disable = 0U,
    Interrupt_Enable  = 1U,
} GPIO_Interrupt_State_e;

typedef struct {
    uint32_t mode;
    uint32_t pull;
    uint32_t speed;
    uint32_t alternate;
    GPIO_Interrupt_State_e use_interrupt;
    gpio_irq_callback_t *irq_callback;
} GPIOx_Config_t;

/*
 * GPIO_Model, providing the basic interface for upstream to
 * call nesseary GPIO operations but feel free to different hardwares.
 */
typedef struct gpio_model {
    GPIOx_Type_t GPIOx;
    Pinx_Type_t Pinx;
    /* stores the configuration for this GPIO */
    GPIOx_Config_t config;

    void (*init)(struct gpio_model *self, GPIOx_Config_t *init_conf);
    void (*write_pin)(struct gpio_model *self, GPIO_Pin_State_e stat);
    GPIO_Pin_State_e (*read_pin)(struct gpio_model *self);
    void (*toggle_pin)(struct gpio_model *self);
    void (*attach_interrupt)(struct gpio_model *self);
} GPIO_Model_t;

void gpio_register(GPIO_Model_t *gpio_model, GPIOx_Type_t gpiox, Pinx_Type_t pinx);
void gpio_init(GPIO_Model_t *gpio_model, GPIOx_Config_t *init_conf);
void gpio_write_pin(GPIO_Model_t *gpio_model, GPIO_Pin_State_e stat);
GPIO_Pin_State_e gpio_read_pin(GPIO_Model_t *gpio_model);
void gpio_toggle_pin(GPIO_Model_t *gpio_model);
void gpio_attach_interrupt(GPIO_Model_t *gpio_model);
