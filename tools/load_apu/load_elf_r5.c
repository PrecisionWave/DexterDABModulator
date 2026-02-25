#include "load_elf.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

// open
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <elf.h>

#include "memorymap.h"
#include "mmio.h"
#include "apu.h"

extern int g_elf_debug_level;
extern bool g_elf_force_reloc;
extern bool g_elf_ignore_unaligned;
extern char* g_dump_file;

#define local_debug(level, fmt, ...)      \
    {                                     \
        if (g_elf_debug_level >= (level)) \
            printf((fmt), ##__VA_ARGS__); \
    }

#define ERROR "\033[31mError\033[0m: "
#define WARN "\033[33mWarning\033[0m: "
#define CYAN(x) "\033[36m" x "\033[0m"


bool do_rel_r5(struct relocation_context* context, struct memory_map_entry* mme_offset, Elf32_Rela* rela)
{
    Elf32_Sym* symbol = &context->symtab[ELF32_R_SYM(rela->r_info)];
    uint32_t offset = rela->r_offset - mme_offset->apu_linked;

    struct memory_map_entry* mme_value = mm_lookup(context->mm, symbol->st_value);
    uint32_t value = symbol->st_value;
    if (mme_value) {
        value -= mme_value->apu_linked;
        value += mme_value->apu_loaded;
    }

    local_debug(
        2,
        "%s symbol \"%s\": bind %d, type %d, other:%d, value: %08x, r_offset: %08x, r_addend: %08x\n",
        mme_offset != mme_value ? "Foreign" : "Local",
        &context->strtab[symbol->st_name],
        ELF32_ST_BIND(symbol->st_info),
        ELF32_ST_TYPE(symbol->st_info),
        symbol->st_other,
        symbol->st_value,
        rela->r_offset,
        rela->r_addend);

    value += rela->r_addend;

    uint32_t old_value;
    uint32_t new_value;

    switch (ELF32_R_TYPE(rela->r_info)) {
        case R_ARM_NONE:
            // Do nothing
            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_ARM_NONE: addend: %u\n", rela->r_addend);
            return true;

        case R_ARM_ABS32:
            // 32-bit absolute
            // word32      S + A - P
            old_value = ioread32(mme_offset->cpu_virtual, offset);
            new_value = old_value;
            if (mme_value) {
                new_value -= mme_value->apu_linked;
                new_value += mme_value->apu_loaded;
            }
            iowrite32(mme_offset->cpu_virtual, offset, new_value);

            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_ARM_ABS32: %08x -> %08x\n", old_value, new_value);
            return true;

        case R_ARM_PREL31:
            old_value = ioread32(mme_offset->cpu_virtual, offset);
            
            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_ARM_PREL31: %08x -> ??\n", old_value);
            return false;

        case R_ARM_CALL:
            old_value = ioread32(mme_offset->cpu_virtual, offset);

            local_debug(2, "%s @ %04x: ", mme_offset->name, offset);
            local_debug(2, "R_ARM_CALL: %08x -> ??\n", old_value);
            return false;


        default:
            fprintf(
                stderr,
                ERROR "Unknown relocation type %d at %s:%04x\n",
                ELF32_R_TYPE(rela->r_info),
                mme_offset->name,
                offset);
            break;
    }

    return false;
}
