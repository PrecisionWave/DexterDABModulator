#include <stdint.h>
#include <stddef.h>

#include "xuartlite_l.h"

volatile uint8_t* uartlite = (volatile uint8_t*)0x40600000;

void putchar(char c)
{
    XUartLite_SendByte(uartlite, c);
    return;
}


void print(const char* str)
{
    while (*str != '\0') {
        putchar(*str);
        str++;
    }
    return;
}

void Xil_Out32(volatile uint8_t* p, uint32_t v)
{
    *(volatile uint32_t*)(p) = v;
}

uint32_t Xil_In32(volatile uint8_t* p)
{
    return *(volatile uint32_t*)(p);
}

void main(void)
{
    /* Write to the control register to disable the interrupts, don't
     * reset the FIFOs are the user may want the data that's present
     */
    XUartLite_WriteReg(uartlite, XUL_CONTROL_REG_OFFSET, 0);

    print("Hello world!\r\n");

    for(;;);
}
