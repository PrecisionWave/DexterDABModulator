
#ifndef MMAP_H__INCLUDED
#define MMAP_H__INCLUDED

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdio.h>
#include <stdbool.h>

enum device_index {
    APU_DEVICE_SRAM,
    APU_DEVICE_DDR,
};

void* mmap_apu(int fd, enum device_index index, size_t* out_length, size_t* out_physical);
void* mmap_dev(const char* dev, size_t offset, size_t length);
void* mmap_file(const char* file, size_t* out_length, bool rw);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // MMAP_H__INCLUDED
