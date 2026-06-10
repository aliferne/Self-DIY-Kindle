#include "ui_task.h"
#include "bsp_sys.h"
#include "lv_timer.h"
#include "mid_config.h"
#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_demos.h"

#define BACKLIGHT_OFF() TFT_TurnOff(&tft, 0)
#define BACKLIGHT_ON()  TFT_TurnOff(&tft, 1)

void StartUITask(void const *argument)
{
    lv_init();
    lv_port_disp_init();
    lv_demo_widgets();

    for (;;) {
        lv_timer_handler();
        os_delay_ms(5);
    }
}
