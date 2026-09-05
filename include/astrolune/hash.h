#ifndef ASTROLUNE_HASH_H
#define ASTROLUNE_HASH_H

#include "base.h"
#include <cstdio>
#include <string>

inline al_hash256 al_sha256(const void*, size_t) { return {}; }
inline std::string al_hash256_hex(const al_hash256&) { return ""; }

#endif
