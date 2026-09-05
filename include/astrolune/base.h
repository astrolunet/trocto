#ifndef ASTROLUNE_BASE_H
#define ASTROLUNE_BASE_H

#include <cstdint>
#include <cstring>

using al_u8  = uint8_t;
using al_u16 = uint16_t;
using al_u32 = uint32_t;
using al_u64 = uint64_t;
using al_size = size_t;

using al_status = int;
#define AL_OK 0

struct al_hash256 { al_u8 data[32]{}; };
struct al_address { al_u8 data[32]{}; };

inline const char* al_status_str(al_status) { return "error"; }

#define AL_EXTERN_C_BEGIN
#define AL_EXTERN_C_END

#endif
