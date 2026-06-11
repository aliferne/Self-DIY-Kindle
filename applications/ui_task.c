#include "ui_task.h"
#include "bsp_config.h"
#include "bsp_gpio.h"
#include "bsp_sys.h"
#include "lv_timer.h"
#include "mid_config.h"
#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_fs.h"
#include "lv_demos.h"
#include "disp_drv.h"

char buf[20];

void StartUITask(void const *argument)
{
    lv_init();
    lv_port_disp_init();
    lv_port_fs_init();

    // 打开 SD 卡上的书籍文件
    lv_fs_file_t f;
    lv_fs_res_t res = lv_fs_open(&f, "S:/test.txt", LV_FS_MODE_RD);

    if (res == LV_FS_RES_OK) {
        uint32_t br;
        lv_fs_read(&f, buf, sizeof(buf), &br);
        // buf 里就是文件内容...
        lv_fs_close(&f);
    }

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
    lv_label_set_text(label, buf);

    for (;;) {
        if (res == LV_FS_RES_OK)
            gpio_toggle(&usr_led);

        lv_timer_handler();
        os_delay_ms(500);
    }
}
