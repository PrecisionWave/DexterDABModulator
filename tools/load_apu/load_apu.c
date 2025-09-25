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

// open
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// mmap
#include <sys/mman.h>

// getopt, getpagesize
#include <unistd.h>
#include <errno.h>

// APU controls
const off_t APU_CTRL_BASE = 0x044000000U;
const off_t APU_CTRL_LENGTH = 0x30000U;
const off_t APU_SRAM_OFFSET = 0x00000U;
const off_t APU_SRAM_LENGTH = 0x04000U;
const off_t APU_GPIO_OFFSET = 0x10000U;
const off_t APU_MBOX_OFFSET = 0x20000U;

uint32_t ioread(void* ptr, size_t reg)
{
    volatile uint8_t* p8 = (volatile uint8_t*)(ptr) + reg;
    volatile uint32_t* p32 = (volatile uint32_t*)p8;
    return atomic_load(p32);
}

void iowrite(void* ptr, size_t reg, uint32_t data)
{
    volatile uint8_t* p8 = (volatile uint8_t*)(ptr) + reg;
    volatile uint32_t* p32 = (volatile uint32_t*)p8;
    atomic_store(p32, data);
}

void apu_reset(void* ptr, bool assert)
{
    if (assert) {
        // sleep
        iowrite(ptr, APU_GPIO_OFFSET + 0x0, 0);
        // assert reset
        iowrite(ptr, APU_GPIO_OFFSET + 0x8, 1);
    } else {
        // assert reset
        iowrite(ptr, APU_GPIO_OFFSET + 0x8, 0);
        // wakeup
        iowrite(ptr, APU_GPIO_OFFSET + 0x0, 1);
    }
}

void iomemset(void* ptr, size_t reg, uint8_t pattern, size_t length)
{
    volatile uint8_t* p8 = (volatile uint8_t*)(ptr) + reg;
    size_t remaining = length;
    switch (reg & 0x3) {
        case 1:
            atomic_store(p8, pattern);
            p8++;
            remaining--;
            if (remaining == 0)
                return;
        case 2:
            atomic_store(p8, pattern);
            p8++;
            remaining--;
            if (remaining == 0)
                return;
        case 3:
            atomic_store(p8, pattern);
            p8++;
            remaining--;
            if (remaining == 0)
                return;
        case 0:
        default:
            if (remaining == 0)
                return;
            break;
    }

    volatile uint32_t* p32 = (volatile uint32_t*)p8;
    uint32_t pattern32 = (pattern << 24U) | (pattern << 16U) | (pattern << 8U) | pattern;
    while (remaining > 3) {
        atomic_store(p32, pattern32);
        p32++;
        remaining -= 4;
    }

    p8 = (volatile uint8_t*)p32;
    switch (remaining & 0x3) {
        case 3:
            atomic_store(p8, pattern);
            p8++;
            remaining--;
        case 2:
            atomic_store(p8, pattern);
            p8++;
            remaining--;
        case 1:
            atomic_store(p8, pattern);
            p8++;
            remaining--;
        case 0:
        default:
            break;
    }

    assert(remaining == 0);
}

void copytoio(void* ptr, size_t reg, void* src, size_t length)
{
    volatile uint8_t* p8 = (volatile uint8_t*)(ptr) + reg;
    volatile uint8_t* s8 = (volatile uint8_t*)(src);
    size_t remaining = length;
    // copy bytes until aligned
    switch (reg & 0x3) {
        case 1:
            atomic_store(p8, *s8);
            p8++;
            s8++;
            remaining--;
            if (remaining == 0)
                return;
        case 2:
            atomic_store(p8, *s8);
            p8++;
            s8++;
            remaining--;
            if (remaining == 0)
                return;
        case 3:
            atomic_store(p8, *s8);
            p8++;
            s8++;
            remaining--;
            if (remaining == 0)
                return;
        case 0:
        default:
            if (remaining == 0)
                return;
            break;
    }

    // copy words
    volatile uint32_t* p32 = (volatile uint32_t*)p8;
    volatile uint32_t* s32 = (volatile uint32_t*)s8;
    while (remaining > 3) {
        atomic_store(p32, *s32);
        p32++;
        s32++;
        remaining -= 4;
    }

    // copy remaining bytes
    p8 = (volatile uint8_t*)p32;
    s8 = (volatile uint8_t*)s32;
    switch (remaining & 0x3) {
        case 3:
            atomic_store(p8, *s8);
            p8++;
            s8++;
            remaining--;
        case 2:
            atomic_store(p8, *s8);
            p8++;
            s8++;
            remaining--;
        case 1:
            atomic_store(p8, *s8);
            p8++;
            s8++;
            remaining--;
        case 0:
        default:
            break;
    }

    assert(remaining == 0);
}

int main(int argc, char** argv)
{
    int c;
    opterr = 0;
    bool do_reset = false;
    const char* download_file = NULL;
    const char* mmap_dev = "/dev/mem";
    off_t offset = APU_CTRL_BASE;
    size_t map_size = APU_CTRL_LENGTH;
    size_t page_size = getpagesize();
    printf("Page size: %zu bytes\n", page_size);

    while ((c = getopt(argc, argv, "d:l:a:rf:")) != -1) {
        switch (c) {
            case 'd':
                mmap_dev = optarg;
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
                mmap_dev = "/dev/mem";
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

    int fd_uio = open(mmap_dev, O_RDWR | O_SYNC);
    if (fd_uio < 1) {
        fprintf(stderr, "Failed to open uio: %s\n", mmap_dev);
        return -1;
    }

    void* ptr = mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_uio, offset);
    if (ptr == MAP_FAILED) {
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

    int dl_f = open(download_file, O_RDONLY);
    if (dl_f < 0) {
        fprintf(stderr, "Error: Unable to open file %s\n", download_file);
        goto cleanup;
    }

    off_t dl_size = lseek(dl_f, 0, SEEK_END);
    lseek(dl_f, 0, SEEK_SET);
    if (dl_size < 0) {
        fprintf(stderr, "Error: Unable to get download file size\n");
        goto cleanup;
    }

    if (dl_size > APU_SRAM_LENGTH) {
        fprintf(stderr, "Error: Binary file greater than SRAM\n");
        fprintf(stderr, "       SRAM Size: %jd bytes\n", (intmax_t)APU_SRAM_LENGTH);
        fprintf(stderr, "       File Size: %jd bytes\n", (intmax_t)dl_size);
        goto cleanup;
    }

    printf("Downloading %jd bytes\n", (intmax_t)dl_size);

    void* bin = malloc(dl_size);
    if (dl_size != read(dl_f, bin, dl_size)) {
        fprintf(stderr, "Error: Read failed: errno %d\n", errno);
        goto cleanup;
    }

    iomemset(ptr, APU_SRAM_OFFSET, 0, APU_SRAM_LENGTH);
    copytoio(ptr, APU_SRAM_OFFSET, bin, dl_size);

skip_download:
    if (do_reset) {
        printf("Release APU Reset\n");
        apu_reset(ptr, false);
    }

cleanup:
    close(dl_f);
    close(fd_uio);
    munmap(ptr, map_size);

    return 0;
}
