#pragma once

/*
 * ============================================================
 * bsp_i2c.h  —  I2C 硬件抽象层（纯接口）
 * ============================================================
 *
 * 本头文件不 include 任何芯片厂商的头文件。
 * 所有类型和枚举均为自有定义。
 *
 * 当前仅实现软件 I2C (bit-bang)，后续可扩展硬件 I2C。
 *
 * 使用示例：
 *
 *   I2C_Model_t eeprom;
 *   i2c_register(&eeprom, GPIOB, GPIO_PIN_6, GPIOB, GPIO_PIN_7);
 *   i2c_init(&eeprom, &(I2C_Config_t){ .sw = {
 *       .scl_delay_us = 5,
 *       .sda_pull     = GPIO_Pull_Up,
 *       .scl_pull     = GPIO_Pull_Up,
 *   }});
 *
 *   // 写寄存器地址后读数据（含 REPEATED START）
 *   uint8_t reg = 0x10, buf[4];
 *   i2c_write_read(&eeprom, 0x50, &reg, 1, buf, 4);
 */

#include "bsp_gpio.h"
#include "bsp_sys.h"

/* ============================================================
 * 枚举定义
 * ============================================================ */

typedef enum {
    I2C_Write = 0,
    I2C_Read  = 1,
} I2C_RW_t;

typedef enum {
    I2C_Err_OK           = 0,
    I2C_Err_NACK         = 1,
    I2C_Err_BusError     = 2,
    I2C_Err_Invalid_Mode = 3,
} I2C_Err_t;

/* ============================================================
 * 结构体定义
 * ============================================================ */

/* ---------- 软件 I2C 配置 ---------- */
typedef struct {
    uint16_t scl_delay_us; /**< SCL 半周期延时 (μs), 100kHz→5 */
    GPIO_Speed_t speed;    /**< SCL 和 SDA 的速度配置 */
    GPIO_Pull_t sda_pull;  /**< SDA 上拉配置 */
    GPIO_Pull_t scl_pull;  /**< SCL 上拉配置 */
} I2C_SW_Config_t;

/* ---------- 软件 I2C 模型（引脚） ---------- */
typedef struct {
    GPIO_Model_t sda;
    GPIO_Model_t scl;
} I2C_SW_Model_t;
/*
目前看来软件 I2C 的硬件模型可以和硬件 I2C 一致，本质都是引脚
但是配置的功能势必是不同的，硬件 I2C 要求必须使用 AF 功能
后面如果做硬件 I2C 的话再思考如何扩展
 */

/*
 * I2C_Config_t — union 形式，后续可扩展 hw 配置
 */
typedef union {
    I2C_SW_Config_t sw;
} I2C_Config_t;

/*
 * I2C_Model_t — 运行时模型
 */
typedef struct {
    union {
        I2C_SW_Model_t sw; /**< 软件 I2C: 两个 GPIO 引脚 */
        /* TODO: hw_i2c 外设 */
    } src;
    I2C_Config_t config; /**< 配置快照 */
    volatile uint8_t busy : 1;
} I2C_Model_t;

/* ============================================================
 * 外部函数（由芯片层实现）
 * ============================================================ */

I2C_Err_t i2c_register(I2C_Model_t *m, GPIO_Speed_t speed,
                       GPIO_Port_t sda_port, GPIO_Pin_t sda_pin,
                       GPIO_Port_t scl_port, GPIO_Pin_t scl_pin);

I2C_Err_t i2c_init(I2C_Model_t *m, const I2C_Config_t *cfg);
I2C_Err_t i2c_deinit(I2C_Model_t *m);

I2C_Err_t i2c_hw_write(I2C_Model_t *m, uint8_t dev_addr,
                       const uint8_t *data, uint16_t len);

I2C_Err_t i2c_hw_read(I2C_Model_t *m, uint8_t dev_addr,
                      uint8_t *buf, uint16_t len);

/* ============================================================
 * 外部函数，软件 I2C 可在 BSP 层直接实现
 * ============================================================ */

I2C_Err_t i2c_sw_write(I2C_Model_t *m, uint8_t dev_addr,
                       const uint8_t *data, uint16_t len);

I2C_Err_t i2c_sw_read(I2C_Model_t *m, uint8_t dev_addr,
                      uint8_t *buf, uint16_t len);

I2C_Err_t i2c_sw_write_read(I2C_Model_t *m, uint8_t dev_addr,
                            const uint8_t *tx_data, uint16_t tx_len,
                            uint8_t *rx_buf, uint16_t rx_len);

/* ============================================================
 * static inline 芯片级原语
 *
 * 直接操作 GPIO.
 * ============================================================ */

static inline void i2c_sw_sda_high(I2C_Model_t *m)
{
    (void)gpio_write(&m->src.sw.sda, GPIO_Level_High);
}

static inline void i2c_sw_sda_low(I2C_Model_t *m)
{
    (void)gpio_write(&m->src.sw.sda, GPIO_Level_Low);
}

static inline GPIO_Level_t i2c_sw_get_sda(I2C_Model_t *m)
{
    return gpio_read(&m->src.sw.sda);
}

static inline void i2c_sw_scl_high(I2C_Model_t *m)
{
    (void)gpio_write(&m->src.sw.scl, GPIO_Level_High);
}

static inline void i2c_sw_scl_low(I2C_Model_t *m)
{
    (void)gpio_write(&m->src.sw.scl, GPIO_Level_Low);
}

static inline void i2c_sw_start(I2C_Model_t *m)
{
    i2c_sw_sda_high(m);
    i2c_sw_scl_high(m);
    DelayUs(m->config.sw.scl_delay_us);
    i2c_sw_sda_low(m);
    DelayUs(m->config.sw.scl_delay_us);
    i2c_sw_scl_low(m);
}

static inline void i2c_sw_stop(I2C_Model_t *m)
{
    i2c_sw_sda_low(m);
    i2c_sw_scl_high(m);
    DelayUs(m->config.sw.scl_delay_us);
    i2c_sw_sda_high(m);
    DelayUs(m->config.sw.scl_delay_us);
}

/*
 * 发送一个字节，返回 ACK 状态。
 * 返回 0 = ACK，返回 1 = NACK。
 */
static inline uint8_t i2c_sw_send_byte(I2C_Model_t *m, uint8_t byte)
{
    uint32_t delay = m->config.sw.scl_delay_us;

    for (int i = 7; i >= 0; i--) {
        if (byte & (1 << i))
            i2c_sw_sda_high(m);
        else
            i2c_sw_sda_low(m);

        DelayUs(delay);
        i2c_sw_scl_high(m);
        DelayUs(delay);
        i2c_sw_scl_low(m);
    }

    /* 第 9 个时钟：释放 SDA，从机驱动 ACK */
    i2c_sw_sda_high(m);
    DelayUs(delay);
    i2c_sw_scl_high(m);
    DelayUs(delay);
    uint8_t nack = i2c_sw_get_sda(m);
    i2c_sw_scl_low(m);

    return nack; /* 0 = ACK (SDA low), 1 = NACK (SDA high) */
}

/*
 * 接收一个字节，ack 参数决定主机是否发送 ACK。
 * ack = 0: 主机发送 NACK（接收最后一个字节时使用）
 * ack = 1: 主机发送 ACK
 */
static inline uint8_t i2c_sw_recv_byte(I2C_Model_t *m, uint8_t ack)
{
    uint32_t delay = m->config.sw.scl_delay_us;
    uint8_t byte   = 0;

    i2c_sw_sda_high(m); /* 释放 SDA，由从机驱动 */

    for (int i = 7; i >= 0; i--) {
        DelayUs(delay);
        i2c_sw_scl_high(m);
        DelayUs(delay);

        if (i2c_sw_get_sda(m) == GPIO_Level_High)
            byte |= (1 << i);

        i2c_sw_scl_low(m);
    }

    /* 第 9 个时钟：主机发送 ACK/NACK */
    if (ack)
        i2c_sw_sda_low(m); /* ACK  */
    else
        i2c_sw_sda_high(m); /* NACK */

    DelayUs(delay);
    i2c_sw_scl_high(m);
    DelayUs(delay);
    i2c_sw_scl_low(m);

    i2c_sw_sda_high(m); /* 释放 SDA */

    return byte;
}

/*
 * 发送地址+读写位，返回 NACK 状态
 */
static inline uint8_t send_addr(I2C_Model_t *m, uint8_t dev_addr, I2C_RW_t rw)
{
    return i2c_sw_send_byte(m, (uint8_t)(dev_addr << 1) | (uint8_t)rw);
}

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
