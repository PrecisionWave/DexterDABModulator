#include <stdio.h>
#include "fsl.h"
#include "xparameters.h"
#include "xil_cache.h"
#include "xil_printf.h"
#include "sleep.h"


int main()
{
    Xil_ICacheInvalidate();
    Xil_DCacheInvalidate();

    Xil_ICacheEnable();
    Xil_DCacheEnable();

    xil_printf("APU Stream test application\n");
    xil_printf("Built: " __DATE__ " " __TIME__ "\n");

    unsigned counter = 16;
    uint32_t value;

    for (;;) {
        getfslx(value, 0, FSL_DEFAULT);
        putfslx(~value, 0, FSL_DEFAULT);
        counter--;
        if (counter == 0) {
            counter = 16;
            xil_printf("XFER 16\n");
            fflush(stdout);
        }
    }

    Xil_DCacheDisable();
    Xil_ICacheDisable();

    return 0;
}
