#include "rtt_srv.h"
#include "SEGGER_RTT.h"
#include <stdarg.h>
#include <stdint.h>


/*
 * 目前该文件的设计只是单纯为了提供一套兼容层
 * 避免 RTT 的内容直接外泄，由于使用示例本身比较简单，
 * 这里提供的兼容层也会比较简单，
 * 我个人认为最好不要占用 `printf`，
 * 因为如果用这玩意对协处理器或者什么其他外设发串口消息的话“发不出去”会查半天的
 */

#define DEFAULT_BUFFER_IDX 0

static const char *color_list[] = {
    [rtt_color_rst] = RTT_CTRL_RESET,
    [rtt_color_clear] = RTT_CTRL_CLEAR,
    [rtt_color_text_black] = RTT_CTRL_TEXT_BLACK,
    [rtt_color_text_red] = RTT_CTRL_TEXT_RED,
    [rtt_color_text_green] = RTT_CTRL_TEXT_GREEN,
    [rtt_color_text_yellow] = RTT_CTRL_TEXT_YELLOW,
    [rtt_color_text_blue] = RTT_CTRL_TEXT_BLUE,
    [rtt_color_text_magenta] = RTT_CTRL_TEXT_MAGENTA,
    [rtt_color_text_cyan] = RTT_CTRL_TEXT_CYAN,
    [rtt_color_text_white] = RTT_CTRL_TEXT_WHITE,
    [rtt_color_text_bright_black] = RTT_CTRL_TEXT_BRIGHT_BLACK,
    [rtt_color_text_bright_red] = RTT_CTRL_TEXT_BRIGHT_RED,
    [rtt_color_text_bright_green] = RTT_CTRL_TEXT_BRIGHT_GREEN,
    [rtt_color_text_bright_yellow] = RTT_CTRL_TEXT_BRIGHT_YELLOW,
    [rtt_color_text_bright_blue] = RTT_CTRL_TEXT_BRIGHT_BLUE,
    [rtt_color_text_bright_magenta] = RTT_CTRL_TEXT_BRIGHT_MAGENTA,
    [rtt_color_text_bright_cyan] = RTT_CTRL_TEXT_BRIGHT_CYAN,
    [rtt_color_text_bright_white] = RTT_CTRL_TEXT_BRIGHT_WHITE,
    [rtt_color_bg_black] = RTT_CTRL_BG_BLACK,
    [rtt_color_bg_red] = RTT_CTRL_BG_RED,
    [rtt_color_bg_green] = RTT_CTRL_BG_GREEN,
    [rtt_color_bg_yellow] = RTT_CTRL_BG_YELLOW,
    [rtt_color_bg_blue] = RTT_CTRL_BG_BLUE,
    [rtt_color_bg_magenta] = RTT_CTRL_BG_MAGENTA,
    [rtt_color_bg_cyan] = RTT_CTRL_BG_CYAN,
    [rtt_color_bg_white] = RTT_CTRL_BG_WHITE,
    [rtt_color_bg_bright_black] = RTT_CTRL_BG_BRIGHT_BLACK,
    [rtt_color_bg_bright_red] = RTT_CTRL_BG_BRIGHT_RED,
    [rtt_color_bg_bright_green] = RTT_CTRL_BG_BRIGHT_GREEN,
    [rtt_color_bg_bright_yellow] = RTT_CTRL_BG_BRIGHT_YELLOW,
    [rtt_color_bg_bright_blue] = RTT_CTRL_BG_BRIGHT_BLUE,
    [rtt_color_bg_bright_magenta] = RTT_CTRL_BG_BRIGHT_MAGENTA,
    [rtt_color_bg_bright_cyan] = RTT_CTRL_BG_BRIGHT_CYAN,
    [rtt_color_bg_bright_white] = RTT_CTRL_BG_BRIGHT_WHITE,
};

void rtt_init(void)
{
    SEGGER_RTT_Init();
}

/**
 * @brief print strings with formations
 *
 * @retval:
 *  >= 0: number of bytes has sent
 *  <  0: error
 */
static int rtt_vprintf(const char *sFormat, va_list *args)
{
    return SEGGER_RTT_vprintf(DEFAULT_BUFFER_IDX, sFormat, args);
}

/**
 * @brief print strings with formations
 *
 * @retval:
 *  >= 0: number of bytes has sent
 *  <  0: error
 */
int rtt_printf(const char *sFormat, ...)
{
    va_list args;
    va_start(args, sFormat);
    int r = rtt_vprintf(sFormat, &args);
    va_end(args);
    return r;
}

/**
 * @brief print colored strings with formations
 *
 * @retval:
 *  >= 0: number of bytes has sent
 *  <  0: error
 */
int rtt_cprintf(rtt_color_t color, const char *sFormat, ...)
{
    va_list args;

    SEGGER_RTT_WriteString(DEFAULT_BUFFER_IDX, color_list[color]);

    va_start(args, sFormat);
    int r = rtt_vprintf(sFormat, &args);
    va_end(args);

    SEGGER_RTT_WriteString(DEFAULT_BUFFER_IDX, color_list[rtt_color_rst]);

    return r;
}

/**
 * @brief clear screen
 */
void rtt_clear(void)
{
    SEGGER_RTT_WriteString(DEFAULT_BUFFER_IDX, color_list[rtt_color_clear]);
}

/**
 * @brief check if read buffer has available characters waiting for reading
 *
 * @retval: number of bytes available
 */
int rtt_haskey(void)
{
    return SEGGER_RTT_HasKey();
}

/**
 * @brief non-blocking read a single character,
 *        it directly catches the input character from your keyboard
 *
 * @retval:
 *  >= 0: the character
 *  -1:   no data
 */
int rtt_getkey(void)
{
    return SEGGER_RTT_GetKey();
}

/**
 * @brief read a line until newline, with backspace support
 *
 * @retval: pointer to buf
 */
char *rtt_gets(char *buf, int size)
{
    int i = 0;
    int c;

    while (i < size - 1)
    {
        c = SEGGER_RTT_WaitKey();

        if (c == '\r' || c == '\n')
        {
            /* 回车结束，回显换行 */
            buf[i] = '\0';
            SEGGER_RTT_Write(DEFAULT_BUFFER_IDX, "\r\n", 2);
            return buf;
        }

        if (c == '\b' || c == 0x7F)
        {
            /* 退格：删除上一个字符并回显擦除 */
            if (i > 0)
            {
                i--;
                SEGGER_RTT_Write(DEFAULT_BUFFER_IDX, "\b \b", 3);
            }
            continue;
        }

        /* 存入字符 */
        buf[i++] = (char)c;
    }

    buf[i] = '\0';
    return buf;
}
