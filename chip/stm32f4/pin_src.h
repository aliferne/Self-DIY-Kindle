#pragma once

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"

/*
 * Pin source mapping for STM32F4xx series.
 */

/* ============================================================
 * 引脚定义格式：
 *   #define <功能名>_PORT   <GPIO 端口>
 *   #define <功能名>_PIN    <GPIO 引脚>
 * ============================================================ */

/*
 * 按键：
 *   - Page Up    (上翻页)
 *   - Page Down  (下翻页)
 *   - Back       (返回)
 *   - Confirm    (确认)
 *   - Home       (主页)
 * LED：
 *   - User LED
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

#define I2C_SCL_PORT       GPIOA
#define I2C_SCL_PIN        GPIO_PIN_4

#define I2C_SDA_PORT       GPIOA
#define I2C_SDA_PIN        GPIO_PIN_3

