
#ifndef LOAD_ELF_H__INCLUDED
#define LOAD_ELF_H__INCLUDED

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

struct memory_map_entry {
    const char* name;
    void* mmio;
    off_t mmio_offset;
    size_t length;
    uint32_t physical;
    uint32_t allocated;
};


bool load_elf(const char* file, size_t file_len, struct memory_map_entry* mm, size_t mm_len);
bool load_sram(void* mmio_regs, const void* data, size_t data_len);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // LOAD_ELF_H__INCLUDED
