#pragma once

#include "ff.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    Storage_Ok                 = 0,
    Storage_Failed             = 1,
    Storage_InvalidParam       = 2,
    Storage_MaxPathLenExceeded = 3,
    Storage_NotInitialized     = 4,
} StorageState_t;

typedef struct {
    uint8_t is_initialized : 1;
    const char *volume;
    FATFS fs;
    /* 使用字节数(kb) */
    float usage_kb;
    /* 剩余字节数(kb) */
    float free_kb;
} Storage_t;

/*
 * 在列出目录内部内容时调用方需要传入的回调函数，
 * 此处会传入文件名称和大小，同时告知调用方是否为目录
 * 若是目录，则 `fsize` 为无用变量
 * 若回调返回 true，继续遍历，否则终止遍历
 */
typedef bool (*storage_listdir_cb)(char *fname, uint64_t fsize, bool is_dir);

#define storage_eof(fp)       f_eof(fp)
#define storage_err(fp)       f_error(fp)
#define storage_tell(fp)      f_tell(fp)
#define storage_size(fp)      f_size(fp)
#define storage_rewind(fp)    f_rewind(fp)
#define storage_rewinddir(dp) f_rewinddir(dp)
#define storage_rmdir(path)   f_rmdir(path)

StorageState_t storage_init(Storage_t *s);
StorageState_t storage_deinit(Storage_t *s);
StorageState_t storage_mkdir(Storage_t *s, const char *path);
StorageState_t storage_mkdirs(Storage_t *s, const char *paths[], uint32_t len);

FRESULT storage_open(Storage_t *s, FIL *fp, const char *path, BYTE mode);
FRESULT storage_stat(Storage_t *s, FILINFO *fno, const char *path);
FRESULT storage_unlink(Storage_t *s, const char *path);
FRESULT storage_rename(Storage_t *s, const char *old, const char *new);
FRESULT storage_opendir(Storage_t *s, DIR *dp, const char *path);
FRESULT storage_listdir(Storage_t *s, const char *path, storage_listdir_cb cb);

static inline FRESULT storage_sync(FIL *fp)
{
    return f_sync(fp);
}

static inline FRESULT storage_close(FIL *fp)
{
    return f_close(fp);
}

static inline FRESULT storage_read(FIL *fp, void *buf, UINT btr, UINT *br)
{
    return f_read(fp, buf, btr, br);
}

static inline FRESULT storage_write(FIL *fp, const void *buf, UINT btw, UINT *bw)
{
    FRESULT res = f_write(fp, buf, btw, bw);
    /*
     * 减少临界段，尽量避免突然断电导致的 FAT 结构破坏
     * \ref https://elm-chan.org/fsw/ff/doc/appnote.html#critical
     */
    storage_sync(fp);
    return res;
}

static inline FRESULT storage_lseek(FIL *fp, FSIZE_t ofs)
{
    return f_lseek(fp, ofs);
}

static inline FRESULT storage_readdir(DIR *dp, FILINFO *fno)
{
    return f_readdir(dp, fno);
}

static inline FRESULT storage_closedir(DIR *dp)
{
    return f_closedir(dp);
}
