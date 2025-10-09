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
#include <stdint.h>
#include "xil_cache.h"
#include "xil_printf.h"

struct APU_LDR {
	const uint64_t signature;
	const uint32_t ddr_start;
	const uint32_t ddr_length;
}__attribute__((aligned(8),packed)) loader_info = {
		.signature =
		0x10adcba987654321ULL,
};

extern uint8_t* __ddr_start;
extern uint8_t* __ddr_end;

int main() {
	Xil_DCacheDisable();
	Xil_ICacheDisable();

	Xil_DCacheInvalidate()
	;
	Xil_ICacheInvalidate()
	;

	xil_printf("APU loader info\n");
	xil_printf("DDR from 0x%08x to %08x...\n", loader_info.ddr_start,
			loader_info.ddr_start + loader_info.ddr_length - 1);
	xil_printf("\n");
	xil_printf("Linker info\n");
	xil_printf("DDR from 0x%08x to %08x...\n", __ddr_start, __ddr_end);
	xil_printf("\n");
	xil_printf("Clearing Memory...\n");

	uint8_t* p = __ddr_start;
	uint8_t* p_end = __ddr_end;

	if (loader_info.signature == 0x10ad123456789abcULL) {
		p = (uint8_t*) (loader_info.ddr_start);
		p_end = (uint8_t*) (loader_info.ddr_start + loader_info.ddr_length - 1);
	}

	uint8_t* p_start = p;
	do {
		*(volatile uint8_t*) (p++) = 0;
	} while (p != p_end);

	xil_printf("Cleared %ld bytes of memory\n", p_end - p_start);

	return 0;
}
