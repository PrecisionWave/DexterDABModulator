/*
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A 
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR 
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION 
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE 
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO 
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO 
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE 
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY 
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * 
 *
 * This file is a generated sample test application.
 *
 * This application is intended to test and/or illustrate some 
 * functionality of your system.  The contents of this file may
 * vary depending on the IP in your system and may use existing
 * IP driver functions.  These drivers will be generated in your
 * SDK application project when you run the "Generate Libraries" menu item.
 *
 */

#include <stdio.h>
#include "xparameters.h"
#include "xil_cache.h"
#include "xintc.h"
#include "intc_header.h"
#include "xgpio.h"
#include "gpio_header.h"
#include "xtmrctr.h"
#include "tmrctr_header.h"
#include "tmrctr_intr_header.h"
#include "mbox_header.h"
int main () 
{
   static XIntc intc;
   static XTmrCtr accel_mb_axi_timer_Timer;
   Xil_DCacheInvalidate();
   Xil_ICacheInvalidate();
   Xil_ICacheEnable();
   Xil_DCacheEnable();
   print("---Entering main---\n\r");


   {
      int status;

      print("\r\n Running IntcSelfTestExample() for accel_mb_axi_intc...\r\n");

      status = IntcSelfTestExample(XPAR_ACCEL_MB_AXI_INTC_DEVICE_ID);

      if (status == 0) {
         print("IntcSelfTestExample PASSED\r\n");
      }
      else {
         print("IntcSelfTestExample FAILED\r\n");
      }
   }

   {
       int Status;

       Status = IntcInterruptSetup(&intc, XPAR_ACCEL_MB_AXI_INTC_DEVICE_ID);
       if (Status == 0) {
          print("Intc Interrupt Setup PASSED\r\n");
       }
       else {
         print("Intc Interrupt Setup FAILED\r\n");
      }
   }


   volatile uint32_t* pTscLo = (volatile uint32_t*)(0x40000000);
   volatile uint32_t* pTscHi = (volatile uint32_t*)(0x40000008);

    {
        xil_printf("\r\nTimestamp counter test #3\r\n");

        uint32_t tsc_vals[16];
        asm volatile(
            "lwi r5, %0, 0\n"
            "lwi r6, %0, 0\n"
            "swi r5, %1, 0\n"
            "swi r6, %1, 4\n"
            "lwi r5, %0, 0\n"
            "lwi r6, %0, 0\n"
            "swi r5, %1, 8\n"
            "swi r6, %1, 12\n"
            "lwi r5, %0, 0\n"
            "lwi r6, %0, 0\n"
            "swi r5, %1, 16\n"
            "swi r6, %1, 20\n"
            "lwi r5, %0, 0\n"
            "lwi r6, %0, 0\n"
            "swi r5, %1, 24\n"
            "swi r6, %1, 28\n"
            "lwi r5, %0, 0\n"
            "lwi r6, %0, 0\n"
            "swi r5, %1, 32\n"
            "swi r6, %1, 36\n"
            "lwi r5, %0, 0\n"
            "lwi r6, %0, 0\n"
            "swi r5, %1, 40\n"
            "swi r6, %1, 44\n"
            "lwi r5, %0, 0\n"
            "lwi r6, %0, 0\n"
            "swi r5, %1, 48\n"
            "swi r6, %1, 52\n"
            "lwi r5, %0, 0\n"
            "lwi r6, %0, 0\n"
            "swi r5, %1, 56\n"
            "swi r6, %1, 60\n"
            : : "r"(0x40000000), "r"(&tsc_vals[0])
            );

        for(int i=0; i<14; i++) {
            xil_printf("Timestamp counter: 0x%08x - 0x%08x = %d\r\n", tsc_vals[i+1], tsc_vals[i], tsc_vals[i+1] - tsc_vals[i]);
        }
    }

    {
         xil_printf("\r\nTimestamp counter test #2\r\n");

         uint32_t tsc_vals[16];

         for(int i=0; i<15; i++) {
             tsc_vals[i] = *pTscLo;
         }

         for(int i=0; i<14; i++) {
             xil_printf("Timestamp counter: 0x%08x - 0x%08x = %d\r\n", tsc_vals[i+1], tsc_vals[i], tsc_vals[i+1] - tsc_vals[i]);
         }
     }

    for(int i=0; i<15; i++) {
         xil_printf("Timestamp counter: 0x%04x%08x\r\n", *pTscHi , *pTscLo);
    }

    {
      u32 status;
      u32 DataRead;
      
      print("\r\nRunning GpioInputExample() for accel_mb_axi_gpio_tsc...\r\n");

      
      for(int i=0; i<15; i++) {
		  status = GpioInputExample(XPAR_ACCEL_MB_AXI_GPIO_TSC_DEVICE_ID, &DataRead);

		  if (status == 0) {
			 xil_printf("GpioInputExample PASSED. Read data:0x%X\r\n", DataRead);
		  }
		  else {
			 print("GpioInputExample FAILED.\r\n");
		  }
      }
    }

   {
      int status;
      
      print("\r\n Running TmrCtrSelfTestExample() for accel_mb_axi_timer...\r\n");
      
      status = TmrCtrSelfTestExample(XPAR_ACCEL_MB_AXI_TIMER_DEVICE_ID, 0x0);
      
      if (status == 0) {
         print("TmrCtrSelfTestExample PASSED\r\n");
      }
      else {
         print("TmrCtrSelfTestExample FAILED\r\n");
      }
   }

   for(int i=0; i<15; i++) {
	   {
		  int Status;

		  print("\r\n Running Interrupt Test  for accel_mb_axi_timer...\r\n");

		  Status = TmrCtrIntrExample(&intc, &accel_mb_axi_timer_Timer, \
									 XPAR_ACCEL_MB_AXI_TIMER_DEVICE_ID, \
									 XPAR_ACCEL_MB_AXI_INTC_ACCEL_MB_AXI_TIMER_INTERRUPT_INTR, 0);

		  if (Status == 0) {
			 print("Timer Interrupt Test PASSED\r\n");
		  }
		  else {
			 print("Timer Interrupt Test FAILED\r\n");
		  }
	
	   }

	   xil_printf("Timestamp counter: 0x%04x%08x\r\n", *pTscHi , *pTscLo);
   }


   print("---Exiting main---\n\r");
   for(;;);
   return 0;
}
