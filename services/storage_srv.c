#include "storage_srv.h"
#include "bsp_config.h"
#include "bsp_sdio.h"
#include "ff.h"

void storage_erase()
{
    /* FIXME: 擦除不正常， HAL_GetTick 持续返回 5,不再自增 */
    sdio_erase_blocks(&storage, 0, storage.block_count * storage.block_size);
}

