#include "mid_config.h"
#include "bsp_config.h"
#include "disp_drv.h"
#include "display_drv/st7735s/tft.h"
#include "bsp_handle.h"
#include "pin_src.h"
#include "epaper.h"

static void mid_init_epaper(void);
static void mid_init_tft(void);
static void mid_init_epaper(void);

static TFT_Model_t tft = {
    .blk_pin = &tft_blk,
    .dc_pin  = &tft_dc,
    .rst_pin = &tft_rst,
    .spi     = &tft_spi,
};

Disp_Drv_t display;

void mid_init_modules(void)
{
    mid_init_tft();
    // mid_init_epaper();
}

static void mid_init_tft(void)
{
    display.src = &tft;
    display_init(&display);
    disp_backlight_on(&display);
}

static void mid_init_epaper(void)
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
