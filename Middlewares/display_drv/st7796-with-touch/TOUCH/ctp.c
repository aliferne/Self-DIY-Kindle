#include "ctp.h"
#include "touch.h"
#include "bsp_i2c.h"
#include "bsp_gpio.h"
#include "bsp_handle.h"
#include "string.h"

/*
 * 静态资源指针 ―― 通过 ctp_assign_* 注入
 */
static GPIO_Model_t *ctp_rst_pin = NULL;
static GPIO_Model_t *ctp_it_pin  = NULL;
static I2C_Model_t  *ctp_i2c     = NULL;
static void (*ctp_delay_ms)(uint32_t ms) = NULL;

/* I2C 设备地址 (7-bit) */
#define FT6336_ADDR 0x38

/* 触摸点坐标寄存器表 */
static const uint16_t FT5206_TPX_TBL[5] = {
    FT_TP1_REG, FT_TP2_REG, FT_TP3_REG, FT_TP4_REG, FT_TP5_REG
};

/* ============================================================
 * 资源注入
 * ============================================================ */

void ctp_assign_pins(GPIO_Model_t *rst, GPIO_Model_t *it)
{
    ctp_rst_pin = rst;
    ctp_it_pin  = it;
}

void ctp_assign_i2c(I2C_Model_t *i2c)
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

//向FT6336写入一次数据
//reg:起始寄存器地址
//buf:数据缓缓存区
//len:写数据长度
//返回值:0,成功;1,失败.
uint8_t FT6336_WR_Reg(I2C_Model_t *i2c, uint16_t reg, uint8_t *buf, uint8_t len)
{
    /*
     * 构造缓冲区: [寄存器地址, data0, data1, ...]
     * 注意 reg 只取低 8 位 (FT6336 寄存器为 8 位)
     */
    uint8_t tmp[256];
    uint8_t i;
    I2C_Err_t ret;

    tmp[0] = reg & 0xFF;
    for (i = 0; i < len; i++) {
        tmp[1 + i] = buf[i];
    }

    ret = i2c_sw_write(i2c, FT6336_ADDR, tmp, 1 + len);
    return (ret == I2C_Err_OK) ? 0 : 1;
}

//从FT6336读出一次数据
//reg:起始寄存器地址
//buf:数据缓缓存区
//len:读数据长度
void FT6336_RD_Reg(I2C_Model_t *i2c, uint16_t reg, uint8_t *buf, uint8_t len)
{
    uint8_t reg_byte = reg & 0xFF;

    i2c_sw_write_read(i2c, FT6336_ADDR, &reg_byte, 1, buf, len);
}

/* ============================================================
 * FT6336 初始化
 * ============================================================ */

//初始化FT6336触摸屏
void FT6336_Init(void)
{
    if (ctp_rst_pin) {
        gpio_write(ctp_rst_pin, GPIO_Level_Low);   // 复位
        if (ctp_delay_ms) ctp_delay_ms(20);
        gpio_write(ctp_rst_pin, GPIO_Level_High);  // 释放复位
        if (ctp_delay_ms) ctp_delay_ms(300);
    }
}

/* ============================================================
 * FT6336 扫描
 * ============================================================ */

//扫描触摸屏(采用查询方式)
//mode:0,正常扫描.
//返回值:当前触屏状态.
//0,触屏无触摸;1,触屏有触摸
uint8_t FT6336_Scan(uint8_t mode)
{
    uint8_t buf[4];
    uint8_t i = 0;
    uint8_t res = 0;
    uint8_t temp;
    static uint8_t t = 0; //控制查询间隔,从而降低CPU占用率
    t++;
    if ((t % 10) == 0 || t < 10) //空闲时,每进入10次CTP_Scan函数才检测1次,从而节省CPU使用率
    {
        FT6336_RD_Reg(ctp_i2c, FT_REG_NUM_FINGER, &mode, 1); //读取触摸点的状态
        if ((mode & 0x0F) && ((mode & 0x0F) < 6))
        {
            temp = 0xFF << (mode & 0x0F); //将点的个数转换为1的位数,匹配tp_dev.sta定义
            tp_dev.sta = (~temp) | TP_PRES_DOWN | TP_CATH_PRES;
            for (i = 0; i < 5; i++)
            {
                if (tp_dev.sta & (1 << i)) //触摸有效?
                {
                    FT6336_RD_Reg(ctp_i2c, FT5206_TPX_TBL[i], buf, 4); //读取XY坐标值
                    if (tp_dev.touchtype & 0x01) //横屏
                    {
                        tp_dev.y[i] = ((uint16_t)(buf[0] & 0x0F) << 8) + buf[1];
                        tp_dev.x[i] = ((uint16_t)(buf[2] & 0x0F) << 8) + buf[3];
                    } else
                    {
                        tp_dev.x[i] = (((uint16_t)(buf[0] & 0x0F) << 8) + buf[1]);
                        tp_dev.y[i] = ((uint16_t)(buf[2] & 0x0F) << 8) + buf[3];
                    }
                    if ((buf[0] & 0xF0) != 0x80) tp_dev.x[i] = tp_dev.y[i] = 0; //必须是contact事件，才认为有效
                }
            }
            res = 1;
            if (tp_dev.x[0] == 0 && tp_dev.y[0] == 0) mode = 0; //读到的数据都是0,则忽略此次数据
            t = 0; //触发一次,则会最少连续监测10次,从而提高命中率
        }
    }
    if ((mode & 0x1F) == 0) //无触摸点按下
    {
        if (tp_dev.sta & TP_PRES_DOWN) //之前是被按下的
        {
            tp_dev.sta &= ~(1 << 7); //标记按键松开
        } else //之前就没有被按下
        {
            tp_dev.x[0] = 0xFFFF;
            tp_dev.y[0] = 0xFFFF;
            tp_dev.sta &= 0xE0; //清除点有效标记
        }
    }
    if (t > 240) t = 10; //重新从10开始计数
    return res;
}
