#include "storage_srv.h"
#include "bsp_handle.h"
#include "ff.h"
#include "rtt_srv.h"
#include <stdio.h>
#include <string.h>

/**
 * \note
 * FAT 文件系统在某些操作时会修改 FAT 结构，如果在修改过程中突然断电，可能会导致 FAT 结构损坏。
 * 因此在进行文件操作时，应该尽量减少临界段的长度
 * 可以减少以写入模式打开的时间或者在特定操作后立刻执行 f_sync 来减少临界段的长度
 * 目前对 f_open 和 f_write 均加入了 f_sync 的调用
 *
 * 特别地， f_rename 的操作是最危险的，一旦断电，文件有可能丢失
 *
 * \ref https://elm-chan.org/fsw/ff/doc/appnote.html#critical
 */

#define STORAGE_MAX_PATH_LEN (128u)
#define STORAGE_LOG_WHEN_FAILED(cond, acts_when_failed, msg, ...) \
    LOG_WHEN_FAILED(cond, acts_when_failed, msg, ##__VA_ARGS__)

/**
 * 在内部拼接路径，要求路径格式为：
 *
 * `/path/to/file` 或者 `path/to/file`
 *
 * 注意，这取决于你怎么思考，你可以将文件层级结构解读成 0:/path/to/file, 也可以解读成 0:path/to/file
 */
static void make_path(Storage_t *s, const char *path, char *out, size_t sz)
{
    snprintf(out, sz, "%s%s", s->volume, path);
}

StorageState_t storage_init(Storage_t *s, void *dev)
{
    ASSERT_FAIL(s == NULL, return Storage_InvalidParam);
    /* 避免重复初始化 */
    ASSERT_FAIL(s->is_initialized == 1, return Storage_Ok);

    if (dev == NULL) {
        LOG_ERROR("Storage device is NULL\n");
        return Storage_InvalidParam;
    }

    s->dev = dev;

    FRESULT res = f_mount(&s->fs, s->volume, 1);
    STORAGE_LOG_WHEN_FAILED(
        res != FR_OK,
        return Storage_Failed,
        "Failed to mount volume, please check out the FRESULT: %d\n", res);

    s->is_initialized = 1;

    return Storage_Ok;
}

StorageState_t storage_deinit(Storage_t *s)
{
    ASSERT_FAIL(s == NULL, return Storage_InvalidParam);
    /* 避免重复去初始化 */
    ASSERT_FAIL(s->is_initialized == 0, return Storage_Ok);

    s->is_initialized = 0;
    f_unmount(s->volume);
    LOG_DEBUG("Storage deinitialized.\n");

    return Storage_Ok;
}

StorageState_t storage_mkdir(Storage_t *s, const char *path)
{
    ASSERT_FAIL(s == NULL && path != NULL, return Storage_InvalidParam);
    ASSERT_FAIL(s->is_initialized != 1, return Storage_NotInitialized);
    ASSERT_FAIL((strlen(s->volume) + strlen(path) > STORAGE_MAX_PATH_LEN),
                return Storage_MaxPathLenExceeded);

    char full[STORAGE_MAX_PATH_LEN];
    make_path(s, path, full, sizeof(full));

    if (f_stat(full, NULL) == FR_OK) {
        LOG_DEBUG("%s already exists, return\n", full);
        return Storage_Ok;
    }

    FRESULT res = f_mkdir(full);
    STORAGE_LOG_WHEN_FAILED(
        res != FR_OK,
        return Storage_Failed,
        "Failed to create directory\n");

    return Storage_Ok;
}

StorageState_t storage_mkdirs(Storage_t *s, const char *paths[], uint32_t len)
{
    ASSERT_FAIL(s->is_initialized != 1, return Storage_NotInitialized);

    for (uint32_t i = 0; i < len; i++) {
        StorageState_t stat = storage_mkdir(s, paths[i]);

        if (stat != Storage_Ok) {
            STORAGE_LOG_WHEN_FAILED(1, return stat, "Failed when making %s", paths[i]);
        }
    }
    return Storage_Ok;
}

FRESULT storage_open(Storage_t *s, FIL *fp, const char *path, BYTE mode)
{
    ASSERT_FAIL(s->is_initialized != 1, return FR_NOT_READY);

    char full[STORAGE_MAX_PATH_LEN];
    make_path(s, path, full, sizeof(full));

    FRESULT res = f_open(fp, full, mode);
    /*
     * 减少临界段，尽量避免突然断电导致的 FAT 结构破坏
     * \ref https://elm-chan.org/fsw/ff/doc/appnote.html#critical
     */
    f_sync(fp);
    return res;
}

FRESULT storage_stat(Storage_t *s, FILINFO *fno, const char *path)
{
    ASSERT_FAIL(s->is_initialized != 1, return FR_NOT_READY);

    char full[STORAGE_MAX_PATH_LEN];
    make_path(s, path, full, sizeof(full));
    return f_stat(full, fno);
}

FRESULT storage_unlink(Storage_t *s, const char *path)
{
    ASSERT_FAIL(s->is_initialized != 1, return FR_NOT_READY);

    char full[STORAGE_MAX_PATH_LEN];
    make_path(s, path, full, sizeof(full));
    return f_unlink(full);
}

FRESULT storage_rename(Storage_t *s, const char *old, const char *new)
{
    ASSERT_FAIL(s->is_initialized != 1, return FR_NOT_READY);

    char old_full[STORAGE_MAX_PATH_LEN];
    char new_full[STORAGE_MAX_PATH_LEN];
    make_path(s, old, old_full, sizeof(old_full));
    make_path(s, new, new_full, sizeof(new_full));
    return f_rename(old_full, new_full);
}

FRESULT storage_opendir(Storage_t *s, DIR *dp, const char *path)
{
    ASSERT_FAIL(s->is_initialized != 1, return FR_NOT_READY);

    char full[STORAGE_MAX_PATH_LEN];
    make_path(s, path, full, sizeof(full));
    return f_opendir(dp, full);
}

/**
 * \brief 列出目录内所有文件/子目录
 *
 * \param s: 存储句柄
 * \param path: 待读取路径
 * \param cb: 回调函数，每列出一项则调用一次回调，传入相关文件属性
 */
FRESULT storage_listdir(Storage_t *s, const char *path, storage_listdir_cb cb)
{
    FRESULT res;
    DIR dir;
    FILINFO fno;

    ASSERT_FAIL(s->is_initialized != 1, return FR_NOT_READY);

    res = storage_opendir(s, &dir, path);
    ASSERT_FAIL(res != FR_OK,
                RTT_LOG_ERROR("Failed to open dir: %s (errcode: %d)\r\n", path, res);
                return res);

    res = f_readdir(&dir, &fno);
    // 检查错误或是否到达目录末尾
    while (res == FR_OK && fno.fname[0] != '\0') {
        // 忽略 "." 和 ".." 目录
        if (fno.fname[0] == '.')
            continue;

        if (cb != NULL &&
            cb(fno.fname, fno.fsize, fno.fattrib & AM_DIR) != true)
            break;

        res = f_readdir(&dir, &fno);
    }

    res = f_closedir(&dir);

    return res;
}
