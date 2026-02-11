// Load APU SRAM with program and start processor

// Usage:
//  Using /dev/mem:
//      ./load_apu -f mbox.bin
//      ./load_apu -r
//
//  Using /dev/apu0 to write to SRAM
//      ./load_apu -a0 -d/dev/apu0 -f mbox.bin
//      ./load_apu -a0 -d/dev/apu0 -r
//
//  Using /dev/apu0 to wite to DMA memory (experimental)
//      ./load_apu -a0x1000 -d/dev/apu0 -fmbox.bin

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

// getopt, getpagesize
#include <unistd.h>
#include <errno.h>

// open
#include <fcntl.h>

// ioctl
#include <sys/ioctl.h>

// project includes
#include "apu.h"
#include "mmio.h"
#include "mmap.h"
#include "load_elf.h"


void apu_reset(int fd, bool assert)
{
    int val = assert ? 1 : 0;
    ioctl(fd, DEXTER_APU_IOCTL_APU_RESET, &val);
}


void apu_start(int fd, uint32_t start_address)
{
    ioctl(fd, DEXTER_APU_IOCTL_APU_START, &start_address);
}


static const char ELF_SIGNATURE[] = {0x7f, 'E', 'L', 'F'};

static struct memory_map mm = {
    .count = 2,
    .entries =
        {
            {
                .name = "SRAM",
                .index = APU_DEVICE_SRAM,
                .apu_linked = ELF_FILE_SRAM_BASE,
            },
            {
                .name = "DDR",
                .index = APU_DEVICE_DDR,
                .apu_linked = ELF_FILE_DDR_BASE,
            },
        },
};

int g_elf_debug_level = 0;
bool g_elf_force_reloc = false;
bool g_elf_ignore_unaligned = false;

void usage(const char* progname)
{
    fprintf(stderr, "usage: %s [-d /dev/apuX] [-r] [-f file] [-x dumpfile] [-a] [-S] [-U]\n", progname);
#ifndef SIM
    fprintf(stderr, "  -d /dev/apu0     Device to use\n");
#endif
    fprintf(stderr, "  -f file.elf      Download elf file\n");
    fprintf(stderr, "  -f file.bin      Download bin file\n");
#ifndef SIM
    fprintf(stderr, "  -r               Perform APU reset (implied by -f)\n");
#endif
    fprintf(stderr, "  -v               Increase debug level\n");
    fprintf(stderr, "  -a               Force elf relocation\n");
    fprintf(stderr, "  -x basename      Dump RAM contents after download\n");
    fprintf(stderr, "  -U               allow Unaligned relocations\n");
#ifndef SIM
    fprintf(stderr, "  -s <address>     Set CPU start address\n");
#endif
}

struct APU_LDR {
    const uint64_t signature;
    const uint32_t ddr_start;
    const uint32_t ddr_length;
} __attribute__((aligned(8), packed)) loader_info;


void apply_apu_ldr_header(struct memory_map_entry* mme, struct memory_map_entry* mme_ddr)
{
    uint64_t ldr_signature = 0x10adcba987654321ULL;
    uint64_t ldr_complete = 0x10ad123456789abcULL;
    for (size_t i = 0; i < mme->length - sizeof(struct APU_LDR); i++) {
        if (memcmp(mme->cpu_virtual + i, &ldr_signature, sizeof(ldr_signature)) == 0) {
            printf("APU_LDR signature found at %s:%04zx\n", mme->name, i);
            memcpy(mme->cpu_virtual + i, &ldr_complete, sizeof(ldr_complete));
            memcpy(mme->cpu_virtual + i + 8, &mme_ddr->apu_loaded, 4);
            memcpy(mme->cpu_virtual + i + 12, &mme_ddr->length, 4);
        }
    }
}

const char* g_dump_file = NULL;

int main(int argc, char** argv)
{
    int c;
    opterr = 0;
    bool do_reset = false;
    bool do_sim = false;
    const char* dev = "/dev/apu0";
    const char* download_file = NULL;
    uint32_t start_address = 0;
    printf("Page size: %d bytes\n", getpagesize());

    while ((c = getopt(argc, argv, "d:rf:vx:as:US")) != -1) {
        switch (c) {
            case 'd':
                dev = optarg;
                break;
            case 'r':
                do_reset = true;
                break;
            case 'f':
                do_reset = true;
                download_file = optarg;
                break;
            case 'v':
                g_elf_debug_level++;
                break;
            case 'x':
                g_dump_file = optarg;
                break;
            case 'a':
                g_elf_force_reloc = true;
                break;
            case 's':
                start_address = strtoul(optarg, NULL, 16);
                break;
            case 'U':
                g_elf_ignore_unaligned = true;
                break;
            case 'S':
                do_sim = true;
                break;
            case '?':
                usage(argv[0]);
                return 1;
        }
    }

#ifdef SIM
    do_sim = true;
    do_reset = false;
    dev = "/dev/null";
#endif

    if (!download_file && !do_reset) {
        usage(argv[0]);
        return 42;
    }

    int fd = open(dev, O_RDWR | O_SYNC);
    if (fd < 1) {
        fprintf(stderr, "Error: Unable to open device. errno %d\n", errno);
        return 2;
    }

    for (size_t i = 0; i < mm.count; i++) {
        if (do_sim && !mmap_apu_sim(mm.entries[i].index, &mm.entries[i])) {
            fprintf(stderr, "Error: malloc of APU %s failed! errno %d\n", mm.entries[i].name, errno);
            return 3;
        }

        if (!do_sim && !mmap_apu(fd, mm.entries[i].index, &mm.entries[i])) {
            fprintf(stderr, "Error: MMAP of APU %s failed! errno %d\n", mm.entries[i].name, errno);
            return 3;
        }
    }

    if (do_reset || start_address > 0) {
        printf("Assert APU Reset\n");
        if (!do_sim) {
            apu_reset(fd, true);
        }
    }

    if (download_file) {
        printf("Loading file %s\n", download_file);

        size_t flen = 0;
        void* fptr = mmap_file(download_file, &flen, false);
        if (fptr == NULL) {
            fprintf(stderr, "Error: Unable to open file %s\n", download_file);
            return 4;
        }

        bool download_success = false;
        if (memcmp(fptr, ELF_SIGNATURE, 4) == 0) {
            download_success = load_elf(fptr, flen, &mm);
        } else {
            download_success = load_bin(fptr, flen, &mm, ELF_FILE_SRAM_BASE);
        }

        if (download_success) {
            apply_apu_ldr_header(mm_lookup(&mm, ELF_FILE_SRAM_BASE), mm_lookup(&mm, ELF_FILE_DDR_BASE));
            apply_apu_ldr_header(mm_lookup(&mm, ELF_FILE_DDR_BASE), mm_lookup(&mm, ELF_FILE_DDR_BASE));
        }

        if (!download_success) {
            fprintf(stderr, "Error: Download failed!\n");
            return 5;
        }
    }

    if (g_dump_file) {
        for (size_t i = 0; i < mm.count; i++) {
            char dump_file_name[1024];
            memset(dump_file_name, 0, sizeof(dump_file_name));
            snprintf(dump_file_name, sizeof(dump_file_name) - 1, "%s.%s", g_dump_file, mm.entries[i].name);
            printf("Dumping %s to %s...\n", mm.entries[i].name, dump_file_name);
            int dfd = open(dump_file_name, O_CREAT | O_RDWR | O_TRUNC, 0666);
            if (mm.entries[i].length != write(dfd, mm.entries[i].cpu_virtual, mm.entries[i].length)) {
                int tmp_errno = errno;
                fprintf(
                    stderr, "Error: Failed to write %d bytes to file. errno was %d\n", mm.entries[i].length, tmp_errno);
                return 10;
            }
            printf("Wrote %d bytes\n", mm.entries[i].length);
            close(dfd);
        }
    }

    if (do_reset || start_address > 0) {
        printf("Release APU Reset\n");
        if (!do_sim) {
            apu_start(fd, start_address);
        }
    }

    return 0;
}
