#include "load_elf.h"

#include <stdio.h>
#include <assert.h>

#include <elf.h>

#include "memorymap.h"
#include "mmio.h"
#include "apu.h"

static bool load_mem(struct memory_map_entry* mme, const off_t offset, const void* data, size_t data_len)
{
    if (data_len > mme->length - offset) {
        fprintf(stderr, "Error: Binary file greater than %s\n", mme->name);
        fprintf(stderr, "       Mem  Size: %jd bytes\n", (intmax_t)mme->length - offset);
        fprintf(stderr, "       File Size: %jd bytes\n", (intmax_t)data_len);
        return false;
    }

    printf("Downloading %jd bytes to %s\n", (intmax_t)data_len, mme->name);

    copytoio(mme->mmio, offset, data, data_len);
    return true;
}

bool load_bin(const void* data, const size_t data_len, struct memory_map* mm, const uint32_t address)
{
    struct memory_map_entry* mme = mm_lookup(mm, address);
    if (!mme) {
        fprintf(stderr, "Error: Address not found in memory map\n");
        return false;
    }

    iomemset(mme->mmio, 0, 0, mme->length);
    return load_mem(mme, 0, data, data_len);
}

static const char* section_name(const char* names, int index)
{
    return names ? names + index : "?";
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

static bool do_rel(struct memory_map* mm, Elf32_Sym* symbol, Elf32_Addr r_offset, Elf32_Word type, Elf32_Sword r_addend)
{
    struct memory_map_entry* mme_offset = mm_lookup(mm, r_offset);

    // noting to do?
    if (!mme_offset)
        return true;

    uint32_t offset = r_offset - mme_offset->linked;

    struct memory_map_entry* mme_value = mm_lookup(mm, symbol->st_value);
    uint32_t value = symbol->st_value;
    if (mme_value) {
        value -= mme_value->linked;
        value += mme_value->allocated;
    }
    value += r_addend;
    uint32_t old_value;
    uint32_t new_value;

    switch (type) {
        case R_MICROBLAZE_32:
            // A standard 32 bit relocation.    (S + A)
            old_value = ioread32(mme_offset->mmio, offset);
            iowrite32(mme_offset->mmio, offset, value);
            printf("%s @ %04x:", mme_offset->name, offset);
            printf("R_MICROBLAZE_32: %08x -> %08x\n", old_value, value);
            return true;

        case R_MICROBLAZE_64_PCREL:
            // Really needed if PC relative?
            // A 64 bit PCREL relocation.       (S+A-P)&0xFFFF (#imm)
            value -= offset + 4;
            value -= mme_offset->allocated;

            old_value = ioread32(mme_offset->mmio, offset + 0);
            new_value = (old_value & 0xffff0000ULL) | ((value >> 16) & 0xffff);
            iowrite32(mme_offset->mmio, offset + 0, new_value);
            printf("%s @ %04x:", mme_offset->name, offset);
            printf("R_MICROBLAZE_64_PCREL (+0): %08x -> %08x, ", old_value, new_value);

            old_value = ioread32(mme_offset->mmio, offset + 4);
            new_value = (old_value & 0xffff0000ULL) | (value & 0xffff);
            iowrite32(mme_offset->mmio, offset + 4, new_value);
            printf("(+4): %08x -> %08x\n", old_value, new_value);
            return true;

        case R_MICROBLAZE_64:
            // A 64 bit relocation.             (S+A)&0xFFFF (#imm)
            old_value = ioread32(mme_offset->mmio, offset + 0);
            new_value = (old_value & 0xffff0000ULL) | ((value >> 16) & 0xffff);
            iowrite32(mme_offset->mmio, offset + 0, new_value);
            printf("%s @ %04x:", mme_offset->name, offset);
            printf("R_MICROBLAZE_64 (+0): %08x -> %08x, ", old_value, new_value);

            old_value = ioread32(mme_offset->mmio, offset + 4);
            new_value = (old_value & 0xffff0000ULL) | (value & 0xffff);
            iowrite32(mme_offset->mmio, offset + 4, new_value);
            printf("(+4): %08x -> %08x\n", old_value, new_value);
            return true;

#if 0
        case R_MICROBLAZE_GOTPC_64:
            // Really needed if PC relative?
            // A 64 bit GOTPC relocation.       G+A–P (#imm)
            value -= offset + 4;
            value -= mme_offset->allocated;

            old_value = ioread32(mme_offset->mmio, offset + 0);
            new_value = (old_value & 0xffff0000ULL) | ((value >> 16) & 0xffff);
            iowrite32(mme_offset->mmio, offset + 0, new_value);
            // printf("R_MICROBLAZE_GOTPC_64 (+0): %08x -> %08x, ", old_value, new_value);

            old_value = ioread32(mme_offset->mmio, offset + 4);
            new_value = (old_value & 0xffff0000ULL) | (value & 0xffff);
            iowrite32(mme_offset->mmio, offset + 4, new_value);
            // printf("(+4): %08x -> %08x\n", old_value, new_value);
            return true;

        case R_MICROBLAZE_GOT_64:
            // A 64 bit GOT relocation.         G+A (#imm)
            old_value = ioread32(mme_offset->mmio, offset + 0);
            new_value = old_value;
            // new_value = (old_value & 0xffff0000ULL) | ((value >> 16) & 0xffff);
            iowrite32(mme_offset->mmio, offset + 0, new_value);
            printf("%04x: R_MICROBLAZE_GOT_64 (+0): %08x -> %08x, ", offset, old_value, new_value);

            old_value = ioread32(mme_offset->mmio, offset + 4);
            new_value = (old_value & 0xffff0000ULL) | ((value >> 16) & 0xffff);
            iowrite32(mme_offset->mmio, offset + 4, new_value);
            printf("(+4): %08x -> %08x\n", old_value, new_value);
            return true;

        case R_MICROBLAZE_PLT_64:
            // A 64 bit PLT relocation.         L+A (#imm)
            value -= offset + 4;
            value -= mme_offset->allocated;

            old_value = ioread32(mme_offset->mmio, offset + 0);
            new_value = (old_value & 0xffff0000ULL) | ((value >> 16) & 0xffff);
            iowrite32(mme_offset->mmio, offset + 0, new_value);
            // printf("R_MICROBLAZE_PLT_64 (+0): %08x -> %08x, ", old_value, new_value);

            old_value = ioread32(mme_offset->mmio, offset + 4);
            new_value = (old_value & 0xffff0000ULL) | (value & 0xffff);
            iowrite32(mme_offset->mmio, offset + 4, new_value);
            // printf("(+4): %08x -> %08x\n", old_value, new_value);
            return true;
#endif

        case R_MICROBLAZE_32_PCREL_LO:
        case R_MICROBLAZE_NONE:
        case R_MICROBLAZE_64_NONE:
        case 33: /* R_MICROBLAZE_32_NONE.  */
            // This relocation does nothing.    none
            return true;

        default:
            printf("Unknown relocation type %d\n", type);
            break;
    }

    return false;
}


bool load_elf(const char* file, size_t file_len, struct memory_map* mm)
{
    Elf32_Ehdr* ehdr = (Elf32_Ehdr*)file;

    assert(ehdr->e_ident[EI_MAG0] == ELFMAG0);
    assert(ehdr->e_ident[EI_MAG1] == ELFMAG1);
    assert(ehdr->e_ident[EI_MAG2] == ELFMAG2);
    assert(ehdr->e_ident[EI_MAG3] == ELFMAG3);

    assert(ehdr->e_ident[EI_CLASS] == ELFCLASS32);
    assert(ehdr->e_ident[EI_DATA] == ELFDATA2LSB);

    assert(ehdr->e_machine == EM_MICROBLAZE);

    fprintf(stderr, "e_type: %d\n", ehdr->e_type);

    printf("Zeroing memory...\n");
    for (size_t i = 0; i < mm->count; i++) {
        struct memory_map_entry* e = &mm->entries[i];
        printf("  %-8s 0x%08x - %08x\n", e->name, e->allocated, e->allocated + e->length - 1);
        iomemset(e->mmio, 0, 0, e->length);
    }

    /* part 0: load program */
    printf("Program Header:\n");
    bool need_relocation = false;
    for (size_t load = 0; load < ehdr->e_phnum; load++) {
        Elf32_Phdr* phdr = (Elf32_Phdr*)(file + ehdr->e_phoff + load * ehdr->e_phentsize);

        printf(
            "  LOAD off    %08X vaddr %08x paddr %08x, align 2**%d\n",
            phdr->p_offset,
            phdr->p_vaddr,
            phdr->p_paddr,
            phdr->p_align);
        printf("       filesz 0x%08X memsz 0x%08x flags 0x%x\n", phdr->p_filesz, phdr->p_memsz, phdr->p_flags);

        bool success = false;
        struct memory_map_entry* mme = mm_lookup(mm, phdr->p_paddr);
        if (mme) {
            success = load_mem(mme, phdr->p_paddr - mme->linked, file + phdr->p_offset, phdr->p_filesz);
            need_relocation |= mme->linked != mme->allocated;
        }
        if (!success) {
            fprintf(stderr, "Load failed!\n");
            return false;
        }
    }

    /* section name strings */
    const char* names = NULL;
    if (ehdr->e_shstrndx != SHN_UNDEF) {
        Elf32_Shdr* snames = (Elf32_Shdr*)(file + ehdr->e_shoff + ehdr->e_shstrndx * ehdr->e_shentsize);
        fprintf(stderr, "Section names table present\n");
        names = file + snames->sh_offset;
    }

    printf("Sections:\n");
    printf("Idx Type Name                      Size      ADDR      File off  Flags  Align\n");
    Elf32_Sym* sym = NULL;
    const char* sym_names = NULL;
    for (size_t section = 0; section < ehdr->e_shnum; section++) {
        Elf32_Shdr* shdr = (Elf32_Shdr*)(file + ehdr->e_shoff + section * ehdr->e_shentsize);
        const char* name = section_name(names, shdr->sh_name);
        printf(
            "%3d %3d %-25s %08x  %08x  %08x  %04x  2**%u\n",
            section,
            shdr->sh_type,
            name,
            shdr->sh_size,
            shdr->sh_addr,
            shdr->sh_offset,
            shdr->sh_flags,
            shdr->sh_addralign);
        if (shdr->sh_type == SHT_SYMTAB && strcmp(name, ".symtab") == 0) {
            sym = (Elf32_Sym*)(file + shdr->sh_offset);
        }
        if (shdr->sh_type == SHT_STRTAB && strcmp(name, ".strtab") == 0) {
            sym_names = (const char*)(file + shdr->sh_offset);
        }
    }

    /* relocation */
    bool relocation_error = false;
    for (size_t section = 0; section < ehdr->e_shnum; section++) {
        Elf32_Shdr* shdr = (Elf32_Shdr*)(file + ehdr->e_shoff + section * ehdr->e_shentsize);

        if (sym && shdr->sh_type == SHT_REL) {
            printf("REL in section %d: %s (flags %08x)\n", section, section_name(names, shdr->sh_name), shdr->sh_flags);

            for (size_t entry = 0; entry < shdr->sh_size / shdr->sh_entsize; entry++) {
                Elf32_Rel* rel = (Elf32_Rel*)(file + shdr->sh_offset + entry * shdr->sh_entsize);

                struct memory_map_entry* mme = mm_lookup(mm, rel->r_offset);
                if (mme) {
                    Elf32_Sym* symbol = &sym[ELF32_R_SYM(rel->r_info)];
                    relocation_error |= !do_rel(mm, symbol, rel->r_offset, ELF32_R_TYPE(rel->r_info), 0);
                }
            }
        }

        if (sym && shdr->sh_type == SHT_RELA) {
            printf(
                "RELA in section %d: %s (flags %08x)\n", section, section_name(names, shdr->sh_name), shdr->sh_flags);
            for (size_t entry = 0; entry < shdr->sh_size / shdr->sh_entsize; entry++) {
                Elf32_Rela* rela = (Elf32_Rela*)(file + shdr->sh_offset + entry * shdr->sh_entsize);
                struct memory_map_entry* mme = mm_lookup(mm, rela->r_offset);
                if (mme) {
                    Elf32_Sym* symbol = &sym[ELF32_R_SYM(rela->r_info)];
                    relocation_error |= !do_rel(mm, symbol, rela->r_offset, ELF32_R_TYPE(rela->r_info), rela->r_addend);
                }
            }
        }
    }

    // Missing: MMU setup for virtual <> physical DDR address translation
    if (need_relocation && relocation_error) {
        fprintf(stderr, "Error: Relocation failed. See log for details\n");
        fprintf(stderr, "       This ELF file needs relocation OR MMU set for address translation\n");
        return false;
    }


    return true;
}
