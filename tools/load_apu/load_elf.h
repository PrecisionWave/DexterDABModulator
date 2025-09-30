
#ifndef LOAD_ELF_H__INCLUDED
#define LOAD_ELF_H__INCLUDED

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdio.h>
#include <stdbool.h>

bool load_elf(void* mmio_regs, void* ddr_ram, const char* file, size_t file_len);
bool load_sram(void* mmio_regs, const void* data, size_t data_len);
bool load_ddr(void* ddr_ram, const void* data, size_t data_len);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // LOAD_ELF_H__INCLUDED

