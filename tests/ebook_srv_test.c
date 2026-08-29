#include "bsp_handle.h"
#include "ebook_srv.h"
#include "bsp_sys.h"
#include <string.h>

extern const char *suffix_lists[];

/* test configurations */
#define TEST_GET_BOOK_TYPE_AND_SUFFIX 1

#define PRINT_HELP_MSG()              LOG_INFO(  \
    "\n\toption menu:\n"                         \
    "\tl. list specific dir\n"                   \
    "\to. open and read the content of a file\n" \
    "\tx. extract an epub book\n"                \
    "\th. print help info\n"                     \
    "\tt. test some customized functions\n"      \
    "\tc. clear screen\n"                        \
    "\tq. exit test\n")

#define PRINT_BOOK_READER_HELP_MSG()                      \
    LOG_INFO("Book opened, "                              \
             "now you can read it:\n"                     \
             "\tpress 'n' to next page,\n"                \
             "\t'p' to prev page,\n"                      \
             "\t'q' to quit reading,\n"                   \
             "\t'h' to get help message,\n"               \
             "\t'c' to clear screen and print content,\n" \
             "\t's' to save current progress,\n"          \
             "\t'i' to display book info\n",              \
             "\tNotice that when you quit, the progress will be saved\n")
/*
 * ERROR:
 *   too many format specifiers leads to unexpected output (`total_page: 0`)
 *   when concatenating those strings into one `LOG_INFO` call
 *   but ok when separated into two calls, so I just do that
 *
 * `suffix_lists[br.meta.type] + 1` is used to skip the dot in the suffix
 */
#define PRINT_BOOK_READER_INFO(br)               \
    do {                                         \
        LOG_INFO(                                \
            "\n\tBook Name: %s\n"                \
            "\tBook Type: %s\n"                  \
            "\tFile Size: %u\n",                 \
            (br).meta.book_name,                 \
            suffix_lists[(br).meta.type] + 1,    \
            (br).priv.fsize);                    \
        LOG_INFO(                                \
            "\n\tCurrent/Total Page: %hu/%hu\n", \
            (br).ctn.prog.cur_page,              \
            (br).ctn.prog.total_page);           \
    } while (0)

#define PRINT_BOOK_READER_CONTENT(br)      \
    do {                                   \
        LOG_INFO("Book Content:\n");       \
        LOG_NORMAL("%s", (br).ctn.buffer); \
    } while (0);

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
    LOG_DEBUG("in ebook test mode.\n");
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

            if (!ebook_open_book(&reader, path))
                continue;

            ebook_load_progress(&reader);
            PRINT_BOOK_READER_HELP_MSG();

            char c = 0;
            while ((c = LOG_GET_CHAR()) != 'q') {
                if (c == 'n') {
                    ebook_next_page(&reader);
                } else if (c == 'p') {
                    ebook_prev_page(&reader);
                } else if (c == 'h') {
                    PRINT_BOOK_READER_HELP_MSG();
                } else if (c == 'i') {
                    PRINT_BOOK_READER_INFO(reader);
                } else if (c == 'c') {
                    LOG_CLEAR_SCREEN();
                    PRINT_BOOK_READER_CONTENT(reader);
                } else if (c == 's') {
                    ebook_save_progress(&reader);
                }
            }

            ebook_save_progress(&reader);
            ebook_close_book(&reader);
        } else if (ch == 'x') {
            LOG_INFO("Please input a path: ");
            LOG_GET_STR(path, 128);
            LOG_INFO("Content in %s:\n\t", path);
        } else if (ch == 't') {

#if TEST_GET_BOOK_TYPE_AND_SUFFIX
            char *testPrefix     = "[TEST_GET_BOOK_TYPE_AND_SUFFIX]";
            char *pSuffix        = ".unknown";
            const char *name1    = "test.txt";
            const char *name2    = "test.epub";
            const char *name3    = "test";
            BookType_t book_type = BOOK_TYPE_UNKNOWN;

            LOG_INFO("%s total 3 tests, silent when passed.\n", testPrefix);

            book_type = get_book_type_and_suffix(name1, &pSuffix);
            ASSERT((book_type == BOOK_TYPE_TXT) &&
                       (strcmp(pSuffix, suffix_lists[BOOK_TYPE_TXT]) == 0),
                   LOG_ERROR("%s book type or suffix not matched"
                             "(bookname: %s, detected booktype: %d, "
                             "detected suffix: %s)\n",
                             testPrefix,
                             name1, book_type, *pSuffix));

            book_type = get_book_type_and_suffix(name2, &pSuffix);
            ASSERT((book_type == BOOK_TYPE_EPUB) &&
                       (strcmp(pSuffix, suffix_lists[BOOK_TYPE_EPUB]) == 0),
                   LOG_ERROR("%s book type or suffix not matched"
                             "(bookname: %s, detected booktype: %d, "
                             "detected suffix: %s)\n",
                             testPrefix,
                             name2, book_type, *pSuffix));

            book_type = get_book_type_and_suffix(name3, &pSuffix);
            ASSERT(book_type == BOOK_TYPE_UNKNOWN,
                   LOG_ERROR("%s book type or suffix not matched"
                             "(bookname: %s, detected booktype: %d, "
                             "detected suffix: %s)\n",
                             testPrefix,
                             name3, book_type, *pSuffix));

            LOG_INFO("%s all tests done.\n", testPrefix);
#endif /* TEST_GET_BOOK_TYPE_AND_SUFFIX */
        }
    }

    LOG_DEBUG("exit ebook test mode.\n");
}
