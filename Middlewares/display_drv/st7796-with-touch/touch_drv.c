#include "touch_drv.h"
#include "disp_drv.h"
#include "bsp_handle.h"
#include "ctp.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

/*
 * 根据 spin_dir 将原始触摸坐标变换到显示屏的逻辑坐标空间。
 *
 * spin_dir 值与 disp_drv.h 中 DISP_SPIN_* 一致：
 *   0 = 不旋转, 1 = 90° CCW, 2 = 180°, 3 = 270° CCW
 *
 * 变换使用触摸面板的物理宽高（phys_width/phys_height），
 * 而非显示屏的逻辑分辨率（旋转后可能互换）。
 */
static void touch_transform_coords(touch_drv_t *t)
{
    uint16_t pw = t->src->phys_width;
    uint16_t ph = t->src->phys_height;
    uint8_t dir = t->spin_dir;

    if (dir == DISP_SPIN_180) return; /* 无需变换 */

    for (int i = 0; i < TOUCH_MAX_POINTS; i++) {
        if (!(t->tp_state & (1 << i))) continue;

        uint16_t rx = t->x[i];
        uint16_t ry = t->y[i];

        switch (dir) {
            case DISP_NO_SPIN: /* 180° */
                t->x[i] = pw - 1 - rx;
                t->y[i] = ph - 1 - ry;
                break;
            case DISP_SPIN_90: /* 270° CCW */
                t->x[i] = ph - 1 - ry;
                t->y[i] = rx;
                break;
            case DISP_SPIN_270: /* 90° CCW */
                t->x[i] = ry;
                t->y[i] = pw - 1 - rx;
                break;
            default:
                break;
        }
    }
}

#define DRV_NULL_CHECK(t, act_when_null) \
    LOG_WHEN_FAILED(                     \
        t == NULL,                       \
        act_when_null,                   \
        "touch pad passed a NULL drv pointer")

void touch_init(touch_drv_t *t)
{
    DRV_NULL_CHECK(t, return);

    LOG_WHEN_FAILED(
        t->src == NULL,
        for (;;),
        "touch pad should have hardware source");

    LOG_WHEN_FAILED(
        t->src->i2c == NULL,
        for (;;),
        "touch pad i2c bus should not be NULL");

    LOG_WHEN_FAILED(
        t->delay_cb == NULL,
        for (;;),
        "touch pad should have a delay callback");

    /* 底层没东西 */
    t->priv = NULL;

    ctp_assign_pins(t->src->rst, t->src->it);
    ctp_assign_i2c(t->src->i2c);
    ctp_assign_delay(t->delay_cb);

    FT6336_Init();
    t->is_initialized = 1;
}

void touch_deinit(touch_drv_t *t)
{
    DRV_NULL_CHECK(t, return);
    t->is_initialized = 0;
}

// 扫描触摸屏(采用查询方式)
// x/y: 输出缓冲区 (至少 5 个 uint16_t)
// sta: 输出状态 (bit7:按下, bit4~0:触摸点数)
// 返回值:0,触屏无触摸;1,触屏有触摸
int touch_scan(touch_drv_t *t)
{
    DRV_NULL_CHECK(t, return 0);

    int ret = 0;
    ASSERT_FAIL(!TOUCH_IS_INIT(t), return 0);

    ret           = FT6336_Scan(t->x, t->y, &t->tp_state);
    t->is_pressed = ret;

    /* 根据旋转方向重映射触摸坐标 */
    touch_transform_coords(t);

    return ret;
}

void touch_test(touch_drv_t *t)
{
    DRV_NULL_CHECK(t, return);
    ASSERT_FAIL(!TOUCH_IS_INIT(t), return);

    touch_scan(t);

    /* 不看前面三位 */
    uint8_t temp = t->tp_state & ~(7 << 5);
    uint8_t cnt  = 0;

    /*
     * 0x0000 1010 => 0x0000 0101 => 0x0000 0010 => 0x0000 0001 => 0x0
     * cnt = 2
     */
    while (temp) {
        cnt += temp & 0x01;
        temp >>= 1;
    }

    /**
     * NOTE:
     * 目前只能双点触摸，AI 整合过后的驱动有些问题
     * 不过实际也只会用到一个点位，可以不支持多点触摸
     */
    LOG_DEBUG(
        "touch: state=0x%02x, cnt=%u,\r\n"
        "x[0]=%d, y[0]=%d,\r\n"
        "x[1]=%d, y[1]=%d,\r\n"
        "x[2]=%d, y[2]=%d,\r\n"
        "x[3]=%d, y[3]=%d,\r\n"
        "x[4]=%d, y[4]=%d\r\n",
        t->tp_state, cnt,
        t->x[0], t->y[0],
        t->x[1], t->y[1],
        t->x[2], t->y[2],
        t->x[3], t->y[3],
        t->x[4], t->y[4]);
}

bool touch_is_pressed(touch_drv_t *t)
{
    DRV_NULL_CHECK(t, return false);

    return t->is_pressed;
}
