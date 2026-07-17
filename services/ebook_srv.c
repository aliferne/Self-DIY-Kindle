#include "bsp_handle.h"
#include "ff.h"
#include "srv_config.h"
#include "storage_srv.h"
#include <stdbool.h>

#define LOG_HEADER "[ebook service]"
#define BOOK_PATH "/books/"
#define BOOK_MARK_PATH (BOOK_PATH "marks/")
#define BOOK_STORAGE (&sdcard)

static DIR book_dir;
static FIL book;
static bool is_dir_init = false;
static bool is_book_opened = false;

/* 电子书服务初始化 */
void ebook_srv_init(void)
{
    LOG_INFO(
        "%s initializing,"
        "please ensure all books are in %s\n",
        LOG_HEADER,
        BOOK_PATH);

    FRESULT res;
    res = storage_opendir(BOOK_STORAGE, &book_dir, BOOK_PATH);
    if (res != FR_OK)
    {
        LOG_ERROR("%s failed to open dir (errcode: %d)\n", res);
        return;
    }
    is_dir_init = true;

    LOG_INFO("%s initialized.", LOG_HEADER);
}

/* 电子书服务去初始化 */
void ebook_srv_deinit(void)
{
    LOG_INFO("%s deinitializing.\n", LOG_HEADER);

    if (is_book_opened)
        storage_close(&book);
    if (is_dir_init)
        storage_closedir(&book_dir);

    is_dir_init = false;
    LOG_INFO("%s deinitialized.\n", LOG_HEADER);
}

/* 电子书服务展示书籍目录下的书本 */
void ebook_srv_list_books(storage_listdir_cb cb)
{
    // TODO: 不太灵活，只能展示 BOOK_PATH 的书
    if (is_dir_init)
        storage_listdir(BOOK_STORAGE, BOOK_PATH, cb);
    else
        LOG_WARN("%s Book dir has not been intialized\n", LOG_HEADER);
}

/**
 * \brief 阅读电子书
 *
 * \param book_path: 书籍路径
 * \param buf: 存储缓冲区
 * \param len: 待读取长度
 */
void ebook_srv_read_book(const char *book_path, void *buf, size_t len)
{
    FRESULT res;
    static FIL book_file;
    static char *last_book_path;
    static bool is_opened = false;

    if (!is_opened || book_path != last_book_path)
    {
        res = storage_open(BOOK_STORAGE, &book_file, book_path, FA_READ);
        if (res != FR_OK)
            goto err_open;

        is_opened = true;
        last_book_path = (char *)book_path;
    }
    // TODO:
    UINT byte_read = 0;
    res = storage_read(&book_file, buf, len, &byte_read);
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

void ebook_srv_test()
{
    LOG_DEBUG("%s in test mode.\n", LOG_HEADER);
    PRINT_HELP_MSG();

    char ch = 0;
    static char path[128];

    while ((ch = LOG_GET_CHAR()) != 'q')
    {
        if (ch == 'c')
        {
            LOG_CLEAR_SCREEN();
        }
        else if (ch == 'h')
        {
            PRINT_HELP_MSG();
        }
        else if (ch == 'l')
        {
            LOG_INFO("Please input a path(press enter to see files in root dir): ");
            LOG_GET_STR(path, 128);
            LOG_INFO("\nFiles in path: %s\n", path);
            storage_listdir(BOOK_STORAGE, path, print_dir_files);
        }
        else if (ch == 'o')
        {
            LOG_INFO("Please input a path: ");
            LOG_GET_STR(path, 128);
            LOG_INFO("Content in %s:\n\t", path);
        }
        else if (ch == 'x')
        {
            LOG_INFO("Please input a path: ");
            LOG_GET_STR(path, 128);
            LOG_INFO("Content in %s:\n\t", path);
        }
    }

    LOG_DEBUG("%s exit test mode.\n", LOG_HEADER);
}
