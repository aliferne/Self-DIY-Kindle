#pragma once

#include <stdint.h>
#include "bsp_spi.h"
#include "bsp_gpio.h"

/*
 * 提供统一的显示屏驱动 API
 *
 * 使用方式：
 *   1. 调用方自行初始化 SPI_Model_t 和 GPIO_Model_t（DC/RST/BLK）
 *   2. 填充 TFT_t 上下文
 *   3. 调用 TFT_Init(&tft)
 *
 *   示例：
 *     SPI_Model_t lcd_spi = { ... };    // 已初始化
 *     GPIO_Model_t lcd_dc  = { ... };
 *     GPIO_Model_t lcd_rst = { ... };
 *
 *     TFT_t tft = {
 *         .spi     = &lcd_spi,
 *         .dc_pin  = &lcd_dc,
 *         .rst_pin = &lcd_rst,
 *         .blk_pin = NULL,               // 不使用背光控制
 *     };
 *     TFT_Init(&tft);
 */

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

/* 显示屏硬件资源，假定使用 SPI 作为通信协议 */
typedef struct {
    SPI_Model_t *spi;      /**< SPI 总线（软件或硬件均可） */
    GPIO_Model_t *dc_pin;  /**< Data/Command 引脚 */
    GPIO_Model_t *rst_pin; /**< Reset 引脚 */
    GPIO_Model_t *blk_pin; /**< 背光引脚（无需控制可传 NULL） */
} Disp_Src_t;

/* 显示驱动 */
typedef struct _disp_drv {
    Disp_Src_t *src; /**< 底层驱动资源 */
    uint8_t is_initialized : 1;
    uint32_t width;
    uint32_t height;
    /* 延时函数指针 */
    void (*delay_cb)(uint32_t ms);
} Disp_Drv_t;

static inline void disp_set_delay_cb(Disp_Drv_t *drv, void (*cb)(uint32_t ms))
{
    drv->delay_cb = cb;
}

void display_init(Disp_Drv_t *drv); /**< 和 LVGL 的 disp_init 区分开 */
void disp_deinit(Disp_Drv_t *drv);
void disp_rst(Disp_Drv_t *drv);

void disp_backlight_on(Disp_Drv_t *drv);
void disp_backlight_off(Disp_Drv_t *drv);
void disp_set_region(Disp_Drv_t *drv,
                     uint32_t x_start, uint32_t y_start,
                     uint32_t x_end, uint32_t y_end);
void disp_spin_screen(Disp_Drv_t *drv, uint8_t dir);

void disp_clean_screen(Disp_Drv_t *drv, uint16_t color);
void disp_fill_screen(Disp_Drv_t *drv,
                      uint32_t x_start, uint32_t y_start,
                      uint32_t x_end, uint32_t y_end,
                      uint16_t color);

void disp_set_cursor(Disp_Drv_t *drv, uint32_t x, uint32_t y);
void disp_write_pixels(Disp_Drv_t *drv,
                       uint32_t x, uint32_t y,
                       uint32_t w, uint32_t h,
                       const uint16_t *pixels);
void disp_draw_point(Disp_Drv_t *drv,
                     uint32_t x, uint32_t y, uint16_t color);
void disp_draw_line(Disp_Drv_t *drv,
                    uint32_t x0, uint32_t y0,
                    uint32_t x1, uint32_t y1, uint16_t color);
void disp_draw_circle(Disp_Drv_t *drv,
                      uint32_t x, uint32_t y,
                      uint32_t r, uint16_t color);
void disp_draw_rect(Disp_Drv_t *drv,
                    uint32_t x, uint32_t y,
                    uint32_t w, uint32_t h, uint16_t color);
void disp_draw_string(Disp_Drv_t *drv,
                      uint32_t x, uint32_t y,
                      uint16_t fc, uint16_t bc,
                      const char *str);
void disp_draw_number(Disp_Drv_t *drv,
                      uint32_t x, uint32_t y,
                      uint16_t fc, uint16_t bc,
                      int num);

/* 测试显示效果，可以不实现 */
void disp_test(Disp_Drv_t *display);
