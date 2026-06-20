#include "input_srv.h"
#include "bsp_config.h"
#include "fifo.h"

/* TODO: EXTI 中断不正常，此外这里的如何产生输入事件也应当斟酌一下 */
#define HANDLE_BUTTON_PRESS(btn, event, id)    \
    do {                                       \
        if (btn.irq_flag) {                    \
            gpio_clear_irq_flag(&btn);         \
            event.event_id = id;               \
        }                                      \
        if (event.event_id != NONE)            \
            fifo_push(&event_fifo, &event, 0); \
        event.event_id = NONE;                 \
    } while (0)

static void detect_button_press(void);

/* 事件队列，用于存储输入事件 */
fifo_t event_fifo;
input_event_t event_queue[10];
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
    fifo_init(&event_fifo, event_queue, sizeof(event_queue), sizeof(input_event_t));
}

/*
 * 输入服务处理函数，
 * 检测按钮按下事件并将事件放入事件队列，
 * 需要放置在循环中
 */
void input_srv_handler(void)
{
    /* 检测数据（生产者） */
    detect_button_press();
    /* 处理事件队列中的事件（消费者） */
    while (!fifo_is_empty(&event_fifo)) {
        input_event_t event;
        fifo_pop(&event_fifo, &event);

        if (dispatcher != NULL) {
            dispatcher(&event);
        }
    }
}

/* 处理按钮按下事件 */
static void detect_button_press(void)
{
    input_event_t event = {
        .event_type = Button_Input,
        .event_id   = NONE,
    };

    HANDLE_BUTTON_PRESS(pgup_btn, event, PAGEUP);
    HANDLE_BUTTON_PRESS(pgdown_btn, event, PAGEDOWN);
    HANDLE_BUTTON_PRESS(back_btn, event, BACK);
    HANDLE_BUTTON_PRESS(home_btn, event, HOME);
    HANDLE_BUTTON_PRESS(confirm_btn, event, CONFIRM);
}
