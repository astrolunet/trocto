#ifndef ASTROLUNE_ARENA_H
#define ASTROLUNE_ARENA_H

#include "base.h"
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>

struct al_arena {
    uint8_t* base;
    size_t size;
    size_t offset;
};

inline al_status al_arena_init(al_arena* a, size_t size) {
    a->base = static_cast<uint8_t*>(std::malloc(size));
    if (!a->base) return -1;
    a->size = size;
    a->offset = 0;
    return AL_OK;
}

inline void al_arena_destroy(al_arena* a) {
    std::free(a->base);
    a->base = nullptr;
    a->size = 0;
    a->offset = 0;
}

#endif
