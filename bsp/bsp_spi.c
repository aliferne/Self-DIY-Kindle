#include "bsp_spi.h"
#include "bsp_sys.h"

/* SPI 延时函数 */
#define SPI_DELAY(delay) dwt_delay_us(delay)

/* ============================================================
 * 外部函数，软件 SPI 可在 BSP 层直接实现
 * ============================================================ */

SPI_Err_t spi_sw_write_read(SPI_Model_t *m,
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

SPI_Err_t spi_sw_write(SPI_Model_t *m, const uint8_t *tx, uint16_t len)
{
    return spi_sw_write_read(m, tx, NULL, len);
}

SPI_Err_t spi_sw_read(SPI_Model_t *m, uint8_t *rx, uint16_t len)
{
    return spi_sw_write_read(m, NULL, rx, len);
}

/* ============================================================
 * 芯片级原语
 *
 * 直接操作 GPIO。
 *
 * spi_init() 已将 CPOL/CPHA 预计算为 sclk_idle / sclk_active / cpha，
 * 下面的函数直接使用这些预计算值，运行时不判断模式。
 * ============================================================ */

void spi_sw_cs_select(SPI_Model_t *m)
{
    (void)gpio_write(&m->src.sw.cs, GPIO_Level_Low);
}

void spi_sw_cs_deselect(SPI_Model_t *m)
{
    (void)gpio_write(&m->src.sw.cs, GPIO_Level_High);
}

/**
 * CPHA=0: 数据在第一个沿之前准备好，第一个沿采样
 *
 * 时序：设 MOSI → delay → idle→active(采样 MISO) → delay → active→idle
 */
uint8_t spi_sw_xfer_byte_cpha0(
    SPI_Model_t *m,
    uint8_t tx_byte,
    uint32_t delay,
    GPIO_Level_t idle,
    GPIO_Level_t active)
{
    uint8_t rx_byte = 0;

    for (int i = 7; i >= 0; i--) {
        /* 在时钟空闲期准备好 MOSI */
        /* 保证 MOSI 一定为输出模式 */
        gpio_init(&m->src.sw.mosi, &m->src.sw.mosi.config);
        if (tx_byte & (1 << i))
            (void)gpio_write(&m->src.sw.mosi, GPIO_Level_High);
        else
            (void)gpio_write(&m->src.sw.mosi, GPIO_Level_Low);

        SPI_DELAY(delay);

        /* 采样沿 (idle → active) + 采样 MISO */
        (void)gpio_write(&m->src.sw.sclk, active);
        SPI_DELAY(delay);
        /* 保证 MISO 一定为输入模式 */
        gpio_init(&m->src.sw.miso, &m->src.sw.miso.config);
        if (gpio_read(&m->src.sw.miso) == GPIO_Level_High)
            rx_byte |= (1 << i);

        /* 回到空闲 (active → idle) */
        (void)gpio_write(&m->src.sw.sclk, idle);
    }

    return rx_byte;
}

/**
 * CPHA=1: 第一个沿改变数据，第二个沿采样
 *
 * 时序：设 MOSI + idle→active → delay → active→idle(采样 MISO) → delay
 */
uint8_t spi_sw_xfer_byte_cpha1(
    SPI_Model_t *m,
    uint8_t tx_byte,
    uint32_t delay,
    GPIO_Level_t idle,
    GPIO_Level_t active)
{
    uint8_t rx_byte = 0;

    for (int i = 7; i >= 0; i--) {
        /* 第一个沿 (idle → active)：同时改变 MOSI */
        /* 保证 MOSI 一定为输出模式 */
        gpio_init(&m->src.sw.mosi, &m->src.sw.mosi.config);
        if (tx_byte & (1 << i))
            (void)gpio_write(&m->src.sw.mosi, GPIO_Level_High);
        else
            (void)gpio_write(&m->src.sw.mosi, GPIO_Level_Low);

        (void)gpio_write(&m->src.sw.sclk, active);
        SPI_DELAY(delay);

        /* 第二个沿 (active → idle)：采样 MISO */
        (void)gpio_write(&m->src.sw.sclk, idle);
        /* 保证 MISO 一定为输入模式 */
        gpio_init(&m->src.sw.miso, &m->src.sw.miso.config);
        SPI_DELAY(delay);
        if (gpio_read(&m->src.sw.miso) == GPIO_Level_High)
            rx_byte |= (1 << i);
    }

    return rx_byte;
}

/**
 * 全双工收发一个字节。
 *
 * 根据预计算的 cpha 选择时序路径，idle/active 电平也已在 spi_init 中确定。
 * 运行时每个 bit 内部无分支（除 MOSI 设置和 MISO 采样外）。
 */
uint8_t spi_sw_xfer_byte(SPI_Model_t *m, uint8_t tx_byte)
{
    uint32_t delay      = m->config.sw.bit_delay_us;
    GPIO_Level_t idle   = m->config.sw.sclk_idle;
    GPIO_Level_t active = m->config.sw.sclk_active;

    if (m->config.sw.cpha) {
        return spi_sw_xfer_byte_cpha1(m, tx_byte, delay, idle, active);
    } else {
        return spi_sw_xfer_byte_cpha0(m, tx_byte, delay, idle, active);
    }
}
