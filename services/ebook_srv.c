#include "ebook_srv.h"
#include "bsp_handle.h"
#include "ff.h"
#include "srv_config.h"
#include "storage_srv.h"
#include <stdbool.h>
#include <string.h>

#define LOG_HEADER          "[ebook service]"
#define BOOK_STORAGE        (&sdcard)

#define BOOK_OPEN(br)       ((br)->priv.is_open = true)
#define BOOK_CLOSE(br)      ((br)->priv.is_open = false)
#define IS_BOOK_OPEN(br)    ((br)->priv.is_open == true)

#define IS_PATH_VALID(path) (strncmp(path, BOOK_PATH, strlen(BOOK_PATH)) == 0)
#define LOG_PATH_ERROR(cur_path)                         \
    LOG_ERROR(                                           \
        "%s The path should contains %s, but now: %s\n", \
        BOOK_PATH, cur_path);

/* 根据文件后缀返回书本类型 */
static BookType_t get_book_type(const char *path)
{
    static const struct
    {
        const char *suffix;
        BookType_t type;
    } suffix_type_map[] = {
        {".txt", BOOK_TYPE_TXT},
        {".epub", BOOK_TYPE_EPUB},
    };

    for (int i = 0; i < LEN(suffix_type_map); i++) {
        size_t pl = strlen(path);
        size_t sl = strlen(suffix_type_map[i].suffix);
        if ((pl >= sl) &&
            // start of str => path[pl - sl]
            // e.g. /books/test.epub
            //      ↑          ↑
            //     path     path[pl-sl]
            (strcmp(path + pl - sl, suffix_type_map[i].suffix) == 0)) {
            return suffix_type_map[i].type;
        }
    }
    return BOOK_TYPE_UNKNOWN;
}

/* 打开书本 */
void ebook_open_book(BookReader_t *br, const char *path)
{
    if (!IS_PATH_VALID(path)) {
        LOG_PATH_ERROR(path);
        return;
    }

    FIL *fp          = &br->priv.fp;
    MetaData_t *meta = &br->meta;
    char *book_name  = strrchr(path, '/');
    FRESULT res      = storage_open(BOOK_STORAGE, fp, path, FA_READ);

    if (res != FR_OK) {
        LOG_ERROR("%s Failed to open book: %s\n", LOG_HEADER, path);
        return;
    }

    LOG_INFO("%s Opened book: %s\n", LOG_HEADER, path);
    memcpy(meta->book_name, book_name, LEN(meta->book_name));
    meta->type = get_book_type(path);

    br->priv.fsize     = (fp->obj.sclust != 0) ? f_size(fp) : 0;
    br->ctn.total_page = br->priv.fsize / LEN(br->ctn.buffer);

    BOOK_OPEN(br);
}

/* 关闭书本 */
void ebook_close_book(BookReader_t *br)
{
    if (storage_close(&br->priv.fp) != FR_OK) {
        LOG_ERROR("%s Failed to close book\n", LOG_HEADER);
        return;
    }

    LOG_INFO("%s Closed book\n", LOG_HEADER);
    BOOK_CLOSE(br);
}

/* 上一页 */
void ebook_prev_page(BookReader_t *br)
{
    if (!IS_BOOK_OPEN(br)) {
        LOG_WARN("%s [line: %d] Book is not opened!\n", LOG_HEADER, __LINE__);
        return;
    }
    /* 阅读，然后填入缓冲区 */
    __NOT_USED char *buf = br->ctn.buffer;
}

/* 下一页 */
void ebook_next_page(BookReader_t *br)
{
    if (!IS_BOOK_OPEN(br)) {
        LOG_WARN("%s [line: %d] Book is not opened!\n", LOG_HEADER, __LINE__);
        return;
    }
    /* 阅读，然后填入缓冲区 */
    __NOT_USED char *buf = br->ctn.buffer;
}

/* 前往指定页面 */
void ebook_goto_page(BookReader_t *br, uint16_t page)
{
    if (!IS_BOOK_OPEN(br)) {
        LOG_WARN("%s [line: %d] Book is not opened!\n", LOG_HEADER, __LINE__);
        return;
    }
    /* 阅读，然后填入缓冲区 */
    __NOT_USED char *buf = br->ctn.buffer;
}

/* 保存当前进度，当退出阅读界面时调用 */
void ebook_save_progress(BookReader_t *br)
{
    if (!IS_BOOK_OPEN(br)) {
        LOG_WARN("%s [line: %d] Book is not opened!\n", LOG_HEADER, __LINE__);
        return;
    }
    GIVEUP(br);
}

/* 读取当前进度，当进入阅读界面时调用 */
void ebook_load_progress(BookReader_t *br)
{
    GIVEUP(br);
}

/* 保存书签 */
void ebook_save_bookmark(BookReader_t *br)
{
    if (!IS_BOOK_OPEN(br)) {
        LOG_WARN("%s [line: %d] Book is not opened!\n", LOG_HEADER, __LINE__);
        return;
    }
    GIVEUP(br);
}

/* 加载书签 */
void ebook_load_bookmark(BookReader_t *br)
{
    GIVEUP(br);
}

/* 展示书籍目录下的书本 */
void ebook_list_books(const char *book_dir, storage_listdir_cb cb)
{
    (IS_PATH_VALID(book_dir))
        ? storage_listdir(BOOK_STORAGE, book_dir, cb)
        : LOG_PATH_ERROR(book_dir);
}

/**
 * \brief 阅读电子书
 *
 * \param book_path: 书籍路径
 * \param buf: 存储缓冲区
 * \param len: 待读取长度
 */
__DEPRECATED("waiting for another implementation")
void ebook_srv_read_book(const char *book_path, void *buf, size_t len)
{
    FRESULT res;
    static FIL book_file;
    static char *last_book_path;
    static bool is_opened = false;

    if (!is_opened || book_path != last_book_path) {
        res = storage_open(BOOK_STORAGE, &book_file, book_path, FA_READ);
        if (res != FR_OK)
            goto err_open;

        is_opened      = true;
        last_book_path = (char *)book_path;
    }

    UINT byte_read = 0;
    res            = storage_read(&book_file, buf, len, &byte_read);
    if (res != FR_OK)
        goto err_read;

err_read:
    LOG_INFO(
        "%s read %s failed(errcode: %d)\n",
        LOG_HEADER, book_path, res);
    storage_close(&book_file);
    is_opened = false;
    return;

err_open:
    LOG_INFO(
        "%s open %s failed(errcode: %d)\n",
        LOG_HEADER, book_path, res);
    return;
}

/* test contents ----------------------------------- */

#define PRINT_HELP_MSG() LOG_INFO(               \
    "%s\n\toption menu:\n"                       \
    "\tl. list specific dir\n"                   \
    "\to. open and read the content of a file\n" \
    "\tx. extract an epub book\n"                \
    "\th. print help info\n"                     \
    "\tc. clear screen\n"                        \
    "\tq. exit test\n",                          \
    LOG_HEADER);

/* 用于测试 listdir 的辅助测试函数 */
static bool print_dir_files(char *fname, uint64_t fsize, bool is_dir)
{
    LOG_DEBUG(
        "File Type: %s, File Name: %s, File Size: %d\n",
        is_dir == 1 ? "directory" : "file", fname, fsize);

    return true;
}

void ebook_test()
{
    LOG_DEBUG("%s in test mode.\n", LOG_HEADER);
    PRINT_HELP_MSG();

    char ch = 0;
    static char path[128];

    while ((ch = LOG_GET_CHAR()) != 'q') {
        if (ch == 'c') {
            LOG_CLEAR_SCREEN();
        } else if (ch == 'h') {
            PRINT_HELP_MSG();
        } else if (ch == 'l') {
            LOG_INFO("Please input a path(press enter to see files in root dir): ");
            LOG_GET_STR(path, 128);
            LOG_INFO("\nFiles in path: %s\n", path);
            storage_listdir(BOOK_STORAGE, path, print_dir_files);
        } else if (ch == 'o') {
            LOG_INFO("Please input a path: ");
            LOG_GET_STR(path, 128);
            LOG_INFO("Content in %s:\n\t", path);
        } else if (ch == 'x') {
            LOG_INFO("Please input a path: ");
            LOG_GET_STR(path, 128);
            LOG_INFO("Content in %s:\n\t", path);
        }
    }

    LOG_DEBUG("%s exit test mode.\n", LOG_HEADER);
}
