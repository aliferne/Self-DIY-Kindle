#include "FreeRTOS.h"
#include "bsp_handle.h"
#include "task.h"

void vApplicationStackOverflowHook(TaskHandle_t xTask,
                                   char *pcTaskName)
{
    LOG_ERROR("Stack overflow in task %s\r\n", pcTaskName);
}
