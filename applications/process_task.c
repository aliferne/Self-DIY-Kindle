#include "bsp_config.h"
#include "bsp_gpio.h"
#include "bsp_sys.h"
#include "ebook_srv.h"

void StartProcessTask(void const *argument)
{
    ebook_test();
    for (;;)
    {
        gpio_toggle(&usr_led);
        os_delay_ms(1000);
    }
}
