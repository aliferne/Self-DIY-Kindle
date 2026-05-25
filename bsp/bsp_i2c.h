#pragma once

/*
 * ============================================================
 * bsp_i2c.h  —  I2C 硬件抽象层（纯接口）
 * ============================================================
 *
 * 本头文件不 include 任何芯片厂商的头文件。
 * 所有类型和枚举均为自有定义。
 *
 * 芯片相关实现位于 chip/<vendor>/i2c_chip.c，
 *
 * 使用示例：
 * TODO:
 */

/*
 * 实际上需要区分软件 I2C 和硬件 I2C，简便起见这里就只使用软件 I2C
 */

#include "bsp_gpio.h"

typedef enum {
    I2C_Err_OK = 0,
} I2C_Err_t;

typedef struct {
    union {
        GPIO_Config_t sw_i2c;
        /* hw_i2c */
    } cfg;
} I2C_Config_t;

/*
 * hardware source of i2c
 */
typedef struct {
    GPIO_HWSource_t sda;
    GPIO_HWSource_t scl;
} I2C_HWSource_t;

typedef struct {
    I2C_HWSource_t src;
    /* TODO: */
} I2C_Model_t;

I2C_Err_t i2c_register(
    I2C_Model_t *m,
    GPIO_Port_t sda_port, GPIO_Pin_t sda_pin,
    GPIO_Port_t scl_port, GPIO_Pin_t scl_pin);
I2C_Err_t i2c_init(I2C_Model_t *m, I2C_Config_t *cfg);
I2C_Err_t i2c_deinit(I2C_Model_t *m);

I2C_Err_t i2c_transmit(I2C_Model_t *m);
I2C_Err_t i2c_receive(I2C_Model_t *m);

/* TODO: */
