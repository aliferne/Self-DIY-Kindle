#pragma once

/**
 * 简易环形队列实现
 *
 * 部分设计参考安富莱电子 bsp_msg.c/.h 文件
 */

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* MAX_FIFO_SIZE 见源文件 */

typedef struct {
    /* 数据缓冲区，可以是任意数据，最大到 MAX_FIFO_SIZE */
    void *buffer;
    /* 缓冲区大小，超出 MAX_FIFO_SIZE 时会强制令其为 MAX_FIFO_SIZE */
    uint8_t size;
    /* 数据大小 */
    size_t elem_size;
    /*
     * 一读一写指针
     *
     * 当读指针 == 写指针时， FIFO 为空
     * 当(写指针 + 1) % MAX_FIFO_SIZE == 读指针时，FIFO 为满
     */
    uint8_t read_index;
    uint8_t write_index;
} fifo_t;

/* 初始化 FIFO 缓冲区，size 为缓冲区大小，超出 MAX_FIFO_SIZE 时会强制令其为 MAX_FIFO_SIZE */
void fifo_init(fifo_t *fifo, void *buffer, uint8_t size, size_t elem_size);
/* 向 FIFO 中推送数据， forced: 是否强制覆盖已存在的数据 */
void fifo_push(fifo_t *fifo, void *data, bool forced);
/* 从 FIFO 中弹出数据 */
void fifo_pop(fifo_t *fifo, void *data);
/* 清空 FIFO 缓冲区 */
void fifo_clear(fifo_t *fifo);
/* 判断 FIFO 是否为空 */
bool fifo_is_empty(fifo_t *fifo);
/* 判断 FIFO 是否已满 */
bool fifo_is_full(fifo_t *fifo);
