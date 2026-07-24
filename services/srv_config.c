#include "srv_config.h"
#include "storage_srv.h"
#include "bsp_handle.h"
#include "bsp_config.h"

/* sdcard_storage 实例 */
Storage_t sdcard_storage = {.volume = "0:", .fs = {0}};

void service_init(void)
{
    /* 对存储的初始化 */
    storage_init(&sdcard_storage, &sdcard);
    /*
     * 目录分配：
     * - logs: 用于存放日志文件
     * - cache: 用于存放缓存文件，如解压出来的临时文件
     * - books: 用于存放书籍文件
     * - music: 用于存放音乐文件
     * - fonts: 用于存放字体文件
     */
    const char *paths[] = {"/logs", "/cache", "/books", "/music", "/fonts"};

    storage_mkdirs(&sdcard_storage, paths, LEN(paths));
}
