#include "input_srv.h"
#include "bsp_config.h"
#include "bsp_gpio.h"
#include "bsp_sys.h"
#include "fifo.h"
#include "ebtn.h"
#include "bsp_handle.h"

/*
 * 去抖时间 20ms
 * 释放去抖 0ms
 * 点击时间 20ms - 300ms
 * 最长单击时间 300ms，超过不算单击
 * 连击间隔 200ms
 * 长按保持周期 500ms
 * 最大连击次数 0 （不处理连击）
 */
static const ebtn_btn_param_t btn_param = EBTN_PARAMS_INIT(20, 0, 20, 300, 200, 500, 0);
static ebtn_btn_t btns[]                = {
    EBTN_BUTTON_INIT(HOME, &btn_param),
    EBTN_BUTTON_INIT(PAGEDOWN, &btn_param),
    EBTN_BUTTON_INIT(PAGEUP, &btn_param),
    EBTN_BUTTON_INIT(CONFIRM, &btn_param),
    EBTN_BUTTON_INIT(BACK, &btn_param),
};

static uint8_t input_srv_get_btn_state(ebtn_btn_t *ebtn);
static void input_srv_evt_cb(ebtn_btn_t *btn, ebtn_evt_t evt);

/* 事件队列，用于存储输入事件 */
fifo_t event_fifo;
input_event_t event_queue[24];
/*
 * 回调（消费者）
 * 注册的函数示例为：
 *
 * ```c
 * void dispatcher(InputEvent_t *e)
 * {
 *     switch (e->event_id) {
 *         case ...:
 *             break;
 *         ...
 *         default:
 *             break;
 *     }
 * }
 * ```
 */
input_dispatcher_t dispatcher;

/*
 * 注册的函数示例为：
 *
 * ```c
 * void dispatcher(InputEvent_t *e)
 * {
 *     switch (e->event_id) {
 *         case ...:
 *             break;
 *         ...
 *         default:
 *             break;
 *     }
 * }
 * ```
 */
void input_srv_register_dispatcher(input_dispatcher_t cb)
{
    dispatcher = cb;
}

/* 初始化输入服务 */
void input_srv_init(void)
{
    fifo_init(
        &event_fifo, event_queue, LEN(event_queue),
        sizeof(input_event_t));

    ebtn_init(
        btns,
        EBTN_ARRAY_SIZE(btns),
        NULL, 0,
        input_srv_get_btn_state,
        input_srv_evt_cb);
}

/*
 * 输入服务处理函数，
 * 检测按钮按下事件并将事件放入事件队列，
 * 需要放置在循环中
 */
void input_srv_handler(void)
{
    /* 检测数据（生产者） */
    ebtn_process(chip_get_tick());

    /* 处理事件队列中的事件（消费者） */
    while (!fifo_is_empty(&event_fifo)) {
        input_event_t event;
        fifo_pop(&event_fifo, &event);

        if (dispatcher != NULL) {
            dispatcher(&event);
        }
    }
}

/* 读取按键状态 */
static uint8_t input_srv_get_btn_state(ebtn_btn_t *ebtn)
{
    /* NOTE: 这里要求按键必须默认是高电平状态， 否则逻辑可能不正常 */

    GPIO_Level_t btn_stat = GPIO_Level_Low;

    switch (ebtn->key_id) {
        // case HOME:
        //     btn_stat = !gpio_read(&home_btn);
        //     break;
        // case PAGEUP:
        //     btn_stat = !gpio_read(&pgup_btn);
        //     break;
        // case PAGEDOWN:
        //     btn_stat = !gpio_read(&pgdown_btn);
        //     break;
        // case CONFIRM:
        //     btn_stat = !gpio_read(&confirm_btn);
        //     break;
        /* TODO: 按键测试正常，但响应速度过低 */
        case BACK:
            btn_stat = !gpio_read(&back_btn);
            break;
        default:
            return 0;
    }

    return (uint8_t)btn_stat;
}

/* 事件回调函数 */
static void input_srv_evt_cb(ebtn_btn_t *btn, ebtn_evt_t evt)
{
    /* 目前先处理点击事件，还会有其他的事件（如长按） */

    input_event_t event = {
        .event_id = btn->key_id,
    };

    if (evt == EBTN_EVT_ONCLICK) {
        event.event_type = BUTTON_PRESS;
    } else if (evt == EBTN_EVT_KEEPALIVE) {
        event.event_type = BUTTON_LONG_PRESS;
    } else {
        return;
    }

    fifo_push(&event_fifo, &event, 0);
}
