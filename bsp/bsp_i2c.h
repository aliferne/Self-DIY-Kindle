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
    I2C_Err_OK       = 0,
    I2C_Err_NACK     = 1,
    I2C_Err_BusError = 2,
} I2C_Err_t;

/* ============================================================
 * 结构体定义
 * ============================================================ */

/* ---------- 软件 I2C 配置 ---------- */
typedef struct {
    uint16_t    scl_delay_us;     /**< SCL 半周期延时 (μs), 100kHz→5 */
    GPIO_Pull_t sda_pull;         /**< SDA 上拉配置 */
    GPIO_Pull_t scl_pull;         /**< SCL 上拉配置 */
} I2C_SW_Config_t;

/* ---------- 软件 I2C 模型（引脚） ---------- */
typedef struct {
    GPIO_Model_t sda;
    GPIO_Model_t scl;
} I2C_SW_Model_t;

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
        I2C_SW_Model_t sw;        /**< 软件 I2C: 两个 GPIO 引脚 */
        /* TODO: hw_i2c 外设 */
    } src;
    I2C_Config_t      config;     /**< 配置快照 */
    volatile uint8_t  busy : 1;
} I2C_Model_t;

/* ============================================================
 * static inline 芯片级原语
 *
 * 直接操作 GPIO，不依赖任何 HAL 寄存器，芯片无关。
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

    return nack;  /* 0 = ACK (SDA low), 1 = NACK (SDA high) */
}

/*
 * 接收一个字节，ack 参数决定主机是否发送 ACK。
 * ack = 0: 主机发送 NACK（接收最后一个字节时使用）
 * ack = 1: 主机发送 ACK
 */
static inline uint8_t i2c_sw_recv_byte(I2C_Model_t *m, uint8_t ack)
{
    uint32_t delay = m->config.sw.scl_delay_us;
    uint8_t  byte = 0;

    i2c_sw_sda_high(m);  /* 释放 SDA，由从机驱动 */

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
        i2c_sw_sda_low(m);   /* ACK  */
    else
        i2c_sw_sda_high(m);  /* NACK */

    DelayUs(delay);
    i2c_sw_scl_high(m);
    DelayUs(delay);
    i2c_sw_scl_low(m);

    i2c_sw_sda_high(m);  /* 释放 SDA */

    return byte;
}

/* ============================================================
 * public API 声明（由芯片层实现）
 * ============================================================ */

I2C_Err_t i2c_register(I2C_Model_t *m,
                        GPIO_Port_t sda_port, GPIO_Pin_t sda_pin,
                        GPIO_Port_t scl_port, GPIO_Pin_t scl_pin);

I2C_Err_t i2c_init(I2C_Model_t *m, const I2C_Config_t *cfg);
I2C_Err_t i2c_deinit(I2C_Model_t *m);

I2C_Err_t i2c_transmit(I2C_Model_t *m, uint8_t dev_addr,
                        const uint8_t *data, uint16_t len);

I2C_Err_t i2c_receive(I2C_Model_t *m, uint8_t dev_addr,
                       uint8_t *buf, uint16_t len);

/*
 * 混合模式：write + REPEATED START → read
 *
 * 时序：
 *   S  [dev_addr + W]  ACK  [tx_data...]  ACK  Sr  [dev_addr + R]  ACK  [rx_data...]  NACK  P
 *
 * 典型场景：先写寄存器地址，后读数据。
 */
I2C_Err_t i2c_write_read(I2C_Model_t *m, uint8_t dev_addr,
                          const uint8_t *tx_data, uint16_t tx_len,
                          uint8_t *rx_buf, uint16_t rx_len);
