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
#include "bsp_i2c.h"
#include "bsp_spi.h"

extern gpio_t usr_led;
extern gpio_t pgup_btn;
extern gpio_t pgdown_btn;
extern gpio_t back_btn;
extern gpio_t home_btn;
extern gpio_t confirm_btn;

extern sdio_t storage;

extern spi_t tft_spi;
extern gpio_t tft_dc;
extern gpio_t tft_rst;
extern gpio_t tft_blk;

extern gpio_t tp_int;
extern gpio_t tp_rst;
extern iic_t tp_i2c;

void bsp_init_hardware(void);

/*
 * FatFs 初始化设备的接口
 */

sdio_err_t bsp_init_storage(void);
