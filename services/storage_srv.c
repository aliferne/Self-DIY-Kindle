#include "storage_srv.h"
#include "bsp_config.h"
#include "bsp_sdio.h"
#include "bsp_sys.h"
#include "ff.h"
#include <stdio.h>

/*
 * 在 FatFs 的基础上提供存储功能
 */

#define VOLUME_NAME  "0:"
#define LOG_FOLDER   "log"
#define BOOK_FOLDER  "books"
#define MUSIC_FOLDER "music"
#define STORAGE_HANDLE_ERROR(errcode, msg, acts_when_failed, acts_when_ok) \
    HANDLE_ERROR(errcode, FR_OK, 1, msg, acts_when_failed, acts_when_ok)

static FATFS storage_fs;

static FRESULT storage_mkdir(const char *path)
{
    FRESULT res = FR_OK;
    if (path == NULL)
        return res;

    /* 不重复创建目录 */
    if (f_stat(path, NULL) == FR_OK)
        return res;

    res = f_mkdir(path);

    STORAGE_HANDLE_ERROR(
        res, "Failed to create directory",
        return res,
        return res);
}

void storage_srv_init(Storage_t *storage)
{
    if (storage == NULL) {
        return;
    }

    SET_STATE(storage->state, Storage_Initialized);

    FRESULT res;

    /* TODO: 需要研读 FatFs 的文档以知道什么操作是安全的 */
    res = f_mount(&storage_fs, VOLUME_NAME, 1);
    STORAGE_HANDLE_ERROR(
        res, "Failed to mount volume",
        return,
        return);

    /* TODO: 看下这里如何错误处理 */
    storage_mkdir(LOG_FOLDER);
    storage_mkdir(BOOK_FOLDER);
    storage_mkdir(MUSIC_FOLDER);
}
