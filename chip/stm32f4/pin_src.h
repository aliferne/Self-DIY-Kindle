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
 * 墨水屏 (4.2inch e-Paper V2)：
 *   - SCK 
 *   - MOSI 
 *   - MISO →  复用 MOSI 引脚（电子纸无需回读）
 *   - CS
 *   - DC
 *   - RST
 *   - BUSY
 *
 * 以下为占位符，请在确定实际接线后修改。
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
