#ifndef ASTROLUNE_VM_H
#define ASTROLUNE_VM_H

#include "arena.h"
#include "base.h"
#include "bytes.h"

#define AL_VM_MAX_CODE_SIZE (1024u * 1024u)
#define AL_VM_MAX_FUNCTIONS 1024u
#define AL_VM_CONTAINER_VERSION 1u
#define AL_VM_ISA_VERSION 1u

struct al_vm_function {
    uint32_t offset;
    uint16_t parameter_count;
    uint16_t result_count;
    uint16_t max_stack;
    uint16_t reserved;
};

al_status al_vm_container_encode(const al_vm_function* functions, size_t count,
                                 al_bytes code, al_bytes_mut out,
                                 al_size* written);

al_status al_vm_validate(al_bytes container, void* host_table,
                         al_arena* arena);

#endif
