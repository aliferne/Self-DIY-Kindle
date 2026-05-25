#pragma once

/*
 * 输入服务层，处理各种输入事件
 */

#include <stdint.h>

typedef enum {
    Btn_ID_None     = 0,
    Btn_ID_PageUp   = 1,
    Btn_ID_PageDown = 2,
    Btn_ID_Back     = 3,
    Btn_ID_Home     = 4,
    Btn_ID_Confirm  = 5,
} Btn_ID_t;

typedef enum {
    /* 板子上的按键按下 */
    InputEventType_BtnPress,
    /* 专指键盘按下 */
    InputEventType_KeyPress,
} InputEventType_t;

typedef struct {
    InputEventType_t type;
    /* 事件数据，含义视具体事件而定 */
    union {
        struct {
            Btn_ID_t btn_id;
        } btn_press;
        struct {
            uint8_t key_code;
        } key_press;
    } data;
} InputEvent_t;

extern InputEvent_t btn_event;

void handle_input_events(InputEvent_t *event);
