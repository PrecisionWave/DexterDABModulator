
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
    int index;
    void* mmio;
    uint32_t length;
    uint32_t linked;
    uint32_t allocated;
};


bool load_elf(const char* file, const size_t file_len, struct memory_map_entry* mm, const size_t mm_len);
bool load_bin(const void* data, const size_t data_len, const uint32_t address, struct memory_map_entry* mm, const size_t mm_len);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // LOAD_ELF_H__INCLUDED
