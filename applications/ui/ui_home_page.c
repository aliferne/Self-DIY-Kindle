#include "lv_area.h"
#include "lv_color.h"
#include "lv_disp.h"
#include "lv_label.h"
#include "lv_btn.h"
#include "lv_port_disp.h"
#include "lv_obj.h"
#include "lv_style.h"
#include "ui_style_config.h"
#include "lv_timer.h"
#include "lv_obj_pos.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

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

/* status bar part --------------------------- */
#define STATUS_BAR_HEIGHT (40)
#define STATUS_BAR_COLOR  (LV_DISP_RED)

static lv_obj_t *status_bar;
static lv_obj_t *time_label;
static lv_obj_t *battery_label;
static lv_obj_t *wifi_label;

/* main func part --------------------------- */
#define MAIN_FUNC_LIST_HEIGHT (SCR_HEIGHT - STATUS_BAR_HEIGHT)
#define MAIN_FUNC_LIST_COLOR  (LV_DISP_WHITE)
#define LIST_ITEM_HEIGHT      (MAIN_FUNC_LIST_HEIGHT / 5)

static lv_obj_t *main_func_list_label;
static lv_obj_t *bookshelf_btn;
static lv_obj_t *music_btn;
static lv_obj_t *net_btn;
static lv_obj_t *game_btn;
static lv_obj_t *settings_btn;

/* 点击事件回调 */
static void main_func_item_click_cb(lv_event_t *e)
{
    lv_obj_t *btn = lv_event_get_target(e);
    /* 后续可加上页面切换逻辑 */
}

/* "hh:mm:ss", 9 char in total, default to 00:00:00 */
static char time_txt[9] = "00:00:00";

static void draw_status_bar(void);
static void draw_main_func_list(void);
static void update_status_bar(void);
static bool home_page_inited = false;

void ui_disp_home_page(void)
{
    if (home_page_inited == false) {
        draw_status_bar();
        draw_main_func_list();

        home_page_inited = true;
    }
    update_status_bar();
}

static void draw_status_bar(void)
{
    /* status bar disp ---------------------- */
    status_bar = lv_obj_create(lv_scr_act());
    lv_obj_set_size(status_bar, SCR_WIDTH, STATUS_BAR_HEIGHT);
    lv_obj_set_pos(status_bar, 0, 0);
    lv_obj_set_style_bg_color(status_bar, STATUS_BAR_COLOR, 0);
    lv_obj_set_style_border_width(status_bar, 0, 0);
    lv_obj_set_style_radius(status_bar, 0, 0);
    lv_obj_clear_flag(status_bar, LV_OBJ_FLAG_SCROLLABLE);

    /* timer disp --------------------------- */
    time_label = lv_label_create(status_bar);
    lv_label_set_text_static(time_label, time_txt);
    lv_obj_set_style_text_color(time_label, UI_TEXT_COLOR, 0);
    lv_obj_set_pos(time_label, 10, 5);

    /* logo disp ---------------------------- */
    lv_obj_t *dev_label = lv_label_create(status_bar);
    lv_label_set_text(dev_label, "My Reader");
    lv_obj_set_style_text_color(dev_label, UI_TEXT_COLOR, 0);
    lv_obj_center(dev_label);

    /* netstat disp ------------------------- */
    wifi_label = lv_label_create(status_bar);
    lv_label_set_text(wifi_label, "📶"); // 可用符号或图标
    lv_obj_set_style_text_color(wifi_label, UI_TEXT_COLOR, 0);
    lv_obj_align(wifi_label, LV_ALIGN_TOP_RIGHT, -30, 5);

    /* battery disp ------------------------- */
    battery_label = lv_label_create(status_bar);
    lv_label_set_text(battery_label, "🔋 85%");
    lv_obj_set_style_text_color(battery_label, UI_TEXT_COLOR, 0);
    lv_obj_align(battery_label, LV_ALIGN_TOP_RIGHT, -5, 5);
}

/* TODO: 感觉这个创建列表的可以抽象到一个地方去 */
static lv_obj_t *create_list_item(lv_obj_t *parent, const char *text, int index)
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_remove_style_all(btn); // 重新设置一下样式
    lv_obj_set_size(btn, SCR_WIDTH, LIST_ITEM_HEIGHT);
    lv_obj_set_pos(btn, 0, index * LIST_ITEM_HEIGHT);
    lv_obj_set_style_bg_color(btn, UI_BG_COLOR, 0);
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_set_style_radius(btn, 0, 0);
    lv_obj_clear_flag(btn, LV_OBJ_FLAG_SCROLLABLE);

    /* 下方分割线 */
    lv_obj_set_style_border_width(btn, 1, 0);
    lv_obj_set_style_border_side(btn, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_color(btn, UI_UNDERLINE_COLOR, 0);

    /* TODO: 添加事件回调 */
    lv_obj_add_event_cb(btn, main_func_item_click_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, UI_TEXT_COLOR, 0);
    lv_obj_center(label);

    return btn;
}

static void draw_main_func_list(void)
{
    /* background bar disp -------------------- */
    main_func_list_label = lv_obj_create(lv_scr_act());
    lv_obj_set_size(main_func_list_label, SCR_WIDTH, MAIN_FUNC_LIST_HEIGHT);
    lv_obj_set_pos(main_func_list_label, 0, STATUS_BAR_HEIGHT);
    lv_obj_set_style_bg_color(main_func_list_label, MAIN_FUNC_LIST_COLOR, 0);
    lv_obj_set_style_border_width(main_func_list_label, 0, 0);
    lv_obj_set_style_radius(main_func_list_label, 0, 0);
    lv_obj_clear_flag(main_func_list_label, LV_OBJ_FLAG_SCROLLABLE);

    /* bookshelf disp ------------------------- */
    // bookshelf_btn = create_list_item(main_func_list_label, "📚  书架", 0);
    // bookshelf_btn = create_list_item(main_func_list_label, "书架", 0);
    bookshelf_btn = create_list_item(main_func_list_label, "bookshelf", 0);

    /* music disp ----------------------------- */
    // music_btn = create_list_item(main_func_list_label, "🎵  音乐", 1);
    // music_btn = create_list_item(main_func_list_label, "音乐", 1);
    music_btn = create_list_item(main_func_list_label, "music", 1);

    /* net disp ------------------------------- */
    // net_btn = create_list_item(main_func_list_label, "🌐  网络", 2);
    // net_btn = create_list_item(main_func_list_label, "网络", 2);
    net_btn = create_list_item(main_func_list_label, "net", 2);

    /* game disp ------------------------------ */
    // game_btn = create_list_item(main_func_list_label, "🎮  游戏", 3);
    // game_btn = create_list_item(main_func_list_label, "游戏", 3);
    game_btn = create_list_item(main_func_list_label, "game", 3);

    /* setting disp --------------------------- */
    // settings_btn = create_list_item(main_func_list_label, "⚙️  设置", 4);
    // settings_btn = create_list_item(main_func_list_label, "设置", 4);
    settings_btn = create_list_item(main_func_list_label, "settings", 4);
}

static void update_status_bar(void)
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

    // FIXME: 由于刷新导致画面撕裂，这里暂时不刷新
    // snprintf(time_txt, 9, "%02d:%02d:%02d", hour, minute, 0);
    // lv_label_set_text_static(time_label, time_txt);

    // // 演示电量随机变化
    // static int batt = 85;
    // batt            = (batt + 1) % 100;
    // lv_label_set_text_fmt(battery_label,"🔋 %d%%", batt);

    // // Wi-Fi状态可读取ESP8266返回的信息更新
    // lv_label_set_text(wifi_label, wifi_connected ? "📶" : "🚫");
}
