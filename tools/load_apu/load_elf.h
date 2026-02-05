
#ifndef LOAD_ELF_H__INCLUDED
#define LOAD_ELF_H__INCLUDED

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include <stdint.h>
#include <stdbool.h>

#include <elf.h>

#include "memorymap.h"

bool load_elf(const char* file, const size_t file_len, struct memory_map* mm);
bool load_bin(const void* data, const size_t data_len, struct memory_map* mm, const uint32_t address);

// Relocation
typedef bool (*do_rel_t)(struct memory_map* mm, Elf32_Sym* symbol, Elf32_Addr r_offset, Elf32_Word type, Elf32_Sword r_addend);
bool do_rel_mb(struct memory_map* mm, Elf32_Sym* symbol, Elf32_Addr r_offset, Elf32_Word type, Elf32_Sword r_addend);
bool do_rel_rv(struct memory_map* mm, Elf32_Sym* symbol, Elf32_Addr r_offset, Elf32_Word type, Elf32_Sword r_addend);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // LOAD_ELF_H__INCLUDED
