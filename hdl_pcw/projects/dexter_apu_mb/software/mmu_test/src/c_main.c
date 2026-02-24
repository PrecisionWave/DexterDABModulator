
#include <xil_printf.h>
#include <xil_exception.h>
#include <mb_interface.h>
#include <assert.h>
#include "mmu.h"

#define set_pid(value)          \
    asm volatile(               \
        "mts    rpid, %0    \n" \
        "nop                \n" \
        "bri    4           \n" \
        :                       \
        : "r"(value)            \
        :)

#define set_tlb_entry(index, hi, lo) \
    asm volatile(                    \
        "mts     rtlbx, %0  \n"      \
        "mts     rtlbhi, %1 \n"      \
        "mts     rtlblo, %2 \n"      \
        "nop                \n"      \
        :                            \
        : "r"(index), "r"(hi), "r"(lo))

void set_tlb(int index, uint32_t virtual, uint32_t physical, size_t length, uint32_t tlblo_flags)
{
    uint32_t tlbhi = TLBHI_V;  // set valid
    uint32_t tlblo = tlblo_flags;
    uint32_t mask = 0;

    switch (length) {
        case 1 * 1024:
            tlbhi |= TLBHI_SIZE_1KB;
            mask = TLB_ADDR_MASK_1KB;
            break;
        case 4 * 1024:
            tlbhi |= TLBHI_SIZE_4KB;
            mask = TLB_ADDR_MASK_4KB;
            break;
        case 16 * 1024:
            tlbhi |= TLBHI_SIZE_16KB;
            mask = TLB_ADDR_MASK_16KB;
            break;
        case 64 * 1024:
            tlbhi |= TLBHI_SIZE_64KB;
            mask = TLB_ADDR_MASK_64KB;
            break;
        case 256 * 1024:
            tlbhi |= TLBHI_SIZE_256KB;
            mask = TLB_ADDR_MASK_256KB;
            break;
        case 1 * 1024 * 1024:
            tlbhi |= TLBHI_SIZE_1MB;
            mask = TLB_ADDR_MASK_1MB;
            break;
        case 4 * 1024 * 1024:
            tlbhi |= TLBHI_SIZE_4MB;
            mask = TLB_ADDR_MASK_4MB;
            break;
        case 16 * 1024 * 1024:
            tlbhi |= TLBHI_SIZE_16MB;
            mask = TLB_ADDR_MASK_16MB;
            break;
        default:
            xil_printf("set_tlb: Invalid size %u requested!\n", length);
            for (;;)
                ;
    }

    // Set RPN to physical address
    tlblo &= ~TLBLO_RPN_MASK;
    tlblo |= physical & mask;
    if ((physical & mask) != physical) {
        xil_printf("set_tlb: Physical address not correctly aligned!\n");
        xil_printf("  0x%08x & 0x%08x != 0x%08x\n", physical, mask, physical & mask);
        for (;;)
            ;
    }

    // Set TAG to virtual address
    tlbhi &= ~TLBHI_TAG_MASK;
    tlbhi |= virtual & mask;
    if ((virtual & mask) != virtual) {
        xil_printf("set_tlb: Virtual address not correctly aligned!\n");
        xil_printf("  0x%08x & 0x%08x != 0x%08x\n", virtual, mask, virtual & mask);
        for (;;)
            ;
    }

    // xil_printf("set_tlb_entry(0x%x, 0x%08x 0x%08x\n", index, tlbhi, tlblo);
    set_tlb_entry(index, tlbhi, tlblo);
}

#define enable_vm()         \
    asm volatile(           \
        "msrset r0, %0  \n" \
        "nop            \n" \
        "bri    4       \n" \
        :                   \
        : "i"(MSR_VM))

struct APU_LDR {
    const uint64_t signature;
    const uint32_t ddr_start;
    const uint32_t ddr_length;
} __attribute__((aligned(8), packed)) loader_info = {
    .signature = 0x10adcba987654321ULL,
};

#define writei(addr, value) asm volatile("swi %1, %0, 0 \n" : : "r"(addr), "r"(value))

#define readi(addr)                                                \
    ({                                                             \
        uint32_t value;                                            \
        asm volatile("lwi %0, %1, 0\n" : "=r"(value) : "r"(addr)); \
        value;                                                     \
    })

int main()
{
    microblaze_enable_exceptions();

    xil_printf("Hello, World from physical land!\n");
    xil_printf("Loader DDR start: %08x\n", loader_info.ddr_start);

    // Set PID to 0 (will be used as TID when writing TLB entries)
    set_pid(0);

    // Clear all TLBs, especially the V flag.
    for (int i = 0; i < MICROBLAZE_TLB_ENTRIES; i++)
        set_tlb_entry(i, 0, 0);

    int tlb_i = 0;

    // Null pointer protection
    set_tlb(tlb_i++, 0, 0, 16 * 1024 * 1024, TLBLO_G);

    // Peripherals
    set_tlb(tlb_i++, 0x40000000, 0x40000000, 16 * 1024 * 1024, TLBLO_I | TLBLO_WR);
    set_tlb(tlb_i++, 0x41000000, 0x41000000, 16 * 1024 * 1024, TLBLO_I | TLBLO_WR);
    set_tlb(tlb_i++, 0x42000000, 0x42000000, 16 * 1024 * 1024, TLBLO_I | TLBLO_WR);
    set_tlb(tlb_i++, 0x43000000, 0x43000000, 16 * 1024 * 1024, TLBLO_I | TLBLO_WR);

    // SRAM
    set_tlb(tlb_i++, XPAR_BRAM_0_BASEADDR, XPAR_BRAM_0_BASEADDR, 64 * 1024, TLBLO_WR | TLBLO_EX);

    // DDR
    set_tlb(tlb_i++, loader_info.ddr_start, loader_info.ddr_start, 4 * 1024 * 1024, TLBLO_WR | TLBLO_EX);

    enable_vm();

    xil_printf("Hello, World from virtual land!\n");

    // trigger unaligned pointer exception
    //  EAR is 0xBEEF
    //  ESR is 0xC81 (EC=0x01, ESS=0x64, DS=1)
    //  R17 points to the instruction after the offending store
    //*(volatile uint32_t*)(0xbeef) = 0xbadf00d;
    // readi(0xbeef);

    // trigger null pointer protection
    //  EAR is 0x8   (read address 0x8)
    //  ESR is 0x490 (EC=0x10, ESS=0x24, DS=0)
    //  R17 points to the offending store instruction
    //*(volatile uint32_t*)(0x8) = 0xbadf00d;
    writei(0x8, 0xbadf00d);

    // trigger null pointer protection
    //  EAR is 0x10  (write address 0x10)
    //  ESR is 0x864 (EC = 0x04, ESS = 0x43, DS = 1)
    //  R17 points to the instruction after the offending load
    //*(volatile uint32_t*)(0x10);
    // readi(0x10);

    for (;;) {
    };
}
