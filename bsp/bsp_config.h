#pragma once

#include "bsp_gpio.h"

/*
TODO: hardware configurations

pin definition format:
    #define <FUNC_PORT> <GPIO_PORT_NAME>
    #define <FUNC_PIN> <GPIO_PIN_NAME>
*/

/* ============= Template =============== */

/* ============= GPIO Configurations =============== */

/*
Buttons:
    - Page Up
    - Page Down
    - Back
    - Confirm
    - Home

LEDs:
    - User LED
*/

#define PAGEUP_BTN_PORT   GPIOE
#define PAGEUP_BTN_PIN    GPIO_PIN_7

#define PAGEDOWN_BTN_PORT GPIOE
#define PAGEDOWN_BTN_PIN  GPIO_PIN_8

#define BACK_BTN_PORT     GPIOE
#define BACK_BTN_PIN      GPIO_PIN_9

#define CONFIRM_BTN_PORT  GPIOE
#define CONFIRM_BTN_PIN   GPIO_PIN_10

#define HOME_BTN_PORT     GPIOE
#define HOME_BTN_PIN      GPIO_PIN_11

#define USER_LED_PORT     GPIOB
#define USER_LED_PIN      GPIO_PIN_2

extern GPIO_Model_t usr_led;
extern GPIO_Model_t pgup_btn;
extern GPIO_Model_t pgdown_btn;
extern GPIO_Model_t back_btn;
extern GPIO_Model_t home_btn;
extern GPIO_Model_t confirm_btn;
/* ============= GPIO Configurations =============== */

void bsp_init_hardware();
