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

void ebook_srv_init(void)
{
    LOG_INFO("%s initializing,"
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

void ebook_srv_list_books(storage_listdir_cb cb)
{
    if (is_dir_init)
        storage_listdir(BOOK_STORAGE, BOOK_PATH, cb);
    else
        LOG_WARN("%s Book dir has not been intialized\n", LOG_HEADER);
}

#define PRINT_HELP_MSG() LOG_INFO( \
    "%s\n\toption menu:\n"         \
    "\tl. list specific dir\n"     \
    "\th. print help info\n"       \
    "\tc. clear screen\n"          \
    "\tq. exit test\n",            \
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
            static char path[128];
            LOG_INFO("Please input a path(press enter to see files in root dir): ");
            LOG_GET_STR(path, 128);
            LOG_INFO("\nFiles in path: %s\n", path);
            storage_listdir(BOOK_STORAGE, path, print_dir_files);
        }
    }

    LOG_DEBUG("%s exit test mode.\n", LOG_HEADER);
}
