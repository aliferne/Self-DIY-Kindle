#include "ui_task.h"
#include "bsp_config.h"
#include "bsp_gpio.h"
#include "bsp_sys.h"
#include "cmsis_os.h"

#include "ImageData.h"
#include "epaper.h"
#include "epd_4in2_v2.h"
#include "gui_paint.h"

/* 外部 epaper 实例（定义在 Middlewares/epaper/epaper.c） */
extern EPaper_Model_t e_paper;

/* 帧缓冲区 — 静态分配，避免 malloc */
#define IMAGE_SIZE (((EPD_4IN2_V2_WIDTH % 8 == 0)         \
                         ? (EPD_4IN2_V2_WIDTH / 8)        \
                         : (EPD_4IN2_V2_WIDTH / 8 + 1)) * \
                    EPD_4IN2_V2_HEIGHT)

static UBYTE black_image[IMAGE_SIZE];

/* ============================================================
 * 辅助：绘制测试图形
 * ============================================================ */
static void draw_test_pattern(void)
{
    Paint_DrawPoint(&e_paper.painter, 10, 80, BLACK, DOT_PIXEL_1X1, DOT_STYLE_DFT);
    Paint_DrawPoint(&e_paper.painter, 10, 90, BLACK, DOT_PIXEL_2X2, DOT_STYLE_DFT);
    Paint_DrawPoint(&e_paper.painter, 10, 100, BLACK, DOT_PIXEL_3X3, DOT_STYLE_DFT);
    Paint_DrawLine(&e_paper.painter, 20, 70, 70, 120, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(&e_paper.painter, 70, 70, 20, 120, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawRectangle(&e_paper.painter, 20, 70, 70, 120, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawRectangle(&e_paper.painter, 80, 70, 130, 120, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawCircle(&e_paper.painter, 45, 95, 20, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawCircle(&e_paper.painter, 105, 95, 20, WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawLine(&e_paper.painter, 85, 95, 125, 95, BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
    Paint_DrawLine(&e_paper.painter, 105, 75, 105, 115, BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
    Paint_DrawString_EN(&e_paper.painter, 10, 0, "waveshare", &Font16, BLACK, WHITE);
    Paint_DrawString_EN(&e_paper.painter, 10, 20, "hello world", &Font12, WHITE, BLACK);
    Paint_DrawNum(&e_paper.painter, 10, 33, 123456789, &Font12, BLACK, WHITE);
    Paint_DrawNum(&e_paper.painter, 10, 50, 987654321, &Font16, WHITE, BLACK);
    Paint_DrawString_CN(&e_paper.painter, 130, 0, " 你好abc", &Font12CN, BLACK, WHITE);
    Paint_DrawString_CN(&e_paper.painter, 130, 20, "微雪电子", &Font24CN, WHITE, BLACK);
}

/* ============================================================
 * 辅助：局部刷新的数字时钟
 * ============================================================ */
static void draw_partial_clock(void)
{
    UBYTE clock_buf[200 * 50 / 8]; /* 200x50 局部缓冲区，1bpp */
    PAINT_TIME t;

    Paint_NewImage(&e_paper.painter, clock_buf, 200, 50, 0, WHITE);

    t.Hour = 12;
    t.Min  = 34;
    t.Sec  = 56;

    for (UBYTE n = 0; n < 10; n++) {
        t.Sec++;
        if (t.Sec == 60) {
            t.Sec = 0;
            t.Min++;
            if (t.Min == 60) {
                t.Min = 0;
                t.Hour++;
                if (t.Hour == 24)
                    t.Hour = 0;
            }
        }

        Paint_Clear(&e_paper.painter, WHITE);
        Paint_DrawTime(&e_paper.painter, 20, 10, &t, &Font20, WHITE, BLACK);
        EPD_4IN2_V2_PartialDisplay(&e_paper, clock_buf, 80, 200, 280, 250);
        os_delay_ms(600);
    }
}

void test(void);

void StartUITask(void const *argument)
{
    test();
    /* Task 主循环：UI 平时只做低功耗等待 */
    for (;;) {
        os_delay_ms(1000);
    }
}

void test()
{
    EPaper_Model_t *e  = &e_paper;
    Painter_Model_t *p = &e->painter;

    EPD_4IN2_V2_Init_Normal(e);
    EPD_4IN2_V2_Clear(e);
    os_delay_ms(600);

    Paint_NewImage(p, black_image, EPD_4IN2_V2_WIDTH, EPD_4IN2_V2_HEIGHT, 0, WHITE);
    
    /* show bmp -------------------------- */
    // Paint_SelectImage(p, black_image);
    // Paint_Clear(p, WHITE);
    // Paint_DrawBitMap(p, gImage_4in2);
    // /* FIXME: 显示图像有些不正常 */
    // EPD_4IN2_V2_Display(e, black_image);
    // os_delay_ms(4000);

    /* show test pattern ----------------- */
    EPD_4IN2_V2_Init_Fast(e, Seconds_1S);
    Paint_SelectImage(p, black_image);
    Paint_Clear(p, WHITE);
    draw_test_pattern();
    EPD_4IN2_V2_Display(e, black_image);
    os_delay_ms(4000);

    gpio_toggle(&usr_led);
    epaper_sleep(&e_paper, 0);
    os_delay_ms(4000);

    EPD_4IN2_V2_Init_Fast(e, Seconds_1S);
    Paint_SelectImage(p, black_image);
    // Paint_Clear(p, WHITE);
    Paint_DrawString_EN(&e_paper.painter, 10, 200, "waveshare", &Font16, BLACK, WHITE);
    EPD_4IN2_V2_Display(e, black_image);
    os_delay_ms(4000);

    draw_partial_clock();

    gpio_toggle(&usr_led);
    epaper_sleep(&e_paper, 0);
    os_delay_ms(4000);
}
