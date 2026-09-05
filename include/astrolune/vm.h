#ifndef ASTROLUNE_VM_H
#define ASTROLUNE_VM_H

#include "arena.h"
#include "base.h"
#include "bytes.h"

#define AL_VM_MAX_CODE_SIZE (1024u * 1024u)
#define AL_VM_MAX_FUNCTIONS 1024u

struct al_vm_function {
    uint32_t offset;
    uint16_t parameter_count;
    uint16_t result_count;
    uint16_t max_stack;
};

inline al_status al_vm_container_encode(
    const al_vm_function* /*funcs*/, size_t /*count*/,
    al_bytes /*code*/, al_bytes_mut /*out*/, al_size* /*written*/) {
    return -1;  // stub
}

inline al_status al_vm_validate(al_bytes /*container*/,
                                 void* /*host_table*/, al_arena* /*arena*/) {
    return AL_OK;  // stub - always pass for toolchain
}

#endif
