
#include <xil_printf.h>

extern void enable_mmu(void);

int main()
{
	xil_printf("Hello, World from physical land!\n");
	enable_mmu();
	xil_printf("Hello, World from virtual land!\n");
	for(;;){};
}

