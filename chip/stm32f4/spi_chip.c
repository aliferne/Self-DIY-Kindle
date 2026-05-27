/*
 * ============================================================
 * chip/stm32f4/spi_chip.c
 *
 * BSP SPI 抽象的 STM32F4 芯片实现。
 *
 * 软件 SPI (bit-bang) 支持 Mode 0/1/2/3：
 *   - CS   → Output_PP
 *   - SCLK → Output_PP
 *   - MOSI → Output_PP
 *   - MISO → Input
 *   - SCLK 空闲电平由模式决定（Mode 0/1 低，Mode 2/3 高）
 *   使用 bsp_gpio API 控制引脚电平。
 * ============================================================
 */

#include "bsp_spi.h"

/* ============================================================
 * API 实现
 * ============================================================ */

SPI_Err_t spi_register(SPI_Model_t *m,
                       GPIO_Port_t cs_port, GPIO_Pin_t cs_pin,
                       GPIO_Port_t sclk_port, GPIO_Pin_t sclk_pin,
                       GPIO_Port_t mosi_port, GPIO_Pin_t mosi_pin,
                       GPIO_Port_t miso_port, GPIO_Pin_t miso_pin)
{
    gpio_register(&m->src.sw.cs, cs_port, cs_pin);
    gpio_register(&m->src.sw.sclk, sclk_port, sclk_pin);
    gpio_register(&m->src.sw.mosi, mosi_port, mosi_pin);
    gpio_register(&m->src.sw.miso, miso_port, miso_pin);

    m->busy            = 0;

    return SPI_Err_OK;
}

SPI_Err_t spi_init(SPI_Model_t *m, const SPI_Config_t *cfg)
{
    GPIO_Config_t gpio_cfg = {
        .mode      = GPIO_Mode_Output_PP,
        .pull      = GPIO_Pull_None,
        .speed     = cfg->sw.speed,
        .alternate = 0,
    };

    gpio_init(&m->src.sw.cs, &gpio_cfg);
    gpio_init(&m->src.sw.sclk, &gpio_cfg);
    gpio_init(&m->src.sw.mosi, &gpio_cfg);

    gpio_cfg.mode = GPIO_Mode_Input,
    gpio_init(&m->src.sw.miso, &gpio_cfg);

    m->config = *cfg;
    m->busy   = 0;

    /* ================================================================
     * 预计算：将 mode 解析为 CPOL/CPHA，推导 sclk_idle / sclk_active / cpha。
     * 枚举值与 (CPOL<<1)|CPHA 对齐，直接用位运算取出。
     * ================================================================ */
    {
        uint8_t cpol = SPI_CPOL(cfg->sw.mode);
        uint8_t cpha = SPI_CPHA(cfg->sw.mode);

        m->config.sw.sclk_idle   = cpol ? GPIO_Level_High : GPIO_Level_Low;
        m->config.sw.sclk_active = cpol ? GPIO_Level_Low : GPIO_Level_High;
        m->config.sw.cpha        = cpha;
    }

    /* 默认状态：CS 取消选中，SCLK 设置为空闲电平 */
    spi_sw_cs_deselect(m);
    (void)gpio_write(&m->src.sw.sclk, m->config.sw.sclk_idle);

    return SPI_Err_OK;
}

SPI_Err_t spi_deinit(SPI_Model_t *m)
{
    gpio_deinit(&m->src.sw.cs);
    gpio_deinit(&m->src.sw.sclk);
    gpio_deinit(&m->src.sw.mosi);
    gpio_deinit(&m->src.sw.miso);
    m->busy = 0;
    return SPI_Err_OK;
}
