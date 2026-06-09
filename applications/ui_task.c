#include "ui_task.h"
#include "bsp_sys.h"
#include "cmsis_os.h"
#include "display_srv.h"

int colr[20] = {0xf800, 0x07e0, 0x001f, 0x1c9f, 0x8811, 0xd8a7, 0xfa20, 0xffff, 0xFFE0, 0x07ff, 0xf81f, 0xdb92};

void StartUITask(void const *argument)
{
    display_init();

    /* TODO: 为了保证后续方便改屏幕，实际上是不可以直接使用屏幕 API 的，现在先糙一点 */
    /* Task 主循环：UI 平时只做低功耗等待 */
    for (;;) {
        for (int i = 1; i < 5; i++) {
            TFT_ShowChinese(&tft, 0, 0, colr[i - 1], TFT_BLACK, "Chinese sample:");
            TFT_ShowChinese(&tft, 0, 16, colr[i], TFT_BLACK, "我是一只猫快乐的星猫从来没烦恼你快乐就好");
            os_delay_ms(50);
            TFT_Clear(&tft, TFT_BLACK);
        }

        TFT_ShowChinese(&tft, 0, 0, TFT_BLUE2, TFT_BLACK, "Mix sample");
        TFT_ShowChinese(&tft, 0, 16, TFT_PURPLE, TFT_BLACK, "我是一只猫2525,快乐的星猫3434~从来没烦恼,你快乐就好!2233445,ahahahhahah");
        os_delay_ms(700);

        TFT_Clear(&tft, TFT_BLACK);
        TFT_ShowChinese(&tft, 0, 0, TFT_PURPLE3, TFT_BLACK, "special_font");
        TFT_ShowChinese(&tft, 0, 32, TFT_ORANGE, TFT_BLACK, "你是光");
        TFT_ShowChinese(&tft, 0, 64, TFT_CYAN, TFT_BLACK, "你是电");
        TFT_ShowChinese(&tft, 0, 96, TFT_PURPLE2, TFT_BLACK, "你是唯一的信仰");
        os_delay_ms(1000);
        TFT_Clear(&tft, TFT_BLACK);
        os_delay_ms(1000);
    }
}
