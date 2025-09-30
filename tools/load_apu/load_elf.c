#include "load_elf.h"

#include <stdio.h>
#include <assert.h>

#include <elf.h>

#include "mmio.h"
#include "apu.h"

bool load_sram(void* mmio_regs, const void* data, size_t data_len)
{
    if (data_len > APU_CTRL_SRAM_LENGTH) {
        fprintf(stderr, "Error: Binary file greater than SRAM\n");
        fprintf(stderr, "       SRAM Size: %jd bytes\n", (intmax_t)APU_CTRL_SRAM_LENGTH);
        fprintf(stderr, "       File Size: %jd bytes\n", (intmax_t)data_len);
        return false;
    }

    printf("Downloading %jd bytes to SRAM\n", (intmax_t)data_len);

    iomemset(mmio_regs, APU_CTRL_SRAM_OFFSET, 0, APU_CTRL_SRAM_LENGTH);
    copytoio(mmio_regs, APU_CTRL_SRAM_OFFSET, data, data_len);
    return true;
}

static bool load_sram_partial(void* mmio_regs, const off_t offset, const void* data, size_t data_len)
{
    if (data_len > APU_CTRL_SRAM_LENGTH - offset) {
        fprintf(stderr, "Error: Binary file greater than SRAM\n");
        fprintf(stderr, "       SRAM Size: %jd bytes\n", (intmax_t)APU_CTRL_SRAM_LENGTH - offset);
        fprintf(stderr, "       File Size: %jd bytes\n", (intmax_t)data_len);
        return false;
    }

    printf("Downloading %jd bytes to SRAM\n", (intmax_t)data_len);

    copytoio(mmio_regs, APU_CTRL_SRAM_OFFSET + offset, data, data_len);
    return true;
}

static bool load_ddr_partial(void* ddr_ram, const off_t offset, const void* data, size_t data_len)
{
    if (data_len > DDR_MEM_LENGTH - offset) {
        fprintf(stderr, "Error: Binary file greater than DDR size\n");
        fprintf(stderr, "       DDR  Size: %jd bytes\n", (intmax_t)DDR_MEM_LENGTH - offset);
        fprintf(stderr, "       File Size: %jd bytes\n", (intmax_t)data_len);
        return false;
    }

    printf("Downloading %jd bytes to DDR\n", (intmax_t)data_len);

    copytoio(ddr_ram, offset, data, data_len);
    return true;
}

static const char* section_name(const char* names, int index)
{
    return names ? names + index : "?";
}

bool load_elf(void* mmio_regs, void* ddr_ram, const char* file, size_t file_len)
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
    iomemset(ddr_ram, 0, 0, DDR_MEM_LENGTH);
    iomemset(mmio_regs, APU_CTRL_SRAM_OFFSET, 0, APU_CTRL_SRAM_LENGTH);

    /* part 0: load program */
    printf("Program Header:\n");
    for (size_t load = 0; load < ehdr->e_phnum; load++) {
        Elf32_Phdr* phdr = (Elf32_Phdr*)(file + ehdr->e_phoff + load * ehdr->e_phentsize);

        printf(
            "    LOAD off    0x%08X vaddr 0x%08x paddr 0x%08x, align 2**%d\n",
            phdr->p_offset,
            phdr->p_vaddr,
            phdr->p_paddr,
            phdr->p_align);
        printf("         filesz 0x%08X memsz 0x%08x flags 0x%x\n", phdr->p_filesz, phdr->p_memsz, phdr->p_flags);

        bool success = false;
        if (phdr->p_paddr >= ELF_FILE_SRAM_BASE && phdr->p_paddr < ELF_FILE_SRAM_BASE + ELF_FILE_SRAM_LENGTH) {
            success =
                load_sram_partial(mmio_regs, phdr->p_paddr - ELF_FILE_SRAM_BASE, file + phdr->p_offset, phdr->p_filesz);
        }

        if (phdr->p_paddr >= ELF_FILE_DDR_BASE && phdr->p_paddr < ELF_FILE_DDR_BASE + ELF_FILE_DDR_LENGTH) {
            success =
                load_ddr_partial(ddr_ram, phdr->p_paddr - ELF_FILE_DDR_BASE, file + phdr->p_offset, phdr->p_filesz);

            // Missing: MMU setup for virtual <> physical DDR address translation
            fprintf(stderr, "Missing: MMU setup for virtual <> physical DDR address translation!\n");
            return false;
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

    /* part 1: allocate memory */
    for (size_t section = 0; section < ehdr->e_shnum; section++) {
        Elf32_Shdr* shdr = (Elf32_Shdr*)(file + ehdr->e_shoff + section * ehdr->e_shentsize);

        if (shdr->sh_type == SHT_NOBITS) {
            if (shdr->sh_flags & SHF_ALLOC) {
                printf(
                    "Allocate memory for section %d: %s: %d bytes (%d bytes aligned, flags %x)\n",
                    section,
                    section_name(names, shdr->sh_name),
                    shdr->sh_size,
                    shdr->sh_addralign,
                    shdr->sh_flags);
            }
        }
    }

    /* part 2: load program */
    for (size_t section = 0; section < ehdr->e_shnum; section++) {
        Elf32_Shdr* shdr = (Elf32_Shdr*)(file + ehdr->e_shoff + section * ehdr->e_shentsize);

        if (shdr->sh_type == SHT_PROGBITS) {
            if (shdr->sh_flags & SHF_ALLOC) {
                printf(
                    "Load program for section %d: %s: %d bytes (%d bytes aligned, flags %x)\n",
                    section,
                    section_name(names, shdr->sh_name),
                    shdr->sh_size,
                    shdr->sh_addralign,
                    shdr->sh_flags);
            }
        }
    }

    /* part 3: relocate */
    for (size_t section = 0; section < ehdr->e_shnum; section++) {
        Elf32_Shdr* shdr = (Elf32_Shdr*)(file + ehdr->e_shoff + section * ehdr->e_shentsize);

        if (shdr->sh_type == SHT_REL) {
            printf("REL for section %d: %s\n", section, section_name(names, shdr->sh_name));

            for (size_t i = 0; i < shdr->sh_size / shdr->sh_entsize; i++) {
                Elf32_Rel* rel = (Elf32_Rel*)(file + shdr->sh_offset + i * shdr->sh_entsize);
                printf("  Entry %d: %d (info %d)\n", i, rel->r_offset, rel->r_info);
            }

            fprintf(stderr, "Relocation not implemented yet");
            return false;
        }

        if (shdr->sh_type == SHT_RELA) {
            printf("RELA for section %d: %s\n", section, section_name(names, shdr->sh_name));
            for (size_t i = 0; i < shdr->sh_size / shdr->sh_entsize; i++) {
                Elf32_Rela* rela = (Elf32_Rela*)(file + shdr->sh_offset + i * shdr->sh_entsize);
                printf("  Entry %d: %d (info %d, addend %d)\n", i, rela->r_offset, rela->r_info, rela->r_addend);
            }

            fprintf(stderr, "Relocation not implemented yet");
            return false;
        }
    }

    return true;
}
