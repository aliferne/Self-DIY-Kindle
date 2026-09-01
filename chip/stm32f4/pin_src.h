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

/* SD 卡检测 */
#define SDCARD_DET_PORT GPIOD
#define SDCARD_DET_PIN  GPIO_PIN_3

/*
 * TFT 屏驱动
 * SCK: PA5
 * MOSI: PA7
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
 * I2S 接口，用于 PCM5102A
 * PCM5102A 本身会借助 PLL 产生主时钟，因此此处不需要 MCK
 * 目前使用 I2S2 接口
 */

/* I2S SD */
#define PCM_DIN_PORT GPIOC
#define PCM_DIN_PIN  GPIO_PIN_3

/* I2S CK */
#define PCM_BCK_PORT GPIOB
#define PCM_BCK_PIN  GPIO_PIN_10

/* I2S WS */
#define PCM_LRCK_PORT GPIOB
#define PCM_LRCK_PIN  GPIO_PIN_12

/*
 * 触摸芯片
 */
#define TOUCH_SDA_PORT GPIOB
#define TOUCH_SDA_PIN  GPIO_PIN_12

#define TOUCH_SCL_PORT GPIOB
#define TOUCH_SCL_PIN  GPIO_PIN_11

#define TOUCH_INT_PORT GPIOB
#define TOUCH_INT_PIN  GPIO_PIN_13

#define TOUCH_RST_PORT GPIOB
#define TOUCH_RST_PIN  GPIO_PIN_10

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
