#pragma once

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_gpio.h"
#include "spi.h"

/*
 * Pin source mapping for STM32F4xx series.
 */

/* ============================================================
 * 引脚定义格式：
 *   #define <功能名>_PORT   <GPIO 端口>
 *   #define <功能名>_PIN    <GPIO 引脚>
 *
 * 此外为了避免在 bsp 层造成芯片库相关内容的泄漏，
 * 应当在源文件中包含， 避免在头文件中包含
 * ============================================================ */

/*
 * 按键：
 *   - Page Up    (上翻页)  PE7
 *   - Page Down  (下翻页)  PE8
 *   - Back       (返回)    PE9
 *   - Confirm    (确认)    PE10
 *   - Home       (主页)    PE11
 *
 * LED：
 *   - User LED             PB2
 *
 * ESCREEN:
 *
 *
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

/*
 * TFT 屏驱动
 */
// #define TFT_SCK_PORT
// #define TFT_SCK_PIN

// #define TFT_MOSI_PORT
// #define TFT_MOSI_PIN
#define TFT_HSPI     (&hspi1)

#define TFT_CS_PORT  GPIOA
#define TFT_CS_PIN   GPIO_PIN_2

#define TFT_DC_PORT  GPIOA
#define TFT_DC_PIN   GPIO_PIN_1

#define TFT_RST_PORT GPIOA
#define TFT_RST_PIN  GPIO_PIN_0

#define TFT_BLK_PORT GPIOA
#define TFT_BLK_PIN  GPIO_PIN_3

/*
 * 墨水屏 (4.2inch e-Paper V2)：
 *
 * 目前不再使用
 */
#define EPAPER_SCK_PORT  GPIOE
#define EPAPER_SCK_PIN   GPIO_PIN_7

#define EPAPER_MOSI_PORT GPIOE
#define EPAPER_MOSI_PIN  GPIO_PIN_9

#define EPAPER_MISO_PORT GPIOE
#define EPAPER_MISO_PIN  GPIO_PIN_9

#define EPAPER_CS_PORT   GPIOE
#define EPAPER_CS_PIN    GPIO_PIN_15

#define EPAPER_DC_PORT   GPIOE
#define EPAPER_DC_PIN    GPIO_PIN_13

#define EPAPER_RST_PORT  GPIOE
#define EPAPER_RST_PIN   GPIO_PIN_11

#define EPAPER_BUSY_PORT GPIOE
#define EPAPER_BUSY_PIN  GPIO_PIN_14

/*
 * 触摸屏 (FT6336 电容触摸 I2C)
 * TODO: 根据实际硬件接线填充引脚
 */
#define TOUCH_SDA_PORT
#define TOUCH_SDA_PIN
#define TOUCH_SCL_PORT
#define TOUCH_SCL_PIN
#define TOUCH_INT_PORT
#define TOUCH_INT_PIN
#define TOUCH_RST_PORT
#define TOUCH_RST_PIN
