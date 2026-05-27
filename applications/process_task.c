#include "process_task.h"
#include "bsp_sys.h"

void StartProcessTask(void const *argument)
{
    for (;;) {
        os_delay_ms(50);
    }
}
