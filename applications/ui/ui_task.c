#include "bsp_sys.h"
#include "bsp_handle.h"
#include "lv_obj_pos.h"
#include "lv_printf.h"
#include "lv_timer.h"
#include "lv_port_disp.h"
#include <stddef.h>
#include <stdio.h>

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
extern void ui_disp_settings_page(void);
extern void ui_disp_reader_page(void);
extern void ui_disp_player_page(void);
void flusher(lv_timer_t *timer);

void StartUITask(void const *argument)
{
    /* Initializations ------------- */
    lv_init();
    lv_port_disp_init();

    // lv_timer_t *timer = lv_timer_create(flusher, 1000, label);
    // 
    for (;;) {
        /*
         * FIXME: UI 局部更新时的绘制是破碎的，但是全局刷新（旋转屏幕）非常流畅
         */
        if (current_page == HOME_PAGE)
            ui_disp_home_page();

        lv_timer_handler();
        os_delay_ms(10);
    }
}

void flusher(lv_timer_t *timer)
{
    static int i       = 0;
    const char *txts[] = {"yes", "no", "ok", "confirm"};

    lv_obj_t *label = (lv_obj_t *)timer->user_data;
    lv_label_set_text_static(label, txts[i]);
    i = (i + 1) % LEN(txts);
}
