#include "input_srv.h"
#include "bsp_config.h"
#include "bsp_handle.h"

static void handle_button_press(InputEvent_t *e);

InputEvent_t btn_event = {
    .event_type = Button_Input,
    .event_id   = NONE,
};

InputEventId_t get_input_event_id(InputEvent_t *e)
{
    ASSERT_FAIL(e == NULL, return NONE);

    if (e->event_type == Button_Input) 
        handle_button_press(e);
    
    return e->event_id;
}

static void handle_button_press(InputEvent_t *e)
{
    if (pgup_btn.irq_flag) {
        gpio_clear_irq_flag(&pgup_btn);
        e->event_id = PAGEUP;
    } else if (pgdown_btn.irq_flag) {
        gpio_clear_irq_flag(&pgdown_btn);
        e->event_id = PAGEDOWN;
    } else if (back_btn.irq_flag) {
        gpio_clear_irq_flag(&back_btn);
        e->event_id = BACK;
    } else if (home_btn.irq_flag) {
        gpio_clear_irq_flag(&home_btn);
        e->event_id = HOME;
    } else if (confirm_btn.irq_flag) {
        gpio_clear_irq_flag(&confirm_btn);
        e->event_id = CONFIRM;
    }
}
