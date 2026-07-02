#include <stdint.h>
#include "lvgl.h"
#include "storage_srv.h"
#include "bsp_handle.h"
#include "srv_config.h"

#define LOG_HEADER   "[ebook service]"
#define BOOK_PATH    "/books/"
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

void ebook_srv_list_books()
{
}

#define PRINT_HELP_MSG() LOG_INFO( \
    "%s\n\toption menu:\n"         \
    "\t1. do something\n"          \
    "\t2. do something\n"          \
    "\t3. do something\n"          \
    "\t4. do something\n"          \
    "\th. print help info\n"       \
    "\tc. clear screen\n"          \
    "\tq. exit testing.\n",        \
    LOG_HEADER);

void ebook_srv_test()
{
    LOG_DEBUG("%s in test mode.\n", LOG_HEADER);
    PRINT_HELP_MSG();

    char ch = 0;
    while ((ch = LOG_GET_CHAR()) != 'q') {
        if (ch == 'c')
            LOG_CLEAR_SCREEN();
        else if (ch == 'h')
            PRINT_HELP_MSG();
    }

    LOG_DEBUG("%s exit test mode.\n", LOG_HEADER);
}
