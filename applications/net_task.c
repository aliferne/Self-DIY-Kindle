#include "net_task.h"

#include "cmsis_os.h"
void StartNetTask(void const *argument)
{
    for (;;) {
        osDelay(1);
    }
}
