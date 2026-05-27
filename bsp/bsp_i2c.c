#include "bsp_i2c.h"
#include "bsp_sys.h"

/* I2C 延时函数 */
#define I2C_DELAY(delay) dwt_delay_us(delay)

/* ============================================================
 * 外部函数，软件 I2C 可在 BSP 层直接实现
 * ============================================================ */

I2C_Err_t i2c_sw_write(I2C_Model_t *m, uint8_t dev_addr,
                       const uint8_t *data, uint16_t len)
{
    m->busy = 1;

    i2c_sw_start(m);
    if (send_addr(m, dev_addr, I2C_Write)) {
        i2c_sw_stop(m);
        m->busy = 0;
        return I2C_Err_NACK;
    }

    for (uint16_t i = 0; i < len; i++) {
        if (i2c_sw_send_byte(m, data[i])) {
            i2c_sw_stop(m);
            m->busy = 0;
            return I2C_Err_NACK;
        }
    }

    i2c_sw_stop(m);
    m->busy = 0;
    return I2C_Err_OK;
}

I2C_Err_t i2c_sw_read(I2C_Model_t *m, uint8_t dev_addr,
                      uint8_t *buf, uint16_t len)
{
    if (len == 0)
        return I2C_Err_OK;

    m->busy = 1;

    i2c_sw_start(m);
    if (send_addr(m, dev_addr, I2C_Read)) {
        i2c_sw_stop(m);
        m->busy = 0;
        return I2C_Err_NACK;
    }

    for (uint16_t i = 0; i < len; i++) {
        /* 最后一个字节发 NACK，之前的发 ACK */
        uint8_t ack = (i < len - 1) ? 1 : 0;
        buf[i]      = i2c_sw_recv_byte(m, ack);
    }

    i2c_sw_stop(m);
    m->busy = 0;
    return I2C_Err_OK;
}

/*
 * 混合模式：write + REPEATED START → read
 *
 * 时序：
 *   S  [dev_addr + W]  ACK  [tx_data...]  ACK  Sr  [dev_addr + R]  ACK  [rx_data...]  NACK  P
 *
 * 典型场景：先写寄存器地址，后读数据。
 */
I2C_Err_t i2c_sw_write_read(I2C_Model_t *m, uint8_t dev_addr,
                            const uint8_t *tx_data, uint16_t tx_len,
                            uint8_t *rx_buf, uint16_t rx_len)
{
    if (rx_len == 0)
        return I2C_Err_OK;

    m->busy = 1;

    /* --- 写阶段 --- */
    i2c_sw_start(m);
    if (send_addr(m, dev_addr, I2C_Write)) {
        i2c_sw_stop(m);
        m->busy = 0;
        return I2C_Err_NACK;
    }

    for (uint16_t i = 0; i < tx_len; i++) {
        if (i2c_sw_send_byte(m, tx_data[i])) {
            i2c_sw_stop(m);
            m->busy = 0;
            return I2C_Err_NACK;
        }
    }

    /* --- REPEATED START（不发送 STOP） --- */
    i2c_sw_start(m); /* 重复 START */

    /* --- 读阶段 --- */
    if (send_addr(m, dev_addr, I2C_Read)) {
        i2c_sw_stop(m);
        m->busy = 0;
        return I2C_Err_NACK;
    }

    for (uint16_t i = 0; i < rx_len; i++) {
        uint8_t ack = (i < rx_len - 1) ? 1 : 0;
        rx_buf[i]   = i2c_sw_recv_byte(m, ack);
    }

    i2c_sw_stop(m);
    m->busy = 0;
    return I2C_Err_OK;
}

/* ============================================================
 * 芯片级原语
 *
 * 直接操作 GPIO.
 * ============================================================ */

void i2c_sw_sda_high(I2C_Model_t *m)
{
    (void)gpio_write(&m->src.sw.sda, GPIO_Level_High);
}

void i2c_sw_sda_low(I2C_Model_t *m)
{
    (void)gpio_write(&m->src.sw.sda, GPIO_Level_Low);
}

GPIO_Level_t i2c_sw_get_sda(I2C_Model_t *m)
{
    return gpio_read(&m->src.sw.sda);
}

void i2c_sw_scl_high(I2C_Model_t *m)
{
    (void)gpio_write(&m->src.sw.scl, GPIO_Level_High);
}

void i2c_sw_scl_low(I2C_Model_t *m)
{
    (void)gpio_write(&m->src.sw.scl, GPIO_Level_Low);
}

void i2c_sw_start(I2C_Model_t *m)
{
    i2c_sw_sda_high(m);
    i2c_sw_scl_high(m);
    I2C_DELAY(m->config.sw.scl_delay_us);
    i2c_sw_sda_low(m);
    I2C_DELAY(m->config.sw.scl_delay_us);
    i2c_sw_scl_low(m);
}

void i2c_sw_stop(I2C_Model_t *m)
{
    i2c_sw_sda_low(m);
    i2c_sw_scl_high(m);
    I2C_DELAY(m->config.sw.scl_delay_us);
    i2c_sw_sda_high(m);
    I2C_DELAY(m->config.sw.scl_delay_us);
}

/*
 * 发送一个字节，返回 ACK 状态。
 * 返回 0 = ACK，返回 1 = NACK。
 */
uint8_t i2c_sw_send_byte(I2C_Model_t *m, uint8_t byte)
{
    uint32_t delay = m->config.sw.scl_delay_us;

    for (int i = 7; i >= 0; i--) {
        if (byte & (1 << i))
            i2c_sw_sda_high(m);
        else
            i2c_sw_sda_low(m);

        I2C_DELAY(delay);
        i2c_sw_scl_high(m);
        I2C_DELAY(delay);
        i2c_sw_scl_low(m);
    }

    /* 第 9 个时钟：释放 SDA，从机驱动 ACK */
    i2c_sw_sda_high(m);
    I2C_DELAY(delay);
    i2c_sw_scl_high(m);
    I2C_DELAY(delay);
    uint8_t nack = i2c_sw_get_sda(m);
    i2c_sw_scl_low(m);

    return nack; /* 0 = ACK (SDA low), 1 = NACK (SDA high) */
}

/*
 * 接收一个字节，ack 参数决定主机是否发送 ACK。
 * ack = 0: 主机发送 NACK（接收最后一个字节时使用）
 * ack = 1: 主机发送 ACK
 */
uint8_t i2c_sw_recv_byte(I2C_Model_t *m, uint8_t ack)
{
    uint32_t delay = m->config.sw.scl_delay_us;
    uint8_t byte   = 0;

    i2c_sw_sda_high(m); /* 释放 SDA，由从机驱动 */

    for (int i = 7; i >= 0; i--) {
        I2C_DELAY(delay);
        i2c_sw_scl_high(m);
        I2C_DELAY(delay);

        if (i2c_sw_get_sda(m) == GPIO_Level_High)
            byte |= (1 << i);

        i2c_sw_scl_low(m);
    }

    /* 第 9 个时钟：主机发送 ACK/NACK */
    if (ack)
        i2c_sw_sda_low(m); /* ACK  */
    else
        i2c_sw_sda_high(m); /* NACK */

    I2C_DELAY(delay);
    i2c_sw_scl_high(m);
    I2C_DELAY(delay);
    i2c_sw_scl_low(m);

    i2c_sw_sda_high(m); /* 释放 SDA */

    return byte;
}

/*
 * 发送地址+读写位，返回 NACK 状态
 */
uint8_t send_addr(I2C_Model_t *m, uint8_t dev_addr, I2C_RW_t rw)
{
    return i2c_sw_send_byte(m, (uint8_t)(dev_addr << 1) | (uint8_t)rw);
}
