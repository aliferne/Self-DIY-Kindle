#pragma once

/* 用于调用 fs 相关操作 */
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
#define OPEN_FONT_FILE(fp, filename)                                \
    do {                                                            \
        FRESULT res =                                               \
            storage_open(&sdcard, fp, "/fonts/" filename, FA_READ); \
        ASSERT_FAIL(res != FR_OK, return NULL);                     \
    } while (0)

/**
 * 从字体文件中读取数据
 *
 * @param fp 文件指针
 * @param buf 读取数据的缓冲区
 * @param offset 读取数据的偏移量
 * @param size 要读取的数据大小
 */
#define READ_FONT_FILE(fp, buf, offset, size)    \
    do {                                         \
        UINT nread;                              \
        FRESULT res = FR_OK;                     \
        res         = storage_lseek(fp, offset); \
        ASSERT_FAIL(res != FR_OK, return NULL);  \
        res = f_read(fp, buf, size, &nread);     \
        ASSERT_FAIL(res != FR_OK, return NULL);  \
        ASSERT_FAIL(nread != size, return NULL); \
    } while (0)
