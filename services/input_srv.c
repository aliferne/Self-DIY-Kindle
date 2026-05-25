#include "input_srv.h"
#include "bsp_config.h"

static void handle_button_press(InputEvent_t *btn_event);

InputEvent_t btn_event = {
    .type                  = InputEventType_BtnPress,
    .data.btn_press.btn_id = Btn_ID_None,
};

void handle_input_events(InputEvent_t *event)
{
    switch (event->type) {
        case InputEventType_BtnPress:
            handle_button_press(event);
            break;
        case InputEventType_KeyPress:
            break;
        default:
            break;
    }
}

static void handle_button_press(InputEvent_t *btn_event)
{
    if (pgup_btn.irq_flag) {
        gpio_clear_irq_flag(&pgup_btn);
        btn_event->data.btn_press.btn_id = Btn_ID_PageUp;
    } else if (pgdown_btn.irq_flag) {
        gpio_clear_irq_flag(&pgdown_btn);
        btn_event->data.btn_press.btn_id = Btn_ID_PageDown;
    } else if (back_btn.irq_flag) {
        gpio_clear_irq_flag(&back_btn);
        btn_event->data.btn_press.btn_id = Btn_ID_Back;
    } else if (home_btn.irq_flag) {
        gpio_clear_irq_flag(&home_btn);
        btn_event->data.btn_press.btn_id = Btn_ID_Home;
    } else if (confirm_btn.irq_flag) {
        gpio_clear_irq_flag(&confirm_btn);
        btn_event->data.btn_press.btn_id = Btn_ID_Confirm;
    } else {
        /* 无按键事件 */
        btn_event->data.btn_press.btn_id = Btn_ID_None;
        return;
    }
}
