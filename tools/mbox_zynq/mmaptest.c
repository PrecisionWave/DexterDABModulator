#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>

// open
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// mmap
#include <sys/mman.h>

// getopt, getpagesize
#include <unistd.h>

static int fd_dev_apu0 = -1;


static uintptr_t get_ddr_phys_addr()
{
    if (fd_dev_apu0 == -1) exit(1);
    uintptr_t val = 0;

#define DEXTER_APU_IOCTL_GET_DDR_PHYS       _IOR(0, 4, uint32_t)
    ioctl(fd_dev_apu0, DEXTER_APU_IOCTL_GET_DDR_PHYS, &val);
    return val;
}


int main(int argc, char** argv)
{
    const char* mmap_dev = "/dev/apu0";

    size_t page_size = getpagesize();
    printf("Page size:  %zu bytes\n", page_size);
    printf("Device:     %s\n", mmap_dev);

    fd_dev_apu0 = open(mmap_dev, O_RDWR | O_SYNC);
    if (fd_dev_apu0 < 1) {
        fprintf(stderr, "Failed to mmap %s\n", mmap_dev);
        return -1;
    }

    const uintptr_t ADDR = 0x169108d8;
    const int LEN = 64;

    const uintptr_t ddr_phys_base = get_ddr_phys_addr();
    uintptr_t data_offset = ADDR - ddr_phys_base;

    size_t mmap_size = data_offset + LEN;
    printf("Map size:   %zu bytes (%zu pages)\n", mmap_size, mmap_size / page_size);

#define DEXTER_APU_MMAP_DDR 1
    const __off_t mmap_offset = DEXTER_APU_MMAP_DDR * page_size;
    void* ptr = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_dev_apu0, mmap_offset);
    if (ptr == MAP_FAILED) {
        fprintf(stderr, "MMAP Failed\n");
        return -1;
    }

    const uint8_t *data = ptr + data_offset;
    for (size_t i = 0; i < LEN; i++) {
        printf("%d ", data[i]);
    }
    printf("\n");

    munmap(ptr, mmap_size);
}
