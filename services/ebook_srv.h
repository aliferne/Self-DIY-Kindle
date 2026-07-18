#pragma once

#include "ff.h"
#include "storage_srv.h"
#include <stdbool.h>
#include <stdint.h>

/*
 * 电子书服务框架，需要支持如下功能：
 *
 * - 书签
 * - 提供当前页面的文本供 UI 渲染
 * - 能够定位到任意页面（通过 UI 产生输入并传到这里，进行处理）
 * - 基础切换上下页
 * - 显示阅读进度
 * - 尽可能优化第二次打开的时间（专门针对 epub， epub 需要处理一些复杂的解析）
 * - 批注（优先级较低）
 *
 */

/*
 * 书本文件的路径，
 * 对于某些需要传入 path 的函数，
 * path 的开头必须为该值，如 /books/test.epub
 * 否则将会出错
 */
#define BOOK_PATH          "/books/"
#define BOOK_MARK_PATH     (BOOK_PATH "mark/")
#define BOOK_PROGRESS_PATH (BOOK_PATH "progress/")

typedef enum {
    BOOK_TYPE_UNKNOWN = 0,
    BOOK_TYPE_TXT,
    BOOK_TYPE_EPUB,
} BookType_t;

typedef struct {
    BookType_t type;
    char book_name[64];
    char author[64];
} MetaData_t;

typedef struct {
    char buffer[2048];
    uint16_t cur_page;
    uint16_t total_page;
} PageContent_t;

typedef struct {
    /* 此部分由服务层自行处理，因此不需外露 */
    struct {
        FIL fp;
        bool is_open;
    } priv; // 私有变量

    MetaData_t meta;
    PageContent_t ctn;
} BookReader_t;

void ebook_open_book(BookReader_t *br, const char *path);
void ebook_close_book(BookReader_t *br);
void ebook_prev_page(BookReader_t *br);
void ebook_next_page(BookReader_t *br);
void ebook_goto_page(BookReader_t *br, uint16_t page);
void ebook_save_progress(BookReader_t *br);
void ebook_load_progress(BookReader_t *br);
void ebook_save_bookmark(BookReader_t *br);
void ebook_load_bookmark(BookReader_t *br);
void ebook_list_books(const char *book_dir, storage_listdir_cb cb);

void ebook_test();
