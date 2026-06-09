#include "display_srv.h"
#include "bsp_gpio.h"

void display_init(void)
{
    /* TODO: 总觉得对于所有需要 init 的都应该有个标志位指示一下，避免重复初始化 */
    // TFT_Init(&tft);
    display_enlight();
}

void display_deinit(void)
{
    display_delight();
    TFT_Reset(&tft);
    TFT_DeInit(&tft);
}
