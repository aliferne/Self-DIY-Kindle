#include "ebook_srv.h"
#include "bsp_handle.h"
#include "ff.h"
#include "srv_config.h"
#include "storage_srv.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#define LOG_HEADER          "[ebook service]"
#define BOOK_STORAGE        (&sdcard)

#define BOOK_OPEN(br)       ((br)->priv.is_open = true)
#define BOOK_CLOSE(br)      ((br)->priv.is_open = false)
#define IS_BOOK_OPEN(br)    ((br)->priv.is_open == true)

#define IS_PATH_VALID(path) (                             \
    (strncmp(path, BOOK_PATH, strlen(BOOK_PATH)) == 0) && \
    get_book_type(path) != BOOK_TYPE_UNKNOWN)
/* BUG: 这个 LOG 是硬编码的格式，这不好 */
#define LOG_PATH_ERROR(cur_path)                                                 \
    LOG_ERROR(                                                                   \
        "%s The path should contains %s or ends with .txt/.epub, but now: %s\n", \
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

    for (uint32_t i = 0; i < LEN(suffix_type_map); i++) {
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

/* 书本初始化，后续再做，用于支持 txt 和 epub 等的多态 */
void ebook_init(BookReader_t *br)
{
    GIVEUP(br);
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
    memcpy(meta->book_name, book_name, BOOK_NAME_SIZE);
    meta->type = get_book_type(path);

    br->priv.fsize     = (fp->obj.sclust != 0) ? f_size(fp) : 0;
    br->ctn.total_page = br->priv.fsize / PAGE_BUF_SIZE;

    BOOK_OPEN(br);
}

/* 关闭书本 */
void ebook_close_book(BookReader_t *br)
{
    if (f_close(&br->priv.fp) != FR_OK) {
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

    ebook_goto_page(br, br->ctn.cur_page - 1);
}

/* 下一页 */
void ebook_next_page(BookReader_t *br)
{
    if (!IS_BOOK_OPEN(br)) {
        LOG_WARN("%s [line: %d] Book is not opened!\n", LOG_HEADER, __LINE__);
        return;
    }

    ebook_goto_page(br, br->ctn.cur_page + 1);
}

/* 前往指定页面 */
void ebook_goto_page(BookReader_t *br, uint16_t page)
{
    if (!IS_BOOK_OPEN(br)) {
        LOG_WARN("%s [line: %d] Book is not opened!\n", LOG_HEADER, __LINE__);
        return;
    }

    /*
     * 由于 uint16_t 不可表示负数，
     * 因此这种情况要么是 0 - 1 回环到 0xFFFF 了，要么是 page + 1 > total_page 了
     * WARN: 这里 page 是从 1 开始的
     */
    ASSERT_FAIL(
        page > br->ctn.total_page,
        LOG_WARN("%s Invalid page (%lu / %lu) to go, return",
                 LOG_HEADER, page, br->ctn.total_page);
        return);

    /* 基本思路也就是获取偏移量然后定位到那里，接下来阅读一定的字节数 */
    FIL *fp     = &br->priv.fp;
    DWORD fsize = br->priv.fsize;
    char *buf   = br->ctn.buffer;

    DWORD offset = (DWORD)page * PAGE_BUF_SIZE;
    ASSERT_FAIL(
        offset > fsize,
        LOG_WARN("%s Offset 0x%lx exceeds file size 0x%lx\n", LOG_HEADER, offset, fsize);
        return);

    FRESULT res = f_lseek(fp, offset);
    ASSERT_FAIL(
        res != FR_OK,
        LOG_ERROR("%s f_lseek failed, err: %d\n", LOG_HEADER, res);
        return);

    DWORD bytes_to_read = PAGE_BUF_SIZE;
    if (offset + bytes_to_read > fsize) {
        bytes_to_read = fsize - offset;
    }

    UINT bytes_read = 0;
    res             = f_read(fp, buf, bytes_to_read, &bytes_read);
    if (res != FR_OK) {
        LOG_ERROR("%s f_read failed, err: %d\n", LOG_HEADER, res);
        return;
    }

    buf[bytes_read] = '\0';

    br->ctn.cur_page = page;

    LOG_DEBUG("%s Goto page %u, read %u bytes\n", LOG_HEADER, page, bytes_read);
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
    static BookReader_t reader;

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
            // FIXME: 这些操作最好得返回错误码
            ebook_open_book(&reader, path);
            /* 先这样简单测一下，毕竟加载功能还没写 */

            char c = 0;
            while ((c = LOG_GET_CHAR()) != 'q') {
                if (c == 'n') {
                    ebook_next_page(&reader);
                } else if (c == 'p') {
                    ebook_prev_page(&reader);
                }
            }

            ebook_close_book(&reader);
            LOG_INFO("Book closed\n");
        } else if (ch == 'x') {
            LOG_INFO("Please input a path: ");
            LOG_GET_STR(path, 128);
            LOG_INFO("Content in %s:\n\t", path);
        }
    }

    LOG_DEBUG("%s exit test mode.\n", LOG_HEADER);
}
