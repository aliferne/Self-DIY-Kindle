#pragma once

#include <stdint.h>

typedef struct {
    const uint8_t *name;
    const uint8_t *author;
    uint16_t current_chapter;
    uint16_t total_chapter;
    uint8_t *page_buf;

    /* 读写锁，
     * 如果有上锁操作则意味着数据正在被修改，
     * 此时不应该读取任何信息（可能不准） 
     * 对于这种需求应当考虑使用互斥锁
     */
    uint8_t locked: 1; 
} Ebook_t;

// TODO: 可以参考一下
// https://oshwhub.com/w_jy12_3/li-chuang-development-board-lian

void ebook_srv_test();
