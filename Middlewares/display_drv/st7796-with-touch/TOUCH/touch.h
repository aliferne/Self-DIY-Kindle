#ifndef __TOUCH_H__
#define __TOUCH_H__

#include <stdint.h>
#include "ctp.h"

#define TP_PRES_DOWN 0x80 // 触屏被按下
#define TP_TYPE_CTP  0x80 // 触摸类型: 电容屏
#define TP_CATH_PRES 0x40 // 有按键按下了
#define CT_MAX_TOUCH 5    // 电容屏支持的点数,固定为5点

// 触摸屏控制器 (电容屏 only)
typedef struct
{
    uint16_t x[CT_MAX_TOUCH]; // 当前坐标
    uint16_t y[CT_MAX_TOUCH]; // 电容屏有最多5组坐标
    /*
     * 笔的状态
     *  b7:按下1/松开0;
     *  b6:0,没有按键按下;1,有按键按下.
     *  b5:保留
     *  b4~b0:电容触摸屏按下的点数(0,表示未按下,1表示按下)
     */
    uint8_t sta;
    uint8_t touchtype; // 触屏类型(见CTP/RTP宏),FT6336_Scan据此判断横竖屏坐标映射
} _m_tp_dev;

extern _m_tp_dev tp_dev; // 触屏控制器在touch.c里面定义

// 电容屏/电阻屏 共用函数

uint8_t TP_Scan(uint8_t tp); // 扫描
void TP_Init(void);          // 初始化

#endif
