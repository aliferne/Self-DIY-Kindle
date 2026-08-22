#include "bsp_sys.h"
#include "test_cfg.h"

void StartTestTask(void const *argument)
{

#if configRUN_EBOOK_TEST == 1
    ebook_test();
#endif

#if configRUN_INPUT_TEST == 1
#endif

#if configRUN_MUSIC_TEST == 1
#endif

#if configRUN_NET_TEST == 1
#endif

#if configRUN_STORAGE_TEST == 1
#endif

#if configRUN_TIME_TEST == 1
#endif

    /* actually can just destory that function */
    for (;;) {
        os_delay_ms(1);
    }
}
