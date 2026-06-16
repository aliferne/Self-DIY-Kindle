#pragma once

#include <stdint.h>
#include "bsp_i2c.h"
#include "bsp_spi.h"
#include "bsp_gpio.h"

/*
 * 提供统一的显示屏驱动 API
 *
 * 使用方式：
 *   1. 调用方自行初始化 SPI_Model_t 和 GPIO_Model_t（DC/RST/BLK）
 *   2. 填充 Disp_Src_t 上下文 (tft)
 *   3. 提供 Disp_Drv_t 结构体 (disp)
 *   4. 令 disp.src = &tft;
 *   5. 调用 display_init(&disp)
 *
 *   示例：
 *     SPI_Model_t lcd_spi = { ... };    // 已初始化
 *     GPIO_Model_t lcd_dc  = { ... };
 *     GPIO_Model_t lcd_rst = { ... };
 *
 *     Disp_Src_t tft = {
 *         .spi     = &lcd_spi,
 *         .dc_pin  = &lcd_dc,
 *         .rst_pin = &lcd_rst,
 *         .blk_pin = NULL,               // 不使用背光控制
 *     };
 *    Disp_Drv_t disp = {
 *         .src = &tft,
 *    };
 *    display_init(&disp);
 */

/* 这个颜色对应是 RGB565 模式下的颜色 */
#define DISP_RED     0xF800
#define DISP_GREEN   0x07E0
#define DISP_BLUE    0x001F
#define DISP_BLUE2   0x1C9F
#define DISP_PINK    0xD8A7
#define DISP_ORANGE  0xFA20
#define DISP_WHITE   0xFFFF
#define DISP_BLACK   0x0000
#define DISP_YELLOW  0xFFE0
#define DISP_CYAN    0x07FF
#define DISP_PURPLE  0xF81F
#define DISP_PURPLE2 0xDB92
#define DISP_PURPLE3 0x8811
#define DISP_GRAY0   0xEF7D
#define DISP_GRAY1   0x8410
#define DISP_GRAY2   0x4208

/* 显示屏是否具有触控功能 */
#define DISP_HAS_TOUCH (1)

/* 显示屏硬件资源，假定使用 SPI 作为通信协议 */
typedef struct {
    SPI_Model_t *spi;      /**< SPI 总线（软件或硬件均可） */
    GPIO_Model_t *dc_pin;  /**< Data/Command 引脚 */
    GPIO_Model_t *rst_pin; /**< Reset 引脚 */
    GPIO_Model_t *blk_pin; /**< 背光引脚（无需控制可传 NULL） */
} Disp_Src_t;

#if DISP_HAS_TOUCH == 1
/* 显示屏触摸硬件资源，假定使用 I2C 作为通信协议 */
typedef struct {
    I2C_Model_t *i2c;  /**< I2C 总线 */
    GPIO_Model_t *it;  /**< 中断引脚 */
    GPIO_Model_t *rst; /**< 复位引脚 */
} Disp_Touch_Src_t;
#endif

/* 显示驱动 */
typedef struct _disp_drv {
    Disp_Src_t *src; /**< 底层驱动资源 */
#if DISP_HAS_TOUCH == 1
    Disp_Touch_Src_t *touch;
#endif
    uint8_t is_initialized : 1;
    uint32_t width;
    uint32_t height;
    /* 延时函数指针 */
    void (*delay_cb)(uint32_t ms);
} Disp_Drv_t;

static inline void display_set_delay_cb(Disp_Drv_t *drv, void (*cb)(uint32_t ms))
{
    drv->delay_cb = cb;
}

void display_init(Disp_Drv_t *drv); /**< 和 LVGL 的 disp_init 区分开 */
void display_deinit(Disp_Drv_t *drv);
void display_rst(Disp_Drv_t *drv);

void display_backlight_on(Disp_Drv_t *drv);
void display_backlight_off(Disp_Drv_t *drv);
void display_set_region(Disp_Drv_t *drv,
                        uint32_t x_start, uint32_t y_start,
                        uint32_t x_end, uint32_t y_end);
void display_spin_screen(Disp_Drv_t *drv, uint8_t dir);

void display_clean_screen(Disp_Drv_t *drv, uint16_t color);
void display_fill_screen(Disp_Drv_t *drv,
                         uint32_t x_start, uint32_t y_start,
                         uint32_t x_end, uint32_t y_end,
                         uint16_t color);

void display_set_cursor(Disp_Drv_t *drv, uint32_t x, uint32_t y);
void display_write_pixels(Disp_Drv_t *drv,
                          uint32_t x, uint32_t y,
                          uint32_t w, uint32_t h,
                          const uint16_t *pixels);
void display_draw_point(Disp_Drv_t *drv,
                        uint32_t x, uint32_t y, uint16_t color);
void display_draw_line(Disp_Drv_t *drv,
                       uint32_t x0, uint32_t y0,
                       uint32_t x1, uint32_t y1, uint16_t color);
void display_draw_circle(Disp_Drv_t *drv,
                         uint32_t x, uint32_t y,
                         uint32_t r, uint16_t color);
void display_draw_rect(Disp_Drv_t *drv,
                       uint32_t x, uint32_t y,
                       uint32_t w, uint32_t h, uint16_t color);
void display_draw_string(Disp_Drv_t *drv,
                         uint32_t x, uint32_t y,
                         uint16_t fc, uint16_t bc,
                         const char *str);
void display_draw_number(Disp_Drv_t *drv,
                         uint32_t x, uint32_t y,
                         uint16_t fc, uint16_t bc,
                         int num);

/* 测试显示效果，可以不实现 */
void display_test(Disp_Drv_t *display);
