#include "ebook_srv.h"
#include "bsp_handle.h"
#include "ff.h"
#include "storage_srv.h"
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

const char *suffix_lists[] = {
    [BOOK_TYPE_TXT]  = ".txt",
    [BOOK_TYPE_EPUB] = ".epub",
};

#define BOOK_OPEN(br)       ((br)->priv.is_open = true)
#define BOOK_CLOSE(br)      ((br)->priv.is_open = false)
#define IS_BOOK_OPEN(br)    ((br)->priv.is_open == true)

#define IS_PATH_VALID(path) (                             \
    (strncmp(path, BOOK_PATH, strlen(BOOK_PATH)) == 0) && \
    get_book_type_and_suffix(path, NULL) != BOOK_TYPE_UNKNOWN)

/* 为 pSuffix 填充占位符 */
#define FILL_PSUFFIX(pSuffix) (pSuffix = ".unknown")

/* BUG: 这个 LOG 是硬编码的格式，这不好 */
#define LOG_PATH_ERROR(cur_path)                                              \
    LOG_ERROR(                                                                \
        "The path should contains %s or ends with .txt/.epub, but now: %s\n", \
        BOOK_PATH, cur_path);

/* 根据文件后缀返回书本类型 */
BookType_t get_book_type_and_suffix(const char *path, char **suffix_idx)
{
    char *suffix = NULL;
    size_t pl    = strlen(path);

    for (int i = 0; i < LEN(suffix_lists); i++) {
        const char *type_suffix = suffix_lists[i];
        size_t sl               = strlen(type_suffix);
        suffix                  = (char *)(path + pl - sl);

        // start of str => path[pl - sl]
        // e.g. /books/test.epub
        //      ↑          ↑
        //     path     path[pl-sl]
        if ((pl >= sl) &&
            (strcmp(suffix, type_suffix) == 0)) {
            if (suffix_idx != NULL) {
                *suffix_idx = (char *)suffix;
            }
            return (BookType_t)i;
        }
    }
    *suffix_idx = NULL;
    return BOOK_TYPE_UNKNOWN;
}

/* 书本初始化，后续再做，用于支持 txt 和 epub 等的多态 */
void ebook_init(BookReader_t *br)
{
    GIVEUP(br);
}

/* 打开书本 */
bool ebook_open_book(BookReader_t *br, const char *path)
{
    if (!IS_PATH_VALID(path)) {
        LOG_PATH_ERROR(path);
        return false;
    }

    if (IS_BOOK_OPEN(br)) {
        LOG_INFO("Book is already opened\n");
        return true;
    }

    FIL *fp           = &br->priv.fp;
    MetaData_t *meta  = &br->meta;
    char *book_name   = strrchr(path, '/');
    ASSERT_FAIL(
        /* NULL or only contains '/' */
        book_name == NULL || strlen(book_name) == 1,
        LOG_ERROR("Empty book name detected, can not open, return\n");
        return false);
    /* cross the '/' */
    book_name += 1;

    FRESULT res = storage_open(BOOK_STORAGE, fp, path, FA_READ);

    ASSERT_FAIL(
        res != FR_OK,
        LOG_ERROR("Failed to open book: %s\n", path);
        return false);

    LOG_INFO("Opened book: %s\n", path);
    meta->type = get_book_type_and_suffix(path, &meta->pSuffix);
    snprintf(meta->book_name, BOOK_NAME_SIZE, "%s", book_name);

    br->priv.fsize          = f_size(fp);
    br->ctn.prog.total_page = br->priv.fsize / PAGE_BUF_SIZE;

    BOOK_OPEN(br);

    LOG_INFO("Book opened\n");

    return true;
}

/* 关闭书本 */
void ebook_close_book(BookReader_t *br)
{
    if (!IS_BOOK_OPEN(br)) {
        LOG_INFO("Book is already closed\n");
        return;
    }

    if (f_close(&br->priv.fp) != FR_OK) {
        LOG_ERROR("Failed to close book\n");
        return;
    }

    LOG_INFO("Book closed\n");

    BOOK_CLOSE(br);
}

/* 上一页 */
void ebook_prev_page(BookReader_t *br)
{
    if (!IS_BOOK_OPEN(br)) {
        LOG_WARN("Book is not opened!\n");
        return;
    }

    ebook_goto_page(br, br->ctn.prog.cur_page - 1);
}

/* 当前页 */
void ebook_cur_page(BookReader_t *br)
{
    if (!IS_BOOK_OPEN(br)) {
        LOG_WARN("Book is not opened!\n");
        return;
    }

    ebook_goto_page(br, br->ctn.prog.cur_page);
}

/* 下一页 */
void ebook_next_page(BookReader_t *br)
{
    if (!IS_BOOK_OPEN(br)) {
        LOG_WARN("Book is not opened!\n");
        return;
    }

    ebook_goto_page(br, br->ctn.prog.cur_page + 1);
}

/* 前往指定页面 */
void ebook_goto_page(BookReader_t *br, uint16_t page)
{
    if (!IS_BOOK_OPEN(br)) {
        LOG_WARN("[line: %d] Book is not opened!\n", __LINE__);
        return;
    }

    ASSERT_FAIL(
        /* notice that unsigned can skip border check */
        page > br->ctn.prog.total_page,
        LOG_WARN("Invalid page (%lu / %lu) to go, return\n",
                 page, br->ctn.prog.total_page);
        return);

    /* 基本思路也就是获取偏移量然后定位到那里，接下来阅读一定的字节数 */
    FIL *fp     = &br->priv.fp;
    DWORD fsize = br->priv.fsize;
    char *buf   = br->ctn.buffer;

    DWORD offset = (DWORD)page * PAGE_BUF_SIZE;
    ASSERT_FAIL(
        offset > fsize,
        LOG_WARN("Offset 0x%lx exceeds file size 0x%lx\n", offset, fsize);
        return);

    FRESULT res = f_lseek(fp, offset);
    ASSERT_FAIL(
        res != FR_OK,
        LOG_ERROR("f_lseek failed, err: %d\n", res);
        return);

    DWORD bytes_to_read = PAGE_BUF_SIZE;
    if (offset + bytes_to_read > fsize) {
        bytes_to_read = fsize - offset;
    }

    UINT bytes_read = 0;

    res = f_read(fp, buf, bytes_to_read, &bytes_read);
    ASSERT_FAIL(
        res != FR_OK,
        LOG_ERROR("f_lseek failed, err: %d\n", res);
        return);

    /* ends with '\0' */
    buf[bytes_read] = '\0';

    br->ctn.prog.cur_page = page;

    LOG_DEBUG("Goto page %u, intend to read %u bytes, actually read %u bytes\n", page, bytes_to_read, bytes_read);
}

/* 保存当前进度，当退出阅读界面时调用 */
void ebook_save_progress(BookReader_t *br)
{
    if (!IS_BOOK_OPEN(br)) {
        LOG_WARN("Book is not opened!\n");
        return;
    }

    FIL fp;
    FRESULT res;

    const int MAX_PATH_LEN = 128;
    char path[MAX_PATH_LEN];
    int pres = snprintf(
        path,
        MAX_PATH_LEN,
        "%s%s",
        BOOK_PROGRESS_PATH,
        br->meta.book_name);

    if (pres >= MAX_PATH_LEN) {
        LOG_WARN("Output was truncated, required size: %d\n", pres);
    }

    storage_mkdir(BOOK_STORAGE, BOOK_PROGRESS_PATH);

    res = storage_open(
        BOOK_STORAGE, &fp,
        path, FA_WRITE | FA_CREATE_ALWAYS);

    ASSERT_FAIL(
        res != FR_OK,
        LOG_ERROR("Failed to open progress file: %s\n", path);
        return);

    UINT bw                    = 0;
    const int PROGRESS_BUF_LEN = 16;
    char progress[PROGRESS_BUF_LEN];
    /* basic formation: `prog: br->ctn.prog.cur_page` */
    snprintf(progress, PROGRESS_BUF_LEN, "prog: %u\n", br->ctn.prog.cur_page);
    res = storage_write(&fp, progress, PROGRESS_BUF_LEN, &bw);

    ASSERT_FAIL(
        res != FR_OK,
        LOG_ERROR("Failed to write progress file: %s\n", path));

    ASSERT_FAIL(
        bw != PROGRESS_BUF_LEN,
        LOG_ERROR("Full of storage, can not save progress for %s\n", path));

    storage_close(&fp);
}

/* 读取当前进度，当进入阅读界面时调用 */
void ebook_load_progress(BookReader_t *br)
{
    if (!IS_BOOK_OPEN(br)) {
        LOG_WARN("Book is not opened!\n");
        return;
    }

    FIL fp;
    FRESULT res = FR_OK;

    const int MAX_PATH_LEN = 128;
    char path[MAX_PATH_LEN];
    int pres = snprintf(
        path,
        MAX_PATH_LEN,
        "%s%s",
        BOOK_PROGRESS_PATH,
        br->meta.book_name);

    if (pres >= MAX_PATH_LEN) {
        LOG_WARN("Output was truncated, required size: %d\n", pres);
    }

    res = storage_open(
        BOOK_STORAGE, &fp,
        path, FA_READ);

    if (res == FR_NO_FILE || res == FR_NO_PATH) {
        LOG_DEBUG("No progress file for %s, start from page 0\n",
                  br->meta.book_name);
        br->ctn.prog.cur_page = 0;
        return;
    }

    ASSERT_FAIL(
        res != FR_OK,
        LOG_ERROR("Failed to open progress file: %s (err: %d)\n", path, res);
        return);

    char line[16];
    if (f_gets(line, sizeof(line), &fp) == NULL) {
        LOG_WARN("Progress file %s is empty, start from page 0\n", path);
        storage_close(&fp);
        br->ctn.prog.cur_page = 0;
        return;
    }
    storage_close(&fp);

    uint16_t page = 0;
    if (sscanf(line, "prog: %hu", &page) != 1) {
        LOG_WARN("Malformed progress file %s: \"%s\", start from page 0\n", path, line);
        br->ctn.prog.cur_page = 0;
        return;
    }

    ebook_goto_page(br, page);
}

/* Mark will be a later function */

/* 保存书签 */
void ebook_save_bookmark(BookReader_t *br)
{
    if (!IS_BOOK_OPEN(br)) {
        LOG_WARN("Book is not opened!\n");
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
