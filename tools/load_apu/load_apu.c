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

#include "mmio.h"
#include "mmap.h"

// APU controls
const off_t APU_CTRL_BASE = 0x044000000U;
const off_t APU_CTRL_LENGTH = 0x30000U;
const off_t APU_SRAM_OFFSET = 0x00000U;
const off_t APU_SRAM_LENGTH = 0x04000U;
const off_t APU_GPIO_OFFSET = 0x10000U;
const off_t APU_MBOX_OFFSET = 0x20000U;

void apu_reset(void* p_regs, bool assert)
{
    if (assert) {
        // sleep
        iowrite32(p_regs, APU_GPIO_OFFSET + 0x0, 0);
        // assert reset
        iowrite32(p_regs, APU_GPIO_OFFSET + 0x8, 1);
    } else {
        // assert reset
        iowrite32(p_regs, APU_GPIO_OFFSET + 0x8, 0);
        // wakeup
        iowrite32(p_regs, APU_GPIO_OFFSET + 0x0, 1);
    }
}

int main(int argc, char** argv)
{
    int c;
    opterr = 0;
    bool do_reset = false;
    const char* download_file = NULL;
    const char* dev = "/dev/mem";
    off_t offset = APU_CTRL_BASE;
    size_t map_size = APU_CTRL_LENGTH;
    size_t page_size = getpagesize();
    printf("Page size: %zu bytes\n", page_size);

    while ((c = getopt(argc, argv, "d:l:a:rf:")) != -1) {
        switch (c) {
            case 'd':
                dev = optarg;
                break;
            case 'l':
                map_size = strtoul(optarg, NULL, 16);
                if ((map_size & (page_size - 1)) != 0) {
                    fprintf(stderr, "Error: length must be page aligned\n");
                    return 1;
                }
                break;
            case 'a':
                offset = strtoul(optarg, NULL, 16);
                if ((offset & (page_size - 1)) != 0) {
                    fprintf(stderr, "Error: Address must be page aligned\n");
                    return 1;
                }
                dev = "/dev/mem";
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

    void* ptr = mmap_dev(dev, offset, map_size);
    if (ptr == NULL) {
        fprintf(stderr, "Error: MMAP Failed. errno %d\n", errno);
        return -1;
    }

    if (do_reset) {
        printf("Assert APU Reset\n");
        apu_reset(ptr, true);
    }

    if (!download_file)
        goto skip_download;

    printf("Loading file %s\n", download_file);

    size_t flen = 0;
    void* fptr = mmap_file(download_file, &flen, false);
    if (fptr == NULL) {
        fprintf(stderr, "Error: Unable to open file %s\n", download_file);
        return -1;
    }

    if (flen > APU_SRAM_LENGTH) {
        fprintf(stderr, "Error: Binary file greater than SRAM\n");
        fprintf(stderr, "       SRAM Size: %jd bytes\n", (intmax_t)APU_SRAM_LENGTH);
        fprintf(stderr, "       File Size: %jd bytes\n", (intmax_t)flen);
        return -1;
    }

    printf("Downloading %jd bytes\n", (intmax_t)flen);

    iomemset(ptr, APU_SRAM_OFFSET, 0, APU_SRAM_LENGTH);
    copytoio(ptr, APU_SRAM_OFFSET, fptr, flen);

skip_download:
    if (do_reset) {
        printf("Release APU Reset\n");
        apu_reset(ptr, false);
    }

    return 0;
}
