#include "bsp_sys.h"
#include "lv_timer.h"
#include "lv_port_disp.h"
#include <stdio.h>

LV_FONT_DECLARE(simhei_size14)
LV_FONT_DECLARE(simhei_size16)
LV_FONT_DECLARE(simhei_size18)

/*
 * 显示页面
 */
typedef enum {
    HOME_PAGE = 0,
    SETTINGS_PAGE,
    PLAYER_PAGE,
    READER_PAGE,
    NET_PAGE,
    GAME_PAGE,
} ui_page_t;

static ui_page_t current_page = HOME_PAGE;

extern void ui_disp_home_page(void);
extern void ui_update_home_page(void);

extern void ui_disp_settings_page(void);
extern void ui_disp_reader_page(void);
extern void ui_disp_player_page(void);

void StartUITask(void const *argument)
{
    /* Initializations ------------- */
    lv_init();
    lv_port_disp_init();

    if (current_page == HOME_PAGE)
        ui_disp_home_page();

    // static lv_style_t style;
    // lv_style_init(&style);
    // lv_style_set_radius(&style, 5);

    // /*Make a gradient*/
    // lv_style_set_width(&style, 128);
    // lv_style_set_height(&style, LV_SIZE_CONTENT);

    // lv_style_set_pad_ver(&style, 0);
    // lv_style_set_pad_left(&style, 0);

    // lv_style_set_x(&style, lv_pct(0));
    // lv_style_set_y(&style, 0);

    // /*Create an object with the new style*/
    // lv_obj_t *obj = lv_obj_create(lv_scr_act());
    // lv_obj_add_style(obj, &style, 0);

    // lv_obj_t *label = lv_label_create(obj);
    /* FIXME: 目前看来可能是配置字体失效，导致无法显示，先用英文画 UI 吧 */
    // lv_obj_set_style_text_font(label, &simhei_size14, 0);

    // lv_label_set_text(label, "confirm");

    for (;;) {
        /* FIXME: UI 的绘制是破碎的 */
        if (current_page == HOME_PAGE)
            ui_update_home_page();

        lv_timer_handler();
        os_delay_ms(10);
    }
}
