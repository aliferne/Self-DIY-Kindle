/**
 * @file lv_port_disp_templ.h
 *
 */

/*Copy this file as "lv_port_disp.h" and set this value to "1" to enable content*/
#if 1

#ifndef LV_PORT_DISP_TEMPL_H
#define LV_PORT_DISP_TEMPL_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#include "disp_drv.h"

extern lv_disp_t *disp;
extern lv_disp_drv_t lv_disp_drv;

/*********************
 *      DEFINES
 *********************/
#define SCR_WIDTH  lv_disp_drv.hor_res
#define SCR_HEIGHT lv_disp_drv.ver_res

/* FIXME: 下面直接根据驱动来显示的颜色不太正常 */
#define LV_DISP_RED     lv_color_hex(DISP_RED)
#define LV_DISP_GREEN   lv_color_hex(DISP_GREEN)
#define LV_DISP_BLUE    lv_color_hex(DISP_BLUE)
#define LV_DISP_BLUE2   lv_color_hex(DISP_BLUE2)
#define LV_DISP_PINK    lv_color_hex(DISP_PINK)
#define LV_DISP_ORANGE  lv_color_hex(DISP_ORANGE)
#define LV_DISP_WHITE   lv_color_hex(DISP_WHITE)
#define LV_DISP_BLACK   lv_color_hex(DISP_BLACK)
#define LV_DISP_YELLOW  lv_color_hex(DISP_YELLOW)
#define LV_DISP_CYAN    lv_color_hex(DISP_CYAN)
#define LV_DISP_PURPLE  lv_color_hex(DISP_PURPLE)
#define LV_DISP_PURPLE2 lv_color_hex(DISP_PURPLE2)
#define LV_DISP_PURPLE3 lv_color_hex(DISP_PURPLE3)
#define LV_DISP_GRAY0   lv_color_hex(DISP_GRAY0)
#define LV_DISP_GRAY1   lv_color_hex(DISP_GRAY1)
#define LV_DISP_GRAY2   lv_color_hex(DISP_GRAY2)

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/* Initialize low level display driver */
void lv_port_disp_init(void);

/* Enable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_enable_update(void);

/* Disable updating the screen (the flushing process) when disp_flush() is called by LVGL
 */
void disp_disable_update(void);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_PORT_DISP_TEMPL_H*/

#endif /*Disable/Enable content*/
