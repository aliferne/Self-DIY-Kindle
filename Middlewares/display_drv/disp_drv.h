#pragma once

#include <stdint.h>

/*
 * 提供统一的显示屏驱动 API
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

/* 显示驱动 */
typedef struct _disp_drv {
    void *src; /**< 底层驱动资源 */
    uint8_t is_initialized : 1;
    uint32_t width;
    uint32_t height;
} Disp_Drv_t;

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
