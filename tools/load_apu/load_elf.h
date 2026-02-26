
#ifndef LOAD_ELF_H__INCLUDED
#define LOAD_ELF_H__INCLUDED

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

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

static inline uint32_t replace_imm(
    uint32_t* instr,
    const uint32_t instr_bit_hi,
    const uint32_t instr_bit_lo,
    const uint32_t imm,
    const uint32_t imm_bit_hi,
    const uint32_t imm_bit_lo)
{
    assert(imm_bit_hi < 32);
    assert(imm_bit_lo < 32);
    assert(imm_bit_lo <= imm_bit_hi);
    assert(instr_bit_hi < 32);
    assert(instr_bit_lo < 32);
    assert(instr_bit_lo <= instr_bit_hi);
    assert((imm_bit_hi - imm_bit_lo) == (instr_bit_hi - instr_bit_lo));

    uint32_t result = *instr;
    uint32_t bits = imm_bit_hi - imm_bit_lo;
    uint32_t mask_bits = (1 << bits) - 1;
    uint32_t imm_masked = (imm >> imm_bit_lo) & mask_bits;
    result &= ~(mask_bits << instr_bit_lo);
    result |= imm_masked << instr_bit_lo;
    *instr = result;
    return result;
}

static inline uint32_t extract_imm(
    const uint32_t instr,
    const uint32_t instr_bit_hi,
    const uint32_t instr_bit_lo,
    const uint32_t imm_bit_hi,
    const uint32_t imm_bit_lo)
{
    assert(imm_bit_hi < 32);
    assert(imm_bit_lo < 32);
    assert(imm_bit_lo <= imm_bit_hi);
    assert(instr_bit_hi < 32);
    assert(instr_bit_lo < 32);
    assert(instr_bit_lo <= instr_bit_hi);
    assert((imm_bit_hi - imm_bit_lo) == (instr_bit_hi - instr_bit_lo));

    uint32_t bits = imm_bit_hi - imm_bit_lo;
    uint32_t mask_bits = (1 << bits) - 1;
    return ((instr >> instr_bit_lo) & mask_bits) << imm_bit_lo;
}

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // LOAD_ELF_H__INCLUDED
