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

extern lv_disp_t *lv_disp;
extern uint32_t disp_hor_res;
extern uint32_t disp_ver_res;

/*********************
 *      DEFINES
 *********************/
#define SCR_WIDTH  disp_hor_res
#define SCR_HEIGHT disp_ver_res

#define LV_DISP_RED     lv_color_make(255, 0, 0)
#define LV_DISP_GREEN   lv_color_make(0, 255, 0)
#define LV_DISP_BLUE    lv_color_make(0, 0, 255)
#define LV_DISP_BLUE2   lv_color_make(28, 57, 255)
#define LV_DISP_PINK    lv_color_make(255, 105, 180)
#define LV_DISP_ORANGE  lv_color_make(255, 165, 0)
#define LV_DISP_WHITE   lv_color_make(255, 255, 255)
#define LV_DISP_BLACK   lv_color_make(0, 0, 0)
#define LV_DISP_YELLOW  lv_color_make(255, 255, 0)
#define LV_DISP_CYAN    lv_color_make(0, 255, 255)
#define LV_DISP_PURPLE  lv_color_make(128, 0, 128)
#define LV_DISP_PURPLE2 lv_color_make(186, 85, 211)
#define LV_DISP_PURPLE3 lv_color_make(75, 0, 130)
#define LV_DISP_GRAY0   lv_color_make(239, 239, 239)
#define LV_DISP_GRAY1   lv_color_make(128, 128, 128)
#define LV_DISP_GRAY2   lv_color_make(64, 64, 64)

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
