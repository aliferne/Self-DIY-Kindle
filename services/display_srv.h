#pragma once

#include "tft.h"
#include "mid_config.h"

void display_init(void);
void display_deinit(void);

/* 开启背光 */
static inline void display_enlight(void)
{
    TFT_TurnOff(&tft, 1);
}

/* 关闭背光 */
static inline void display_delight(void)
{
    TFT_TurnOff(&tft, 0);
}
