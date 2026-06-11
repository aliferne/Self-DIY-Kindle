#include "process_task.h"
#include "bsp_sys.h"

/* TODO: 应该要定义一个能够处理线程间通信的机制，并需要作为大后方处理任务逻辑 */

void StartProcessTask(void const *argument)
{
    for (;;) {
        os_delay_ms(50);
    }
}
