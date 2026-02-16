#include "load_elf.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>

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

// From UG984: Relocations
//
// Relocation information is used by linkers in order to bind symbols and addresses that could not be determined when
// the initial object was generated. Relocation entries describe how to alter the instruction and data relocation fields
// Relocations applied to executable or shared object files are similar and accomplish the same result. All relocations
// are listed and described in the following table, including the operation performed to compute the value of the
// relocation.
//
// Relocation Entries
// Code Name                        64?    Description                                         Operation
//  0   R_MICROBLAZE_NONE                  This relocation does nothing.                       none
//  1   R_MICROBLAZE_32                    A standard 32 bit relocation.                       S+A
//  2   R_MICROBLAZE_32_PCREL              A standard PCREL 32 bit relocation.                 S+A-P
//  3   R_MICROBLAZE_64_PCREL       yes    A 64 bit PCREL relocation.                          (S+A-P)&0xFFFF (#imm)
//  4   R_MICROBLAZE_32_PCREL_LO           The low half of a PCREL 32 bit relocation.          (S+A-P)&0xFFFF
//  5   R_MICROBLAZE_64             yes    A 64 bit relocation.                                (S+A)&0xFFFF (#imm)
//  6   R_MICROBLAZE_32_LO                 The low half of a 32 bit relocation.                (S+A)&0xFFFF
//  7   R_MICROBLAZE_SRO32                 Read-only small data section relocation.            (S+A - _SDA_BASE_)
//  8   R_MICROBLAZE_SRW32                 Read-write small data area relocation.              (S+A - _SDA_BASE_)
//  9   R_MICROBLAZE_64_NONE               This relocation does nothing. Used for relaxation.  none
// 10   R_MICROBLAZE_32_SYM_OP_SYM         Symbol Op Symbol relocation.                        none
// 11   R_MICROBLAZE_GNU_VTINHERIT         GNU extension to record C++ vtable hierarchy.
// 12   R_MICROBLAZE_GNU_VTENTRY           GNU extension to record C++ vtable member usage.
// 13   R_MICROBLAZE_GOTPC_64       yes    A 64 bit GOTPC relocation.                          G+A–P (#imm)
// 14   R_MICROBLAZE_GOT_64                A 64 bit GOT relocation.                            G+A (#imm)
// 15   R_MICROBLAZE_PLT_64                A 64 bit PLT relocation.                            L+A (#imm)
// 16   R_MICROBLAZE_REL                   Table-entry not used.                               ((B + A)>>16) & 0xFFFF
// 17   R_MICROBLAZE_JUMP_SLOT             Table-entry not used.                               (S >> 16) & 0xFFFF
// 18   R_MICROBLAZE_GLOB_DAT              Table-entry not used.                               (S >> 16) & 0xFFFF
// 19   R_MICROBLAZE_GOTOFF_64             A 64 bit GOT relative relocation.                   (S+A-GOT)&0xFFFF
// 20   R_MICROBLAZE_GOTOFF_32             A 32 bit GOT relative relocation.                   (S+A-GOT)&0xFFFF
// 21   R_MICROBLAZE_COPY                  COPY relocation.                                    none
// 22   R_MICROBLAZE_TLS                   TLS relocations for TLS.                            none
// 23   R_MICROBLAZE_TLSGD                 TLSGD relocations for TLS.                          @got@tlsgd
// 24   R_MICROBLAZE_TLSLD                 TLSLD relocations for TLS.                          @got@tlsld
// 25   R_MICROBLAZE_TLSDTPMOD32           Computes the load module.                           @got@dtpmod
// 26   R_MICROBLAZE_TLSDTPREL32           Computes a dtv-relative displacement.               @got@dtprel
// 27   R_MICROBLAZE_TLSDTPREL64           Computes a dtv-relative displacement.               @got@dtprel
// 28   R_MICROBLAZE_TLSGOTTPREL32         Computes a tp-relative displacement.                @got@prel
// 29   R_MICROBLAZE_TLSTPREL32            Computes a tp-relative displacement.                @got@prel
// 33   R_MICROBLAZE_32_NONE               Standard 32-bit relocation.                         none

//  A   The addend used to compute the value of the relocatable field.
//  B   The base address at which a shared object is loaded into memory during execution.
//      Generally, a shared object file is built with a 0 base virtual address,
//      but the execution address is different. See "Program Header".
//  G   The offset into the global offset table at which the address of the relocation entry's
//      symbol resides during execution. See "Global Offset Table (Processor-Specific)".
// GOT  The address of the global offset table. See "Global Offset Table (Processor-Specific)".
//  L   The section offset or address of the procedure linkage table entry for a symbol.
//      See "Procedure Linkage Table (Processor-Specific)".
//  P   The section offset or address of the storage unit being relocated, computed using r_offset.
//  S   The value of the symbol whose index resides in the relocation entry.

bool do_rel_mb(struct relocation_context* context, struct memory_map_entry* mme_offset, Elf32_Rela* rela)
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

    if (((offset & 0x3) != 0)) {
        fprintf(
            stderr,
            "%s: Relocation of unaligned data requested for: %s @ %04x",
            g_elf_ignore_unaligned ? "Critical" : "Error",
            mme_offset->name,
            offset);

        if (!g_elf_ignore_unaligned)
            return false;
    }

    value += rela->r_addend;
    uint32_t old_value;
    uint32_t new_value;

    switch (ELF32_R_TYPE(rela->r_info)) {
        case R_MICROBLAZE_32:
            // A standard 32 bit relocation.    (S + A)
            old_value = ioread32(mme_offset->cpu_virtual, offset);
            iowrite32(mme_offset->cpu_virtual, offset, value);
            local_debug(2, "%s @ %04x:", mme_offset->name, offset);
            local_debug(2, "R_MICROBLAZE_32: %08x -> %08x\n", old_value, value);
            return true;

        case R_MICROBLAZE_64_PCREL:
            // Really needed if PC relative?
            // A 64 bit PCREL relocation.       (S+A-P)&0xFFFF (#imm)
            value -= offset + 4;
            value -= mme_offset->apu_loaded;

            old_value = ioread32(mme_offset->cpu_virtual, offset + 0);
            new_value = (old_value & 0xffff0000ULL) | ((value >> 16) & 0xffff);
            iowrite32(mme_offset->cpu_virtual, offset + 0, new_value);
            local_debug(2, "%s @ %04x:", mme_offset->name, offset);
            local_debug(2, "R_MICROBLAZE_64_PCREL (+0): %08x -> %08x, ", old_value, new_value);

            old_value = ioread32(mme_offset->cpu_virtual, offset + 4);
            new_value = (old_value & 0xffff0000ULL) | (value & 0xffff);
            iowrite32(mme_offset->cpu_virtual, offset + 4, new_value);
            local_debug(2, "(+4): %08x -> %08x\n", old_value, new_value);
            return true;

        case R_MICROBLAZE_64:
            // A 64 bit relocation.             (S+A)&0xFFFF (#imm)
            old_value = ioread32(mme_offset->cpu_virtual, offset + 0);
            new_value = (old_value & 0xffff0000ULL) | ((value >> 16) & 0xffff);
            iowrite32(mme_offset->cpu_virtual, offset + 0, new_value);
            local_debug(2, "%s @ %04x:", mme_offset->name, offset);
            local_debug(2, "R_MICROBLAZE_64 (+0): %08x -> %08x, ", old_value, new_value);

            old_value = ioread32(mme_offset->cpu_virtual, offset + 4);
            new_value = (old_value & 0xffff0000ULL) | (value & 0xffff);
            iowrite32(mme_offset->cpu_virtual, offset + 4, new_value);
            local_debug(2, "(+4): %08x -> %08x\n", old_value, new_value);
            return true;

        case R_MICROBLAZE_GOTPC_64:
            // A 64 bit GOTPC relocation.       G+A–P (#imm)
            // Added hint to rebuild ELF without this
            local_debug(2, "%s @ %04x:", mme_offset->name, offset);
            old_value = ioread32(mme_offset->cpu_virtual, offset + 0);
            local_debug(2, "R_MICROBLAZE_GOTPC_64 (+0): %08x, ", old_value);
            old_value = ioread32(mme_offset->cpu_virtual, offset + 4);
            local_debug(2, "(+4): %08x\n", old_value);
            return false;

        case R_MICROBLAZE_GOT_64:
            // A 64 bit GOT relocation.         G+A (#imm)
            // Added hint to rebuild ELF without this
            local_debug(2, "%s @ %04x:", mme_offset->name, offset);
            old_value = ioread32(mme_offset->cpu_virtual, offset + 0);
            local_debug(2, "R_MICROBLAZE_GOT_64 (+0): %08x, ", old_value);
            old_value = ioread32(mme_offset->cpu_virtual, offset + 4);
            local_debug(2, "(+4): %08x\n", old_value);
            return false;

        case R_MICROBLAZE_PLT_64:
            // A 64 bit PLT relocation.         L+A (#imm)
            // Added hint to rebuild ELF without this
            local_debug(2, "%s @ %04x:", mme_offset->name, offset);
            old_value = ioread32(mme_offset->cpu_virtual, offset + 0);
            local_debug(2, "R_MICROBLAZE_PLT_64 (+0): %08x, ", old_value);
            old_value = ioread32(mme_offset->cpu_virtual, offset + 4);
            local_debug(2, "(+4): %08x\n", old_value);
            return false;

        case R_MICROBLAZE_32_PCREL_LO:
            // The low half of a PCREL 32 bit relocation. (S+A-P)&0xFFFF
            value -= offset;
            value -= mme_offset->apu_loaded;

            old_value = ioread32(mme_offset->cpu_virtual, offset + 0);
            new_value = (old_value & 0xffff0000ULL) | (value & 0xffff);
            iowrite32(mme_offset->cpu_virtual, offset + 0, new_value);
            local_debug(2, "%s @ %04x:", mme_offset->name, offset);
            local_debug(2, "R_MICROBLAZE_32_PCREL_LO: %08x -> %08x\n", old_value, new_value);

            return true;

        case R_MICROBLAZE_NONE:
            // This relocation does nothing.    none
            local_debug(2, "%s @ %04x:", mme_offset->name, offset);
            local_debug(2, "R_MICROBLAZE_NONE\n");
            return true;

        case R_MICROBLAZE_64_NONE:
            // This relocation does nothing.    none
            local_debug(2, "%s @ %04x:", mme_offset->name, offset);
            local_debug(2, "R_MICROBLAZE_64_NONE\n");
            return true;

        case 33: /* R_MICROBLAZE_32_NONE.  */
            // This relocation does nothing.    none
            local_debug(2, "%s @ %04x:", mme_offset->name, offset);
            local_debug(2, "R_MICROBLAZE_32_NONE\n");
            return true;

        default:
            fprintf(stderr, "Unknown relocation type %d\n", ELF32_R_TYPE(rela->r_info));
            break;
    }

    return false;
}
