/*
 * ============================================================
 * chip/stm32f4/spi_chip.c
 *
 * BSP SPI 抽象的 STM32F4 芯片实现。
 *
 * 软件 SPI (bit-bang) Mode 0：
 *   - CS   → Output_PP
 *   - SCLK → Output_PP
 *   - MOSI → Output_PP
 *   - MISO → Input
 *   使用 bsp_gpio API 控制引脚电平。
 * ============================================================
 */

#include "bsp_spi.h"

/* ============================================================
 * API 实现
 * ============================================================ */

SPI_Err_t spi_register(SPI_Model_t *m,
                        GPIO_Port_t cs_port,   GPIO_Pin_t cs_pin,
                        GPIO_Port_t sclk_port, GPIO_Pin_t sclk_pin,
                        GPIO_Port_t mosi_port, GPIO_Pin_t mosi_pin,
                        GPIO_Port_t miso_port, GPIO_Pin_t miso_pin)
{
    gpio_register(&m->src.sw.cs,   cs_port,   cs_pin);
    gpio_register(&m->src.sw.sclk, sclk_port, sclk_pin);
    gpio_register(&m->src.sw.mosi, mosi_port, mosi_pin);
    gpio_register(&m->src.sw.miso, miso_port, miso_pin);

    m->busy = 0;
    return SPI_Err_OK;
}

SPI_Err_t spi_init(SPI_Model_t *m, const SPI_Config_t *cfg)
{
    GPIO_Config_t out_cfg = {
        .mode      = GPIO_Mode_Output_PP,
        .pull      = GPIO_Pull_None,
        .speed     = GPIO_Speed_Low,
        .alternate = 0,
    };
    GPIO_Config_t in_cfg = {
        .mode      = GPIO_Mode_Input,
        .pull      = GPIO_Pull_None,
        .speed     = GPIO_Speed_Low,
        .alternate = 0,
    };

    gpio_init(&m->src.sw.cs,   &out_cfg);
    gpio_init(&m->src.sw.sclk, &out_cfg);
    gpio_init(&m->src.sw.mosi, &out_cfg);
    gpio_init(&m->src.sw.miso, &in_cfg);

    m->config = *cfg;
    m->busy   = 0;

    /* 默认状态：CS 取消选中，SCLK 空闲低 */
    spi_sw_cs_deselect(m);
    (void)gpio_write(&m->src.sw.sclk, GPIO_Level_Low);

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

SPI_Err_t spi_transmit_receive(SPI_Model_t *m,
                                const uint8_t *tx, uint8_t *rx, uint16_t len)
{
    if (len == 0)
        return SPI_Err_OK;

    m->busy = 1;
    spi_sw_cs_select(m);

    for (uint16_t i = 0; i < len; i++) {
        uint8_t tx_byte = (tx != NULL) ? tx[i] : 0xFF;
        uint8_t rx_byte = spi_sw_xfer_byte(m, tx_byte);
        if (rx != NULL)
            rx[i] = rx_byte;
    }

    spi_sw_cs_deselect(m);
    m->busy = 0;
    return SPI_Err_OK;
}

SPI_Err_t spi_transmit(SPI_Model_t *m, const uint8_t *tx, uint16_t len)
{
    return spi_transmit_receive(m, tx, NULL, len);
}

SPI_Err_t spi_receive(SPI_Model_t *m, uint8_t *rx, uint16_t len)
{
    return spi_transmit_receive(m, NULL, rx, len);
}
