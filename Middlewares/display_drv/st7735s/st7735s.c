#include "disp_drv.h"
#include "tft.h"
#include "bsp_spi.h"
#include "bsp_gpio.h"
#include "bsp_sys.h"
#include "bsp_handle.h"
#include "tft_font.h"
#include <stdint.h>
#include <string.h>

/*
 * 这里需要根据你自己的需要去自定义
 *
 * ST7735S 支持分配 RGB bit 的占比，对应的颜色分辨率也会有所不同
 * RGB 4-4-4-bit (4k 颜色) => 0x3A = 0x03
 * RGB 5-6-5-bit (65k 颜色) => 0x3A = 0x05
 * RGB 6-6-6-bit (262k 颜色) => 0x3A = 0x06
 *
 * 由于这里需要考虑 LVGL 支持的颜色深度显示模式，故选择 565 作为显示方式
 * 若移除 LVGL，则可以从 444, 565, 666 中任选
 *
 * \ref ST7735S 数据手册, lv_conf.h
 */
#define DISP_RGB_BIT_INPUT (0x05)

#define disp_delay_ms(ms)  chip_delay_ms(ms)

/* ============================================================
 * SPI 批量写原语
 * ============================================================ */

static void tft_write_cmd_bulk(TFT_Model_t *tft, const uint8_t *data, uint32_t len)
{
    ASSERT_FAIL(tft == NULL || tft->dc_pin == NULL || data == NULL, return);
    // DC=低 → 指令
    gpio_write(tft->dc_pin, GPIO_Level_Low);

    spi_cs_select(tft->spi);
    spi_write(tft->spi, data, len);
    spi_cs_deselect(tft->spi);
}

static void tft_write_data_bulk(TFT_Model_t *tft, const uint8_t *data, uint32_t len)
{
    ASSERT_FAIL(tft == NULL || tft->dc_pin == NULL || data == NULL, return);
    // DC=高 → 数据
    gpio_write(tft->dc_pin, GPIO_Level_High);

    spi_cs_select(tft->spi);
    spi_write(tft->spi, data, len);
    spi_cs_deselect(tft->spi);
}

static void tft_write_index(TFT_Model_t *tft, uint8_t cmd)
{
    tft_write_cmd_bulk(tft, &cmd, 1);
}

/* ============================================================
 * 驱动实现：disp_drv.h → ST7735S
 * ============================================================ */

/* 显示屏初始化 */
void display_init(Disp_Drv_t *drv)
{
    ASSERT_FAIL(drv == NULL || drv->src == NULL, return);
    TFT_Model_t *tft = (TFT_Model_t *)drv->src;
    ASSERT_FAIL(tft->is_initialized != 0, return);
    tft->is_initialized = 1;

    /* 硬件复位 */
    disp_rst(drv);

    tft_write_index(tft, 0x11); // 唤醒
    disp_delay_ms(120);

    // ---- 基本配置 ----
    tft_write_index(tft, 0x36);
    tft_write_data_bulk(tft, (const uint8_t[]){0x00}, 1);

    tft_write_index(tft, 0x3A);
    tft_write_data_bulk(tft, (const uint8_t[]){DISP_RGB_BIT_INPUT}, 1);

    // 帧率
    tft_write_index(tft, 0xB1);
    tft_write_data_bulk(tft, (const uint8_t[]){0x05, 0x3C, 0x3C}, 3);

    tft_write_index(tft, 0xB2);
    tft_write_data_bulk(tft, (const uint8_t[]){0x05, 0x3C, 0x3C}, 3);

    tft_write_index(tft, 0xB3);
    tft_write_data_bulk(tft, (const uint8_t[]){0x05, 0x3C, 0x3C, 0x05, 0x3C, 0x3C}, 6);

    tft_write_index(tft, 0xB4);
    tft_write_data_bulk(tft, (const uint8_t[]){0x03}, 1);

    // 电源控制
    tft_write_index(tft, 0xC0);
    tft_write_data_bulk(tft, (const uint8_t[]){0x2E, 0x06, 0x04}, 3);

    tft_write_index(tft, 0xC1);
    tft_write_data_bulk(tft, (const uint8_t[]){0xC0, 0xC2}, 2);

    tft_write_index(tft, 0xC2);
    tft_write_data_bulk(tft, (const uint8_t[]){0x0D, 0x0D}, 2);

    tft_write_index(tft, 0xC3);
    tft_write_data_bulk(tft, (const uint8_t[]){0x8D, 0xEE}, 2);

    tft_write_index(tft, 0xC4);
    tft_write_data_bulk(tft, (const uint8_t[]){0x8D, 0xEE}, 2);

    tft_write_index(tft, 0xC5);
    tft_write_data_bulk(tft, (const uint8_t[]){0x00}, 1);

    // 数据访问方式
    tft_write_index(tft, 0x36);
    tft_write_data_bulk(tft, (const uint8_t[]){0xC0}, 1);

    // 伽马
    tft_write_index(tft, 0xE0);
    tft_write_data_bulk(
        tft,
        (const uint8_t[]){0x1B, 0x21, 0x10, 0x15, 0x2B, 0x25, 0x1F, 0x23, 0x22, 0x22, 0x2B, 0x37, 0x00, 0x15, 0x02, 0x3F},
        16);
    tft_write_index(tft, 0xE1);

    tft_write_data_bulk(
        tft,
        (const uint8_t[]){0x1A, 0x20, 0x0F, 0x15, 0x2A, 0x25, 0x1E, 0x23, 0x23, 0x22, 0x2B, 0x37, 0x00, 0x15, 0x02, 0x3F},
        16);

    tft_write_index(tft, 0x2C);
    tft_write_index(tft, 0x29); // 开屏

    drv->width          = 128;
    drv->height         = 160;
    drv->is_initialized = 1;
}

/* 显示屏去初始化 */
void disp_deinit(Disp_Drv_t *drv)
{
    ASSERT_FAIL(drv == NULL || drv->src == NULL, return);
    TFT_Model_t *tft = (TFT_Model_t *)drv->src;

    gpio_deinit(tft->blk_pin);
    gpio_deinit(tft->dc_pin);
    gpio_deinit(tft->rst_pin);
    spi_deinit(tft->spi);

    tft->is_initialized = 0;
}

/* 复位屏幕 */
void disp_rst(Disp_Drv_t *drv)
{
    ASSERT_FAIL(drv == NULL || drv->src == NULL, return);
    TFT_Model_t *tft = (TFT_Model_t *)drv->src;
    ASSERT_FAIL(tft->rst_pin == NULL, return);

    gpio_write(tft->rst_pin, GPIO_Level_Low);
    disp_delay_ms(100);
    gpio_write(tft->rst_pin, GPIO_Level_High);
    disp_delay_ms(50);
}

/* 写入 16 位数据 */
void disp_write_16bit(Disp_Drv_t *drv, uint16_t data)
{
    ASSERT_FAIL(drv == NULL || drv->src == NULL, return);

    TFT_Model_t *tft = (TFT_Model_t *)drv->src;
    uint8_t buf[2]   = {(uint8_t)(data >> 8), (uint8_t)(data & 0xFF)};
    tft_write_data_bulk(tft, buf, 2);
}

/* 写入命令 */
void disp_write_cmd(Disp_Drv_t *drv, uint8_t *data, uint32_t len)
{
    ASSERT_FAIL(drv == NULL || drv->src == NULL, return);

    tft_write_cmd_bulk((TFT_Model_t *)drv->src, data, len);
}

/* 写入数据 */
void disp_write_data(Disp_Drv_t *drv, uint8_t *data, uint32_t len)
{
    ASSERT_FAIL(drv == NULL || drv->src == NULL, return);

    tft_write_data_bulk((TFT_Model_t *)drv->src, data, len);
}

/* 开启背光 */
void disp_backlight_on(Disp_Drv_t *drv)
{
    ASSERT_FAIL(drv == NULL || drv->src == NULL, return);

    TFT_Model_t *tft = (TFT_Model_t *)drv->src;

    ASSERT_FAIL(tft->blk_pin == NULL, return);

    gpio_write(tft->blk_pin, GPIO_Level_High);
}

/* 关闭背光 */
void disp_backlight_off(Disp_Drv_t *drv)
{
    ASSERT_FAIL(drv == NULL || drv->src == NULL, return);

    TFT_Model_t *tft = (TFT_Model_t *)drv->src;

    ASSERT_FAIL(tft->blk_pin == NULL, return);

    gpio_write(tft->blk_pin, GPIO_Level_Low);
}

/* 设置显示区域 */
void disp_set_region(Disp_Drv_t *drv,
                     uint32_t x_start, uint32_t y_start,
                     uint32_t x_end, uint32_t y_end)
{
    ASSERT_FAIL(drv == NULL || drv->src == NULL, return);
    TFT_Model_t *tft = (TFT_Model_t *)drv->src;

    uint8_t col_data[] = {0x00, (uint8_t)x_start, 0x00, (uint8_t)x_end};
    uint8_t row_data[] = {0x00, (uint8_t)y_start, 0x00, (uint8_t)y_end};
    tft_write_index(tft, 0x2A);
    tft_write_data_bulk(tft, col_data, LEN(col_data));

    tft_write_index(tft, 0x2B);
    tft_write_data_bulk(tft, row_data, LEN(row_data));

    tft_write_index(tft, 0x2C);
}

/* 旋转屏幕 */
void disp_spin_screen(Disp_Drv_t *drv, uint8_t dir)
{
    ASSERT_FAIL(drv == NULL || drv->src == NULL, return);

    static const uint8_t vals[] = {0xC0, 0xA0, 0x00, 0x60};
    if (dir > 3) dir = 0;

    tft_write_index((TFT_Model_t *)drv->src, 0x36);
    tft_write_data_bulk((TFT_Model_t *)drv->src, &vals[dir], 1);
}

/* 清除屏幕 */
void disp_clean_screen(Disp_Drv_t *drv, uint16_t color)
{
    disp_fill_screen(drv, 0, 0, drv->width, drv->height, color);
}

/* 填充屏幕 */
void disp_fill_screen(Disp_Drv_t *drv,
                      uint32_t x_start, uint32_t y_start,
                      uint32_t x_end, uint32_t y_end,
                      uint16_t color)
{
    ASSERT_FAIL(drv == NULL || drv->src == NULL, return);

    TFT_Model_t *tft = (TFT_Model_t *)drv->src;
    uint32_t w       = x_end - x_start + 1;
    uint32_t h       = y_end - y_start + 1;
    uint32_t total   = w * h;

    disp_set_region(drv, x_start, y_start, x_end, y_end);
    gpio_write(tft->dc_pin, GPIO_Level_High);
    spi_cs_select(tft->spi);

    // uint8_t row_buf[256];
    // uint32_t row_pixels = w > 128 ? 128 : w;
    // for (uint32_t i = 0; i < row_pixels; i++) {
    //     row_buf[2 * i]     = (uint8_t)(color >> 8);
    //     row_buf[2 * i + 1] = (uint8_t)(color & 0xFF);
    // }

    // uint32_t remaining = total;
    // while (remaining > row_pixels) {
    //     spi_write(tft->spi, row_buf, row_pixels * 2);
    //     remaining -= row_pixels;
    // }
    // if (remaining > 0)
    //     spi_write(tft->spi, row_buf, remaining * 2);
    while (total--)
        disp_write_16bit(drv, color);

    spi_cs_deselect(tft->spi);
    disp_set_region(drv, 0, 0, drv->width - 1, drv->height - 1);
}

/* 设置光标 */
void disp_set_cursor(Disp_Drv_t *drv, uint32_t x, uint32_t y)
{
    ASSERT_FAIL(drv == NULL || drv->src == NULL, return);

    disp_set_region(drv, x, y, x, y);
}

/* 绘制连续的像素点(每个 pixel 均对应一个颜色) */
void disp_write_pixels(Disp_Drv_t *drv,
                       uint32_t x, uint32_t y,
                       uint32_t w, uint32_t h,
                       const uint16_t *pixels)
{
    ASSERT_FAIL(drv == NULL || drv->src == NULL || pixels == NULL, return);
    TFT_Model_t *tft = (TFT_Model_t *)drv->src;

    disp_set_region(drv, x, y, x + w - 1, y + h - 1);

    gpio_write(tft->dc_pin, GPIO_Level_High);
    spi_cs_select(tft->spi);
    spi_write(tft->spi, (const uint8_t *)pixels, w * h * sizeof(uint16_t));
    spi_cs_deselect(tft->spi);

    disp_set_region(drv, 0, 0, drv->width - 1, drv->height - 1);
}

/* 绘制点 */
void disp_draw_point(Disp_Drv_t *drv,
                     uint32_t x, uint32_t y, uint16_t color)
{
    ASSERT_FAIL(drv == NULL || drv->src == NULL, return);

    disp_set_cursor(drv, x, y);
    disp_write_16bit(drv, color);
}

/* 绘制直线 */
void disp_draw_line(Disp_Drv_t *drv,
                    uint32_t x0, uint32_t y0,
                    uint32_t x1, uint32_t y1, uint16_t color)
{
    ASSERT_FAIL(drv == NULL || drv->src == NULL, return);

    int16_t dx    = (int16_t)x1 - (int16_t)x0;
    int16_t dy    = (int16_t)y1 - (int16_t)y0;
    int16_t x_inc = (dx >= 0) ? 1 : -1;
    int16_t y_inc = (dy >= 0) ? 1 : -1;
    int16_t dx2, dy2, error;

    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
    dx2 = dx << 1;
    dy2 = dy << 1;

    if (dx > dy) {
        error = dy2 - dx;
        for (int16_t i = 0; i <= dx; i++) {
            disp_draw_point(drv, (uint32_t)x0, (uint32_t)y0, color);
            if (error >= 0) {
                error -= dx2;
                y0 += y_inc;
            }
            error += dy2;
            x0 += x_inc;
        }
    } else {
        error = dx2 - dy;
        for (int16_t i = 0; i <= dy; i++) {
            disp_draw_point(drv, (uint32_t)x0, (uint32_t)y0, color);
            if (error >= 0) {
                error -= dy2;
                x0 += x_inc;
            }
            error += dx2;
            y0 += y_inc;
        }
    }
}

/* 绘制圆 */
void disp_draw_circle(Disp_Drv_t *drv,
                      uint32_t x, uint32_t y,
                      uint32_t r, uint16_t color)
{
    ASSERT_FAIL(drv == NULL || drv->src == NULL, return);

    int16_t cx = (int16_t)x, cy = (int16_t)y, cr = (int16_t)r;
    int16_t a = 0, b = cr;
    int16_t c = 3 - 2 * cr;

    while (a < b) {
        disp_draw_point(drv, cx + a, cy + b, color);
        disp_draw_point(drv, cx - a, cy + b, color);
        disp_draw_point(drv, cx + a, cy - b, color);
        disp_draw_point(drv, cx - a, cy - b, color);
        disp_draw_point(drv, cx + b, cy + a, color);
        disp_draw_point(drv, cx - b, cy + a, color);
        disp_draw_point(drv, cx + b, cy - a, color);
        disp_draw_point(drv, cx - b, cy - a, color);

        if (c < 0)
            c = c + 4 * a + 6;
        else {
            c = c + 4 * (a - b) + 10;
            b -= 1;
        }
        a += 1;
    }
    if (a == b) {
        disp_draw_point(drv, cx + a, cy + b, color);
        disp_draw_point(drv, cx + a, cy - b, color);
        disp_draw_point(drv, cx - a, cy - b, color);
        disp_draw_point(drv, cx + b, cy + a, color);
        disp_draw_point(drv, cx - b, cy + a, color);
        disp_draw_point(drv, cx + b, cy - a, color);
        disp_draw_point(drv, cx - b, cy - a, color);
    }
}

/* 绘制矩形 */
void disp_draw_rect(Disp_Drv_t *drv,
                    uint32_t x, uint32_t y,
                    uint32_t w, uint32_t h, uint16_t color)
{
    ASSERT_FAIL(drv == NULL || drv->src == NULL, return);
    disp_draw_line(drv, x, y, x + w, y, color);
    disp_draw_line(drv, x + w, y, x + w, y + h, color);
    disp_draw_line(drv, x, y + h, x + w, y + h, color);
    disp_draw_line(drv, x, y, x, y + h, color);
}

/* 绘制字符串 */
void disp_draw_string(Disp_Drv_t *drv,
                      uint32_t x, uint32_t y,
                      uint16_t fc, uint16_t bc,
                      const char *str)
{
    ASSERT_FAIL(drv == NULL || drv->src == NULL || str == NULL, return);
    uint32_t sx = x, sy = y;

    for (const char *p = str; *p != '\0';) {
        uint8_t c = (uint8_t)*p;

        if (c < 128) {
            if (sx + 8 >= drv->width) {
                sx = 0;
                sy += 16;
            }
            int k = (c - 32) * 16;
            for (int i = 0; i < 16; i++) {
                for (int j = 0; j < 8; j++) {
                    uint16_t col = (asc[k + i] & (0x80 >> j)) ? fc : bc;
                    disp_draw_point(drv, sx + j, sy + i, col);
                }
            }
            sx += 8;
            p++;
        } else {
            char seq[4] = {p[0], p[1], p[2], '\0'};
            p += 3;

            /* 在字模表中查找 */
            int idx = -1;
            int l1  = strlen(font_sample);
            for (int i = 0; i < l1; i += 3) {
                if (font_sample[i] == seq[0] &&
                    font_sample[i + 1] == seq[1] &&
                    font_sample[i + 2] == seq[2]) {
                    idx = i / 3;
                    break;
                }
            }

            if (idx < 0) {
                if (sx + 8 >= drv->width) {
                    sx = 0;
                    sy += 16;
                }
                for (int i = 0; i < 16; i++)
                    for (int j = 0; j < 8; j++)
                        disp_draw_point(drv, sx + j, sy + i, (i < 2 || i >= 14 || j < 2 || j >= 6) ? fc : bc);
                sx += 8;
                continue;
            }

            if (sx + 16 >= drv->width) {
                sx = 0;
                sy += 16;
            }

            for (int i = 0; i < 16; i++) {
                for (int j = 0; j < 8; j++) {
                    uint16_t col = (chinese_font[idx * 32 + 2 * i] & (0x80 >> j)) ? fc : bc;
                    disp_draw_point(drv, sx + j, sy + i, col);
                }
                for (int j = 0; j < 8; j++) {
                    uint16_t col = (chinese_font[idx * 32 + 2 * i + 1] & (0x80 >> j)) ? fc : bc;
                    disp_draw_point(drv, sx + j + 8, sy + i, col);
                }
            }
            sx += 16;
        }
    }
}

/* 绘制数字 */
void disp_draw_number(Disp_Drv_t *drv,
                      uint32_t x, uint32_t y,
                      uint16_t fc, uint16_t bc,
                      int num)
{
    ASSERT_FAIL(drv == NULL || drv->src == NULL, return);

    char buf[16];
    int pos = 14;
    buf[15] = '\0';

    if (num == 0) {
        buf[14] = '0';
        disp_draw_string(drv, x, y, fc, bc, &buf[14]);
        return;
    }

    int negative = 0;
    if (num < 0) {
        negative = 1;
        num      = -num;
    }

    while (num && pos > 0) {
        buf[--pos] = '0' + num % 10;
        num /= 10;
    }
    if (negative) buf[--pos] = '-';

    disp_draw_string(drv, x, y, fc, bc, &buf[pos]);
}

void disp_test(Disp_Drv_t *display)
{
    uint16_t colors[] = {
        DISP_RED,
        DISP_GREEN,
        DISP_BLUE,
        DISP_CYAN,
        DISP_YELLOW,
        DISP_ORANGE,
        DISP_PURPLE,
        DISP_PURPLE2,
    };

    disp_backlight_on(display);
    disp_draw_string(display, 0, 0, DISP_PINK, DISP_BLACK, "Chinese sample:");

    for (;;) {
        int n = sizeof(colors) / sizeof(colors[0]);

        for (int i = 0; i < n; i++) {
            disp_clean_screen(display, DISP_BLACK);
            disp_draw_string(display, 0, 0, colors[i], DISP_BLACK, "Chinese sample:");
            disp_draw_string(display, 0, 16, colors[(i + 1) % n], DISP_BLACK,
                             "我是一只猫快乐的星猫从来没烦恼你快乐就好");
            chip_delay_ms(50);
        }

        disp_clean_screen(display, DISP_BLACK);
        disp_draw_string(display, 0, 0, DISP_BLUE2, DISP_BLACK, "Mix sample");
        disp_draw_string(display, 0, 16, DISP_PURPLE, DISP_BLACK,
                         "我是一只猫2525,快乐的星猫3434~从来没烦恼,你快乐就好!2233445,ahahahhahah");
        chip_delay_ms(700);

        disp_clean_screen(display, DISP_BLACK);
        disp_draw_string(display, 0, 0, DISP_PURPLE3, DISP_BLACK, "special_font");
        disp_draw_string(display, 0, 32, DISP_ORANGE, DISP_BLACK, "你是光");
        disp_draw_string(display, 0, 64, DISP_CYAN, DISP_BLACK, "你是电");
        disp_draw_string(display, 0, 96, DISP_PURPLE2, DISP_BLACK, "你是唯一的信仰");
        chip_delay_ms(1000);
        disp_clean_screen(display, DISP_BLACK);

        chip_delay_ms(5);
    }
}