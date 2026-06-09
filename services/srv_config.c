#include "srv_config.h"
#include "storage_srv.h"
#include "bsp_handle.h"

/* sdcard 实例 */
Storage_t sdcard = {.volume = "0:", .fs = {0}};

void service_init(void)
{
    /* 对存储的初始化 */
    storage_init(&sdcard);
    /*
     * 目录分配：
     * - logs: 用于存放日志文件
     * - cache: 用于存放缓存文件，如解压出来的临时文件
     * - books: 用于存放书籍文件
     * - music: 用于存放音乐文件
     */
    const char *paths[] = {"logs", "cache", "books", "music"};
    storage_mkdirs(&sdcard, paths, LEN(paths));
}
