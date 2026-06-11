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

char buf[256 * 100];

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

    for (;;) {
        if (res == LV_FS_RES_OK)
            gpio_toggle(&usr_led);

        lv_timer_handler();
        os_delay_ms(500);
    }
}
