#pragma once

/*
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

typedef enum {
    I2C_OK           = 0,
    I2C_NACK         = 1,
    I2C_BUSY         = 2,
    I2C_INVALID_MODE = 3,
} iic_err_t;

typedef struct {
    gpio_t sda;
    gpio_t scl;
} iic_sw_t;

/*
 * I2C_Config_t — union 形式，后续可扩展 hw 配置
 */
typedef union {
    struct {
        uint16_t scl_delay_us; /**< SCL 半周期延时 (μs), 100kHz→5 */
        GPIO_Speed_t speed;    /**< SCL 和 SDA 的速度配置 */
        GPIO_Pull_t sda_pull;  /**< SDA 上拉配置 */
        GPIO_Pull_t scl_pull;  /**< SCL 上拉配置 */
    } sw;
} iic_cfg_t;

/*
 * I2C_Model_t — 运行时模型
 */
typedef struct i2c_model {
    union {
        iic_sw_t sw;
    } src;
    iic_cfg_t config; /**< 配置快照 */

    iic_err_t (*write)(struct i2c_model *m, uint8_t dev_addr,
                       const uint8_t *data, uint16_t len);
    iic_err_t (*read)(struct i2c_model *m, uint8_t dev_addr,
                      uint8_t *data, uint16_t len);
    iic_err_t (*read_write)(struct i2c_model *m, uint8_t dev_addr,
                            uint8_t *rx_buf, uint16_t rx_len,
                            const uint8_t *tx_buf, uint16_t tx_len);
} iic_t;

iic_err_t iic_init(iic_t *m,
                   GPIO_Port_t sda_port, GPIO_Pin_t sda_pin,
                   GPIO_Port_t scl_port, GPIO_Pin_t scl_pin,
                   const iic_cfg_t *cfg);
iic_err_t iic_deinit(iic_t *m);

iic_err_t iic_write(iic_t *m, uint8_t dev_addr,
                    const uint8_t *data, uint16_t len);
iic_err_t iic_read(iic_t *m, uint8_t dev_addr,
                   uint8_t *buf, uint16_t len);
iic_err_t iic_read_write(iic_t *m, uint8_t dev_addr,
                         uint8_t *rx_buf, uint16_t rx_len,
                         const uint8_t *tx_buf, uint16_t tx_len);
