#ifndef ASTROLUNE_BYTES_H
#define ASTROLUNE_BYTES_H

#include "base.h"
#include <cstddef>
#include <cstdint>
#include <vector>

struct al_bytes {
    const uint8_t* data;
    size_t size;
};

struct al_bytes_mut {
    uint8_t* data;
    size_t size;
};

inline al_bytes al_bytes_make(const uint8_t* data, size_t size) {
    return {data, size};
}

#endif
