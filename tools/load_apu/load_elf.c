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
extern char* g_dump_file;

#define local_debug(level, fmt, ...)      \
    {                                     \
        if (g_elf_debug_level >= (level)) \
            printf((fmt), ##__VA_ARGS__); \
    }

static bool
load_mem(struct memory_map_entry* mme, const off_t offset, const void* data, size_t load_len, size_t mem_len)
{
    if (load_len > mme->length - offset) {
        fprintf(stderr, "Error: Binary file greater than %s\n", mme->name);
        fprintf(stderr, "       Mem  Size: %jd bytes\n", (intmax_t)mme->length - offset);
        fprintf(stderr, "       File Size: %jd bytes\n", (intmax_t)load_len);
        return false;
    }

    if (load_len > 0) {
        printf(
            "Downloading %jd bytes to %s @ 0x%08x\n",
            (intmax_t)load_len,
            mme->name,
            (uint32_t)(mme->apu_loaded + offset));
        copytoio(mme->cpu_virtual, offset, data, load_len);

        if (g_dump_file) {
            char dump_file_name[1024];
            memset(dump_file_name, 0, sizeof(dump_file_name));
            snprintf(dump_file_name, sizeof(dump_file_name) - 1, "%s.ELF.%s", g_dump_file, mme->name);
            printf("Dumping ELF %s to %s...\n", mme->name, dump_file_name);
            int dfd = open(dump_file_name, O_CREAT | O_RDWR | O_TRUNC, 0666);
            if (load_len != write(dfd, (const uint8_t*)(data) + offset, load_len)) {
                int tmp_errno = errno;
                fprintf(stderr, "Error: Failed to write %d bytes to file. errno was %d\n", load_len, tmp_errno);
                return 10;
            }
            printf("Wrote %d bytes\n", load_len);
            close(dfd);
        }
    }

    if (mem_len > load_len) {
        printf(
            "Filling remaining %jd bytes with 0 (%s @ 0x%08x)\n",
            (intmax_t)mem_len - load_len,
            mme->name,
            (uint32_t)(mme->apu_loaded + offset + load_len));
        iomemset(mme->cpu_virtual, offset + load_len, 0, mem_len - load_len);
    }

    return true;
}

bool load_bin(const void* data, const size_t data_len, struct memory_map* mm, const uint32_t address)
{
    struct memory_map_entry* mme = mm_lookup(mm, address);
    if (!mme) {
        fprintf(stderr, "Error: Address 0x%08x not found in memory map\n", address);
        return false;
    }

    return load_mem(mme, 0, data, data_len, mme->length);
}

static const char* lookup_name(const char* names, int index)
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

    uint32_t offset = r_offset - mme_offset->apu_linked;

    struct memory_map_entry* mme_value = mm_lookup(mm, symbol->st_value);
    uint32_t value = symbol->st_value;
    if (mme_value) {
        value -= mme_value->apu_linked;
        value += mme_value->apu_loaded;
    }

    local_debug(
        2,
        "%s symbol: bind %d, type %d, other:%d, value: %08x, r_offset: %08x, r_addend: %08x\n",
        mme_offset != mme_value ? "Foreign" : "Local",
        ELF32_ST_BIND(symbol->st_info),
        ELF32_ST_TYPE(symbol->st_info),
        symbol->st_other,
        symbol->st_value,
        r_offset,
        r_addend);

    value += r_addend;
    uint32_t old_value;
    uint32_t new_value;

    switch (type) {
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
            fprintf(stderr, "Unknown relocation type %d\n", type);
            break;
    }

    return false;
}


bool load_elf(const char* file, size_t file_len, struct memory_map* mm)
{
    Elf32_Ehdr* ehdr = (Elf32_Ehdr*)file;

    // Check all assumptions
    assert(ehdr->e_ident[EI_MAG0] == ELFMAG0);
    assert(ehdr->e_ident[EI_MAG1] == ELFMAG1);
    assert(ehdr->e_ident[EI_MAG2] == ELFMAG2);
    assert(ehdr->e_ident[EI_MAG3] == ELFMAG3);

    assert(ehdr->e_ident[EI_CLASS] == ELFCLASS32);
    assert(ehdr->e_ident[EI_DATA] == ELFDATA2LSB);

    assert(ehdr->e_machine == EM_MICROBLAZE);

    assert(ehdr->e_type == ET_EXEC);

    printf("Zeroing memory...\n");
    for (size_t i = 0; i < mm->count; i++) {
        struct memory_map_entry* e = &mm->entries[i];
        printf("  %-8s 0x%08x - %08x\n", e->name, e->apu_loaded, e->apu_loaded + e->length - 1);
        iomemset(e->cpu_virtual, 0, 0, e->length);
    }

    /* Load program */
    printf("Loading program...\n");
    bool need_relocation = false;
    for (size_t load = 0; load < ehdr->e_phnum; load++) {
        Elf32_Phdr* phdr = (Elf32_Phdr*)(file + ehdr->e_phoff + load * ehdr->e_phentsize);

        local_debug(
            1,
            "  LOAD off    %08X vaddr %08x paddr %08x, align 2**%d\n",
            phdr->p_offset,
            phdr->p_vaddr,
            phdr->p_paddr,
            phdr->p_align);
        local_debug(1, "       filesz 0x%08X memsz 0x%08x flags 0x%x\n", phdr->p_filesz, phdr->p_memsz, phdr->p_flags);

        bool success = false;
        // Try virtual address first
        struct memory_map_entry* mme = mm_lookup(mm, phdr->p_paddr);
        if (mme) {
            success =
                load_mem(mme, phdr->p_paddr - mme->apu_linked, file + phdr->p_offset, phdr->p_filesz, phdr->p_memsz);
            need_relocation |= mme->apu_linked != mme->apu_loaded;
        }
        // Try physical address (might be linked to correct address)
        if (!success) {
            mme = mm_lookup_loaded(mm, phdr->p_paddr);
            if (mme) {
                success = load_mem(
                    mme, phdr->p_paddr - mme->apu_loaded, file + phdr->p_offset, phdr->p_filesz, phdr->p_memsz);
                need_relocation |= mme->apu_linked != mme->apu_loaded;
            }
        }
        if (!success) {
            fprintf(stderr, "Load failed!\n");
            return false;
        }
    }

    if (!need_relocation && !g_elf_force_reloc) {
        return true;
    }

    /* section name strings */
    const char* names = NULL;
    if (ehdr->e_shstrndx != SHN_UNDEF) {
        Elf32_Shdr* snames = (Elf32_Shdr*)(file + ehdr->e_shoff + ehdr->e_shstrndx * ehdr->e_shentsize);
        local_debug(1, "Section names table present\n");
        names = file + snames->sh_offset;
    }

    /* relocation */
    local_debug(1, "Sections:\n");
    local_debug(1, "  Idx Type Name                        Size      ADDR      File off  Flags Align\n");
    Elf32_Sym* sym = NULL;
    for (size_t section = 0; section < ehdr->e_shnum; section++) {
        Elf32_Shdr* shdr = (Elf32_Shdr*)(file + ehdr->e_shoff + section * ehdr->e_shentsize);
        const char* name = lookup_name(names, shdr->sh_name);
        local_debug(
            1,
            "  %3d %4d %-27s %08x  %08x  %08x  %04x  2**%u\n",
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
    }

    printf("Relocating...\n");
    size_t relocation_total_count = 0;
    size_t relocation_error_count = 0;
    for (size_t section = 0; section < ehdr->e_shnum; section++) {
        Elf32_Shdr* shdr = (Elf32_Shdr*)(file + ehdr->e_shoff + section * ehdr->e_shentsize);

        if (sym && shdr->sh_type == SHT_REL) {
            local_debug(
                1, "  REL in section %2d: flags %04x %s\n", section, shdr->sh_flags, lookup_name(names, shdr->sh_name));

            for (size_t entry = 0; entry < shdr->sh_size / shdr->sh_entsize; entry++) {
                Elf32_Rel* rel = (Elf32_Rel*)(file + shdr->sh_offset + entry * shdr->sh_entsize);

                struct memory_map_entry* mme = mm_lookup(mm, rel->r_offset);
                if (mme) {
                    Elf32_Sym* symbol = &sym[ELF32_R_SYM(rel->r_info)];
                    if (!do_rel(mm, symbol, rel->r_offset, ELF32_R_TYPE(rel->r_info), 0)) {
                        relocation_error_count++;
                        fprintf(
                            stderr,
                            "Relocation failed for symbol %d in section %d (type %d)\n",
                            entry,
                            section,
                            ELF32_R_TYPE(rel->r_info));
                    }
                    relocation_total_count++;
                }
            }
            if ((relocation_error_count > 0) || (g_elf_debug_level > 1))
                printf("  Relocation stats: Total %zu, Errors: %zu\n", relocation_total_count, relocation_error_count);
        }

        if (sym && shdr->sh_type == SHT_RELA) {
            local_debug(
                1,
                "  RELA in section %2d: flags %04x %s\n",
                section,
                shdr->sh_flags,
                lookup_name(names, shdr->sh_name));
            for (size_t entry = 0; entry < shdr->sh_size / shdr->sh_entsize; entry++) {
                Elf32_Rela* rela = (Elf32_Rela*)(file + shdr->sh_offset + entry * shdr->sh_entsize);
                struct memory_map_entry* mme = mm_lookup(mm, rela->r_offset);
                if (mme) {
                    Elf32_Sym* symbol = &sym[ELF32_R_SYM(rela->r_info)];
                    if (!do_rel(mm, symbol, rela->r_offset, ELF32_R_TYPE(rela->r_info), rela->r_addend)) {
                        relocation_error_count++;
                        fprintf(
                            stderr,
                            "Relocation failed for symbol %d in section %d (type %d)\n",
                            entry,
                            section,
                            ELF32_R_TYPE(rela->r_info));
                    }
                    relocation_total_count++;
                }
            }
            if ((relocation_error_count > 0) || (g_elf_debug_level > 1))
                printf("  Relocation stats: Total %zu, Errors: %zu\n", relocation_total_count, relocation_error_count);
        }
    }

    printf("Successfully applied %zu relocations\n", relocation_total_count - relocation_error_count);

    if (relocation_total_count == 0) {
        fprintf(stderr, "Error: This ELF file needs relocation OR MMU set for address translation\n");
        fprintf(stderr, "Hint:  Relink ELF with \"-Wl,--emit-reloc\" added\n");
        fprintf(stderr, "       and -fpie \"-Wl,--gc-sections\" and \"-Wl,--gc-keep-exported\" removed\n");
        return false;
    }

    if (relocation_error_count > 0) {
        fprintf(stderr, "Error: Relocation failed for %zu relocations.\n", relocation_error_count);
        fprintf(stderr, "Hint:  Relink ELF with -fpie \"-Wl,--gc-sections\" and \"-Wl,--gc-keep-exported\" removed\n");
        return false;
    }

    return true;
}
