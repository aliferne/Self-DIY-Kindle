#include "ui_task.h"
#include "bsp_config.h"
#include "bsp_gpio.h"
#include "bsp_sys.h"
#include "lv_hal_disp.h"
#include "lv_style.h"
#include "lv_timer.h"
#include "lv_port_disp.h"
#include <stdio.h>

LV_FONT_DECLARE(simhei_size14)
LV_FONT_DECLARE(simhei_size16)
LV_FONT_DECLARE(simhei_size18)

#include "FreeRTOS.h"
#include "task.h"
void vApplicationStackOverflowHook(TaskHandle_t xTask,
                                   char *pcTaskName)
{
    printf("Stack overflow in task %s\r\n", pcTaskName);
}

uint8_t turn_screen = 0;

void StartUITask(void const *argument)
{
    lv_init();
    lv_port_disp_init();

    static lv_style_t style;
    lv_style_init(&style);
    lv_style_set_radius(&style, 5);

    /*Make a gradient*/
    lv_style_set_width(&style, 128);
    lv_style_set_height(&style, LV_SIZE_CONTENT);

    lv_style_set_pad_ver(&style, 0);
    lv_style_set_pad_left(&style, 0);

    lv_style_set_x(&style, lv_pct(0));
    lv_style_set_y(&style, 0);

    /*Create an object with the new style*/
    lv_obj_t *obj = lv_obj_create(lv_scr_act());
    lv_obj_add_style(obj, &style, 0);

    lv_obj_t *label = lv_label_create(obj);
    lv_obj_set_style_text_font(label, &simhei_size14, 0);

    lv_label_set_text(label, "确认");

    uint8_t i = 1;
    for (;;) {
        lv_timer_handler();
        os_delay_ms(10);
        lv_disp_set_rotation(disp, 0);

        if (turn_screen == 1) {
            turn_screen = 0;
            i           = (i + 1) % 4;
            gpio_toggle(&usr_led);
        }
    }
}
