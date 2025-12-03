#include <stdio.h>
#include "xparameters.h"
#include "xil_cache.h"
#include "xmbox.h"

#define TIMEOUT_MAX_COUNT 0xF0000000 /* Max count to wait for the message */

static int MailboxSendDataResponse(XMbox* MboxInstancePtr, char command, uintptr_t addr, uint32_t len);
static int MailboxReceiveCommand(XMbox* MboxInstancePtr);

static XMbox Mbox;

static uint32_t received_command[4] __attribute__((aligned(4)));

#define DATA_APU2ARM_LEN 4
static uint32_t volatile data_apu2arm[1];

#define DATA_ARM2APU_LEN 64
static uint8_t volatile data_arm2apu[DATA_ARM2APU_LEN];

// A simple hash function from K&R C
static uint32_t kr_hash(const volatile uint8_t *buf, uint32_t buflen) {
    uint32_t h = 0;
    while (buflen-- > 0) {
        h += *(buf++) + 31 * h;
    }
    return h;
}

int main()
{
    Xil_ICacheInvalidate();
    Xil_DCacheInvalidate();

    Xil_ICacheEnable();
    Xil_DCacheEnable();

    printf("Entering main, data is at %p\n", data_arm2apu);

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
        Status = MailboxReceiveCommand(&Mbox);
        if (Status != XST_SUCCESS) {
            print("Mailbox Receive FAILED\n");
        }
        else {
            const char command = received_command[0] & 0xFF;
            //const uint8_t *buf;
            //buf = (uint8_t*)received_command[1];
            //const uint32_t len = received_command[2];
            const char period = received_command[3] & 0xFF;

            printf("Command: %c 0x%08lx 0x%08lx %c\n",
                    command, received_command[1], received_command[2], period);

            if (command == 'H' && period == '.') {
                Xil_DCacheInvalidateRange((UINTPTR)data_arm2apu, DATA_ARM2APU_LEN);
                const uint32_t hash = kr_hash(data_arm2apu, DATA_ARM2APU_LEN);

                printf("Hash at %p = %ld\n", data_apu2arm, hash);

                data_apu2arm[0] = hash;
                Xil_DCacheFlushRange((UINTPTR)data_apu2arm, DATA_APU2ARM_LEN);
                MailboxSendDataResponse(&Mbox, 'o', (uintptr_t)data_apu2arm, DATA_APU2ARM_LEN);
            }
            else if (command == 'a' && period == '.') {
                MailboxSendDataResponse(&Mbox, 'a', (uintptr_t)data_arm2apu, DATA_ARM2APU_LEN);
            }
            else {
                printf("Unknown command\n");
            }
        }
    }

    Xil_DCacheDisable();
    Xil_ICacheDisable();
    return 0;
}

static int MailboxSendDataResponse(XMbox* MboxInstancePtr, char command, uintptr_t addr, uint32_t len)
{
    uint32_t data[4];
    data[0] = command;
    data[1] = addr;
    data[2] = len;
    data[3] = '.';
#define DATA_SIZE (4*4)

    u32 Nbytes = 0;
    int Status = 0;
    u32 BytesSent = 0;

    while (Nbytes != DATA_SIZE) {
        Status = XMbox_Write(MboxInstancePtr, (u32*)((u8*)data + Nbytes), DATA_SIZE - Nbytes, &BytesSent);

        if (Status == XST_SUCCESS)
            Nbytes += BytesSent;
    }

    return XST_SUCCESS;
}

static int MailboxReceiveCommand(XMbox* MboxInstancePtr)
{
    int Status;
    u32 n = 0;
    u32 BytesRcvd;
    int Timeout = 0;

    while (n < 4) {
        Status = XMbox_Read(MboxInstancePtr, (u32*)(received_command + n), 4, &BytesRcvd);

        if (Status == XST_SUCCESS) {
            if (BytesRcvd == 4) { n++; }
            else return XST_FAILURE;
        }

        if (Timeout++ > TIMEOUT_MAX_COUNT)
            return XST_FAILURE;
    }

    return XST_SUCCESS;
}
