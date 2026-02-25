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
        munmap(item->ptr, item->length);
        struct mmap* next = item->next;
        free(item);
        item = next;
    }
}

static struct mmap* simmem_list = NULL;

static void simmem_cleanup(void)
{
    struct mmap* item = simmem_list;
    while (item != NULL) {
        free(item->ptr);
        struct mmap* next = item->next;
        free(item);
        item = next;
    }
}

bool mmap_apu(int fd, enum device_index index, struct memory_map_entry* mme)
{
    assert(mme != NULL);

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

        case APU_DEVICE_ATCM:
            map_length = 64 * 1024;
            physical = 0xffe00000U;
            map_offset = DEXTER_APU_MMAP_SRAM * page_size;
            break;

        default:
            return NULL;
    }

    assert(map_length % page_size == 0);

    void* ptr = mmap(NULL, map_length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, map_offset);
    if (ptr == MAP_FAILED) {
        return false;
    }

    mme->length = map_length;
    mme->apu_loaded = physical;
    mme->cpu_virtual = ptr;

    if (mmap_list == NULL) {
        atexit(mmap_cleanup);
    }

    struct mmap* next = mmap_list;
    mmap_list = malloc(sizeof(struct mmap));
    mmap_list->next = next;
    mmap_list->ptr = ptr;
    mmap_list->length = map_length;

    return true;
}

bool mmap_apu_sim(enum device_index index, struct memory_map_entry* mme)
{
    assert(mme != NULL);

    switch (index) {
        case APU_DEVICE_SRAM:
            mme->length = 16 * 1024;
            mme->cpu_virtual = calloc(mme->length, 1);
            mme->apu_loaded = 0x20000000U;
            break;

        case APU_DEVICE_DDR:
            mme->length = 4 * 1024 * 1024;
            mme->cpu_virtual = calloc(mme->length, 1);
            mme->apu_loaded = 0x16900000U;
            break;

        case APU_DEVICE_ATCM:
            mme->length = 64 * 1024;
            mme->cpu_virtual = calloc(mme->length, 1);
            mme->apu_loaded = 0x00000000U;
            break;

        default:
            return NULL;
    }

    if (mme->cpu_virtual == MAP_FAILED) {
        return false;
    }

    if (simmem_list == NULL) {
        atexit(simmem_cleanup);
    }

    struct mmap* next = simmem_list;
    mmap_list = malloc(sizeof(struct mmap));
    mmap_list->next = next;
    mmap_list->ptr = mme->cpu_virtual;
    mmap_list->length = mme->length;

    return true;
}


bool mmap_reg(int fd, enum register_index index, struct memory_map_entry* mme)
{
    assert(mme != NULL);

    int page_size = getpagesize();

    uint32_t map_length = 0;
    uint32_t map_offset = 0;
    switch (index) {
        case APU_REGISTERS:
            map_offset = DEXTER_APU_MMAP_REGS * page_size;
            map_length = 0x30000;
            break;

        case APU_REGISTERS2:
            map_offset = DEXTER_APU_MMAP_REGS2 * page_size;
            map_length = 0x10000;
            break;

        default:
            return NULL;
    }

    assert(map_length % page_size == 0);

    void* ptr = mmap(NULL, map_length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, map_offset);
    if (ptr == MAP_FAILED) {
        return false;
    }

    mme->length = map_length;
    mme->cpu_virtual = ptr;

    if (mmap_list == NULL) {
        atexit(mmap_cleanup);
    }

    struct mmap* next = mmap_list;
    mmap_list = malloc(sizeof(struct mmap));
    mmap_list->next = next;
    mmap_list->ptr = ptr;
    mmap_list->length = map_length;

    return true;
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
