#include "mid_config.h"
#include "bsp_config.h"
#include "disp_drv.h"
#include "bsp_handle.h"
#include "pin_src.h"
#include "epaper.h"

static void mid_init_epaper(void);
static void mid_init_tft(void);
static void mid_init_epaper(void);

static Disp_Src_t tft = {
    .blk_pin = &tft_blk,
    .dc_pin  = &tft_dc,
    .rst_pin = &tft_rst,
    .spi     = &tft_spi,
};

/* TODO: 启用触摸时，取消注释以下资源，并在 bsp_config 中初始化 I2C/GPIO
#include "bsp_i2c.h"
#include "bsp_gpio.h"

static I2C_Model_t touch_i2c;
static GPIO_Model_t touch_int;
static GPIO_Model_t touch_rst;

static Disp_Touch_Src_t touch = {
    .i2c = &touch_i2c,
    .it  = &touch_int,
    .rst = &touch_rst,
};
*/

Disp_Drv_t display;

void mid_init_modules(void)
{
    mid_init_tft();
    // mid_init_epaper();
}

static void mid_init_tft(void)
{
    display.src = &tft;
    /* TODO: 启用触摸时，取消下行注释 */
    // display.touch = &touch;
    display_init(&display);
    display_backlight_on(&display);
}

__NOT_USED static void mid_init_epaper(void)
{
    EPaper_Config_t cfg = {
        .fast_init_time = 1.0f,
        .init_mode      = EPaper_Fast_Init,
    };

    EPaper_Err_t err = epaper_init(
        &e_paper, &cfg,
        (GPIO_Port_t)EPAPER_SCK_PORT, (GPIO_Pin_t)EPAPER_SCK_PIN,
        (GPIO_Port_t)EPAPER_MOSI_PORT, (GPIO_Pin_t)EPAPER_MOSI_PIN,
        (GPIO_Port_t)EPAPER_MISO_PORT, (GPIO_Pin_t)EPAPER_MISO_PIN,
        (GPIO_Port_t)EPAPER_CS_PORT, (GPIO_Pin_t)EPAPER_CS_PIN,
        (GPIO_Port_t)EPAPER_DC_PORT, (GPIO_Pin_t)EPAPER_DC_PIN,
        (GPIO_Port_t)EPAPER_RST_PORT, (GPIO_Pin_t)EPAPER_RST_PIN,
        (GPIO_Port_t)EPAPER_BUSY_PORT, (GPIO_Pin_t)EPAPER_BUSY_PIN);
    GIVEUP(err);
}
