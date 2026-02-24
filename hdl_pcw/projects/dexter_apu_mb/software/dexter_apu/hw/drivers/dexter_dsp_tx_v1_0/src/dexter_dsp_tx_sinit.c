/**
* @file dexter_dsp_tx_sinit.c
*
* The implementation of the dexter_dsp_tx driver's static initialzation
* functionality.
*
* @note
*
* None
*
*/
#ifndef __linux__
#include "xstatus.h"
#include "xparameters.h"
#include "dexter_dsp_tx.h"
extern dexter_dsp_tx_Config dexter_dsp_tx_ConfigTable[];
/**
* Lookup the device configuration based on the unique device ID.  The table
* ConfigTable contains the configuration info for each device in the system.
*
* @param DeviceId is the device identifier to lookup.
*
* @return
*     - A pointer of data type dexter_dsp_tx_Config which
*    points to the device configuration if DeviceID is found.
*    - NULL if DeviceID is not found.
*
* @note    None.
*
*/
dexter_dsp_tx_Config *dexter_dsp_tx_LookupConfig(u16 DeviceId) {
    dexter_dsp_tx_Config *ConfigPtr = NULL;
    int Index;
    for (Index = 0; Index < XPAR_DEXTER_DSP_TX_NUM_INSTANCES; Index++) {
        if (dexter_dsp_tx_ConfigTable[Index].DeviceId == DeviceId) {
            ConfigPtr = &dexter_dsp_tx_ConfigTable[Index];
            break;
        }
    }
    return ConfigPtr;
}
int dexter_dsp_tx_Initialize(dexter_dsp_tx *InstancePtr, u16 DeviceId) {
    dexter_dsp_tx_Config *ConfigPtr;
    Xil_AssertNonvoid(InstancePtr != NULL);
    ConfigPtr = dexter_dsp_tx_LookupConfig(DeviceId);
    if (ConfigPtr == NULL) {
        InstancePtr->IsReady = 0;
        return (XST_DEVICE_NOT_FOUND);
    }
    return dexter_dsp_tx_CfgInitialize(InstancePtr, ConfigPtr);
}
#endif
