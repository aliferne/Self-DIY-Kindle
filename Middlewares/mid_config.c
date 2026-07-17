#include "mid_config.h"
#include "bsp_config.h"
#include "bsp_sys.h"
#include "disp_drv.h"
#include "touch_drv.h"
#include "bsp_handle.h"
#include "pin_src.h"
#include "bsp_config.h"
#include "epaper.h"

static void mid_init_epaper(void);
static void mid_init_tft(void);
static void mid_init_touch(void);
static void mid_init_epaper(void);

static disp_src_t tft = {
    .blk_pin = &tft_blk,
    .dc_pin  = &tft_dc,
    .rst_pin = &tft_rst,
    .spi     = &tft_spi,
};

static touch_src_t touch_src = {
    .i2c         = &tp_i2c,
    .it          = &tp_int,
    .rst         = &tp_rst,
    .phys_width  = 320,
    .phys_height = 480,
};

disp_drv_t screen;
static touch_drv_t screen_touch;

void mid_init_modules(void)
{
    mid_init_tft();
    mid_init_touch();
    // mid_init_epaper();
}

static void mid_init_tft(void)
{
    screen.src   = &tft;
    screen.touch = &screen_touch;

    display_init(&screen);
    display_backlight_on(&screen);
    LOG_DEBUG("TFT Initialized.\n");
}

static void mid_init_touch(void)
{
    screen_touch.src      = &touch_src;
    screen_touch.delay_cb = os_delay_ms;

    touch_init(&screen_touch);
    LOG_DEBUG("Touch Initialized.\n");
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
    
    LOG_DEBUG("Epaper Initialized.\n");
}
