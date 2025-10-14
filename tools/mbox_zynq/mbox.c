// Shared mailbox example
// Zynq part

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <ctype.h>

// open
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

static char ProducerHello[] = "Hello! The Producer greets the Consumer";
static_assert(sizeof(ProducerHello) % 4 == 0, "Message must be a multiple of 4 bytes!");
static_assert(sizeof(ProducerHello) == 40, "Example expects 40 bytes");

static char ConsumerHello[] = "Hello! The Consumer greets the Producer";
static_assert(sizeof(ConsumerHello) % 4 == 0, "Message must be a multiple of 4 bytes!");
static_assert(sizeof(ConsumerHello) == 40, "Example expects 40 bytes");


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

int main(int argc, char** argv)
{
    int c;
    opterr = 0;
    bool do_flush = false;
    bool do_read = false;
    bool do_write = false;
    const char* mmap_dev = "/dev/apu0";
    off_t mbox_offset = 0x20000U;
    off_t mmap_offset = 0U;
    size_t page_size = getpagesize();
    printf("Page size:  %zu bytes\n", page_size);

    while ((c = getopt(argc, argv, "frwo:a:d:")) != -1) {
        switch (c) {
            case 'd':
                mmap_dev = optarg;
                if (strcmp(optarg, "/dev/mem") == 0) {
                    mmap_offset = 0x44000000U;
                    mbox_offset = 0x20000U;
                }
                break;
            case 'f':
                do_flush = true;
                break;
            case 'w':
                do_write = true;
                break;
            case 'r':
                do_read = true;
                break;
            case 'o':
                mmap_offset = strtoul(optarg, NULL, 0);
                break;
            case 'a':
                mbox_offset = strtoul(optarg, NULL, 0);
                break;
            case '?':
                fprintf(stderr, "usage: %s [-d /dev/apuX] -rwf [-o mmap offset] [-a mbox offset]\n", *argv);
                fprintf(stderr, "   -d /dev/apuX    Optional, use device /dev/apuX\n");
                fprintf(stderr, "   -d /dev/mem     Optional, use device /dev/mem\n");
                fprintf(
                    stderr,
                    "   -o 0            Optional, offset used for mmap call (Default is 0 and 0x44000000 for "
                    "/dev/mem)\n");
                fprintf(stderr, "   -a 0x20000      Optional, offset to mailbox registers (Default is 0x20000)\n");
                fprintf(stderr, "   -r              Read from Mailbox\n");
                fprintf(stderr, "   -w              Write to Mailbox\n");
                fprintf(stderr, "   -f              Flush Mailbox\n");
                return 1;
        }
    }

    printf("Device:     %s\n", mmap_dev);
    printf("Map offset: 0x%08lx\n", mmap_offset);
    printf("Mbox reg:   0x%08lx\n", mbox_offset);

    if (!do_flush && !do_read && !do_write) { 
        printf("Warning: Using default action \"RW\"\n");
        do_read = true;
        do_write = true;
    }

    int fd_uio = open(mmap_dev, O_RDWR | O_SYNC);
    if (fd_uio < 1) {
        fprintf(stderr, "Failed to open uio: %s\n", mmap_dev);
        return -1;
    }

    size_t mmap_size = ((mbox_offset + page_size) / page_size) * page_size;
    printf("Map size:   %zu bytes (%zu pages)\n", mmap_size, mmap_size / page_size);

    void* ptr = mmap(NULL, mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_uio, mmap_offset & ~(mmap_size - 1));
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

    if (do_write) {
        printf("mbox write....\n");
        size_t bytes_written = 0;
        while (bytes_written != sizeof(ConsumerHello)) {
            if (mbox_is_full(mbox)) {
                usleep(100);
                continue;
            }

            uint32_t data = *(uint32_t*)(ConsumerHello + bytes_written);
            mbox_write_fifo(mbox, data);
            char* pdata = (char*)&data;
            printf(
                "%4d: %08x %c%c%c%c\n",
                bytes_written,
                data,
                isprint(pdata[0]) ? pdata[0] : '.',
                isprint(pdata[1]) ? pdata[1] : '.',
                isprint(pdata[2]) ? pdata[2] : '.',
                isprint(pdata[3]) ? pdata[3] : '.');

            bytes_written += 4;
        }
        printf("Wrote %u bytes\n", bytes_written);
    }

    if (do_read) {
        printf("mbox read....\n");

        size_t bytes_read = 0;

        while (bytes_read != sizeof(ProducerHello)) {
            if (mbox_is_empty(mbox)) {
                usleep(100);
                continue;
            }

            uint32_t data = mbox_read_fifo(mbox);
            char* pdata = (char*)&data;
            printf(
                "%4d: %08x %c%c%c%c\n",
                bytes_read,
                data,
                isprint(pdata[0]) ? pdata[0] : '.',
                isprint(pdata[1]) ? pdata[1] : '.',
                isprint(pdata[2]) ? pdata[2] : '.',
                isprint(pdata[3]) ? pdata[3] : '.');
            bytes_read += 4;
        }

        printf("...done\n");
    }

    close(fd_uio);
    munmap(ptr, mmap_size);

    return 0;
}
