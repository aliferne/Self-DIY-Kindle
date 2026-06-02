#pragma once

#include "bsp_sdio.h"

void storage_erase();
void storage_write_blocks(const uint8_t *data, uint32_t block_addr, uint32_t block_count);
void storage_read_blocks(uint8_t *data, uint32_t block_addr, uint32_t block_count);
