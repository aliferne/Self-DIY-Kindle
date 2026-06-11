#include "ui_task.h"
#include "bsp_sys.h"
#include "lv_timer.h"
#include "mid_config.h"
#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_demos.h"
#include "disp_drv.h"

void StartUITask(void const *argument)
{
    for (;;) {
        disp_test(&display);
        os_delay_ms(5);
    }
}
