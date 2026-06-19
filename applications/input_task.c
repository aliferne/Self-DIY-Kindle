#include "input_task.h"
#include "bsp_gpio.h"
#include "bsp_config.h"
#include "input_srv.h"
#include "bsp_sys.h"

/* TODO: 这里的 input 还考虑了触摸事件，后续也许可以和 LVGL 做集成 */

static void handle_btn_event();

void StartInputTask(void const *argument)
{
    for (;;) {
        // handle_btn_event();
        os_delay_ms(1);
    }
}

static void handle_btn_event()
{
    get_input_event_id(&btn_event);
}
