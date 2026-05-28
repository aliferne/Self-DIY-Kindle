#include "mid_config.h"
#include "bsp_sys.h"
#include "pin_src.h"
#include "epaper.h"

static void mid_init_epaper(void);

void mid_init_modules(void)
{
    mid_init_epaper();
}

static void mid_init_epaper(void)
{
    EPaper_Err_t err;

    epaper_register(
        &e_paper,
        (GPIO_Port_t)EPAPER_SCK_PORT, (GPIO_Pin_t)EPAPER_SCK_PIN,
        (GPIO_Port_t)EPAPER_MOSI_PORT, (GPIO_Pin_t)EPAPER_MOSI_PIN,
        (GPIO_Port_t)EPAPER_MISO_PORT, (GPIO_Pin_t)EPAPER_MISO_PIN,
        (GPIO_Port_t)EPAPER_CS_PORT, (GPIO_Pin_t)EPAPER_CS_PIN,
        (GPIO_Port_t)EPAPER_DC_PORT, (GPIO_Pin_t)EPAPER_DC_PIN,
        (GPIO_Port_t)EPAPER_RST_PORT, (GPIO_Pin_t)EPAPER_RST_PIN,
        (GPIO_Port_t)EPAPER_BUSY_PORT, (GPIO_Pin_t)EPAPER_BUSY_PIN);

    EPaper_Config_t cfg = {
        .fast_init_time = 1.0f,
        .init_mode      = EPaper_Fast_Init,
    };

    err = epaper_init(&e_paper, &cfg);
    GIVEUP(err);
}
