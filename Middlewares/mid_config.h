#pragma once

/*
 * 中间层配置相关
 */

#include "disp_drv.h"
#include "touch_drv.h"

extern disp_drv_t display;
extern touch_drv_t touch;

void mid_init_modules(void);
