#pragma once

#include "ff.h"
#include "storage_srv.h"
#include "srv_config.h"
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

#define BOOK_STORAGE        (&sdcard_storage)
/*
 * 书本文件的路径，
 * 对于某些需要传入 path 的函数，
 * path 的开头必须为该值，如 /books/test.epub
 * 否则将会出错
 */
#define BOOK_PATH          "/books/"
#define BOOK_MARK_PATH     (BOOK_PATH "mark/")
#define BOOK_PROGRESS_PATH (BOOK_PATH "progress/")
#define PAGE_BUF_SIZE      (2048U)
#define BOOK_NAME_SIZE     (64U)

typedef enum {
    /* 这个编号是故意用的 -1, 不要修改 */
    BOOK_TYPE_UNKNOWN = -1,
    BOOK_TYPE_TXT,
    BOOK_TYPE_EPUB,
} BookType_t;

typedef struct {
    BookType_t type;
    char *pSuffix;                  // 后缀名，定位后缀起始点用
    char book_name[BOOK_NAME_SIZE]; // 书本名
    /* author 是一个比较次要的信息 */
} MetaData_t;

/*
 * 目前看来保存进度应该只需要当前页面，
 * 总页面作为附属信息加入即可，
 * 保存书签也差不多
 */
typedef struct {
    /* TODO: 有没有书的页面会大于 uint16_t 可表示范围？ */
    uint16_t cur_page;
    uint16_t total_page;
} Progress_Dump_t;

typedef struct {
    char buffer[PAGE_BUF_SIZE];
    Progress_Dump_t prog;
} PageContent_t;

typedef struct {
    /* 此部分由服务层自行处理，因此不需外露 */
    struct {
        FIL fp;
        QWORD fsize;
        bool is_open;
    } priv; // 私有变量

    MetaData_t meta;
    PageContent_t ctn;

    /*
     * func-oprs:
     *   - open/close book
     *   - go prev/next page
     *   - goto any page
     *   - list all books in a spec dir(but it should not be here)
     *   - save/load progress
     *   - save/load bookmarks when asked to
     *   - underline and add notes?
     *   - change rendering(like text style)
     *
     * but now we decided to only support text, then epub
     */
} BookReader_t;

void ebook_init(BookReader_t *br);
bool ebook_open_book(BookReader_t *br, const char *path);
void ebook_close_book(BookReader_t *br);
void ebook_prev_page(BookReader_t *br);
void ebook_cur_page(BookReader_t *br);
void ebook_next_page(BookReader_t *br);
void ebook_goto_page(BookReader_t *br, uint16_t page);
void ebook_save_progress(BookReader_t *br);
void ebook_load_progress(BookReader_t *br);
void ebook_save_bookmark(BookReader_t *br);
void ebook_load_bookmark(BookReader_t *br);
void ebook_list_books(const char *book_dir, storage_listdir_cb cb);

/* this should be only called in ebook_srv.c and ebook_srv_test.c */
BookType_t get_book_type_and_suffix(const char *path, char **suffix_idx);
