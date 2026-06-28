#include "lv_port_disp.h"

LV_FONT_DECLARE(simhei_size14)
LV_FONT_DECLARE(simhei_size16)
LV_FONT_DECLARE(simhei_size18)

/* 夜晚模式 */
#define NIGHT_MODE 0

/* TODO: 需要想下如何配置白天黑夜的切换，我想后面可以直接把配置文件写入 SD 卡 */

#if NIGHT_MODE == 0
#define UI_TEXT_COLOR      LV_DISP_BLACK
#define UI_BG_COLOR        LV_DISP_WHITE
#define UI_UNDERLINE_COLOR LV_DISP_GRAY0
#else

#endif

/* 设置字体为 simhei，当前默认设置为 size14 */
static inline void ui_set_simhei_font(lv_obj_t *label)
{
    /* TODO: 后面应当仿照 Flutter 的 AppTheme 配置统一管理 */
    lv_obj_set_style_text_font(label, &simhei_size14, 0);
}
