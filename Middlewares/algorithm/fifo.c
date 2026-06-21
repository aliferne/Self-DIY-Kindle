#include "fifo.h"
#include <string.h>

#define MAX_FIFO_SIZE 32

void fifo_init(fifo_t *fifo, void *buffer, uint8_t buffer_size, size_t elem_size)
{
    if (fifo == NULL || buffer == NULL) return;
    if (buffer_size == 0 || elem_size == 0) return;

    memset(fifo, 0, sizeof(fifo_t));

    fifo->buffer    = buffer;
    fifo->buffer_size      = buffer_size >= MAX_FIFO_SIZE ? MAX_FIFO_SIZE : buffer_size;
    fifo->elem_size = elem_size;
}

void fifo_push(fifo_t *fifo, void *data, bool forced)
{
    if (fifo == NULL || data == NULL) return;

    if (forced || !fifo_is_full(fifo)) {
        /* 等价于 buffer[write_index] */
        memcpy(fifo->buffer + fifo->write_index * fifo->elem_size,
               data, fifo->elem_size);
        fifo->write_index = (fifo->write_index + 1) % fifo->buffer_size;
    }
}

void fifo_pop(fifo_t *fifo, void *data)
{
    if (fifo == NULL || data == NULL) return;

    if (!fifo_is_empty(fifo)) {
        memcpy(data,
               /* 等价于 buffer[write_index] */
               fifo->buffer + fifo->read_index * fifo->elem_size,
               fifo->elem_size);
        fifo->read_index = (fifo->read_index + 1) % fifo->buffer_size;
    }
}

void fifo_clear(fifo_t *fifo)
{
    if (fifo == NULL) return;

    fifo->read_index = fifo->write_index = 0;
}

bool fifo_is_empty(fifo_t *fifo)
{
    if (fifo == NULL) return true;

    
    return fifo->read_index == fifo->write_index;
}

bool fifo_is_full(fifo_t *fifo)
{
    if (fifo == NULL) return true;

    return (fifo->write_index + 1) % fifo->buffer_size == fifo->read_index;
}
