#include "mmap.h"

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

// mmap
#include <sys/mman.h>

// open
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// ioctl
#include <sys/ioctl.h>

// lseek
#include <unistd.h>

// project
#include "dexter_apu.h"

struct mmap {
    struct mmap* next;
    void* ptr;
    size_t length;
};

static struct mmap* mmap_list = NULL;

static void mmap_cleanup(void)
{
    struct mmap* item = mmap_list;
    while (item != NULL) {
        // printf("Unmaping %p (%zu)\n", item->ptr, item->length);
        munmap(item->ptr, item->length);
        struct mmap* next = item->next;
        free(item);
        item = next;
    }
}

void* mmap_apu(int fd, enum device_index index, size_t* out_length, size_t* out_physical)
{
    int page_size = getpagesize();

    uint32_t map_length = 0;
    uint32_t map_offset = 0;
    uint32_t physical = 0;
    int err;
    switch (index) {
        case APU_DEVICE_SRAM:
            if ((err = ioctl(fd, DEXTER_APU_IOCTL_GET_SRAM_SIZE, &map_length))) {
                fprintf(stderr, "Error: DEXTER_APU_IOCTL_GET_SRAM_SIZE failed with err = %d!\n", err);
                return NULL;
            }
            physical = 0x20000000U;
            map_offset = DEXTER_APU_MMAP_SRAM * page_size;
            break;

        case APU_DEVICE_DDR:
            if ((err = ioctl(fd, DEXTER_APU_IOCTL_GET_DDR_SIZE, &map_length))) {
                fprintf(stderr, "Error: DEXTER_APU_IOCTL_GET_DDR_SIZE failed with err = %d!\n", err);
                return NULL;
            }
            if ((err = ioctl(fd, DEXTER_APU_IOCTL_GET_DDR_PHYS, &physical))) {
                fprintf(stderr, "Error: DEXTER_APU_IOCTL_GET_DDR_PHYS failed with err = %d!\n", err);
                return NULL;
            }
            map_offset = DEXTER_APU_MMAP_DDR * page_size;
            break;

        default:
            return NULL;
    }

    assert(map_length % page_size == 0);

    void* ptr = mmap(NULL, map_length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, map_offset);
    if (ptr == MAP_FAILED) {
        return NULL;
    }

    if (out_length)
        *out_length = map_length;
    if (out_physical)
        *out_physical = physical;

    if (mmap_list == NULL) {
        atexit(mmap_cleanup);
    }

    struct mmap* next = mmap_list;
    mmap_list = malloc(sizeof(struct mmap));
    mmap_list->next = next;
    mmap_list->ptr = ptr;
    mmap_list->length = map_length;

    return ptr;
}

void* mmap_dev(const char* dev, size_t offset, size_t length)
{
    int fd = open(dev, O_RDWR | O_SYNC);
    if (fd < 1) {
        return NULL;
    }

    void* ptr = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);
    if (ptr == MAP_FAILED) {
        close(fd);
        return NULL;
    }

    if (mmap_list == NULL) {
        atexit(mmap_cleanup);
    }

    struct mmap* next = mmap_list;
    mmap_list = malloc(sizeof(struct mmap));
    mmap_list->next = next;
    mmap_list->ptr = ptr;
    mmap_list->length = length;

    close(fd);

    return ptr;
}

void* mmap_file(const char* file, size_t* out_length, bool rw)
{
    int fd = open(file, rw ? O_RDWR | O_SYNC : O_RDONLY | O_SYNC);
    if (fd < 1) {
        return NULL;
    }

    off_t length = lseek(fd, 0, SEEK_END);
    if (length < 0) {
        close(fd);
        return NULL;
    }

    if (out_length) {
        *out_length = length;
    }

    size_t page_size = getpagesize();
    length = (length + page_size - 1) / page_size * page_size;

    void* ptr = mmap(NULL, length, rw ? PROT_READ | PROT_WRITE : PROT_READ, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        close(fd);
        return NULL;
    }

    if (mmap_list == NULL) {
        atexit(mmap_cleanup);
    }

    struct mmap* next = mmap_list;
    mmap_list = malloc(sizeof(struct mmap));
    mmap_list->next = next;
    mmap_list->ptr = ptr;
    mmap_list->length = length;

    close(fd);

    return ptr;
}
