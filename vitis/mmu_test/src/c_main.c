
#include <xil_printf.h>
#include <xil_exception.h>
#include <mb_interface.h>

extern void enable_mmu(void);

void ExceptionHandler_FSL(void* data)
{
    print("Exception: FSL!\n");
    for (;;)
        ;
}

void ExceptionHandler_Unaligned(void* data)
{
    print("Exception: Unaligned access!\n");
    for (;;)
        ;
}

void ExceptionHandler_IllOp(void* data)
{
    print("Exception: \n");
        for (;;)
        ;
}

void ExceptionHandler_I_Axi(void* data)
{
    print("Exception: M_AXI_I bus error\n");
        for (;;)
        ;
}

void ExceptionHandler_I_PLB(void* data)
{
    // PLB: Processor Local Bus
    print("Exception: I_PLB BUS errorn");
        for (;;)
        ;
}

void ExceptionHandler_D_Axi(void* data)
{
    print("Exception: M_AXI_D bus error\n");
        for (;;)
        ;
}

void ExceptionHandler_D_PLB(void* data)
{
    // PLB: Processor Local Bus
    print("Exception: D_PLB\n");
        for (;;)
        ;
}

void ExceptionHandler_Div0(void* data)
{
    print("Exception: Division through 0\n");
        for (;;)
        ;
}

void ExceptionHandler_FPU(void* data)
{
    print("Exception: FPU\n");
        for (;;)
        ;
}

void ExceptionHandler_Stack(void* data)
{
    print("Exception: Stack Violation!\n");
        for (;;)
        ;
}

void ExceptionHandler_MMU(void* data)
{
	uint32_t ear = mfear();
	uint32_t esr = mfesr();
    print("Exception: MMU!\n");
    xil_printf("  EAR: %08x\n", ear);
    xil_printf("  ESR: %08x\n", esr);
    for (;;)
        ;
}

int main()
{
    Xil_ExceptionInit();
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_FSL, ExceptionHandler_FSL, NULL);
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_UNALIGNED_ACCESS, ExceptionHandler_Unaligned, NULL);
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_ILLEGAL_OPCODE, ExceptionHandler_IllOp, NULL);
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_M_AXI_I_EXCEPTION, ExceptionHandler_I_Axi, NULL);
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IPLB_EXCEPTION, ExceptionHandler_I_PLB, NULL);
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_M_AXI_D_EXCEPTION, ExceptionHandler_D_Axi, NULL);
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_DPLB_EXCEPTION, ExceptionHandler_D_PLB, NULL);
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_DIV_BY_ZERO, ExceptionHandler_Div0, NULL);
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_FPU, ExceptionHandler_FPU, NULL);
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_STACK_VIOLATION, ExceptionHandler_Stack, NULL);
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_MMU, ExceptionHandler_MMU, NULL);
    Xil_ExceptionEnable();

    xil_printf("Hello, World from physical land!\n");

    enable_mmu();

    xil_printf("Hello, World from virtual land!\n");

    // trigger null pointer exception
    *(volatile uint32_t*)(0) = 0xbadf00d;

    for (;;) {
    };
}
