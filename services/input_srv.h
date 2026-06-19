#pragma once

/*
 * 输入服务层，处理各种输入事件
 */

#include <stdint.h>

typedef enum {
    NONE     = 0,
    PAGEUP   = 1,
    PAGEDOWN = 2,
    BACK     = 3,
    HOME     = 4,
    CONFIRM  = 5,
    TOUCH    = 6,
} InputEventId_t;

typedef enum {
    Button_Input = 0,
    Touch_Input,
} InputType_t;

typedef struct
{
    InputType_t event_type;
    InputEventId_t event_id;
    void *data;
} InputEvent_t;

extern InputEvent_t btn_event;

InputEventId_t get_input_event_id(InputEvent_t *e);
