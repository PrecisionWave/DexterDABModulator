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
#include "xmbox.h"

/*
 * If the XPAR_CPU_ID != 0 for this instance, the only other option is 1
 * to make the example work. It is possible that there are more than
 * 2 processors in the system but there must always be a 0
 */
#if XPAR_CPU_ID != 0
#define MY_CPU_ID 1
#else
#define MY_CPU_ID XPAR_CPU_ID
#endif /* XPAR_CPU_ID != 0 */

#define MSGSIZ 128 /* Size of the buffer for received message */

#define HELLO_SIZE 40

#define TIMEOUT_MAX_COUNT 0xF0000000 /* Max count to wait for the message */

static int MailboxExample_Send(XMbox* MboxInstancePtr, int CPU_Id);
static int MailboxExample_Receive(XMbox* MboxInstancePtr, int CPU_Id);

static XMbox Mbox; /* Instance of the Mailbox driver */

/* Buffer for storing received the message */
static char RecvMsg[MSGSIZ] __attribute__((aligned(4)));

/* Sent Message */
static char* ProducerHello = "Hello! The Producer greets the Consumer";
static char* ConsumerHello = "Hello! The Consumer greets the Producer";


int main()
{
    Xil_ICacheInvalidate();
    Xil_DCacheInvalidate();

    Xil_ICacheEnable();
    Xil_DCacheEnable();
    print("---Entering main---\n");

    XMbox_Config* ConfigPtr;
    int Status;

    /*
     * Lookup configuration data in the device configuration table.
     * Use this configuration info down below when initializing this
     * component.
     */
    ConfigPtr = XMbox_LookupConfig(XPAR_ACCEL_SHARED_MAILBOX_TESTAPP_ID);
    if (ConfigPtr == (XMbox_Config*)NULL) {
        print("XMbox_LookupConfig FAILED\n");
    }

    print("XMbox_LookupConfig OK\n");

    /*
     * Perform the rest of the initialization.
     */
    Status = XMbox_CfgInitialize(&Mbox, ConfigPtr, ConfigPtr->BaseAddress);
    if (Status != XST_SUCCESS) {
        print("XMbox_CfgInitialize FAILED\n");
    }

    print("XMbox_CfgInitialize OK\n");

    while (1) {
        /* Send the hello */
        Status = MailboxExample_Send(&Mbox, MY_CPU_ID);
        if (Status != XST_SUCCESS) {
            print("MailboxExample_Send FAILED\n");
        }

        print("MailboxExample_Send OK\n");

        /* Receive the hello and verify the message */
        Status = MailboxExample_Receive(&Mbox, MY_CPU_ID);
        if (Status != XST_SUCCESS) {
            print("MailboxExample_Receive FAILED\n");
        }

        print("MailboxExample_Receive OK\n");
    }

    Xil_DCacheDisable();
    Xil_ICacheDisable();
    return 0;
}

/*****************************************************************************/
/**
 *
 * This function sends the hello message to the other processor.
 *
 * @param	MboxInstancePtr is the instance pointer for the XMbox.
 * @param	CPU_Id is the CPU ID for the program that is running on.
 *
 * @return	- XST_SUCCESS if the send succeeds.
 *		- XST_FAILURE if the send fails.
 *
 * @note		None.
 *
 ******************************************************************************/
static int MailboxExample_Send(XMbox* MboxInstancePtr, int CPU_Id)
{
    int Status;
    u32 Nbytes;
    u32 BytesSent;

    Nbytes = 0;

    while (Nbytes != HELLO_SIZE) {
        /* Write a message to the mbox */
        Status = XMbox_Write(MboxInstancePtr, (u32*)((u8*)ProducerHello + Nbytes), HELLO_SIZE - Nbytes, &BytesSent);

        if (Status == XST_SUCCESS)
            Nbytes += BytesSent;
    }

    return XST_SUCCESS;
}

/*****************************************************************************/
/**
 *
 * This function receives a message from the other processor and verifies that
 * it's the expected message.
 *
 * @param	MboxInstancePtr is the instance pointer for the XMbox.
 * @param	CPU_Id is the CPU ID for the program that is running on.
 *
 * @return	- XST_SUCCESS if the receive succeeds.
 *		- XST_FAILURE if the receive fails.
 *
 * @note		None.
 *
 ******************************************************************************/
static int MailboxExample_Receive(XMbox* MboxInstancePtr, int CPU_Id)
{
    int Status;
    u32 Nbytes;
    u32 BytesRcvd;
    int Timeout;

    Nbytes = 0;
    Timeout = 0;

    while (Nbytes < HELLO_SIZE) {
        /* Read a message from the mbox */
        Status = XMbox_Read(MboxInstancePtr, (u32*)(RecvMsg + Nbytes), HELLO_SIZE - Nbytes, &BytesRcvd);

        if (Status == XST_SUCCESS)
            Nbytes += BytesRcvd;

        if (Timeout++ > TIMEOUT_MAX_COUNT)
            return XST_FAILURE;
    }

    /* Compare the recieved the message is the same as we expect */
    if (memcmp(RecvMsg, ConsumerHello, HELLO_SIZE)) {
        return XST_FAILURE;
    } else {
        return XST_SUCCESS;
    }
}
