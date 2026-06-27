#pragma once

/*
 * 输入服务层，处理各种输入事件
 *
 * 触摸比较复杂，
 * 交由 lv_port_indev 处理，
 * 此外部分涉及到 UI 交互的也会给
 * lv_port_indev 处理，
 * 这里实现了一个最基本的模型
 */

#include <stdint.h>

typedef enum {
    NONE     = 0,
    PAGEUP   = 1,
    PAGEDOWN = 2,
    BACK     = 3,
    HOME     = 4,
    CONFIRM  = 5,
} input_id_t;

typedef enum {
    BUTTON_PRESS = 0,
    BUTTON_LONG_PRESS,
} input_type_t;

typedef struct input_event {
    /* 事件类型 */
    input_type_t event_type;
    /* 事件ID （标识事件的含义） */
    input_id_t event_id;
} input_event_t;

typedef void (*input_dispatcher_t)(input_event_t *e);

void input_srv_init(void);
void input_srv_handler(void);
void input_srv_register_dispatcher(input_dispatcher_t cb);
