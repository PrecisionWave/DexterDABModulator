
#ifndef MMAP_H__INCLUDED
#define MMAP_H__INCLUDED

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdio.h>
#include <stdbool.h>

void* mmap_dev(const char* dev, size_t offset, size_t length);
void* mmap_file(const char* file, size_t* out_length, bool rw);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // MMAP_H__INCLUDED
