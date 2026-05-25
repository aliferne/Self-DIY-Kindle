#pragma once

/*
 * ============================================================
 * bsp_spi.h  —  SPI 硬件抽象层（纯接口）
 * ============================================================
 *
 * 本头文件不 include 任何芯片厂商的头文件。
 * 所有类型和枚举均为自有定义。
 *
 * 当前仅实现软件 SPI (bit-bang) Mode 0，后续可扩展硬件 SPI 及其他模式。
 *
 * 使用示例：
 *
 *   SPI_Model_t display;
 *   spi_register(&display,
 *       GPIOA, GPIO_PIN_4,   // CS
 *       GPIOA, GPIO_PIN_5,   // SCLK
 *       GPIOA, GPIO_PIN_7,   // MOSI
 *       GPIOA, GPIO_PIN_6);  // MISO
 *   spi_init(&display, &(SPI_Config_t){ .sw = {
 *       .mode         = SPI_Mode_0,
 *       .bit_delay_us = 1,
 *   }});
 *
 *   uint8_t tx[2] = {0x90, 0x00}, rx[2];
 *   spi_transmit_receive(&display, tx, rx, 2);
 *   // 或仅发送：
 *   spi_transmit(&display, tx, 2);
 *   // 或仅接收：
 *   spi_receive(&display, rx, 2);
 */

#include "bsp_gpio.h"
#include "bsp_sys.h"

/* ============================================================
 * 枚举定义
 * ============================================================ */

typedef enum {
    SPI_Err_OK = 0,
    SPI_Err_IO = 1,
} SPI_Err_t;

typedef enum {
    SPI_Mode_0 = 0,  /**< CPOL=0, CPHA=0 */
    SPI_Mode_1 = 1,  /**< CPOL=0, CPHA=1 */
    SPI_Mode_2 = 2,  /**< CPOL=1, CPHA=0 */
    SPI_Mode_3 = 3,  /**< CPOL=1, CPHA=1 */
} SPI_Mode_t;

/* ============================================================
 * 结构体定义
 * ============================================================ */

/* ---------- 软件 SPI 配置 ---------- */
typedef struct {
    SPI_Mode_t mode;              /**< SPI 模式 0-3 */
    uint16_t   bit_delay_us;      /**< 半位周期延时 (μs) */
} SPI_SW_Config_t;

/* ---------- 软件 SPI 模型（引脚） ---------- */
typedef struct {
    GPIO_Model_t cs;
    GPIO_Model_t sclk;
    GPIO_Model_t mosi;
    GPIO_Model_t miso;
} SPI_SW_Model_t;

/*
 * SPI_Config_t — union 形式，后续可扩展 hw 配置
 */
typedef union {
    SPI_SW_Config_t sw;
} SPI_Config_t;

/*
 * SPI_Model_t — 运行时模型
 */
typedef struct {
    union {
        SPI_SW_Model_t sw;        /**< 软件 SPI: 四个 GPIO 引脚 */
        /* TODO: hw_spi 外设 */
    } src;
    SPI_Config_t      config;
    volatile uint8_t  busy : 1;
} SPI_Model_t;

/* ============================================================
 * static inline 芯片级原语
 *
 * 直接操作 GPIO，不依赖任何 HAL 寄存器，芯片无关。
 * 当前按 Mode 0 (CPOL=0, CPHA=0) 实现：
 *   - SCLK 空闲为低
 *   - MOSI 在 SCLK 上升沿之前准备好
 *   - MISO 在 SCLK 上升沿采样
 * ============================================================ */

static inline void spi_sw_cs_select(SPI_Model_t *m)
{
    (void)gpio_write(&m->src.sw.cs, GPIO_Level_Low);
}

static inline void spi_sw_cs_deselect(SPI_Model_t *m)
{
    (void)gpio_write(&m->src.sw.cs, GPIO_Level_High);
}

/*
 * 全双工收发一个字节：发送 tx_byte，返回 rx_byte。
 */
static inline uint8_t spi_sw_xfer_byte(SPI_Model_t *m, uint8_t tx_byte)
{
    uint8_t  rx_byte = 0;
    uint32_t delay   = m->config.sw.bit_delay_us;

    for (int i = 7; i >= 0; i--) {
        /* 设置 MOSI */
        if (tx_byte & (1 << i))
            (void)gpio_write(&m->src.sw.mosi, GPIO_Level_High);
        else
            (void)gpio_write(&m->src.sw.mosi, GPIO_Level_Low);

        DelayUs(delay);

        /* SCLK 上升沿 */
        (void)gpio_write(&m->src.sw.sclk, GPIO_Level_High);
        DelayUs(delay);

        /* 采样 MISO */
        if (gpio_read(&m->src.sw.miso) == GPIO_Level_High)
            rx_byte |= (1 << i);

        /* SCLK 下降沿 */
        (void)gpio_write(&m->src.sw.sclk, GPIO_Level_Low);
    }

    return rx_byte;
}

/* ============================================================
 * public API 声明（由芯片层实现）
 * ============================================================ */

SPI_Err_t spi_register(SPI_Model_t *m,
                        GPIO_Port_t cs_port,   GPIO_Pin_t cs_pin,
                        GPIO_Port_t sclk_port, GPIO_Pin_t sclk_pin,
                        GPIO_Port_t mosi_port, GPIO_Pin_t mosi_pin,
                        GPIO_Port_t miso_port, GPIO_Pin_t miso_pin);

SPI_Err_t spi_init(SPI_Model_t *m, const SPI_Config_t *cfg);
SPI_Err_t spi_deinit(SPI_Model_t *m);

SPI_Err_t spi_transmit_receive(SPI_Model_t *m,
                                const uint8_t *tx, uint8_t *rx, uint16_t len);

SPI_Err_t spi_transmit(SPI_Model_t *m, const uint8_t *tx, uint16_t len);
SPI_Err_t spi_receive(SPI_Model_t *m, uint8_t *rx, uint16_t len);
