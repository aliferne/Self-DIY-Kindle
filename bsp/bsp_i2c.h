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
 *   i2c_init(&eeprom, GPIOB, GPIO_PIN_6, GPIOB, GPIO_PIN_7,
 *            &(I2C_Config_t){ .sw = {
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

I2C_Err_t i2c_init(I2C_Model_t *m,
                   GPIO_Port_t sda_port, GPIO_Pin_t sda_pin,
                   GPIO_Port_t scl_port, GPIO_Pin_t scl_pin,
                   const I2C_Config_t *cfg);
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
 * 芯片级原语
 *
 * 直接操作 GPIO.
 * ============================================================ */

void i2c_sw_sda_high(I2C_Model_t *m);
void i2c_sw_sda_low(I2C_Model_t *m);
GPIO_Level_t i2c_sw_get_sda(I2C_Model_t *m);
void i2c_sw_scl_high(I2C_Model_t *m);
void i2c_sw_scl_low(I2C_Model_t *m);
void i2c_sw_start(I2C_Model_t *m);
void i2c_sw_stop(I2C_Model_t *m);
uint8_t i2c_sw_send_byte(I2C_Model_t *m, uint8_t byte);
uint8_t i2c_sw_recv_byte(I2C_Model_t *m, uint8_t ack);
uint8_t send_addr(I2C_Model_t *m, uint8_t dev_addr, I2C_RW_t rw);
