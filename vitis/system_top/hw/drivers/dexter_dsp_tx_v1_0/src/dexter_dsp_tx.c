#include "dexter_dsp_tx.h"
#ifndef __linux__
int dexter_dsp_tx_CfgInitialize(dexter_dsp_tx *InstancePtr, dexter_dsp_tx_Config *ConfigPtr) {
    Xil_AssertNonvoid(InstancePtr != NULL);
    Xil_AssertNonvoid(ConfigPtr != NULL);

    InstancePtr->dexter_dsp_tx_BaseAddress = ConfigPtr->dexter_dsp_tx_BaseAddress;

    InstancePtr->IsReady = 1;
    return XST_SUCCESS;
}
#endif
void dexter_dsp_tx_stream_start1_msb_write(dexter_dsp_tx *InstancePtr, u32 Data) {

    Xil_AssertVoid(InstancePtr != NULL);

    dexter_dsp_tx_WriteReg(InstancePtr->dexter_dsp_tx_BaseAddress, 144, Data);
}
u32 dexter_dsp_tx_stream_start1_msb_read(dexter_dsp_tx *InstancePtr) {

    u32 Data;
    Xil_AssertVoid(InstancePtr != NULL);

    Data = dexter_dsp_tx_ReadReg(InstancePtr->dexter_dsp_tx_BaseAddress, 144);
    return Data;
}
void dexter_dsp_tx_stream_start1_lsb_write(dexter_dsp_tx *InstancePtr, u32 Data) {

    Xil_AssertVoid(InstancePtr != NULL);

    dexter_dsp_tx_WriteReg(InstancePtr->dexter_dsp_tx_BaseAddress, 140, Data);
}
u32 dexter_dsp_tx_stream_start1_lsb_read(dexter_dsp_tx *InstancePtr) {

    u32 Data;
    Xil_AssertVoid(InstancePtr != NULL);

    Data = dexter_dsp_tx_ReadReg(InstancePtr->dexter_dsp_tx_BaseAddress, 140);
    return Data;
}
void dexter_dsp_tx_rx_inc1_write(dexter_dsp_tx *InstancePtr, u32 Data) {

    Xil_AssertVoid(InstancePtr != NULL);

    dexter_dsp_tx_WriteReg(InstancePtr->dexter_dsp_tx_BaseAddress, 128, Data);
}
u32 dexter_dsp_tx_rx_inc1_read(dexter_dsp_tx *InstancePtr) {

    u32 Data;
    Xil_AssertVoid(InstancePtr != NULL);

    Data = dexter_dsp_tx_ReadReg(InstancePtr->dexter_dsp_tx_BaseAddress, 128);
    return Data;
}
void dexter_dsp_tx_gain_dc1_write(dexter_dsp_tx *InstancePtr, u32 Data) {

    Xil_AssertVoid(InstancePtr != NULL);

    dexter_dsp_tx_WriteReg(InstancePtr->dexter_dsp_tx_BaseAddress, 132, Data);
}
u32 dexter_dsp_tx_gain_dc1_read(dexter_dsp_tx *InstancePtr) {

    u32 Data;
    Xil_AssertVoid(InstancePtr != NULL);

    Data = dexter_dsp_tx_ReadReg(InstancePtr->dexter_dsp_tx_BaseAddress, 132);
    return Data;
}
void dexter_dsp_tx_rx_inc0_write(dexter_dsp_tx *InstancePtr, u32 Data) {

    Xil_AssertVoid(InstancePtr != NULL);

    dexter_dsp_tx_WriteReg(InstancePtr->dexter_dsp_tx_BaseAddress, 64, Data);
}
u32 dexter_dsp_tx_rx_inc0_read(dexter_dsp_tx *InstancePtr) {

    u32 Data;
    Xil_AssertVoid(InstancePtr != NULL);

    Data = dexter_dsp_tx_ReadReg(InstancePtr->dexter_dsp_tx_BaseAddress, 64);
    return Data;
}
void dexter_dsp_tx_pps_settings_write(dexter_dsp_tx *InstancePtr, u32 Data) {

    Xil_AssertVoid(InstancePtr != NULL);

    dexter_dsp_tx_WriteReg(InstancePtr->dexter_dsp_tx_BaseAddress, 4, Data);
}
u32 dexter_dsp_tx_pps_settings_read(dexter_dsp_tx *InstancePtr) {

    u32 Data;
    Xil_AssertVoid(InstancePtr != NULL);

    Data = dexter_dsp_tx_ReadReg(InstancePtr->dexter_dsp_tx_BaseAddress, 4);
    return Data;
}
void dexter_dsp_tx_pps_delay_write(dexter_dsp_tx *InstancePtr, u32 Data) {

    Xil_AssertVoid(InstancePtr != NULL);

    dexter_dsp_tx_WriteReg(InstancePtr->dexter_dsp_tx_BaseAddress, 16, Data);
}
u32 dexter_dsp_tx_pps_delay_read(dexter_dsp_tx *InstancePtr) {

    u32 Data;
    Xil_AssertVoid(InstancePtr != NULL);

    Data = dexter_dsp_tx_ReadReg(InstancePtr->dexter_dsp_tx_BaseAddress, 16);
    return Data;
}
void dexter_dsp_tx_stream_start0_lsb_write(dexter_dsp_tx *InstancePtr, u32 Data) {

    Xil_AssertVoid(InstancePtr != NULL);

    dexter_dsp_tx_WriteReg(InstancePtr->dexter_dsp_tx_BaseAddress, 76, Data);
}
u32 dexter_dsp_tx_stream_start0_lsb_read(dexter_dsp_tx *InstancePtr) {

    u32 Data;
    Xil_AssertVoid(InstancePtr != NULL);

    Data = dexter_dsp_tx_ReadReg(InstancePtr->dexter_dsp_tx_BaseAddress, 76);
    return Data;
}
void dexter_dsp_tx_stream_start0_msb_write(dexter_dsp_tx *InstancePtr, u32 Data) {

    Xil_AssertVoid(InstancePtr != NULL);

    dexter_dsp_tx_WriteReg(InstancePtr->dexter_dsp_tx_BaseAddress, 80, Data);
}
u32 dexter_dsp_tx_stream_start0_msb_read(dexter_dsp_tx *InstancePtr) {

    u32 Data;
    Xil_AssertVoid(InstancePtr != NULL);

    Data = dexter_dsp_tx_ReadReg(InstancePtr->dexter_dsp_tx_BaseAddress, 80);
    return Data;
}
void dexter_dsp_tx_gain_dc0_write(dexter_dsp_tx *InstancePtr, u32 Data) {

    Xil_AssertVoid(InstancePtr != NULL);

    dexter_dsp_tx_WriteReg(InstancePtr->dexter_dsp_tx_BaseAddress, 68, Data);
}
u32 dexter_dsp_tx_gain_dc0_read(dexter_dsp_tx *InstancePtr) {

    u32 Data;
    Xil_AssertVoid(InstancePtr != NULL);

    Data = dexter_dsp_tx_ReadReg(InstancePtr->dexter_dsp_tx_BaseAddress, 68);
    return Data;
}
u32 dexter_dsp_tx_buff_ovflws0_read(dexter_dsp_tx *InstancePtr) {

    u32 Data;
    Xil_AssertVoid(InstancePtr != NULL);

    Data = dexter_dsp_tx_ReadReg(InstancePtr->dexter_dsp_tx_BaseAddress, 72);
    return Data;
}
u32 dexter_dsp_tx_version_read(dexter_dsp_tx *InstancePtr) {

    u32 Data;
    Xil_AssertVoid(InstancePtr != NULL);

    Data = dexter_dsp_tx_ReadReg(InstancePtr->dexter_dsp_tx_BaseAddress, 0);
    return Data;
}
u32 dexter_dsp_tx_pps_error_read(dexter_dsp_tx *InstancePtr) {

    u32 Data;
    Xil_AssertVoid(InstancePtr != NULL);

    Data = dexter_dsp_tx_ReadReg(InstancePtr->dexter_dsp_tx_BaseAddress, 8);
    return Data;
}
u32 dexter_dsp_tx_pps_cnt_read(dexter_dsp_tx *InstancePtr) {

    u32 Data;
    Xil_AssertVoid(InstancePtr != NULL);

    Data = dexter_dsp_tx_ReadReg(InstancePtr->dexter_dsp_tx_BaseAddress, 12);
    return Data;
}
u32 dexter_dsp_tx_pps_clks_lsb_read(dexter_dsp_tx *InstancePtr) {

    u32 Data;
    Xil_AssertVoid(InstancePtr != NULL);

    Data = dexter_dsp_tx_ReadReg(InstancePtr->dexter_dsp_tx_BaseAddress, 20);
    return Data;
}
u32 dexter_dsp_tx_pps_clks_msb_read(dexter_dsp_tx *InstancePtr) {

    u32 Data;
    Xil_AssertVoid(InstancePtr != NULL);

    Data = dexter_dsp_tx_ReadReg(InstancePtr->dexter_dsp_tx_BaseAddress, 24);
    return Data;
}
u32 dexter_dsp_tx_buff_ovflws1_read(dexter_dsp_tx *InstancePtr) {

    u32 Data;
    Xil_AssertVoid(InstancePtr != NULL);

    Data = dexter_dsp_tx_ReadReg(InstancePtr->dexter_dsp_tx_BaseAddress, 136);
    return Data;
}
