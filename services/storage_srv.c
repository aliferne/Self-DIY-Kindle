#include "storage_srv.h"
#include "bsp_config.h"
#include "bsp_sdio.h"
#include "ff.h"

void storage_erase()
{
    sdio_erase_blocks(&storage, 0, storage.block_count);
}
