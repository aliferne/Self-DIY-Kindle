#include "input_task.h"
#include "bsp_gpio.h"
#include "bsp_config.h"
#include "input_srv.h"
#include "bsp_sys.h"

static void handle_btn_event();

void StartInputTask(void const *argument)
{
    for (;;) {
        handle_btn_event();
        Delay(1);
    }
}

static void handle_btn_event()
{
    handle_input_events(&btn_event);

    switch (btn_event.data.btn_press.btn_id) {
        case Btn_ID_PageUp:
            gpio_write(&usr_led, GPIO_Level_Low);
            break;
        case Btn_ID_PageDown:
            gpio_write(&usr_led, GPIO_Level_High);
            break;
        case Btn_ID_Back:
            gpio_write(&usr_led, GPIO_Level_Low);
            break;
        case Btn_ID_Home:
            gpio_write(&usr_led, GPIO_Level_High);
            break;
        case Btn_ID_Confirm:
            gpio_write(&usr_led, GPIO_Level_Low);
            break;
        default:
            break;
    }
}
