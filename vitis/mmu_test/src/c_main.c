
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
    // *(volatile uint32_t*)(0xbeef) = 0xbadf00d;

    // trigger null pointer protection
    *(volatile uint32_t*)(0x8) = 0xbadf00d;

    for (;;) {
    };
}
