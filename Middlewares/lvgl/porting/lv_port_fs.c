/**
 * @file lv_port_fs.c
 *
 * LVGL 文件系统端口 —— 桥接到 storage_srv
 *
 * 所有文件操作最终通过 storage_srv 完成，以复用路径前缀管理、
 * 自动 f_sync（断电保护）及统一错误处理。
 */

#if 0

/*********************
 *      INCLUDES
 *********************/
 #include "lv_port_fs.h"
 #include "bsp_handle.h"
#include "../../../services/srv_config.h"
#include "storage_srv.h"
#include "ff.h"
#include <stdlib.h>
#include <string.h>

/*********************
 *      DEFINES
 *********************/
/** LVGL 文件系统驱动字母 —— 对应 sdcard 卷 */
#define FS_DRIVER_LETTER 'S'

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void fs_init(void);
static bool fs_ready(lv_fs_drv_t *drv);
static void *fs_open(lv_fs_drv_t *drv, const char *path, lv_fs_mode_t mode);
static lv_fs_res_t fs_close(lv_fs_drv_t *drv, void *file_p);
static lv_fs_res_t fs_read(lv_fs_drv_t *drv, void *file_p, void *buf, uint32_t btr, uint32_t *br);
static lv_fs_res_t fs_write(lv_fs_drv_t *drv, void *file_p, const void *buf, uint32_t btw, uint32_t *bw);
static lv_fs_res_t fs_seek(lv_fs_drv_t *drv, void *file_p, uint32_t pos, lv_fs_whence_t whence);
static lv_fs_res_t fs_tell(lv_fs_drv_t *drv, void *file_p, uint32_t *pos_p);

static void *fs_dir_open(lv_fs_drv_t *drv, const char *path);
static lv_fs_res_t fs_dir_read(lv_fs_drv_t *drv, void *rddir_p, char *fn);
static lv_fs_res_t fs_dir_close(lv_fs_drv_t *drv, void *rddir_p);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**
 * @brief 将 FatFs 的 FRESULT 映射为 LVGL 的 lv_fs_res_t
 */
static lv_fs_res_t map_fresult(FRESULT res)
{
    switch (res) {
        case FR_OK:
            return LV_FS_RES_OK;
        case FR_DISK_ERR:
            return LV_FS_RES_HW_ERR;
        case FR_INT_ERR:
            return LV_FS_RES_HW_ERR;
        case FR_NOT_READY:
            return LV_FS_RES_HW_ERR;
        case FR_NOT_ENABLED:
            return LV_FS_RES_HW_ERR;
        case FR_MKFS_ABORTED:
            return LV_FS_RES_HW_ERR;
        case FR_NO_FILE:
            return LV_FS_RES_NOT_EX;
        case FR_NO_PATH:
            return LV_FS_RES_NOT_EX;
        case FR_INVALID_DRIVE:
            return LV_FS_RES_NOT_EX;
        case FR_DENIED:
            return LV_FS_RES_DENIED;
        case FR_EXIST:
            return LV_FS_RES_DENIED;
        case FR_WRITE_PROTECTED:
            return LV_FS_RES_DENIED;
        case FR_NO_FILESYSTEM:
            return LV_FS_RES_FS_ERR;
        case FR_TIMEOUT:
            return LV_FS_RES_TOUT;
        case FR_LOCKED:
            return LV_FS_RES_LOCKED;
        case FR_TOO_MANY_OPEN_FILES:
            return LV_FS_RES_LOCKED;
        case FR_INVALID_NAME:
            return LV_FS_RES_INV_PARAM;
        case FR_INVALID_PARAMETER:
            return LV_FS_RES_INV_PARAM;
        case FR_NOT_ENOUGH_CORE:
            return LV_FS_RES_OUT_OF_MEM;
        default:
            return LV_FS_RES_UNKNOWN;
    }
}

/**
 * @brief 去除 LVGL 传入路径中的驱动字母前缀
 *
 * LVGL 传入路径格式: "S:/books/book.epub"
 * 返回:              "books/book.epub"
 * 后续由 storage_srv 的 make_path() 拼接 volume（如 "0:"）
 */
static const char *strip_driver_letter(const char *path)
{
    const char *colon = strchr(path, ':');
    if (colon == NULL) {
        return path;
    }
    /* 跳过 ':' 和开头的 '/'，避免 make_path 产生双斜杠 */
    const char *relpath = colon + 1;
    if (*relpath == '/') {
        relpath++;
    }
    return relpath;
}

/**
 * @brief 将 LVGL 的打开模式映射为 FatFs 的打开模式
 */
static BYTE map_open_mode(lv_fs_mode_t mode)
{
    BYTE fat_mode = 0;

    if (mode & LV_FS_MODE_RD) {
        fat_mode |= FA_READ;
    }
    if (mode & LV_FS_MODE_WR) {
        fat_mode |= FA_WRITE | FA_CREATE_ALWAYS;
    }

    return fat_mode;
}

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_port_fs_init(void)
{
    /*
     * init fs
     */
    fs_init();

    /*---------------------------------------------------
     * Register the file system interface in LVGL
     *--------------------------------------------------*/

    static lv_fs_drv_t fs_drv;
    lv_fs_drv_init(&fs_drv);

    /*Set up fields...*/
    fs_drv.letter   = FS_DRIVER_LETTER;
    fs_drv.ready_cb = fs_ready;
    fs_drv.open_cb  = fs_open;
    fs_drv.close_cb = fs_close;
    fs_drv.read_cb  = fs_read;
    fs_drv.write_cb = fs_write;
    fs_drv.seek_cb  = fs_seek;
    fs_drv.tell_cb  = fs_tell;

    fs_drv.dir_open_cb  = fs_dir_open;
    fs_drv.dir_read_cb  = fs_dir_read;
    fs_drv.dir_close_cb = fs_dir_close;

    /* 将全局 Storage_t 实例存入 user_data，供回调使用 */
    fs_drv.user_data = &sdcard;

    lv_fs_drv_register(&fs_drv);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

/**
 * @brief 文件系统初始化
 */
static void fs_init(void)
{
    storage_init(&sdcard);
}

/**
 * @brief 检查存储设备是否就绪
 */
static bool fs_ready(lv_fs_drv_t *drv)
{
    Storage_t *s = (Storage_t *)drv->user_data;
    return (s != NULL) && s->is_initialized;
}

/**
 * @brief 打开文件
 *
 * @param drv   文件系统驱动
 * @param path  路径（含驱动字母，如 "S:/books/test.txt"）
 * @param mode  打开模式
 * @return FIL* 指针，失败返回 NULL
 */
static void *fs_open(lv_fs_drv_t *drv, const char *path, lv_fs_mode_t mode)
{
    Storage_t *s        = (Storage_t *)drv->user_data;
    const char *relpath = strip_driver_letter(path);

    FIL *fp = (FIL *)malloc(sizeof(FIL));
    if (fp == NULL) {
        return NULL;
    }

    FRESULT res = storage_open(s, fp, relpath, map_open_mode(mode));
    if (res != FR_OK) {
        free(fp);
        return NULL;
    }

    return fp;
}

/**
 * @brief 关闭文件
 */
static lv_fs_res_t fs_close(lv_fs_drv_t *drv, void *file_p)
{
    GIVEUP(drv);

    if (file_p == NULL) {
        return LV_FS_RES_INV_PARAM;
    }

    FIL *fp     = (FIL *)file_p;
    FRESULT res = storage_close(fp);
    free(fp);

    return map_fresult(res);
}

/**
 * @brief 从文件读取数据
 */
static lv_fs_res_t fs_read(lv_fs_drv_t *drv, void *file_p, void *buf, uint32_t btr, uint32_t *br)
{
    GIVEUP(drv);

    if (file_p == NULL) {
        return LV_FS_RES_INV_PARAM;
    }

    FIL *fp     = (FIL *)file_p;
    UINT br_tmp = 0;
    FRESULT res = storage_read(fp, buf, btr, &br_tmp);

    if (br != NULL) {
        *br = br_tmp;
    }

    return map_fresult(res);
}

/**
 * @brief 向文件写入数据
 *
 * 注意：storage_write 内部已自动调用 f_sync，减少断电风险。
 */
static lv_fs_res_t fs_write(lv_fs_drv_t *drv, void *file_p, const void *buf, uint32_t btw, uint32_t *bw)
{
    GIVEUP(drv);

    if (file_p == NULL) {
        return LV_FS_RES_INV_PARAM;
    }

    FIL *fp     = (FIL *)file_p;
    UINT bw_tmp = 0;
    FRESULT res = storage_write(fp, buf, btw, &bw_tmp);

    if (bw != NULL) {
        *bw = bw_tmp;
    }

    return map_fresult(res);
}

/**
 * @brief 定位文件读写指针
 *
 * FatFs 的 f_lseek 只支持绝对定位（SEEK_SET），
 * 因此 SEEK_CUR 和 SEEK_END 需要先计算出绝对位置再调用。
 */
static lv_fs_res_t fs_seek(lv_fs_drv_t *drv, void *file_p, uint32_t pos, lv_fs_whence_t whence)
{
    GIVEUP(drv);

    if (file_p == NULL) {
        return LV_FS_RES_INV_PARAM;
    }

    FIL *fp = (FIL *)file_p;
    FSIZE_t abs_pos;

    switch (whence) {
        case LV_FS_SEEK_SET:
            abs_pos = pos;
            break;

        case LV_FS_SEEK_CUR:
            abs_pos = storage_tell(fp) + pos;
            break;

        case LV_FS_SEEK_END:
            abs_pos = storage_size(fp) + pos;
            break;

        default:
            return LV_FS_RES_INV_PARAM;
    }

    FRESULT res = storage_lseek(fp, abs_pos);
    return map_fresult(res);
}

/**
 * @brief 获取文件读写指针当前位置
 */
static lv_fs_res_t fs_tell(lv_fs_drv_t *drv, void *file_p, uint32_t *pos_p)
{
    GIVEUP(drv);

    if (file_p == NULL || pos_p == NULL) {
        return LV_FS_RES_INV_PARAM;
    }

    FIL *fp = (FIL *)file_p;
    *pos_p  = (uint32_t)storage_tell(fp);

    return LV_FS_RES_OK;
}

/**
 * @brief 打开目录
 */
static void *fs_dir_open(lv_fs_drv_t *drv, const char *path)
{
    Storage_t *s        = (Storage_t *)drv->user_data;
    const char *relpath = strip_driver_letter(path);

    DIR *dir = (DIR *)malloc(sizeof(DIR));
    if (dir == NULL) {
        return NULL;
    }

    FRESULT res = storage_opendir(s, dir, relpath);
    if (res != FR_OK) {
        free(dir);
        return NULL;
    }

    return dir;
}

/**
 * @brief 读取目录中的下一个条目
 *
 * LVGL 约定：目录名以 '/' 开头，文件名不含 '/'
 */
static lv_fs_res_t fs_dir_read(lv_fs_drv_t *drv, void *rddir_p, char *fn)
{
    GIVEUP(drv);

    if (rddir_p == NULL || fn == NULL) {
        return LV_FS_RES_INV_PARAM;
    }

    DIR *dir = (DIR *)rddir_p;
    FILINFO fno;

    FRESULT res = storage_readdir(dir, &fno);
    if (res != FR_OK) {
        return map_fresult(res);
    }

    /* 没有更多文件 */
    if (fno.fname[0] == '\0') {
        fn[0] = '\0';
        return LV_FS_RES_OK;
    }

    /* 目录名加 '/' 前缀 */
    if (fno.fattrib & AM_DIR) {
        fn[0] = '/';
        strcpy(fn + 1, fno.fname);
    } else {
        strcpy(fn, fno.fname);
    }

    return LV_FS_RES_OK;
}

/**
 * @brief 关闭目录
 */
static lv_fs_res_t fs_dir_close(lv_fs_drv_t *drv, void *rddir_p)
{
    GIVEUP(drv);

    if (rddir_p == NULL) {
        return LV_FS_RES_INV_PARAM;
    }

    DIR *dir    = (DIR *)rddir_p;
    FRESULT res = storage_closedir(dir);
    free(dir);

    return map_fresult(res);
}

#else /*Enable this file at the top*/

/*This dummy typedef exists purely to silence -Wpedantic.*/
typedef int keep_pedantic_happy;
#endif
