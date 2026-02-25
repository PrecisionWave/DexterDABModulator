
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

struct relocation_context {
    struct memory_map* mm;
    Elf32_Sym* symtab;
    char* strtab;
    void* private;
};

typedef bool (*do_rel_t)(struct relocation_context* context, struct memory_map_entry* mme_offset, Elf32_Rela* rela);
bool do_rel_mb(struct relocation_context* context, struct memory_map_entry* mme_offset, Elf32_Rela* rela);
bool do_rel_rv(struct relocation_context* context, struct memory_map_entry* mme_offset, Elf32_Rela* rela);
bool do_rel_r5(struct relocation_context* context, struct memory_map_entry* mme_offset, Elf32_Rela* rela);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // LOAD_ELF_H__INCLUDED
