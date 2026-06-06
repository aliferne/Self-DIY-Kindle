#pragma once

#include "bsp_sdio.h"

enum StorageState {
    Storage_Initialized = 0,
};

typedef struct {
    uint8_t *volume;
    uint8_t state;
} Storage_t;

void storage_srv_init(Storage_t *storage);
