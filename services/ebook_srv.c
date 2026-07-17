#include "bsp_handle.h"
#include "srv_config.h"
#include "storage_srv.h"
#include <stdbool.h>

#define LOG_HEADER "[ebook service]"
#define BOOK_PATH "/books/"
#define BOOK_MARK_PATH (BOOK_PATH "marks/")
#define BOOK_STORAGE (&sdcard)

static DIR book_dir;
static FIL book;

void ebook_srv_init(void)
{
    LOG_INFO("%s initializing,"
             "please ensure all books are in %s\n",
             LOG_HEADER,
             BOOK_PATH);
    storage_opendir(BOOK_STORAGE, &book_dir, BOOK_PATH);

    LOG_INFO("%s initialized.", LOG_HEADER);
}

void ebook_srv_deinit(void)
{
    LOG_INFO("%s deinitializing.\n", LOG_HEADER);
    storage_close(&book);
    storage_closedir(&book_dir);
    LOG_INFO("%s deinitialized.\n", LOG_HEADER);
}

void ebook_srv_list_books(const char *path)
{
    BOOK_PATH
}

#define PRINT_HELP_MSG() LOG_INFO( \
    "%s\n\toption menu:\n"         \
    "\tl. list dir\n"              \
    "\t2. do something\n"          \
    "\t3. do something\n"          \
    "\t4. do something\n"          \
    "\th. print help info\n"       \
    "\tc. clear screen\n"          \
    "\tq. exit testing.\n",        \
    LOG_HEADER);

/* 用于测试 listdir 的辅助测试函数 */
static void print_dir_files(char *fname, uint64_t fsize, bool is_dir)
{
    LOG_DEBUG("File Type: %d, File Name: %s, File Size: %d(is_valid: %d)\n",
              is_dir, fname, fsize, !is_dir);
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
            LOG_INFO("Please input a path: ");
            LOG_GET_STR(path, 128);
            LOG_INFO("\nFiles in path: %s\n", path);
            storage_listdir(&sdcard, path, print_dir_files);
        }
    }

    LOG_DEBUG("%s exit test mode.\n", LOG_HEADER);
}
