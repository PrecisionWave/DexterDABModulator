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

// Mailbox Registers
const size_t REG_MBOX_WRDATA = 0 * 4;  // WRDATA    Write only  N/A     FIFO Write Data (FSL tx)
const size_t REG_MBOX_WRCTRL = 1 * 4;  // WRCTRL    Write only  N/A     FIFO Write control (FSL tx)
const size_t REG_MBOX_RDDATA = 2 * 4;  // RDDATA    Read only   N/A     FIFO Read Data (FSL rx)
const size_t REG_MBOX_RDCTRL = 3 * 4;  // RDCTRL    Read only   N/A     FIFO Read control (FSL rx)
const size_t REG_MBOX_STATUS = 4 * 4;  // STATUS    Read only   0x1     Status flags for mailbox.
const size_t REG_MBOX_ERROR = 5 * 4;   // ERROR     Read only   0x0     Error flags, clear on read.
const size_t REG_MBOX_SIT = 6 * 4;     // SIT       Read/Write  -       Send Interrupt Threshold.
const size_t REG_MBOX_RIT = 7 * 4;     // RIT       Read/Write  -       Receive Interrupt Threshold.
const size_t REG_MBOX_IS = 8 * 4;      // IS        Read/Write  -       Interrupt Status register.
const size_t REG_MBOX_IE = 9 * 4;      // IE        Read/Write  -       Interrupt Enable register.
const size_t REG_MBOX_IP = 10 * 4;     // IP        Read only   -       Interrupt Pending register.
const size_t REG_MBOX_CTRL = 11 * 4;

typedef volatile void* mbox_t;

static int fd_dev_apu0 = -1;

static uint32_t MessageOut[4];
static uint32_t MessageIn[4];
#define MESSAGE_SIZE (4*4)


uint32_t mbox_ioread(mbox_t mbox, size_t reg)
{
    volatile uint8_t* p8 = (volatile uint8_t*)(mbox) + reg;
    volatile uint32_t* p32 = (volatile uint32_t*)p8;

    return atomic_load(p32);
    // return *p32;
}

void mbox_iowrite(mbox_t mbox, size_t reg, uint32_t data)
{
    volatile uint8_t* p8 = (volatile uint8_t*)(mbox) + reg;
    volatile uint32_t* p32 = (volatile uint32_t*)p8;
    //*p32 = data;
    atomic_store(p32, data);
}

bool mbox_is_empty(mbox_t mbox)
{
    return (mbox_ioread(mbox, REG_MBOX_STATUS) & 1) > 0;
}

bool mbox_is_full(mbox_t mbox)
{
    return (mbox_ioread(mbox, REG_MBOX_STATUS) & 2) > 0;
}

uint32_t mbox_read_fifo(mbox_t mbox)
{
    return mbox_ioread(mbox, REG_MBOX_RDDATA);
}

void mbox_write_fifo(mbox_t mbox, uint32_t data)
{
    mbox_iowrite(mbox, REG_MBOX_WRDATA, data);
}

size_t mbox_read(mbox_t mbox, void* buffer, size_t buffer_size)
{
    if (mbox_is_empty(mbox))
        return 0;

    uint32_t* p = buffer;
    size_t buffer_remaining = buffer_size;
    size_t bytes_read = 0;
    while (buffer_remaining >= 4 && !mbox_is_empty(mbox)) {
        *p = mbox_read_fifo(mbox);
        p++;
        bytes_read += 4;
        buffer_remaining -= 4;
    }
    return bytes_read;
}

void mbox_drain(mbox_t mbox)
{
    if (mbox_is_empty(mbox))
        return;

    while (!mbox_is_empty(mbox)) {
        (void)mbox_read_fifo(mbox);
    }
}

void mbox_dump(mbox_t mbox)
{
    printf("REG_MBOX_STATUS: 0x%08x\n", mbox_ioread(mbox, REG_MBOX_STATUS));
    printf("REG_MBOX_ERROR:  0x%08x\n", mbox_ioread(mbox, REG_MBOX_ERROR));
    printf("REG_MBOX_SIT:    0x%08x\n", mbox_ioread(mbox, REG_MBOX_SIT));
    printf("REG_MBOX_RIT:    0x%08x\n", mbox_ioread(mbox, REG_MBOX_RIT));
    printf("REG_MBOX_IS:     0x%08x\n", mbox_ioread(mbox, REG_MBOX_IS));
    printf("REG_MBOX_IE:     0x%08x\n", mbox_ioread(mbox, REG_MBOX_IE));
    printf("REG_MBOX_IP:     0x%08x\n", mbox_ioread(mbox, REG_MBOX_IP));
    fflush(stdout);
}

static uintptr_t get_ddr_phys_addr()
{
    if (fd_dev_apu0 == -1) exit(1);
    uintptr_t val = 0;

#define DEXTER_APU_IOCTL_GET_DDR_PHYS       _IOR(0, 4, uint32_t)
    ioctl(fd_dev_apu0, DEXTER_APU_IOCTL_GET_DDR_PHYS, &val);
    return val;
}

static void send_command(mbox_t mbox, char command)
{
    memset(MessageOut, 0, MESSAGE_SIZE);
    MessageOut[0] = command;
    MessageOut[1] = 0;
    MessageOut[2] = 0;
    MessageOut[3] = '.';

    printf("mbox write....\n");
    size_t written = 0;
    while (written != 4) {
        if (mbox_is_full(mbox)) {
            usleep(100);
            continue;
        }

        uint32_t data = MessageOut[written];
        mbox_write_fifo(mbox, data);
        char* pdata = (char*)&data;
        printf(
                "%4d: %08x %c%c%c%c\n",
                written,
                data,
                isprint(pdata[0]) ? pdata[0] : '.',
                isprint(pdata[1]) ? pdata[1] : '.',
                isprint(pdata[2]) ? pdata[2] : '.',
                isprint(pdata[3]) ? pdata[3] : '.');

        written += 1;
    }
    printf("Wrote %u \n", written);
}


size_t mbox_write(mbox_t mbox, void* buffer, size_t buffer_size)
{
    if (mbox_is_full(mbox))
        return 0;

    uint32_t* p = buffer;
    size_t buffer_remaining = buffer_size;
    size_t bytes_written = 0;
    while (buffer_remaining >= 4 && !mbox_is_full(mbox)) {
        mbox_write_fifo(mbox, *p);
        p++;
        bytes_written += 4;
        buffer_remaining -= 4;
    }
    return bytes_written;
}

static int receive_address(mbox_t mbox, char expected_command)
{
    printf("mbox read....\n");

    size_t bytes_read = 0;

    uint32_t *msg = MessageIn;

    while (bytes_read != MESSAGE_SIZE) {
        if (mbox_is_empty(mbox)) {
            usleep(100);
            continue;
        }

        uint32_t data = mbox_read_fifo(mbox);
        printf("%4d: %08x\n", bytes_read, data);
        *msg = data;
        msg++;
        bytes_read += 4;
    }

    printf("...done\n");

    const char cmd = MessageIn[0] & 0xFF;

    if (cmd != expected_command) {
        printf("Received wrong command from APU!, expected %c, got %c\n", expected_command, cmd);
        return -1;
    }

    if (MessageIn[3] != '.') {
        printf("message from APU not ending with a period!\n");
        return -1;
    }

    return 0;
}

static void* mmap_apu_data(uintptr_t data_offset, size_t data_len, size_t *mmap_size)
{
    size_t page_size = getpagesize();

    *mmap_size = data_offset + data_len;

#define DEXTER_APU_MMAP_DDR 1

    void* ptr = mmap(NULL, *mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_dev_apu0, DEXTER_APU_MMAP_DDR * page_size);
    if (ptr == MAP_FAILED) {
        fprintf(stderr, "MMAP Failed\n");
        return MAP_FAILED;
    }
    printf("Mapped apu data %zx with %zu bytes to %p\n", data_offset, data_len, ptr);
    return ptr;
}

// This returns the expected K&R hash the APU computes
static uint32_t write_arm2apu_data(void* ptr, uint32_t len, int pattern)
{
    printf("write_arm2apu_data %p\n", ptr);

    uint32_t h = 0;

    volatile uint8_t* data = (volatile uint8_t*)ptr;
    for (int i = 0; i < 64; i++) {
        uint32_t val = (pattern * i) % 31;

        h += val + 31 * h;

        atomic_store(data + i, val);
        //printf("%02x %d\n", val, val);
    }

    return h;
}

static int check_hash(void *ptr, uint32_t expected_hash)
{
    printf("check_hash %p\n", ptr);

    volatile uint32_t* data = (volatile uint32_t*)ptr;
    uint32_t apu_hash = atomic_load(data);
    if (apu_hash != expected_hash)
        printf("HASH ERROR 0x%08x 0x%08x\n", apu_hash, expected_hash);
    else
        printf(".");

    return 0;
}

static void dump_data(const uint8_t * buf, size_t len)
{
    printf("DATA at %p (%zu)", buf, len);
    for (size_t i = 0; i < len; i++) {
        printf("%d ", buf[i]);
    }
    printf("\n ");
}

int main(int argc, char** argv)
{
    int c;
    opterr = 0;
    bool do_flush = true;
    const char* mmap_dev = "/dev/apu0";
    off_t mbox_offset = 0x20000U;
    off_t mmap_offset = 0U;
    size_t page_size = getpagesize();
    printf("Page size:  %zu bytes\n", page_size);

    while ((c = getopt(argc, argv, "F")) != -1) {
        switch (c) {
            case 'F':
                do_flush = false;
                break;
            case '?':
                fprintf(stderr, "usage: %s \n", *argv);
                fprintf(stderr, "   -F              do not flush Mailbox\n");
                return 1;
        }
    }

    printf("Device:     %s\n", mmap_dev);
    printf("Map offset: 0x%08lx\n", mmap_offset);
    printf("Mbox reg:   0x%08lx\n", mbox_offset);

    fd_dev_apu0 = open(mmap_dev, O_RDWR | O_SYNC);
    if (fd_dev_apu0 < 1) {
        fprintf(stderr, "Failed to mmap %s\n", mmap_dev);
        return -1;
    }

    size_t mmap_size = ((mbox_offset + page_size) / page_size) * page_size;
    printf("Map size:   %zu bytes (%zu pages)\n", mmap_size, mmap_size / page_size);

    void* ptr = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_dev_apu0, mmap_offset & ~(mmap_size - 1));
    if (ptr == MAP_FAILED) {
        fprintf(stderr, "MMAP Failed\n");
        return -1;
    }

    printf("MMap to virtual address %p\n", ptr);

    mbox_t mbox = (mbox_t)(ptr + mbox_offset);

    printf("Mbox at virtual address %p\n", mbox);

    mbox_dump(mbox);

    if (do_flush) {
        printf("Flushing mbox....\n");
        while (!mbox_is_empty(mbox)) {
            (void)mbox_read_fifo(mbox);
        }
        printf("...done\n");
    }

    printf("Ask the APU for data_in memory address");
    send_command(mbox, 'a');

    if (receive_address(mbox, 'a') == -1) {
        printf("Failed to receive_address");
        return -1;
    }

    const uintptr_t ddr_phys_base = get_ddr_phys_addr();

    const uintptr_t data_arm2apu_phys_address = MessageIn[1];
    const uint32_t data_arm2apu_len = MessageIn[2];
    const uintptr_t data_arm2apu_offset = data_arm2apu_phys_address - ddr_phys_base;

    printf("ARM2APU Data at 0x%08x, offset 0x%08x\n", data_arm2apu_phys_address, data_arm2apu_offset);

    size_t data_arm2apu_mmap_size;
    void* data_arm2apu_ptr = mmap_apu_data(data_arm2apu_offset, data_arm2apu_len, &data_arm2apu_mmap_size);
    if (data_arm2apu_ptr == MAP_FAILED) return -1;

    size_t data_apu2arm_mmap_size = 0;
    void* data_apu2arm_ptr = MAP_FAILED;

    for (int pattern = 3; pattern <= 6; pattern++) {
        const uint32_t expected_hash = write_arm2apu_data(data_arm2apu_ptr + data_arm2apu_offset, data_arm2apu_len, pattern);

        dump_data(data_arm2apu_ptr + data_arm2apu_offset, data_arm2apu_len);

        // Tell the APU to calculate the hash, expect an 'o' message back with the address of the hash output
        send_command(mbox, 'H');

        if (receive_address(mbox, 'o') == -1) {
            printf("Failed to receive_address");
            return -1;
        }

        const uintptr_t data_apu2arm_phys_address = MessageIn[1];
        const uint32_t data_apu2arm_len = MessageIn[2];
        if (data_apu2arm_phys_address < ddr_phys_base) {
            printf("Invalid APU2ARM Data at 0x%08x\n", data_apu2arm_phys_address);
            return -1;
        }
        const uintptr_t data_apu2arm_offset = data_apu2arm_phys_address - ddr_phys_base;
        printf("APU2ARM Data at 0x%08x, offset 0x%08x\n", data_apu2arm_phys_address, data_apu2arm_offset);

        if (data_apu2arm_len != 4) {
            printf("apu2arm len is %d!\n", data_apu2arm_len);
            return -1;
        }

        if (data_apu2arm_ptr == MAP_FAILED) {
            data_apu2arm_ptr = mmap_apu_data(data_apu2arm_offset, data_apu2arm_len, &data_arm2apu_mmap_size);
        }

        if (data_apu2arm_ptr == MAP_FAILED)
            return -1;

        check_hash(data_apu2arm_ptr + data_apu2arm_offset, expected_hash);
    }

    close(fd_dev_apu0);
    munmap(data_apu2arm_ptr, data_apu2arm_mmap_size);
    munmap(data_arm2apu_ptr, data_arm2apu_mmap_size);
    munmap(ptr, mmap_size);

    return 0;
}
