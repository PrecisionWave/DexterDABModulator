
#include <xil_printf.h>
#include <xil_exception.h>
#include <mb_interface.h>

extern void enable_mmu(void);


int main()
{
    microblaze_enable_exceptions();

    xil_printf("Hello, World from physical land!Value : %08x\n", 0x12345);

    enable_mmu();

    xil_printf("Hello, World from virtual land!\n");

    // trigger unaligned pointer exception
    //  EAR is 0xBEEF
    //  ESR is 0xC81 (EC=0x01, ESS=0x64, DS=1)
    //  R17 points to the instruction after the offending store
     //*(volatile uint32_t*)(0xbeef) = 0xbadf00d;

    // trigger null pointer protection
    //  EAR is 0x8
    //  ESR is 0x490 (EC=0x10, ESS=0x24, DS=0)
    //  R17 points to the offending store instruction
    //*(volatile uint32_t*)(0x8) = 0xbadf00d;

    // trigger null pointer protection
    //  EAR is 0x10
    //  ESR is 0x864 (EC = 0x04, ESS = 0x43, DS = 1)
    //  R17 points to the instruction after the offending load
    *(volatile uint32_t*)(0x10);

    for (;;) {
    };
}
