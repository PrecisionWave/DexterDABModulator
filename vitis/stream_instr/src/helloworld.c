/******************************************************************************
 *
 * Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
 ******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>

#include "xparameters.h"
#include "xil_printf.h"
#include "xil_cache.h"
#include "fsl.h"
#include "sleep.h"

#define fsl_clearerror()             asm volatile ("msrclr\tr0,%0\n"  \
                                                   "nop;" : : "i" (0x10))

int main() {

	Xil_ICacheInvalidate();
	Xil_DCacheInvalidate();

	Xil_ICacheEnable();
	Xil_DCacheEnable();

	printf("Stream instructions test application\n");
	printf("Built: " __DATE__ " " __TIME__ "\n");

	{
		uint32_t error;
		uint32_t value;
		uint32_t id = 0;
		fsl_clearerror();
		getdfslx(value, id, FSL_DEFAULT);
		fsl_iserror(error);
		printf("FSL_DEFAULT: value: 0x%08lx, error: %lu\n", value, error);
	}

	{
		uint32_t invalid;
		uint32_t value;
		uint32_t error;
		uint32_t id = 0;

		fsl_clearerror();

		for (;;) {
			// Peek if data is available
			getdfslx(value, id, FSL_NONBLOCKING);
			fsl_isinvalid(invalid);
			fsl_iserror(error);
			printf("Pump Demo: invalid: %lu, error: %lu, value: 0x%08lx\n",
					invalid, error, value);
			if (!invalid) {
				// Get data here will not block as we know that data is available
				getdfslx(value, id, FSL_DEFAULT);

				// Put data might block
				if (error) putdfslx(~value, id, FSL_CONTROL);
				else putdfslx(~value, id, FSL_DEFAULT);
			} else {
				usleep(500 * 1000);
			}

			// clear MSR[FSL] in case of error
			if (error) fsl_clearerror();
		}
	}

	{
		uint32_t invalid;
		uint32_t error;
		uint32_t value;
		uint32_t id = 0;
		fsl_clearerror();
		for (;;) {
			getdfslx(value, id, FSL_NONBLOCKING);
			fsl_isinvalid(invalid);
			fsl_iserror(error);
			printf("Error demo: invalid: %lu, error: %lu, value: 0x%08lx\n",
					invalid, error, value);
	        if (error) fsl_clearerror();
			if (invalid) usleep(500 * 1000);
		}
	}

	return 0;
}
