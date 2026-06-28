#pragma once

/* 用于调用 fs 相关操作 */
#include "ff.h"
#include "srv_config.h"
#include "storage_srv.h"
#include "bsp_handle.h"

/**
 * 打开字体文件，如果没有该文件则返回 NULL
 * 该函数假定文件在 sdcard 的 fonts 目录下
 *
 * @param fp 文件指针
 * @param filename 文件名
 */
#define open_font_file(fp, filename)                                \
    do {                                                            \
        FRESULT res =                                               \
            storage_open(&sdcard, fp, "/fonts/" filename, FA_READ); \
        ASSERT_FAIL(res != FR_OK, for (;;));                        \
    } while (0)

/**
 * 从字体文件中读取数据
 *
 * @param fp 文件指针
 * @param buf 读取数据的缓冲区
 * @param offset 读取数据的偏移量
 * @param size 要读取的数据大小
 */
static bool read_font_file(FIL *fp, uint8_t *buf, size_t offset, size_t size)
{
    UINT nread;
    FRESULT res = FR_OK;
    res         = f_lseek(fp, offset);

    if (res != FR_OK)
        for (;;);

    res = f_read(fp, buf, size, &nread);

    if (res != FR_OK || nread != size)
        return false;
    return true;
}
