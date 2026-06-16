/*
 * display_drv.c — ST7796 带触屏的 Disp_Drv_t 适配层
 *
 * 不重新实现任何绘图算法，全部委托给 LCD.c 和 TOUCH/*.c。
 *
 * 依赖注入（标记为 [DI]）：
 *   1. display_init() 将 drv->src 注入到 LCD.c（spi/dc/rst/blk 句柄）
 *   2. display_init() 将 drv->touch 注入到 ctp.c（i2c/it/rst 句柄）
 *   3. delay 回调通过 display_set_delay_cb() 注入
 */

#include "disp_drv.h"
#include "bsp_handle.h"
#include "LCD.h"
#include <stdio.h>
#include "touch.h"

/* ============================================================
 * [DI] display_init — 初始化 ST7796
 * ============================================================ */
void display_init(Disp_Drv_t *drv)
{
    ASSERT_FAIL(drv == NULL || drv->src == NULL, return);

    display_set_delay_cb(drv, drv->delay_cb);

    /* [DI] 将 Disp_Src_t 注入到 LCD.c */
    lcd_assign_src(drv->src);
    lcd_assign_delay(drv->delay_cb);

    /* [DI] 将 lcd_dev 注册为 Disp_Drv_t 的 priv */
    drv->priv = lcd_get_dev();

    LCD_Init();
    LCD_Display_Dir(0); /* 默认竖屏 */

    drv->width  = ((lcd_dev_t *)drv->priv)->width;
    drv->height = ((lcd_dev_t *)drv->priv)->height;

    /* [DI] 将 Disp_Touch_Src_t 注入到 ctp.c */
#if DISP_HAS_TOUCH
    if (drv->touch) {
        ctp_assign_i2c(drv->touch->i2c);
        ctp_assign_pins(drv->touch->rst, drv->touch->it);
        ctp_assign_delay(drv->delay_cb);

        TP_Init();
    }
#endif

    drv->is_initialized = 1;
}

/* ============================================================
 * display_deinit — 去初始化
 * ============================================================ */
void display_deinit(Disp_Drv_t *drv)
{
    ASSERT_FAIL(drv == NULL, return);
    drv->is_initialized = 0;
}

/* ============================================================
 * display_rst — 硬件复位 (通过 Disp_Src_t.rst_pin)
 * ============================================================ */
void display_rst(Disp_Drv_t *drv)
{
    ASSERT_FAIL(drv == NULL || drv->src == NULL, return);
    Disp_Src_t *src = drv->src;
    ASSERT_FAIL(src->rst_pin == NULL, return);

    gpio_write(src->rst_pin, GPIO_Level_Low);
    if (drv->delay_cb) drv->delay_cb(100);
    gpio_write(src->rst_pin, GPIO_Level_High);
    if (drv->delay_cb) drv->delay_cb(50);
}

/* ============================================================
 * 背光控制
 * ============================================================ */
void display_backlight_on(Disp_Drv_t *drv)
{
    ASSERT_FAIL(drv == NULL || drv->src == NULL, return);
    ASSERT_FAIL(drv->src->blk_pin == NULL, return);
    gpio_write(drv->src->blk_pin, GPIO_Level_High);
}

void display_backlight_off(Disp_Drv_t *drv)
{
    ASSERT_FAIL(drv == NULL || drv->src == NULL, return);
    ASSERT_FAIL(drv->src->blk_pin == NULL, return);
    gpio_write(drv->src->blk_pin, GPIO_Level_Low);
}

/* ============================================================
 * 区域 / 方向
 * ============================================================ */
void display_set_region(Disp_Drv_t *drv,
                        uint32_t x_start, uint32_t y_start,
                        uint32_t x_end, uint32_t y_end)
{
    GIVEUP(drv);
    LCD_Set_Window((uint16_t)x_start, (uint16_t)y_start,
                   (uint16_t)(x_end - x_start + 1),
                   (uint16_t)(y_end - y_start + 1));
}

void display_spin_screen(Disp_Drv_t *drv, uint8_t dir)
{
    GIVEUP(drv);
    LCD_Display_Dir(dir);
}

/* ============================================================
 * 清屏 / 填充
 * ============================================================ */
void display_clean_screen(Disp_Drv_t *drv, uint16_t color)
{
    GIVEUP(drv);
    LCD_Clear(color);
}

void display_fill_screen(Disp_Drv_t *drv,
                         uint32_t x_start, uint32_t y_start,
                         uint32_t x_end, uint32_t y_end,
                         uint16_t color)
{
    GIVEUP(drv);
    LCD_Fill((uint16_t)x_start, (uint16_t)y_start,
             (uint16_t)x_end, (uint16_t)y_end,
             color);
}

/* ============================================================
 * 光标 / 像素
 * ============================================================ */
void display_set_cursor(Disp_Drv_t *drv, uint32_t x, uint32_t y)
{
    GIVEUP(drv);
    LCD_SetCursor((uint16_t)x, (uint16_t)y);
}

void display_write_pixels(Disp_Drv_t *drv,
                          uint32_t x, uint32_t y,
                          uint32_t w, uint32_t h,
                          const uint16_t *pixels)
{
    GIVEUP(drv);
    LCD_Color_Fill((uint16_t)x, (uint16_t)y,
                   (uint16_t)(x + w - 1), (uint16_t)(y + h - 1),
                   (uint16_t *)pixels);
}

/* ============================================================
 * 基本图元
 * ============================================================ */
void display_draw_point(Disp_Drv_t *drv,
                        uint32_t x, uint32_t y, uint16_t color)
{
    GIVEUP(drv);
    ((lcd_dev_t *)drv->priv)->fc = color;
    LCD_Fast_DrawPoint((uint16_t)x, (uint16_t)y, color);
}

void display_draw_line(Disp_Drv_t *drv,
                       uint32_t x0, uint32_t y0,
                       uint32_t x1, uint32_t y1, uint16_t color)
{
    GIVEUP(drv);
    ((lcd_dev_t *)drv->priv)->fc = color;
    LCD_DrawLine((uint16_t)x0, (uint16_t)y0,
                 (uint16_t)x1, (uint16_t)y1);
}

void display_draw_circle(Disp_Drv_t *drv,
                         uint32_t x, uint32_t y,
                         uint32_t r, uint16_t color)
{
    GIVEUP(drv);
    ((lcd_dev_t *)drv->priv)->fc = color;
    LCD_Draw_Circle((uint16_t)x, (uint16_t)y, (uint8_t)r);
}

void display_draw_rect(Disp_Drv_t *drv,
                       uint32_t x, uint32_t y,
                       uint32_t w, uint32_t h, uint16_t color)
{
    GIVEUP(drv);
    ((lcd_dev_t *)drv->priv)->fc = color;
    LCD_DrawRectangle((uint16_t)x, (uint16_t)y,
                      (uint16_t)(x + w - 1), (uint16_t)(y + h - 1));
}

/* ============================================================
 * 文字
 * ============================================================ */
void display_draw_string(Disp_Drv_t *drv,
                         uint32_t x, uint32_t y,
                         uint16_t fc, uint16_t bc,
                         const char *str)
{
    GIVEUP(drv);
    ((lcd_dev_t *)drv->priv)->fc = fc;
    ((lcd_dev_t *)drv->priv)->bc = bc;
    LCD_ShowString((uint16_t)x, (uint16_t)y,
                   200, 200, 16, (uint8_t *)str);
}

void display_draw_number(Disp_Drv_t *drv,
                         uint32_t x, uint32_t y,
                         uint16_t fc, uint16_t bc,
                         int num)
{
    GIVEUP(drv);
    ((lcd_dev_t *)drv->priv)->fc = fc;
    ((lcd_dev_t *)drv->priv)->bc = bc;
    LCD_ShowNum((uint16_t)x, (uint16_t)y, (uint32_t)num, 10, 16);
}

/* ============================================================
 * display_test — 测试显示效果
 * ============================================================ */
void display_test(Disp_Drv_t *drv)
{
    GIVEUP(drv);
    LCD_Clear(RED);
    if (drv->delay_cb) drv->delay_cb(500);
    LCD_Clear(GREEN);
    if (drv->delay_cb) drv->delay_cb(500);
    LCD_Clear(BLUE);
    if (drv->delay_cb) drv->delay_cb(500);
    LCD_Clear(WHITE);
    if (drv->delay_cb) drv->delay_cb(500);

    ((lcd_dev_t *)drv->priv)->fc = RED;
    LCD_ShowString(10, 10, 200, 200, 16, (uint8_t *)"ST7796 + Touch OK");
}
