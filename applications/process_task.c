#include "process_task.h"

#include "cmsis_os.h"
void StartProcessTask(void const *argument)
{
    for (;;) {
        osDelay(1);
    }
}
