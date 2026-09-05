#ifndef ASTROLUNE_HASH_H
#define ASTROLUNE_HASH_H

#include "base.h"
#include <cstdio>
#include <string>

#define AL_TAG_CONTRACT_DATA 1u
#define AL_TAG_EVENT 2u

inline al_hash256 al_sha256(const void*, size_t) { return {}; }
inline std::string al_hash256_hex(const al_hash256&) { return ""; }

inline void al_hash_tagged(al_u32 /*tag*/, const void* data, size_t len, al_hash256* out) {
    // stub: just zero
    *out = {};
}

#endif
