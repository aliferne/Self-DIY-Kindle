#include "lv_color.h"
#include "lv_disp.h"
#include "lv_port_disp.h"
#include "lv_obj.h"
#include "lv_style.h"
#include "lv_timer.h"
#include "lv_obj_pos.h"
#include <stdio.h>

/*
 * 主页的设计
 *
 * ┌──────────────────────────────┐
 * │ 14:30      Logo       🔋 85% │  <- 顶部状态栏
 * ├──────────────────────────────┤
 * │     📚 书架                  │
 * │     🎵 音乐                  │
 * │     🌐 网络                  │
 * │     🎮 游戏                  │
 * │     ⚙️ 设置                  │
 * └──────────────────────────────┘
 *
 * 因此需要拆分成几个块：
 * 1. status bar (状态栏)
 *  - [text] timer
 *  - [text] logo
 *  - [text] battery
 * 2. main func list (主功能列表)
 *  - [text] bookshelf
 *  - [text] music
 *  - [text] net
 *  - [text] game
 *  - [text] settings
 */

#define STATUS_BAR_HEIGHT 40

static lv_obj_t *status_bar;
static lv_obj_t *time_label;
static lv_obj_t *battery_label;
static lv_obj_t *wifi_label;

static void draw_status_bar(void);
static void draw_main_func_list(void);
static bool home_page_inited = false;

void ui_disp_home_page(void)
{
    if (home_page_inited == false) {
        draw_status_bar();
        draw_main_func_list();

        home_page_inited = true;
    }
    // ui_update_home_page();
}

static void draw_status_bar(void)
{
    /* status bar disp ---------------------- */
    status_bar = lv_obj_create(lv_scr_act());
    lv_obj_set_size(status_bar, SCR_WIDTH, STATUS_BAR_HEIGHT);
    lv_obj_set_pos(status_bar, 0, 0);
    lv_obj_set_style_bg_color(status_bar, LV_DISP_RED, 0);
    lv_obj_set_style_border_width(status_bar, 0, 0);
    lv_obj_set_style_radius(status_bar, 0, 0);
    lv_obj_clear_flag(status_bar, LV_OBJ_FLAG_SCROLLABLE);

    /* timer disp --------------------------- */
    time_label = lv_label_create(status_bar);
    lv_label_set_text(time_label, "12:00");
    lv_obj_set_style_text_color(time_label, lv_color_black(), 0);
    lv_obj_set_pos(time_label, 10, 5);

    /* logo disp ---------------------------- */
    lv_obj_t *dev_label = lv_label_create(status_bar);
    lv_label_set_text(dev_label, "My Reader");
    lv_obj_set_style_text_color(dev_label, lv_color_black(), 0);
    lv_obj_center(dev_label);

    /* netstat disp ------------------------- */
    wifi_label = lv_label_create(status_bar);
    lv_label_set_text(wifi_label, "📶"); // 可用符号或图标
    lv_obj_set_style_text_color(wifi_label, lv_color_black(), 0);
    lv_obj_align(wifi_label, LV_ALIGN_TOP_RIGHT, -30, 5);

    /* battery disp ------------------------- */
    battery_label = lv_label_create(status_bar);
    lv_label_set_text(battery_label, "🔋 85%");
    lv_obj_set_style_text_color(battery_label, lv_color_black(), 0);
    lv_obj_align(battery_label, LV_ALIGN_TOP_RIGHT, -5, 5);
}

static void draw_main_func_list(void)
{
    /* bookshelf disp ------------------------- */
    /* music disp ----------------------------- */
    /* net disp ------------------------------- */
    /* game disp ------------------------------ */
    /* setting disp --------------------------- */
}

void ui_update_home_page(void)
{
    // 实际项目中从RTC获取时间
    uint8_t wifi_connected = 0;
    static int hour = 12, minute = 0;
    minute++;
    if (minute >= 60) {
        minute = 0;
        hour++;
        if (hour >= 24) hour = 0;
    }
    char buf[10];
    snprintf(buf, sizeof(buf), "%02d:%02d", hour, minute);
    lv_label_set_text(time_label, buf);

    // 演示电量随机变化
    static int batt = 85;
    batt            = (batt + 1) % 100;
    snprintf(buf, sizeof(buf), "🔋 %d%%", batt);
    lv_label_set_text(battery_label, buf);

    // Wi-Fi状态可读取ESP8266返回的信息更新
    lv_label_set_text(wifi_label, wifi_connected ? "📶" : "🚫");
}
