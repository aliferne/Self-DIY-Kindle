#pragma once

#include "bsp_sdio.h"
#include "ff.h"
#include <stdint.h>

typedef enum {
    Storage_Ok                 = 0,
    Storage_Failed             = 1,
    Storage_InvalidParam       = 2,
    Storage_MaxPathLenExceeded = 3,
    Storage_NotInitialized     = 4,
} StorageState_t;

typedef struct {
    uint8_t is_initialized;
    const char *volume;
    FATFS fs;
} Storage_t;

StorageState_t storage_init(Storage_t *s);
StorageState_t storage_deinit(Storage_t *s);
StorageState_t storage_mkdir(Storage_t *s, const char *path);
StorageState_t storage_mkdirs(Storage_t *s, const char *paths[], uint32_t len);

FRESULT storage_open(Storage_t *s, FIL *fp, const char *path, BYTE mode);
FRESULT storage_stat(Storage_t *s, const char *path, FILINFO *fno);
FRESULT storage_unlink(Storage_t *s, const char *path);
FRESULT storage_rename(Storage_t *s, const char *old, const char *newp);
FRESULT storage_opendir(Storage_t *s, DIR *dp, const char *path);

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

static inline FSIZE_t storage_tell(FIL *fp)
{
    return f_tell(fp);
}

static inline FSIZE_t storage_size(FIL *fp)
{
    return f_size(fp);
}
