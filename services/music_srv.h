#pragma once

#include "bsp_i2s.h"
#include "mp3dec.h"

typedef struct {
    i2s_t *dev;
    HMP3Decoder mp3dec;
} music_t;
