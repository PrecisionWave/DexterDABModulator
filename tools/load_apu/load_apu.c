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

static struct memory_map_entry mmaxifw = {
    0,
};

void axi_fw_status(int fd)
{
    if (!mmaxifw.cpu_virtual)
        return;
    printf("AXI Firewall status\n");
    uint32_t MI_Fault_Status = ioread32(mmaxifw.cpu_virtual, 0x000);
    printf("MI_Fault_Status: %08x\n", MI_Fault_Status);
    if (MI_Fault_Status & (1 << 26))
        printf("\tXILINX_WR_DECERR\n");
    if (MI_Fault_Status & (1 << 25))
        printf("\tXILINX_WR_SLVERR\n");
    if (MI_Fault_Status & (1 << 24))
        printf("\tERRM_WVALID_STABLE\n");
    if (MI_Fault_Status & (1 << 23))
        printf("\tERRM_AWVALID_STABLE\n");
    if (MI_Fault_Status & (1 << 22))
        printf("\tERRM_AWADDR_BOUNDARY\n");
    if (MI_Fault_Status & (1 << 21))
        printf("\tERRM_WDATA_NUM\n");
    if (MI_Fault_Status & (1 << 20))
        printf("\tERRM_AWSIZE\n");
    if (MI_Fault_Status & (1 << 19))
        printf("\tRECM_WVALID_TO_AWVALID_MAX_WAIT\n");
    if (MI_Fault_Status & (1 << 18))
        printf("\tRECM_CONTINUOUS_WTRANSFERS_MAX_WAIT\n");
    if (MI_Fault_Status & (1 << 17))
        printf("\tRECM_BREADY_MAX_WAIT\n");
    if (MI_Fault_Status & (1 << 16))
        printf("\tWRITE_RESPONSE_BUSY\n");
    if (MI_Fault_Status & (1 << 6))
        printf("\tXILINX_RD_DECERR\n");
    if (MI_Fault_Status & (1 << 5))
        printf("\tXILINX_RD_SLVERR\n");
    if (MI_Fault_Status & (1 << 4))
        printf("\tERRM_ARVALID_STABLE\n");
    if (MI_Fault_Status & (1 << 3))
        printf("\tERRM_ARADDR_BOUNDARY\n");
    if (MI_Fault_Status & (1 << 2))
        printf("\tERRM_ARSIZE\n");
    if (MI_Fault_Status & (1 << 1))
        printf("\tRECM_RREADY_MAX_WAIT\n");
    if (MI_Fault_Status & (1 << 0))
        printf("\tREAD_RESPONSE_BUSY\n");

    uint32_t SI_Fault_Status = ioread32(mmaxifw.cpu_virtual, 0x100);
    printf("SI_Fault_Status: %08x\n", SI_Fault_Status);
    if (SI_Fault_Status & (1 << 26))
        printf("\tXILINX_WR_DECERR\n");
    if (SI_Fault_Status & (1 << 25))
        printf("\tXILINX_WR_SLVERR\n");
    if (SI_Fault_Status & (1 << 24))
        printf("\tERRM_WVALID_STABLE\n");
    if (SI_Fault_Status & (1 << 23))
        printf("\tERRM_AWVALID_STABLE\n");
    if (SI_Fault_Status & (1 << 22))
        printf("\tERRM_AWADDR_BOUNDARY\n");
    if (SI_Fault_Status & (1 << 21))
        printf("\tERRM_WDATA_NUM\n");
    if (SI_Fault_Status & (1 << 20))
        printf("\tERRM_AWSIZE\n");
    if (SI_Fault_Status & (1 << 19))
        printf("\tRECM_WVALID_TO_AWVALID_MAX_WAIT\n");
    if (SI_Fault_Status & (1 << 18))
        printf("\tRECM_CONTINUOUS_WTRANSFERS_MAX_WAIT\n");
    if (SI_Fault_Status & (1 << 17))
        printf("\tRECM_BREADY_MAX_WAIT\n");
    if (SI_Fault_Status & (1 << 16))
        printf("\tWRITE_RESPONSE_BUSY\n");
    if (SI_Fault_Status & (1 << 6))
        printf("\tXILINX_RD_DECERR\n");
    if (SI_Fault_Status & (1 << 5))
        printf("\tXILINX_RD_SLVERR\n");
    if (SI_Fault_Status & (1 << 4))
        printf("\tERRM_ARVALID_STABLE\n");
    if (SI_Fault_Status & (1 << 3))
        printf("\tERRM_ARADDR_BOUNDARY\n");
    if (SI_Fault_Status & (1 << 2))
        printf("\tERRM_ARSIZE\n");
    if (SI_Fault_Status & (1 << 1))
        printf("\tRECM_RREADY_MAX_WAIT\n");
    if (SI_Fault_Status & (1 << 0))
        printf("\tREAD_RESPONSE_BUSY\n");

    uint32_t Final_ARADDR_Low = ioread32(mmaxifw.cpu_virtual, 0x210);
    uint32_t Final_ARADDR_High = ioread32(mmaxifw.cpu_virtual, 0x214);
    uint32_t Final_AWADDR_Low = ioread32(mmaxifw.cpu_virtual, 0x218);
    uint32_t Final_AWADDR_High = ioread32(mmaxifw.cpu_virtual, 0x21c);
    printf("Final_ARADDR_Low: %08x\n", Final_ARADDR_Low);
    printf("Final_ARADDR_High: %08x\n", Final_ARADDR_High);
    printf("Final_AWADDR_Low: %08x\n", Final_AWADDR_Low);
    printf("Final_AWADDR_High: %08x\n", Final_AWADDR_High);
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
    fprintf(stderr, "  -d /dev/apu0     Device to use\n");
    fprintf(stderr, "  -f file.elf      Download elf file\n");
    fprintf(stderr, "  -f file.bin      Download bin file\n");
    fprintf(stderr, "  -r               Perform APU reset (implied by -f)\n");
    fprintf(stderr, "  -v               Increase debug level\n");
    fprintf(stderr, "  -a               Force elf relocation\n");
    fprintf(stderr, "  -x basename      Dump RAM contents after download\n");
    fprintf(stderr, "  -S               Show status of AXI Firewall\n");
    fprintf(stderr, "  -U               allow Unaligned relocations\n");
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
            printf("APU_LDR signature found at %s:%04x\n", mme->name, i);
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
    bool do_axifw_status = false;
    const char* download_file = NULL;
    const char* dev = "/dev/apu0";
    printf("Page size: %zu bytes\n", getpagesize());

    while ((c = getopt(argc, argv, "d:rf:vx:aSU")) != -1) {
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
            case 'S':
                do_axifw_status = true;
                break;
            case 'U':
                g_elf_ignore_unaligned = true;
                break;
            case '?':
                usage(argv[0]);
                return 1;
        }
    }

    if (!download_file && !do_reset && !do_axifw_status) {
        usage(argv[0]);
        return 42;
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

    mmap_reg(fd, APU_REGISTERS2, &mmaxifw);

    if (do_axifw_status)
        axi_fw_status(fd);

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

    if (do_reset) {
        printf("Release APU Reset\n");
        apu_reset(fd, false);
    }

    return 0;
}
