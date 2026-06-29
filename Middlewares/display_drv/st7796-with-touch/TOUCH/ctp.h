#ifndef __CTP_H
#define __CTP_H

#include <stdint.h>
#include "bsp_i2c.h"
#include "bsp_gpio.h"

// FT6336 部分寄存器定义
#define FT_REG_NUM_FINGER 0x02 // 触摸状态寄存器

#define FT_TP1_REG        0x03 // 第一个触摸点数据地址
#define FT_TP2_REG        0x09 // 第二个触摸点数据地址
#define FT_TP3_REG        0x0F // 第三个触摸点数据地址
#define FT_TP4_REG        0x15 // 第四个触摸点数据地址
#define FT_TP5_REG        0x1B // 第五个触摸点数据地址

uint8_t FT6336_WR_Reg(iic_t *i2c, uint16_t reg, uint8_t *buf, uint8_t len);
void FT6336_RD_Reg(iic_t *i2c, uint16_t reg, uint8_t *buf, uint8_t len);
void FT6336_Init(void);
/*
 * 扫描触摸屏
 * x/y: 输出缓冲区 (至少 TOUCH_MAX_POINTS 大小)
 * sta: 输出状态 (见 TP_STATE_* 定义)
 * 返回值: 0=无触摸, 1=有触摸
 */
uint8_t FT6336_Scan(uint16_t *x, uint16_t *y, uint8_t *sta);

void ctp_assign_pins(gpio_t *rst, gpio_t *it);
void ctp_assign_i2c(iic_t *i2c);
void ctp_assign_delay(void (*cb)(uint32_t ms));

#endif
