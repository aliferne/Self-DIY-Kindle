#include "process_task.h"
#include "bsp_sys.h"

void oled_init();

void StartProcessTask(void const *argument)
{
    for (;;) {
        Delay(50);
    }
}
