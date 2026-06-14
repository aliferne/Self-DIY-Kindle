#include "ui_task.h"
#include "bsp_config.h"
#include "bsp_gpio.h"
#include "bsp_handle.h"
#include "bsp_sys.h"
#include "ff.h"
#include "lv_font.h"
#include "srv_config.h"
#include "storage_srv.h"
#include "lv_timer.h"
#include "lv_port_disp.h"

char buf[256];

void StartUITask(void const *argument)
{
    lv_init();
    lv_port_disp_init();

    FIL fp;
    FRESULT res = storage_open(&sdcard, &fp, "test.txt", FA_READ | FA_OPEN_EXISTING);
    UINT fnum   = 0;
    ASSERT_FAIL(res != FR_OK,
                storage_close(&fp);
                for (;;) { os_delay_ms(1000); });
    res = storage_read(&fp, buf, LEN(buf), &fnum);
    ASSERT_FAIL(res != FR_OK,
                storage_close(&fp);
                for (;;) { os_delay_ms(1000); });
    storage_close(&fp);

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
    // lv_obj_set_style_text_font(label, &songti, 0);
    // lv_label_set_text(label, "Confirm");

    // lv_style_set_y(&style, 0 + LV_SIZE_CONTENT);
    // lv_obj_add_style(obj, &style, 0);

    /* FIXME: 无法显示中文 */
    if (buf[0] != 0)
        lv_label_set_text(label, buf);
    else
        lv_label_set_text(label, "确认");

    for (;;) {
        gpio_toggle(&usr_led);

        lv_timer_handler();
        os_delay_ms(500);
    }
}
