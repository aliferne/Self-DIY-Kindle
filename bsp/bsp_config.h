#pragma once

/* ============================================================
 * 板级配置文件和资源文件
 *
 * 放置各种外设的全局结构体供上层进行操作，
 * 本身并不包含任何厂商的库，以避免在上层中暴露底层 API，
 * 具体的芯片引脚应当由 chip 层自行实现 pin_src.h 文件，
 * 并在 bsp_src.c 中包含该头文件
 * ============================================================ */

#include "bsp_gpio.h"
#include "bsp_sdio.h"

/* ============================================================
 * 全局 GPIO 模型声明
 * ============================================================ */

extern GPIO_Model_t usr_led;
extern GPIO_Model_t pgup_btn;
extern GPIO_Model_t pgdown_btn;
extern GPIO_Model_t back_btn;
extern GPIO_Model_t home_btn;
extern GPIO_Model_t confirm_btn;

extern SDIO_Model_t storage;

/* ============================================================
 * 初始化入口
 * ============================================================ */

void bsp_init_hardware(void);
