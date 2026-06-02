#include "storage_srv.h"
#include "bsp_config.h"
#include "bsp_sdio.h"
#include "ff.h"

void storage_erase()
{
    sdio_erase_blocks(&storage, 0, storage.block_count);
}

void storage_write_blocks(const uint8_t *data, uint32_t block_addr, uint32_t block_count)
{
    sdio_write_blocks(&storage, data, block_addr, block_count);
}

void storage_read_blocks(uint8_t *data, uint32_t block_addr, uint32_t block_count)
{
    sdio_read_blocks(&storage, data, block_addr, block_count);
}
