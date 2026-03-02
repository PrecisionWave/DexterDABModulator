#include "load_elf.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>

// open
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include "memorymap.h"
#include "mmio.h"
#include "apu.h"

#ifndef EM_RISCV
#define EM_RISCV 243
#endif

extern int g_elf_debug_level;
extern bool g_elf_force_reloc;
extern bool g_elf_ignore_unaligned;
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
                fprintf(stderr, "Error: Failed to write %zu bytes to file. errno was %d\n", load_len, tmp_errno);
                return 10;
            }
            printf("Wrote %zu bytes\n", load_len);
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

static bool do_rel_fail(struct relocation_context* context, struct memory_map_entry* mme_offset, Elf32_Rela* rela)
{
    return false;
}


bool load_elf(const char* file, size_t file_len, struct memory_map* mm)
{
    Elf32_Ehdr* ehdr = (Elf32_Ehdr*)file;
    do_rel_t do_rel = do_rel_fail;

    // Check all assumptions
    if (!((ehdr->e_ident[EI_MAG0] == ELFMAG0) && (ehdr->e_ident[EI_MAG1] == ELFMAG1) &&
          (ehdr->e_ident[EI_MAG2] == ELFMAG2) && (ehdr->e_ident[EI_MAG3] == ELFMAG3))) {
        fprintf(stderr, "Error: ELF magic check failed!\n");
        return false;
    }

    if (!(ehdr->e_type == ET_EXEC)) {
        fprintf(stderr, "Error: ELF type not ET_EXEC!\n");
        return false;
    }

    if (!(ehdr->e_ident[EI_DATA] == ELFDATA2LSB)) {
        fprintf(stderr, "Error: ELF type not LE!\n");
        return false;
    }

    // Microblaze
    if (ehdr->e_machine == EM_MICROBLAZE) {
        if (ehdr->e_ident[EI_CLASS] != ELFCLASS32) {
            fprintf(stderr, "Error: Not 32-Bit ELF!");
            return false;
        }

        do_rel = do_rel_mb;
    }

    // RISC-V
    if (ehdr->e_machine == EM_RISCV) {
        if (ehdr->e_ident[EI_CLASS] != ELFCLASS32) {
            fprintf(stderr, "Error: Not 32-Bit ELF!");
            return false;
        }

        do_rel = do_rel_rv;
    }

    // RPU ARM-R5
    if (ehdr->e_machine == EM_ARM) {
        if (ehdr->e_ident[EI_CLASS] != ELFCLASS32) {
            fprintf(stderr, "Error: Not 32-Bit ELF!");
            return false;
        }

        do_rel = do_rel_r5;
    }

    printf("Zeroing memory...\n");
    for (size_t i = 0; i < mm->count; i++) {
        struct memory_map_entry* e = &mm->entries[i];
        printf("  %-8s 0x%08x - %08x\n", e->name, e->apu_loaded, e->apu_loaded + e->length - 1);
        fflush(stdout);
        iomemset(e->cpu_virtual, 0, 0, e->length);
    }

    /* Load program */
    printf("Loading program...\n");
    bool need_relocation = false;
    for (size_t load = 0; load < ehdr->e_phnum; load++) {
        Elf32_Phdr* phdr = (Elf32_Phdr*)(file + ehdr->e_phoff + load * ehdr->e_phentsize);

        if (phdr->p_type != PT_LOAD)
            continue;

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
    local_debug(1, "  Idx Type Name                        Size      ADDR      File off  Flags Link Info Align\n");

    struct relocation_context context = {
        .mm = mm,
        .symtab = NULL,
        .strtab = NULL,
        .private = NULL,
    };

    for (size_t section = 0; section < ehdr->e_shnum; section++) {
        Elf32_Shdr* shdr = (Elf32_Shdr*)(file + ehdr->e_shoff + section * ehdr->e_shentsize);
        const char* name = lookup_name(names, shdr->sh_name);
        local_debug(
            1,
            "  %3zu %4u %-27s %08x  %08x  %08x  %04x  %04x %04x 2**%u\n",
            section,
            shdr->sh_type,
            name,
            shdr->sh_size,
            shdr->sh_addr,
            shdr->sh_offset,
            shdr->sh_flags,
            shdr->sh_link,
            shdr->sh_info,
            shdr->sh_addralign);
        if (shdr->sh_type == SHT_SYMTAB && strcmp(name, ".symtab") == 0) {
            context.symtab = (Elf32_Sym*)(file + shdr->sh_offset);
        }
        if (shdr->sh_type == SHT_STRTAB && strcmp(name, ".strtab") == 0) {
            context.strtab = (char*)(file + shdr->sh_offset);
        }
    }

    printf("Relocating...\n");
    size_t relocation_total_count = 0;
    size_t relocation_error_count = 0;

    for (size_t section = 0; section < ehdr->e_shnum; section++) {
        Elf32_Shdr* shdr = (Elf32_Shdr*)(file + ehdr->e_shoff + section * ehdr->e_shentsize);

        if (context.symtab && shdr->sh_type == SHT_REL) {
            Elf32_Shdr* shdrL = (Elf32_Shdr*)(file + ehdr->e_shoff + shdr->sh_info * ehdr->e_shentsize);
            if (shdrL->sh_flags & SHF_ALLOC) {
                local_debug(
                    1,
                    "  REL in section %2zu: flags %04x %s\n",
                    section,
                    shdr->sh_flags,
                    lookup_name(names, shdr->sh_name));

                for (size_t entry = 0; entry < shdr->sh_size / shdr->sh_entsize; entry++) {
                    Elf32_Rel* rel = (Elf32_Rel*)(file + shdr->sh_offset + entry * shdr->sh_entsize);
                    Elf32_Rela rela = {
                        .r_offset = rel->r_offset,
                        .r_info = rel->r_info,
                        .r_addend = 0,
                    };

                    struct memory_map_entry* mme = mm_lookup(mm, rel->r_offset);
                    if (mme) {
                        if (!do_rel(&context, mme, &rela)) {
                            relocation_error_count++;
                            fprintf(
                                stderr,
                                "Relocation failed for symbol %zu in section %zu (type %d)\n",
                                entry,
                                section,
                                ELF32_R_TYPE(rel->r_info));
                        }
                        relocation_total_count++;
                    }
                }
                if ((relocation_error_count > 0) || (g_elf_debug_level > 1))
                    printf(
                        "  Relocation stats: Total %zu, Errors: %zu\n", relocation_total_count, relocation_error_count);
            }
        }

        if (context.symtab && shdr->sh_type == SHT_RELA) {
            Elf32_Shdr* shdrL = (Elf32_Shdr*)(file + ehdr->e_shoff + shdr->sh_info * ehdr->e_shentsize);
            if (shdrL->sh_flags & SHF_ALLOC) {
                local_debug(
                    1,
                    "  RELA in section %2zu: flags %04x %s\n",
                    section,
                    shdr->sh_flags,
                    lookup_name(names, shdr->sh_name));
                for (size_t entry = 0; entry < shdr->sh_size / shdr->sh_entsize; entry++) {
                    Elf32_Rela* rela = (Elf32_Rela*)(file + shdr->sh_offset + entry * shdr->sh_entsize);
                    struct memory_map_entry* mme = mm_lookup(mm, rela->r_offset);
                    if (mme) {
                        if (!do_rel(&context, mme, rela)) {
                            relocation_error_count++;
                            fprintf(
                                stderr,
                                "Relocation failed for symbol %zu in section %zu (type %d)\n",
                                entry,
                                section,
                                ELF32_R_TYPE(rela->r_info));
                        }
                        relocation_total_count++;
                    }
                }
                if ((relocation_error_count > 0) || (g_elf_debug_level > 1))
                    printf(
                        "  Relocation stats: Total %zu, Errors: %zu\n", relocation_total_count, relocation_error_count);
            }
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
