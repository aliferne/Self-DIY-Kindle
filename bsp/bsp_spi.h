#pragma once

/*
 * ============================================================
 * bsp_spi.h  —  SPI 硬件抽象层（纯接口）
 * ============================================================
 *
 * 本头文件不 include 任何芯片厂商的头文件。
 * 所有类型和枚举均为自有定义。
 *
 * 芯片相关实现位于 chip/<vendor>/SPI_chip.c，
 *
 * 使用示例：
 * TODO:
 */

/*
 * 实际上需要区分软件 SPI 和硬件 SPI，简便起见这里就只使用软件 SPI
 */

#include "bsp_gpio.h"

typedef enum {
    SPI_Err_OK = 0,
} SPI_Err_t;

typedef enum {
    SPI_Mode_0 = 0,
    SPI_Mode_1 = 1,
    SPI_Mode_2 = 2,
    SPI_Mode_3 = 3,
} SPI_Mode_t;

typedef struct {
    union {
        GPIO_Config_t sw_spi;
        /* hw_SPI */
    } cfg;
} SPI_Config_t;

/*
 * hardware source of SPI
 */
typedef struct {
    GPIO_HWSource_t cs;
    GPIO_HWSource_t sclk;
    GPIO_HWSource_t mosi;
    GPIO_HWSource_t miso;
} SPI_HWSource_t;

typedef struct {
    SPI_HWSource_t src;
    /* TODO: */
} SPI_Model_t;

SPI_Err_t spi_register(
    SPI_Model_t *m /* TODO: */);
SPI_Err_t spi_init(SPI_Model_t *m, SPI_Config_t *cfg);
SPI_Err_t spi_deinit(SPI_Model_t *m);

SPI_Err_t spi_transmit(SPI_Model_t *m);
SPI_Err_t spi_receive(SPI_Model_t *m);

/* TODO: */
