#ifndef __RTT_SRV_H__
#define __RTT_SRV_H__

/*
 * basic supports for `printf` and (u)int-typedefs
 */
#include <stdio.h>
#include <stdint.h>

typedef enum {
    rtt_color_rst = 0,
    rtt_color_clear,
    rtt_color_text_black,
    rtt_color_text_red,
    rtt_color_text_green,
    rtt_color_text_yellow,
    rtt_color_text_blue,
    rtt_color_text_magenta,
    rtt_color_text_cyan,
    rtt_color_text_white,
    rtt_color_text_bright_black,
    rtt_color_text_bright_red,
    rtt_color_text_bright_green,
    rtt_color_text_bright_yellow,
    rtt_color_text_bright_blue,
    rtt_color_text_bright_magenta,
    rtt_color_text_bright_cyan,
    rtt_color_text_bright_white,
    rtt_color_bg_black,
    rtt_color_bg_red,
    rtt_color_bg_green,
    rtt_color_bg_yellow,
    rtt_color_bg_blue,
    rtt_color_bg_magenta,
    rtt_color_bg_cyan,
    rtt_color_bg_white,
    rtt_color_bg_bright_black,
    rtt_color_bg_bright_red,
    rtt_color_bg_bright_green,
    rtt_color_bg_bright_yellow,
    rtt_color_bg_bright_blue,
    rtt_color_bg_bright_magenta,
    rtt_color_bg_bright_cyan,
    rtt_color_bg_bright_white,
} rtt_color_t;

void rtt_init(void);
int rtt_printf(const char *sFormat, ...);
int rtt_cprintf(rtt_color_t color, const char *sFormat, ...);
void rtt_clear(void);
int rtt_haskey(void);
int rtt_getkey(void);
char *rtt_gets(char *buf, int size);

/* FIXME: can't print floating numbers, behavior of `rtt_clear` seems to be a little strange */
#define RTT_LOG_ERROR(sFormat, ...)  rtt_cprintf(rtt_color_text_bright_red,   "[ERROR] " sFormat, ##__VA_ARGS__)
#define RTT_LOG_WARN(sFormat, ...)   rtt_cprintf(rtt_color_text_yellow,       "[WARN] "  sFormat, ##__VA_ARGS__)
#define RTT_LOG_INFO(sFormat, ...)   rtt_cprintf(rtt_color_text_bright_green, "[INFO] "  sFormat, ##__VA_ARGS__)
#define RTT_LOG_DEBUG(sFormat, ...)  rtt_cprintf(rtt_color_text_bright_cyan,  "[DEBUG] " sFormat, ##__VA_ARGS__)
#define RTT_HAS_CHARS()              rtt_haskey()
#define RTT_GET_CHAR()               (char)(rtt_getkey())
#define RTT_GET_STR(buf, size)       rtt_gets((buf), (size))

#endif
