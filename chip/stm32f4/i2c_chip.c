/*
 * ============================================================
 * chip/stm32f4/i2c_chip.c
 *
 * BSP I2C 抽象的 STM32F4 芯片实现。
 *
 * 软件 I2C (bit-bang)：
 *   - SDA/SCL 固定为 Output_OD，强制开漏
 *   - 使用 bsp_gpio API 控制引脚电平
 *   - 使用 dwt_delay_us 控制时序
 * ============================================================
 */

#include "bsp_gpio.h"
#include "bsp_i2c.h"

/* ============================================================
 * API 实现
 * ============================================================ */

I2C_Err_t i2c_init(I2C_Model_t *m,
                   GPIO_Port_t sda_port, GPIO_Pin_t sda_pin,
                   GPIO_Port_t scl_port, GPIO_Pin_t scl_pin,
                   const I2C_Config_t *cfg)
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

    gpio_init(&m->src.sw.sda, sda_port, sda_pin, &pin_cfg);

    pin_cfg.pull = cfg->sw.scl_pull;
    gpio_init(&m->src.sw.scl, scl_port, scl_pin, &pin_cfg);

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
