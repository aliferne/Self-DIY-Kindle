/*
 * ============================================================
 * chip/stm32f4/i2c_chip.c
 *
 * BSP I2C 抽象的 STM32F4 芯片实现。
 *
 * 软件 I2C (bit-bang)：
 *   - SDA/SCL 固定为 Output_OD，强制开漏
 *   - 使用 bsp_gpio API 控制引脚电平
 *   - 使用 DelayUs 控制时序
 * ============================================================
 */

#include "bsp_gpio.h"
#include "bsp_i2c.h"

/* ============================================================
 * API 实现
 * ============================================================ */

I2C_Err_t i2c_register(I2C_Model_t *m, GPIO_Speed_t speed,
                       GPIO_Port_t sda_port, GPIO_Pin_t sda_pin,
                       GPIO_Port_t scl_port, GPIO_Pin_t scl_pin)
{
    gpio_register(&m->src.sw.sda, sda_port, sda_pin);
    gpio_register(&m->src.sw.scl, scl_port, scl_pin);

    m->config.sw.speed = speed;
    m->busy = 0;

    return I2C_Err_OK;
}

I2C_Err_t i2c_init(I2C_Model_t *m, const I2C_Config_t *cfg)
{
    /*
     * 软件 I2C 强制使用 Output_OD（开漏），
     * 由外部上拉电阻或内部上拉提供高电平。
     * 用户不能通过 GPIO_Config_t 选择 PP 模式，
     * 这是硬件协议约束。
     */
    GPIO_Config_t pin_cfg = {
        .mode      = GPIO_Mode_Output_OD,
        .pull      = cfg->sw.sda_pull,
        .speed     = cfg->sw.speed,
        .alternate = 0,
    };

    /* 要么内部上拉，要么外部上拉，不允许内部下拉 */
    if (cfg->sw.sda_pull == GPIO_Pull_Down)
        return I2C_Err_Invalid_Mode;
    if (cfg->sw.scl_pull == GPIO_Pull_Down)
        return I2C_Err_Invalid_Mode;

    gpio_init(&m->src.sw.sda, &pin_cfg);

    pin_cfg.pull = cfg->sw.scl_pull;
    gpio_init(&m->src.sw.scl, &pin_cfg);

    m->config = *cfg;
    m->busy   = 0;

    /* 释放总线：SCL 和 SDA 都回到高电平 */
    i2c_sw_sda_high(m);
    i2c_sw_scl_high(m);

    return I2C_Err_OK;
}

I2C_Err_t i2c_deinit(I2C_Model_t *m)
{
    gpio_deinit(&m->src.sw.sda);
    gpio_deinit(&m->src.sw.scl);
    m->busy = 0;
    return I2C_Err_OK;
}

I2C_Err_t i2c_write(I2C_Model_t *m, uint8_t dev_addr,
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

I2C_Err_t i2c_read(I2C_Model_t *m, uint8_t dev_addr,
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

I2C_Err_t i2c_write_read(I2C_Model_t *m, uint8_t dev_addr,
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
