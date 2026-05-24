#include "music_task.h"

#include "cmsis_os.h"
void StartMusicTask(void const *argument)
{
    for (;;) {
        osDelay(1);
    }
}
