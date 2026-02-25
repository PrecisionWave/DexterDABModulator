
#ifndef MMAP_H__INCLUDED
#define MMAP_H__INCLUDED

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdio.h>
#include <stdbool.h>

#include "memorymap.h"

enum device_index {
    APU_DEVICE_SRAM,
    APU_DEVICE_DDR,
    APU_DEVICE_ATCM,
};

enum register_index {
    APU_REGISTERS,
    APU_REGISTERS2,
};

bool mmap_apu(int fd, enum device_index index, struct memory_map_entry* mme);
bool mmap_apu_sim(enum device_index index, struct memory_map_entry* mme);
bool mmap_reg(int fd, enum register_index index, struct memory_map_entry* mme);
void* mmap_dev(const char* dev, size_t offset, size_t length);
void* mmap_file(const char* file, size_t* out_length, bool rw);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // MMAP_H__INCLUDED
