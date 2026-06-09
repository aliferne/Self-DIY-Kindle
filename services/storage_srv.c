#include "storage_srv.h"
#include "bsp_handle.h"
#include "ff.h"
#include <stdio.h>
#include <string.h>

#define STORAGE_MAX_PATH_LEN (128u)
#define STORAGE_HANDLE_ERROR(errcond, msg, acts_when_failed) \
    HANDLE_ERROR(errcond, msg, acts_when_failed)

static void make_path(Storage_t *s, const char *path, char *out, size_t sz)
{
    snprintf(out, sz, "%s/%s", s->volume, path);
}

StorageState_t storage_init(Storage_t *s)
{
    ASSERT_FAIL(s != NULL, return Storage_InvalidParam);

    s->is_initialized = 1;

    FRESULT res = f_mount(&s->fs, s->volume, 1);
    STORAGE_HANDLE_ERROR(
        res != FR_OK,
        "Failed to mount volume",
        return Storage_Failed);

    return Storage_Ok;
}

StorageState_t storage_deinit(Storage_t *s)
{
    ASSERT_FAIL(s != NULL, return Storage_InvalidParam);

    s->is_initialized = 0;
    f_unmount(s->volume);
    return Storage_Ok;
}

StorageState_t storage_mkdir(Storage_t *s, const char *path)
{
    ASSERT_FAIL(s != NULL && path != NULL, return Storage_InvalidParam);
    ASSERT_FAIL(s->is_initialized, return Storage_NotInitialized);

    if ((strlen(path) + strlen(s->volume)) > STORAGE_MAX_PATH_LEN)
        return Storage_MaxPathLenExceeded;

    char full[STORAGE_MAX_PATH_LEN];
    make_path(s, path, full, sizeof(full));

    if (f_stat(full, NULL) == FR_OK)
        return Storage_Ok;

    FRESULT res = f_mkdir(full);
    STORAGE_HANDLE_ERROR(
        res != FR_OK,
        "Failed to create directory",
        return Storage_Failed);

    return Storage_Ok;
}

StorageState_t storage_mkdirs(Storage_t *s, const char *paths[], uint32_t len)
{
    for (uint32_t i = 0; i < len; i++) {
        StorageState_t stat = storage_mkdir(s, paths[i]);
        STORAGE_HANDLE_ERROR(stat != Storage_Ok, "", return stat);
    }
    return Storage_Ok;
}

FRESULT storage_open(Storage_t *s, FIL *fp, const char *path, BYTE mode)
{
    char full[STORAGE_MAX_PATH_LEN];
    make_path(s, path, full, sizeof(full));
    return f_open(fp, full, mode);
}

FRESULT storage_stat(Storage_t *s, const char *path, FILINFO *fno)
{
    char full[STORAGE_MAX_PATH_LEN];
    make_path(s, path, full, sizeof(full));
    return f_stat(full, fno);
}

FRESULT storage_unlink(Storage_t *s, const char *path)
{
    char full[STORAGE_MAX_PATH_LEN];
    make_path(s, path, full, sizeof(full));
    return f_unlink(full);
}

FRESULT storage_rename(Storage_t *s, const char *old, const char *newp)
{
    char old_full[STORAGE_MAX_PATH_LEN];
    char new_full[STORAGE_MAX_PATH_LEN];
    make_path(s, old, old_full, sizeof(old_full));
    make_path(s, newp, new_full, sizeof(new_full));
    return f_rename(old_full, new_full);
}

FRESULT storage_opendir(Storage_t *s, DIR *dp, const char *path)
{
    char full[STORAGE_MAX_PATH_LEN];
    make_path(s, path, full, sizeof(full));
    return f_opendir(dp, full);
}
