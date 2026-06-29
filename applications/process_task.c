#include "rtt_srv.h"
#include "bsp_sys.h"
#include <stdio.h>

/* TODO: 应该要定义一个能够处理线程间通信的机制，并需要作为大后方处理任务逻辑 */

#include "FreeRTOS.h"
#include "task.h"
void vApplicationStackOverflowHook(TaskHandle_t xTask,
                                   char *pcTaskName)
{
    printf("Stack overflow in task %s\r\n", pcTaskName);
}

void StartProcessTask(void const *argument)
{
    char i = 0;
    for (;;) {
        os_delay_ms(1000);
    }
}
