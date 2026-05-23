#pragma once

#include "stm32f4xx_hal.h"

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
*/

#define PAGE_UP_BTN_PORT
#define PAGE_UP_BTN_PIN

#define PAGE_DOWN_BTN_PORT
#define PAGE_DOWN_BTN_PIN

#define BACK_BTN_PORT
#define BACK_BTN_PIN

#define CONFIRM_BTN_PORT
#define CONFIRM_BTN_PIN

#define HOME_BTN_PORT
#define HOME_BTN_PIN

/* ============= GPIO Configurations =============== */
