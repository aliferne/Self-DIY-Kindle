#include "ui_task.h"

#include "cmsis_os.h"
void StartUITask(void const *argument)
{
    for (;;) {
        osDelay(1);
    }
}
