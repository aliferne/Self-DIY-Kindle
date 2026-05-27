#include "ui_task.h"
#include "cmsis_os.h"
#include "bsp_sys.h"

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
/* 四级灰度需要 2bpp，缓冲区大小翻倍 */
UBYTE gray_buf[((EPD_4IN2_V2_WIDTH % 4 == 0)
                    ? (EPD_4IN2_V2_WIDTH / 4)
                    : (EPD_4IN2_V2_WIDTH / 4 + 1)) *
               EPD_4IN2_V2_HEIGHT];

static UBYTE black_image[IMAGE_SIZE];

/* ============================================================
 * 辅助：绘制测试图形
 * ============================================================ */
static void draw_test_pattern(void)
{
    Paint_DrawPoint(&e_paper.paint, 10, 80, BLACK, DOT_PIXEL_1X1, DOT_STYLE_DFT);
    Paint_DrawPoint(&e_paper.paint, 10, 90, BLACK, DOT_PIXEL_2X2, DOT_STYLE_DFT);
    Paint_DrawPoint(&e_paper.paint, 10, 100, BLACK, DOT_PIXEL_3X3, DOT_STYLE_DFT);
    Paint_DrawLine(&e_paper.paint, 20, 70, 70, 120, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawLine(&e_paper.paint, 70, 70, 20, 120, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
    Paint_DrawRectangle(&e_paper.paint, 20, 70, 70, 120, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawRectangle(&e_paper.paint, 80, 70, 130, 120, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawCircle(&e_paper.paint, 45, 95, 20, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    Paint_DrawCircle(&e_paper.paint, 105, 95, 20, WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    Paint_DrawLine(&e_paper.paint, 85, 95, 125, 95, BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
    Paint_DrawLine(&e_paper.paint, 105, 75, 105, 115, BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);
    Paint_DrawString_EN(&e_paper.paint, 10, 0, "waveshare", &Font16, BLACK, WHITE);
    Paint_DrawString_EN(&e_paper.paint, 10, 20, "hello world", &Font12, WHITE, BLACK);
    Paint_DrawNum(&e_paper.paint, 10, 33, 123456789, &Font12, BLACK, WHITE);
    Paint_DrawNum(&e_paper.paint, 10, 50, 987654321, &Font16, WHITE, BLACK);
    Paint_DrawString_CN(&e_paper.paint, 130, 0, " 你好abc", &Font12CN, BLACK, WHITE);
    Paint_DrawString_CN(&e_paper.paint, 130, 20, "微雪电子", &Font24CN, WHITE, BLACK);
}

/* ============================================================
 * 辅助：局部刷新的数字时钟
 * ============================================================ */
static void draw_partial_clock(void)
{
    UBYTE clock_buf[200 * 50 / 8]; /* 200x50 局部缓冲区，1bpp */
    PAINT_TIME t;

    Paint_NewImage(&e_paper.paint, clock_buf, 200, 50, 0, WHITE);

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

        Paint_Clear(&e_paper.paint, WHITE);
        Paint_DrawTime(&e_paper.paint, 20, 10, &t, &Font20, WHITE, BLACK);
        EPD_4IN2_V2_PartialDisplay(&e_paper, clock_buf, 80, 200, 280, 250);
        os_delay_ms(500);
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
    EPaper_Err_t err;
    /* --- 等待硬件初始化完成 --- */
    os_delay_ms(100);

    /* --- 全屏清屏 --- */
    err = EPD_4IN2_V2_Init(&e_paper);
    if (err != EPaper_Err_OK)
        return;

    err = EPD_4IN2_V2_Clear(&e_paper);
    if (err != EPaper_Err_OK)
        return;

    os_delay_ms(500);

    /* ============================================================
     * 测试 1：绘制图形和文字（标准刷新）
     * ============================================================ */
    Paint_NewImage(&e_paper.paint, black_image, EPD_4IN2_V2_WIDTH, EPD_4IN2_V2_HEIGHT, 0, WHITE);
    Paint_SelectImage(&e_paper.paint, black_image);
    Paint_Clear(&e_paper.paint, WHITE);

    EPD_4IN2_V2_Display(&e_paper, black_image);
    os_delay_ms(3000);

    /* ============================================================
     * 测试 2：快速刷新
     * ============================================================ */
    EPD_4IN2_V2_Init_Fast(&e_paper, Seconds_1S);
    Paint_Clear(&e_paper.paint, WHITE);

    draw_test_pattern();

    EPD_4IN2_V2_Display_Fast(&e_paper, black_image);
    os_delay_ms(3000);

    // /* ============================================================
    //  * 测试 3：正常刷新（再次）
    //  * ============================================================ */
    // EPD_4IN2_V2_Init(&e_paper);
    // Paint_Clear(RED);

    // draw_test_pattern();

    // EPD_4IN2_V2_Display(&e_paper, black_image);
    // os_delay_ms(3000);

    // /* ============================================================
    //  * 测试 4：局部刷新 — 模拟时钟
    //  * ============================================================ */
    // draw_partial_clock();

    // EPD_4IN2_V2_Display(&e_paper, black_image);
    // os_delay_ms(3000);

    // /* ============================================================
    //  * 收尾：清屏 + 休眠
    //  * ============================================================ */
    // EPD_4IN2_V2_Init(&e_paper);
    // EPD_4IN2_V2_Clear(&e_paper);
    // EPD_4IN2_V2_Sleep(&e_paper);
}
