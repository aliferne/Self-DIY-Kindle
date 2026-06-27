#include "input_task.h"
#include "bsp_handle.h"
#include "input_srv.h"
#include "bsp_sys.h"
#include "ui_task.h"
#include "bsp_config.h"

static void input_dispatcher(input_event_t *e)
{
    ASSERT_FAIL(e == NULL, return);
    
    /* 目前是一个相对简单的标志位操作，后续可以换成任务通知 */
    switch (e->event_id) {
        case PAGEUP:
            turn_screen = 1;
            break;
        case PAGEDOWN:
            turn_screen = 0;
            break;
        case BACK:
            turn_screen = !turn_screen;
            break;
        case HOME:
            turn_screen = 0;
            break;
        case CONFIRM:
            turn_screen = 1;
            break;
        case NONE:
        default:
            break;
    }
}

void StartInputTask(void const *argument)
{
    input_srv_init();
    input_srv_register_dispatcher(input_dispatcher);

    for (;;) {
        input_srv_handler();
        os_delay_ms(1);
    }
}
