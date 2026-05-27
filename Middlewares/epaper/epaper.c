/*
 * ============================================================
 * Middlewares/epaper/epaper_hw.c
 *
 * 墨水屏硬件模型实现。
 *
 * 职责：
 *   1. 注册 SPI + GPIO 引脚到 EPaper_Model_t
 *   2. 初始化各 GPIO 方向和 SPI 配置
 *   3. 提供 epaper 专用的 SPI 读写原语（薄封装 bsp_spi/bsp_gpio）
 * ============================================================
 */

#include "epaper.h"
#include "DEV_Config.h"
#include "bsp_gpio.h"

EPaper_Model_t e_paper;

/* ============================================================
 * 注册
 * ============================================================ */

void epaper_hw_register(
    EPaper_Model_t *m,
    GPIO_Port_t sck_port, GPIO_Pin_t sck_pin,
    GPIO_Port_t mosi_port, GPIO_Pin_t mosi_pin,
    GPIO_Port_t miso_port, GPIO_Pin_t miso_pin,
    GPIO_Port_t cs_port, GPIO_Pin_t cs_pin,
    GPIO_Port_t dc_port, GPIO_Pin_t dc_pin,
    GPIO_Port_t rst_port, GPIO_Pin_t rst_pin,
    GPIO_Port_t busy_port, GPIO_Pin_t busy_pin)
{
    /* SPI 四线 */
    spi_register(&m->spi,
                 cs_port, cs_pin,
                 sck_port, sck_pin,
                 mosi_port, mosi_pin,
                 miso_port, miso_pin);

    /* 独立的 GPIO */
    gpio_register(&m->dc, dc_port, dc_pin);
    gpio_register(&m->rst, rst_port, rst_pin);
    gpio_register(&m->busy, busy_port, busy_pin);
}

/* ============================================================
 * 初始化
 * ============================================================ */

EPaper_Err_t epaper_hw_init(EPaper_Model_t *m)
{
    SPI_Err_t spi_err;

    /* ---- SPI ---- */
    SPI_Config_t spi_cfg = {
        .sw = {
            .mode         = SPI_Mode_0,
            .speed        = GPIO_Speed_Low,
            .bit_delay_us = 1,
            /* sclk_idle/active/cpha 由 spi_init 预计算 */
        },
    };
    spi_err = spi_init(&m->spi, &spi_cfg);
    if (spi_err != SPI_Err_OK)
        return EPaper_Err_IO;

    /* ---- DC (输出) ---- */
    {
        GPIO_Config_t out_cfg = {
            .mode  = GPIO_Mode_Output_PP,
            .pull  = GPIO_Pull_None,
            .speed = GPIO_Speed_Low,
        };
        gpio_init(&m->dc, &out_cfg);
    }

    /* ---- RST (输出) ---- */
    {
        GPIO_Config_t out_cfg = {
            .mode  = GPIO_Mode_Output_PP,
            .pull  = GPIO_Pull_None,
            .speed = GPIO_Speed_Low,
        };
        gpio_init(&m->rst, &out_cfg);
    }

    /* ---- BUSY (输入) ---- */
    {
        GPIO_Config_t in_cfg = {
            .mode  = GPIO_Mode_Input,
            .pull  = GPIO_Pull_None,
            .speed = GPIO_Speed_Low,
        };
        gpio_init(&m->busy, &in_cfg);
    }

    /* 默认电平：CS 高（取消选中），DC 低，RST 高 */
    spi_sw_cs_deselect(&m->spi);
    gpio_write(&m->dc, GPIO_Level_Low);
    gpio_write(&m->rst, GPIO_Level_High);

    m->hw_initialized = 1;
    return EPaper_Err_OK;
}

/* ============================================================
 * 去初始化 ( 参考 `DEV_Module_Exit` )
 * ============================================================ */

void epaper_hw_deinit(EPaper_Model_t *m)
{
    gpio_write(&m->dc, GPIO_Level_Low);
    gpio_write(&m->spi.src.sw.cs, GPIO_Level_Low);
    gpio_write(&m->rst, GPIO_Level_Low);

    DEV_Delay_ms(10);

    spi_deinit(&m->spi);
    gpio_deinit(&m->dc);
    gpio_deinit(&m->rst);
    gpio_deinit(&m->busy);
    m->hw_initialized = 0;
}

/* ============================================================
 * epaper 专用 SPI 原语（薄封装 bsp_spi）
 *
 * 电子纸的 SPI 协议特点：
 *   - 只发不收（除极少数情况需回读）
 *   - CS 在命令+数据之间保持低电平（由上层手动控制 CS）
 *   - DC 线由上层在 SendCommand/SendData 中切换
 * ============================================================ */
