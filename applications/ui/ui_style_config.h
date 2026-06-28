#include "lv_port_disp.h"

/* 夜晚模式 */
#define NIGHT_MODE 0

/* TODO: 需要想下如何配置白天黑夜的切换 */

#if NIGHT_MODE == 0
#define UI_TEXT_COLOR      LV_DISP_BLACK
#define UI_BG_COLOR        LV_DISP_WHITE
#define UI_UNDERLINE_COLOR LV_DISP_GRAY0
#else

#endif
