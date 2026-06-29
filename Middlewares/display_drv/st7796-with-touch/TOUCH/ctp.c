#include "ctp.h"
#include "bsp_i2c.h"
#include "bsp_gpio.h"
#include "bsp_handle.h"
#include "string.h"

/*
 * 静态资源指针 ―― 通过 ctp_assign_* 注入
 */
static gpio_t *ctp_rst_pin               = NULL;
static gpio_t *ctp_it_pin                = NULL;
static iic_t *ctp_i2c                    = NULL;
static void (*ctp_delay_ms)(uint32_t ms) = NULL;

/* I2C 设备地址 (7-bit) */
#define FT6336_ADDR 0x38
#define FT6336_PRESS_DOWN  0x80
#define FT6336_CATH_PRESS  0x40

/* 默认触屏类型: 电容屏 */
static uint8_t ctp_touchtype = 0x80;

/* 触摸点坐标寄存器表 */
static const uint16_t FT5206_TPX_TBL[5] = {
    FT_TP1_REG, FT_TP2_REG, FT_TP3_REG, FT_TP4_REG, FT_TP5_REG};

/* ============================================================
 * 资源注入
 * ============================================================ */

void ctp_assign_pins(gpio_t *rst, gpio_t *it)
{
    ctp_rst_pin = rst;
    ctp_it_pin  = it;
}

void ctp_assign_i2c(iic_t *i2c)
{
    ctp_i2c = i2c;
}

void ctp_assign_delay(void (*cb)(uint32_t ms))
{
    ctp_delay_ms = cb;
}

/* ============================================================
 * FT6336 寄存器读写
 * ============================================================ */

// 向FT6336写入一次数据
// reg:起始寄存器地址
// buf:数据缓缓存区
// len:写数据长度
// 返回值:0,成功;1,失败.
uint8_t FT6336_WR_Reg(iic_t *i2c, uint16_t reg, uint8_t *buf, uint8_t len)
{
    /*
     * 构造缓冲区: [寄存器地址, data0, data1, ...]
     * 注意 reg 只取低 8 位 (FT6336 寄存器为 8 位)
     */
    uint8_t tmp[256];
    uint8_t i;
    iic_err_t ret;

    tmp[0] = reg & 0xFF;
    for (i = 0; i < len; i++) {
        tmp[1 + i] = buf[i];
    }

    ret = iic_write(i2c, FT6336_ADDR, tmp, 1 + len);
    return (ret == I2C_OK) ? 0 : 1;
}

// 从FT6336读出一次数据
// reg:起始寄存器地址
// buf:数据缓缓存区
// len:读数据长度
void FT6336_RD_Reg(iic_t *i2c, uint16_t reg, uint8_t *buf, uint8_t len)
{
    uint8_t reg_byte = reg & 0xFF;

    iic_read_write(i2c, FT6336_ADDR, buf, len, &reg_byte, 1);
}

/* ============================================================
 * FT6336 初始化
 * ============================================================ */

// 初始化FT6336触摸屏
void FT6336_Init(void)
{
    if (ctp_rst_pin) {
        gpio_write(ctp_rst_pin, GPIO_Level_Low); // 复位
        if (ctp_delay_ms) ctp_delay_ms(20);
        gpio_write(ctp_rst_pin, GPIO_Level_High); // 释放复位
        if (ctp_delay_ms) ctp_delay_ms(300);
    }
}

/* ============================================================
 * FT6336 扫描
 * ============================================================ */

// 扫描触摸屏(采用查询方式)
// x/y: 输出缓冲区 (至少 5 个 uint16_t)
// sta: 输出状态 (bit7:按下, bit4~0:触摸点数)
// 返回值:0,触屏无触摸;1,触屏有触摸
uint8_t FT6336_Scan(uint16_t *x, uint16_t *y, uint8_t *sta)
{
    uint8_t buf[4];
    uint8_t i   = 0;
    uint8_t res = 0;
    uint8_t temp;
    uint8_t finger_reg;
    static uint8_t t = 0; // 控制查询间隔,从而降低CPU占用率

    t++;
    if ((t % 10) == 0 || t < 10) // 空闲时,每进入10次才检测1次,从而节省CPU使用率
    {
        FT6336_RD_Reg(ctp_i2c, FT_REG_NUM_FINGER, &finger_reg, 1);
        if ((finger_reg & 0x0F) && ((finger_reg & 0x0F) < 6)) {
            temp    = 0xFF << (finger_reg & 0x0F);
            *sta    = (~temp) | FT6336_PRESS_DOWN | FT6336_CATH_PRESS;
            for (i = 0; i < 5; i++) {
                if (*sta & (1 << i)) {
                    FT6336_RD_Reg(ctp_i2c, FT5206_TPX_TBL[i], buf, 4);
                    if (ctp_touchtype & 0x01) {
                        y[i] = ((uint16_t)(buf[0] & 0x0F) << 8) + buf[1];
                        x[i] = ((uint16_t)(buf[2] & 0x0F) << 8) + buf[3];
                    } else {
                        x[i] = ((uint16_t)(buf[0] & 0x0F) << 8) + buf[1];
                        y[i] = ((uint16_t)(buf[2] & 0x0F) << 8) + buf[3];
                    }
                    if ((buf[0] & 0xF0) != 0x80) x[i] = y[i] = 0;
                }
            }
            res = 1;
            if (x[0] == 0 && y[0] == 0) finger_reg = 0;
            t = 0;
        }
    }
    if ((finger_reg & 0x1F) == 0) {
        if (*sta & FT6336_PRESS_DOWN) {
            *sta &= ~(1 << 7);
        } else {
            x[0] = 0xFFFF;
            y[0] = 0xFFFF;
            *sta &= 0xE0;
        }
    }
    if (t > 240) t = 10;
    return res;
}
