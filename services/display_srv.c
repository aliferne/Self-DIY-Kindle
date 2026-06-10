#include "display_srv.h"
#include "tft.h"
#include "lv_port_disp.h"
#include <stdint.h>

#define Drv_
#define Drv_DrawPoint(x, y, color) TFT_DrawPoint()

/*
 * 本驱动假设只会使用一个屏幕
 * 屏幕的绘制可以通过修改相关函数内容来实现不同屏幕之间的切换
 * 但使用时只能有一个屏幕
 */

#define LV_DISP_BUF_SIZE MY_DISP_HOR_RES * 10
static lv_disp_drv_t drv;
static lv_disp_draw_buf_t disp_buf;
static lv_color_t _buf_1[LV_DISP_BUF_SIZE];
static lv_color_t _buf_2[LV_DISP_BUF_SIZE];

/* TODO: 实际上 LVGL 已经帮我们做好兼容层了，把这部分代码移植到三个 port 文件中！ */

/* 用于 LVGL 刷新回调 */
static void display_flush_cb(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    /*
     * the most simple case
     * TODO: 将其改为更加高效的方式
     */
    int32_t x, y;
    for (y = area->y1; y <= area->y2; y++) {
        for (x = area->x1; x <= area->x2; x++) {
            TFT_DrawPoint(&tft, x, y, color_p->full);
            color_p++;
        }
    }

    /*
     * IMPORTANT!
     * Inform the graphics library that you are ready with the flushing
     */
    lv_disp_flush_ready(disp_drv);
}

/**
 * \ref https://lvgl.100ask.net/8.3/porting/project.html#initialization
 *
 * - Step 1. Call `lv_init()`
 * - Step 2. Initialize the display and input drivers
 * - Step 3. Register the display and input drivers in LVGL
 * - Step 4. Call `lv_tick_inc()` periodically to inform LVGL about the elapsed time (e.g. with a timer interrupt)
 * - Step 5. Call `lv_task_handler()` periodically(within ms) to handle LVGL tasks (e.g. in the main loop)
 */
void display_init(void)
{
    /* Initialize LVGL */
    lv_init();
    /*
     * should initialize two variables:
     *  `lv_disp_draw_buf_t` & `lv_disp_drv_t`
     * they should be set as a static or global variable.
     * at least a buffer with the type of `lv_color_t` is also required
     */
    lv_disp_draw_buf_init(&disp_buf, _buf_1, _buf_2, LV_DISP_BUF_SIZE);
    /* should be initialized then registered */
    lv_disp_drv_init(&drv);
    /* no need for full refresh */
    drv.full_refresh = 0;
    drv.draw_buf     = &disp_buf;
    drv.hor_res      = MY_DISP_HOR_RES;
    drv.ver_res      = MY_DISP_VER_RES;
    drv.flush_cb     = display_flush_cb;
    lv_disp_drv_register(&drv);
}

void display_deinit(void)
{
    display_delight();
    TFT_Reset(&tft);
    TFT_DeInit(&tft);
}

/* 开启背光 */
void display_enlight(void)
{
    TFT_TurnOff(&tft, 1);
}

/* 关闭背光 */
void display_delight(void)
{
    TFT_TurnOff(&tft, 0);
}
