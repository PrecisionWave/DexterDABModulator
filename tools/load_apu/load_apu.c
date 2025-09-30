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

// project includes
#include "apu.h"
#include "mmio.h"
#include "mmap.h"
#include "load_elf.h"


void apu_reset(void* mmio_regs, bool assert)
{
    if (assert) {
        // sleep
        iowrite32(mmio_regs, APU_CTRL_GPIO_OFFSET + 0x0, 0);
        // assert reset
        iowrite32(mmio_regs, APU_CTRL_GPIO_OFFSET + 0x8, 1);
    } else {
        // assert reset
        iowrite32(mmio_regs, APU_CTRL_GPIO_OFFSET + 0x8, 0);
        // wakeup
        iowrite32(mmio_regs, APU_CTRL_GPIO_OFFSET + 0x0, 1);
    }
}

static const char ELF_SIGNATURE[] = {0x7f, 'E', 'L', 'F'};

int main(int argc, char** argv)
{
    int c;
    opterr = 0;
    bool do_reset = false;
    const char* download_file = NULL;
    const char* dev = "/dev/apu0";
    size_t map_size = APU_CTRL_LENGTH;
    size_t page_size = getpagesize();
    printf("Page size: %zu bytes\n", page_size);

    while ((c = getopt(argc, argv, "d:l:a:rf:")) != -1) {
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
            case '?':
                fprintf(stderr, "usage: %s [-l length] [-a address] [-d device] [-r] [-f file]\n", *argv);
                return 1;
        }
    }

    void* mmio_regs = mmap_dev(dev, 0 * page_size, map_size);
    if (mmio_regs == NULL) {
        fprintf(stderr, "Error: MMAP of registers Failed. errno %d\n", errno);
        return -1;
    }

    void* ddr_ram = mmap_dev(dev, 1 * page_size, DDR_MEM_LENGTH);
    if (ddr_ram == NULL) {
        fprintf(stderr, "Warning: MMAP of shared DDR memory Failed. errno %d\n", errno);
        return -1;
    }

    if (do_reset) {
        printf("Assert APU Reset\n");
        apu_reset(mmio_regs, true);
    }

    if (download_file) {
        printf("Loading file %s\n", download_file);

        size_t flen = 0;
        void* fptr = mmap_file(download_file, &flen, false);
        if (fptr == NULL) {
            fprintf(stderr, "Error: Unable to open file %s\n", download_file);
            return -1;
        }

        bool download_success = false;
        if (memcmp(fptr, ELF_SIGNATURE, 4) == 0) {
            download_success = load_elf(mmio_regs, ddr_ram, fptr, flen);
        } else {
            download_success = load_sram(mmio_regs, fptr, flen);
        }

        if(!download_success) {
            fprintf(stderr, "Error: Download failed!\n");
            return -1;
        }
    }

    if (do_reset) {
        printf("Release APU Reset\n");
        apu_reset(mmio_regs, false);
    }

    return 0;
}
