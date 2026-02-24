/******************************************************************************
 * Copyright (C) 2002 - 2021 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/****************************************************************************/
/**
*
* @file xuartlite_l.c
* @addtogroup uartlite_v3_6
* @{
*
* This file contains low-level driver functions that can be used to access the
* device.  The user should refer to the hardware device specification for more
* details of the device operation.

* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00b rpm  04/25/02 First release
* 1.12a rpm  07/16/07 Fixed arg type for RecvByte
* 2.00a ktn  10/20/09 The macros have been renamed to remove _m from the name.
* 3.2   sk   11/10/15 Used UINTPTR instead of u32 for Baseaddress CR# 867425.
*                     Changed the prototypes of XUartLite_SendByte,
*                     XUartLite_RecvByte APIs.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xuartlite_l.h"

/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Prototypes ******************************/


/****************************************************************************/
/**
 *
 * This functions sends a single byte using the UART. It is blocking in that it
 * waits for the transmitter to become non-full before it writes the byte to
 * the transmit register.
 *
 * @param	BaseAddress is the base address of the device
 * @param	Data is the byte of data to send
 *
 * @return	None.
 *
 * @note		None.
 *
 ******************************************************************************/
void XUartLite_SendByte(volatile uint8_t* BaseAddress, uint8_t Data)
{
    while (XUartLite_IsTransmitFull(BaseAddress))
        ;

    XUartLite_WriteReg(BaseAddress, XUL_TX_FIFO_OFFSET, Data);
}


/****************************************************************************/
/**
 *
 * This functions receives a single byte using the UART. It is blocking in that
 * it waits for the receiver to become non-empty before it reads from the
 * receive register.
 *
 * @param	BaseAddress is the base address of the device
 *
 * @return	The byte of data received.
 *
 * @note		None.
 *
 ******************************************************************************/
uint8_t XUartLite_RecvByte(volatile uint8_t* BaseAddress)
{
    while (XUartLite_IsReceiveEmpty(BaseAddress))
        ;

    return (uint8_t)XUartLite_ReadReg(BaseAddress, XUL_RX_FIFO_OFFSET);
}


void Xil_Out32(volatile uint8_t* p, uint32_t v)
{
    *(volatile uint32_t*)(p) = v;
}

uint32_t Xil_In32(volatile uint8_t* p)
{
    return *(volatile uint32_t*)(p);
}

volatile uint8_t* uartlite = (volatile uint8_t*)0x40600000;

int _write(int _handle, char* data, int size)
{
    int count;

    for (count = 0; count < size; count++) {
        XUartLite_SendByte(uartlite, data[count]);  // Your low-level output function here.
    }

    return count;
}

__attribute__((constructor))
static void init_uartlite()
{
        /* Write to the control register to disable the interrupts, don't
     * reset the FIFOs are the user may want the data that's present
     */
    XUartLite_WriteReg(uartlite, XUL_CONTROL_REG_OFFSET, 0);

    _write(0, "init_uartlite\n", 14);
}

/** @} */
