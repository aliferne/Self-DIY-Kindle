#pragma once

/*
 * 触摸屏驱动
 */

#include "bsp_gpio.h"
#include "bsp_i2c.h"
#include <stdint.h>
#include <stdbool.h>

/* 最大同时触摸点数 */
#define TOUCH_MAX_POINTS 5

/* 显示屏触摸硬件资源，假定使用 I2C 作为通信协议 */
typedef struct {
    iic_t *i2c;  /**< I2C 总线 */
    gpio_t *it;  /**< 中断引脚 */
    gpio_t *rst; /**< 复位引脚 */
} touch_src_t;

typedef struct _touch_drv {
    uint8_t is_initialized : 1;
    /* 触摸点位 x 轴坐标 */
    uint16_t x[TOUCH_MAX_POINTS];
    /* 触摸点位 y 轴坐标 */
    uint16_t y[TOUCH_MAX_POINTS];
    /*
     * 触摸状态，其定义具体取决于驱动，
     * 一般来说是置标志位，有些支持多点触摸的芯片
     * 如 FT6336，可能会将 bit7 作为总的有无触摸的标志位，
     * b4 ~ b0 指示多点触摸的情况
     */
    uint8_t tp_state;
    /* 此处标志位用以指示总体情况是否存在触摸 */
    bool is_pressed;

    /*
     * 屏幕的旋转方向
     * TODO: FT6336 的原始坐标与屏幕朝向有关。当前 ctp_touchtype 固定为 0x80
     *       (竖屏映射), 后续若需动态切换横竖屏, 应在此字段变更时同步调用
     *       FT6336 的坐标映射或由 touch_scan 做后处理变换。
     */
    uint8_t spin_dir;

    /* 硬件资源 */
    touch_src_t *src;
    /* 底层驱动私有变量 */
    void *priv;

    /* 延时函数 */
    void (*delay_cb)(uint32_t ms);
} touch_drv_t;

/* 判断是否初始化，注意用这个宏之前务必对 touch 进行 NULL Check */
#define TOUCH_IS_INIT(touch) ((touch)->is_initialized == 1)

void touch_init(touch_drv_t *t);
void touch_deinit(touch_drv_t *t);
int touch_scan(touch_drv_t *t);
bool touch_is_pressed(touch_drv_t *t);
void touch_test(touch_drv_t *t);
