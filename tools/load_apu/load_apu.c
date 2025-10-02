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

int main(int argc, char** argv)
{
    int c;
    opterr = 0;
    bool do_reset = false;
    const char* download_file = NULL;
    const char* dev = "/dev/apu0";
    printf("Page size: %zu bytes\n", getpagesize());

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

    int fd = open(dev, O_RDWR | O_SYNC);
    if (fd < 1) {
        fprintf(stderr, "Error: Unable to open device. errno %d\n", errno);
        return 2;
    }

    for (size_t i = 0; i < mm.count; i++) {
        if (!mmap_apu(fd, mm.entries[i].index, &mm.entries[i])) {
            fprintf(stderr, "Error: MMAP of APU %s failed! errno %d\n", mm.entries[i].name, errno);
            return 3;
        }
    }

    if (do_reset) {
        printf("Assert APU Reset\n");
        apu_reset(fd, true);
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

        if (!download_success) {
            fprintf(stderr, "Error: Download failed!\n");
            return 5;
        }
    }

    if (do_reset) {
        printf("Release APU Reset\n");
        apu_reset(fd, false);
    }

    return 0;
}
