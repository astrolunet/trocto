#include "astrolune/vm.h"

#include <cstdint>
#include <cstring>
#include <vector>

namespace {

constexpr size_t kHeaderSize = 12;
constexpr size_t kFunctionSize = 12;
constexpr size_t kInstructionSize = 1;
constexpr size_t kPush64Size = 9;
constexpr size_t kJumpSize = 5;
constexpr size_t kIndexSize = 3;
constexpr uint8_t kMagic[4] = {'A', 'L', 'V', 'M'};

void put_u16(std::vector<uint8_t>& out, uint16_t value) {
    out.push_back(static_cast<uint8_t>(value));
    out.push_back(static_cast<uint8_t>(value >> 8));
}

void put_u32(std::vector<uint8_t>& out, uint32_t value) {
    for (unsigned shift = 0; shift < 32; shift += 8) {
        out.push_back(static_cast<uint8_t>(value >> shift));
    }
}

void put_varint(std::vector<uint8_t>& out, uint64_t value) {
    do {
        uint8_t byte = static_cast<uint8_t>(value & 0x7f);
        value >>= 7;
        if (value != 0) byte |= 0x80;
        out.push_back(byte);
    } while (value != 0);
}

struct Reader {
    const uint8_t* data;
    size_t size;
    size_t offset;

    bool take(size_t length, const uint8_t** bytes) {
        if (size - offset < length) return false;
        *bytes = data + offset;
        offset += length;
        return true;
    }

    bool u8(uint8_t& value) {
        const uint8_t* bytes = nullptr;
        if (!take(1, &bytes)) return false;
        value = bytes[0];
        return true;
    }

    bool u16(uint16_t& value) {
        const uint8_t* bytes = nullptr;
        if (!take(2, &bytes)) return false;
        value = static_cast<uint16_t>(bytes[0]) |
                static_cast<uint16_t>(bytes[1]) << 8;
        return true;
    }

    bool u32(uint32_t& value) {
        const uint8_t* bytes = nullptr;
        if (!take(4, &bytes)) return false;
        value = 0;
        for (unsigned i = 0; i < 4; ++i) {
            value |= static_cast<uint32_t>(bytes[i]) << (i * 8);
        }
        return true;
    }

    bool varint(uint64_t& value) {
        value = 0;
        unsigned shift = 0;
        for (;;) {
            uint8_t byte = 0;
            if (!u8(byte)) return false;
            if (shift >= 64 || (shift == 63 && (byte & 0x7f) > 1)) return false;
            const uint64_t chunk = byte & 0x7f;
            value |= chunk << shift;
            if ((byte & 0x80) == 0) {
                return !(byte == 0 && shift != 0);
            }
            shift += 7;
        }
    }
};

size_t instruction_size(uint8_t opcode) {
    if (opcode == 0x01) return kPush64Size;
    if (opcode == 0x0a || opcode == 0x0b) return kJumpSize;
    if (opcode == 0x1f || opcode == 0x21) return kIndexSize;
    if (opcode <= 0x2e) return kInstructionSize;
    return 0;
}

bool terminal(uint8_t opcode) {
    return opcode == 0x00 || opcode == 0x0e || opcode == 0x0f ||
           opcode == 0x20;
}

uint32_t immediate_u32(const uint8_t* instruction) {
    uint32_t value = 0;
    for (unsigned i = 0; i < 4; ++i) {
        value |= static_cast<uint32_t>(instruction[i + 1]) << (i * 8);
    }
    return value;
}

}  // namespace

al_status al_vm_container_encode(const al_vm_function* functions, size_t count,
                                 al_bytes code, al_bytes_mut out,
                                 al_size* written) {
    if (written == nullptr || functions == nullptr || count == 0 ||
        count > AL_VM_MAX_FUNCTIONS || code.data == nullptr || code.size == 0 ||
        code.size > AL_VM_MAX_CODE_SIZE || out.data == nullptr) {
        return 1;
    }

    std::vector<uint8_t> encoded;
    encoded.reserve(kHeaderSize + count * kFunctionSize + code.size + 16);
    encoded.insert(encoded.end(), kMagic, kMagic + sizeof(kMagic));
    put_u16(encoded, AL_VM_CONTAINER_VERSION);
    put_u16(encoded, AL_VM_ISA_VERSION);
    put_u32(encoded, 0);
    put_varint(encoded, count);
    for (size_t i = 0; i < count; ++i) {
        put_u32(encoded, functions[i].offset);
        put_u16(encoded, functions[i].parameter_count);
        put_u16(encoded, functions[i].result_count);
        put_u16(encoded, functions[i].max_stack);
        put_u16(encoded, functions[i].reserved);
    }
    put_varint(encoded, code.size);
    encoded.insert(encoded.end(), code.data, code.data + code.size);
    if (encoded.size() > out.size) return 3;

    std::memcpy(out.data, encoded.data(), encoded.size());
    *written = encoded.size();
    return 0;
}

al_status al_vm_validate(al_bytes container, void*, al_arena*) {
    Reader reader{container.data, container.size, 0};
    const uint8_t* magic = nullptr;
    if (!reader.take(sizeof(kMagic), &magic) ||
        std::memcmp(magic, kMagic, sizeof(kMagic)) != 0) {
        return 64;
    }

    uint16_t container_version = 0;
    uint16_t isa_version = 0;
    uint32_t flags = 0;
    if (!reader.u16(container_version) || !reader.u16(isa_version) ||
        !reader.u32(flags) || container_version != AL_VM_CONTAINER_VERSION ||
        isa_version != AL_VM_ISA_VERSION || flags != 0) {
        return 5;
    }

    uint64_t function_count = 0;
    if (!reader.varint(function_count) || function_count == 0 ||
        function_count > AL_VM_MAX_FUNCTIONS) {
        return 2;
    }

    std::vector<al_vm_function> functions(static_cast<size_t>(function_count));
    for (al_vm_function& function : functions) {
        if (!reader.u32(function.offset) ||
            !reader.u16(function.parameter_count) ||
            !reader.u16(function.result_count) ||
            !reader.u16(function.max_stack) || !reader.u16(function.reserved)) {
            return 64;
        }
    }

    uint64_t code_size = 0;
    if (!reader.varint(code_size) || code_size == 0 ||
        code_size > AL_VM_MAX_CODE_SIZE ||
        code_size > reader.size - reader.offset) {
        return 2;
    }
    const uint8_t* code = container.data + reader.offset;
    reader.offset += static_cast<size_t>(code_size);
    if (reader.offset != reader.size) return 64;

    std::vector<bool> boundaries(static_cast<size_t>(code_size) + 1, false);
    std::vector<bool> call_targets(static_cast<size_t>(function_count), false);
    for (size_t position = 0; position < code_size;) {
        const size_t size = instruction_size(code[position]);
        if (size == 0 || position + size > code_size) return 64;
        if (code[position] == 0x1f) {
            const uint16_t target = static_cast<uint16_t>(code[position + 1]) |
                                     static_cast<uint16_t>(code[position + 2]) << 8;
            if (target >= function_count) return 64;
            call_targets[target] = true;
        }
        boundaries[position] = true;
        position += size;
    }
    boundaries[code_size] = true;

    uint32_t previous_end = 0;
    for (size_t index = 0; index < functions.size(); ++index) {
        const al_vm_function& function = functions[index];
        const size_t end = index + 1 == functions.size()
                               ? static_cast<size_t>(code_size)
                               : functions[index + 1].offset;
        if (function.reserved != 0 || function.offset >= end ||
            function.offset >= code_size || !boundaries[function.offset] ||
            function.max_stack < function.parameter_count ||
            function.max_stack < function.result_count ||
            (index == 0 && function.parameter_count != 0) ||
            function.offset < previous_end) {
            return 64;
        }
        previous_end = function.offset;

        size_t last = function.offset;
        while (last + instruction_size(code[last]) < end) {
            last += instruction_size(code[last]);
        }
        if (!terminal(code[last])) return 64;
        if (index != 0 && call_targets[index] && code[last] != 0x20) return 64;

        for (size_t position = function.offset; position < end;) {
            const size_t size = instruction_size(code[position]);
            if (code[position] == 0x0a || code[position] == 0x0b) {
                const uint32_t target = immediate_u32(code + position);
                if (target < function.offset || target >= end ||
                    !boundaries[target]) {
                    return 64;
                }
            }
            position += size;
        }
    }
    return 0;
}
