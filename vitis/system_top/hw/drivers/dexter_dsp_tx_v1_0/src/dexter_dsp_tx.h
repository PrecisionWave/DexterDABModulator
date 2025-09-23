#ifndef DEXTER_DSP_TX__H
#define DEXTER_DSP_TX__H
#ifdef __cplusplus
extern "C" {
#endif
/***************************** Include Files *********************************/
#ifndef __linux__
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_io.h"
#else
#include <stdint.h>
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stddef.h>
#endif
#include "dexter_dsp_tx_hw.h"
/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
#else
typedef struct {
    u16 DeviceId;
    u32 dexter_dsp_tx_BaseAddress;
} dexter_dsp_tx_Config;
#endif
/**
* The dexter_dsp_tx driver instance data. The user is required to
* allocate a variable of this type for every dexter_dsp_tx device in the system.
* A pointer to a variable of this type is then passed to the driver
* API functions.
*/
typedef struct {
    u32 dexter_dsp_tx_BaseAddress;
    u32 IsReady;
} dexter_dsp_tx;
/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define dexter_dsp_tx_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define dexter_dsp_tx_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define dexter_dsp_tx_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define dexter_dsp_tx_ReadReg(BaseAddress, RegOffset) \
    *(volatile u32*)((BaseAddress) + (RegOffset))

#define Xil_AssertVoid(expr)    assert(expr)
#define Xil_AssertNonvoid(expr) assert(expr)

#define XST_SUCCESS             0
#define XST_DEVICE_NOT_FOUND    2
#define XST_OPEN_DEVICE_FAILED  3
#define XIL_COMPONENT_IS_READY  1
#endif
/************************** Function Prototypes *****************************/
#ifndef __linux__
int dexter_dsp_tx_Initialize(dexter_dsp_tx *InstancePtr, u16 DeviceId);
dexter_dsp_tx_Config* dexter_dsp_tx_LookupConfig(u16 DeviceId);
int dexter_dsp_tx_CfgInitialize(dexter_dsp_tx *InstancePtr, dexter_dsp_tx_Config *ConfigPtr);
#else
int dexter_dsp_tx_Initialize(dexter_dsp_tx *InstancePtr, const char* InstanceName);
int dexter_dsp_tx_Release(dexter_dsp_tx *InstancePtr);
#endif
/**
* Write to stream_start1_msb gateway of dexter_dsp_tx. Assignments are LSB-justified.
*
* @param	InstancePtr is the stream_start1_msb instance to operate on.
* @param	Data is value to be written to gateway stream_start1_msb.
*
* @return	None.
*
* @note    .
*
*/
void dexter_dsp_tx_stream_start1_msb_write(dexter_dsp_tx *InstancePtr, u32 Data);
/**
* Read from stream_start1_msb gateway of dexter_dsp_tx. Assignments are LSB-justified.
*
* @param	InstancePtr is the stream_start1_msb instance to operate on.
*
* @return	u32
*
* @note    .
*
*/
u32 dexter_dsp_tx_stream_start1_msb_read(dexter_dsp_tx *InstancePtr);
/**
* Write to stream_start1_lsb gateway of dexter_dsp_tx. Assignments are LSB-justified.
*
* @param	InstancePtr is the stream_start1_lsb instance to operate on.
* @param	Data is value to be written to gateway stream_start1_lsb.
*
* @return	None.
*
* @note    .
*
*/
void dexter_dsp_tx_stream_start1_lsb_write(dexter_dsp_tx *InstancePtr, u32 Data);
/**
* Read from stream_start1_lsb gateway of dexter_dsp_tx. Assignments are LSB-justified.
*
* @param	InstancePtr is the stream_start1_lsb instance to operate on.
*
* @return	u32
*
* @note    .
*
*/
u32 dexter_dsp_tx_stream_start1_lsb_read(dexter_dsp_tx *InstancePtr);
/**
* Write to rx_inc1 gateway of dexter_dsp_tx. Assignments are LSB-justified.
*
* @param	InstancePtr is the rx_inc1 instance to operate on.
* @param	Data is value to be written to gateway rx_inc1.
*
* @return	None.
*
* @note    .
*
*/
void dexter_dsp_tx_rx_inc1_write(dexter_dsp_tx *InstancePtr, u32 Data);
/**
* Read from rx_inc1 gateway of dexter_dsp_tx. Assignments are LSB-justified.
*
* @param	InstancePtr is the rx_inc1 instance to operate on.
*
* @return	u32
*
* @note    .
*
*/
u32 dexter_dsp_tx_rx_inc1_read(dexter_dsp_tx *InstancePtr);
/**
* Write to gain_dc1 gateway of dexter_dsp_tx. Assignments are LSB-justified.
*
* @param	InstancePtr is the gain_dc1 instance to operate on.
* @param	Data is value to be written to gateway gain_dc1.
*
* @return	None.
*
* @note    .
*
*/
void dexter_dsp_tx_gain_dc1_write(dexter_dsp_tx *InstancePtr, u32 Data);
/**
* Read from gain_dc1 gateway of dexter_dsp_tx. Assignments are LSB-justified.
*
* @param	InstancePtr is the gain_dc1 instance to operate on.
*
* @return	u32
*
* @note    .
*
*/
u32 dexter_dsp_tx_gain_dc1_read(dexter_dsp_tx *InstancePtr);
/**
* Write to rx_inc0 gateway of dexter_dsp_tx. Assignments are LSB-justified.
*
* @param	InstancePtr is the rx_inc0 instance to operate on.
* @param	Data is value to be written to gateway rx_inc0.
*
* @return	None.
*
* @note    .
*
*/
void dexter_dsp_tx_rx_inc0_write(dexter_dsp_tx *InstancePtr, u32 Data);
/**
* Read from rx_inc0 gateway of dexter_dsp_tx. Assignments are LSB-justified.
*
* @param	InstancePtr is the rx_inc0 instance to operate on.
*
* @return	u32
*
* @note    .
*
*/
u32 dexter_dsp_tx_rx_inc0_read(dexter_dsp_tx *InstancePtr);
/**
* Write to pps_settings gateway of dexter_dsp_tx. Assignments are LSB-justified.
*
* @param	InstancePtr is the pps_settings instance to operate on.
* @param	Data is value to be written to gateway pps_settings.
*
* @return	None.
*
* @note    .
*
*/
void dexter_dsp_tx_pps_settings_write(dexter_dsp_tx *InstancePtr, u32 Data);
/**
* Read from pps_settings gateway of dexter_dsp_tx. Assignments are LSB-justified.
*
* @param	InstancePtr is the pps_settings instance to operate on.
*
* @return	u32
*
* @note    .
*
*/
u32 dexter_dsp_tx_pps_settings_read(dexter_dsp_tx *InstancePtr);
/**
* Write to pps_delay gateway of dexter_dsp_tx. Assignments are LSB-justified.
*
* @param	InstancePtr is the pps_delay instance to operate on.
* @param	Data is value to be written to gateway pps_delay.
*
* @return	None.
*
* @note    .
*
*/
void dexter_dsp_tx_pps_delay_write(dexter_dsp_tx *InstancePtr, u32 Data);
/**
* Read from pps_delay gateway of dexter_dsp_tx. Assignments are LSB-justified.
*
* @param	InstancePtr is the pps_delay instance to operate on.
*
* @return	u32
*
* @note    .
*
*/
u32 dexter_dsp_tx_pps_delay_read(dexter_dsp_tx *InstancePtr);
/**
* Write to stream_start0_lsb gateway of dexter_dsp_tx. Assignments are LSB-justified.
*
* @param	InstancePtr is the stream_start0_lsb instance to operate on.
* @param	Data is value to be written to gateway stream_start0_lsb.
*
* @return	None.
*
* @note    .
*
*/
void dexter_dsp_tx_stream_start0_lsb_write(dexter_dsp_tx *InstancePtr, u32 Data);
/**
* Read from stream_start0_lsb gateway of dexter_dsp_tx. Assignments are LSB-justified.
*
* @param	InstancePtr is the stream_start0_lsb instance to operate on.
*
* @return	u32
*
* @note    .
*
*/
u32 dexter_dsp_tx_stream_start0_lsb_read(dexter_dsp_tx *InstancePtr);
/**
* Write to stream_start0_msb gateway of dexter_dsp_tx. Assignments are LSB-justified.
*
* @param	InstancePtr is the stream_start0_msb instance to operate on.
* @param	Data is value to be written to gateway stream_start0_msb.
*
* @return	None.
*
* @note    .
*
*/
void dexter_dsp_tx_stream_start0_msb_write(dexter_dsp_tx *InstancePtr, u32 Data);
/**
* Read from stream_start0_msb gateway of dexter_dsp_tx. Assignments are LSB-justified.
*
* @param	InstancePtr is the stream_start0_msb instance to operate on.
*
* @return	u32
*
* @note    .
*
*/
u32 dexter_dsp_tx_stream_start0_msb_read(dexter_dsp_tx *InstancePtr);
/**
* Write to gain_dc0 gateway of dexter_dsp_tx. Assignments are LSB-justified.
*
* @param	InstancePtr is the gain_dc0 instance to operate on.
* @param	Data is value to be written to gateway gain_dc0.
*
* @return	None.
*
* @note    .
*
*/
void dexter_dsp_tx_gain_dc0_write(dexter_dsp_tx *InstancePtr, u32 Data);
/**
* Read from gain_dc0 gateway of dexter_dsp_tx. Assignments are LSB-justified.
*
* @param	InstancePtr is the gain_dc0 instance to operate on.
*
* @return	u32
*
* @note    .
*
*/
u32 dexter_dsp_tx_gain_dc0_read(dexter_dsp_tx *InstancePtr);
/**
* Read from buff_ovflws0 gateway of dexter_dsp_tx. Assignments are LSB-justified.
*
* @param	InstancePtr is the buff_ovflws0 instance to operate on.
*
* @return	u32
*
* @note    .
*
*/
u32 dexter_dsp_tx_buff_ovflws0_read(dexter_dsp_tx *InstancePtr);
/**
* Read from version gateway of dexter_dsp_tx. Assignments are LSB-justified.
*
* @param	InstancePtr is the version instance to operate on.
*
* @return	u32
*
* @note    .
*
*/
u32 dexter_dsp_tx_version_read(dexter_dsp_tx *InstancePtr);
/**
* Read from pps_error gateway of dexter_dsp_tx. Assignments are LSB-justified.
*
* @param	InstancePtr is the pps_error instance to operate on.
*
* @return	u32
*
* @note    .
*
*/
u32 dexter_dsp_tx_pps_error_read(dexter_dsp_tx *InstancePtr);
/**
* Read from pps_cnt gateway of dexter_dsp_tx. Assignments are LSB-justified.
*
* @param	InstancePtr is the pps_cnt instance to operate on.
*
* @return	u32
*
* @note    .
*
*/
u32 dexter_dsp_tx_pps_cnt_read(dexter_dsp_tx *InstancePtr);
/**
* Read from pps_clks_lsb gateway of dexter_dsp_tx. Assignments are LSB-justified.
*
* @param	InstancePtr is the pps_clks_lsb instance to operate on.
*
* @return	u32
*
* @note    .
*
*/
u32 dexter_dsp_tx_pps_clks_lsb_read(dexter_dsp_tx *InstancePtr);
/**
* Read from pps_clks_msb gateway of dexter_dsp_tx. Assignments are LSB-justified.
*
* @param	InstancePtr is the pps_clks_msb instance to operate on.
*
* @return	u32
*
* @note    .
*
*/
u32 dexter_dsp_tx_pps_clks_msb_read(dexter_dsp_tx *InstancePtr);
/**
* Read from buff_ovflws1 gateway of dexter_dsp_tx. Assignments are LSB-justified.
*
* @param	InstancePtr is the buff_ovflws1 instance to operate on.
*
* @return	u32
*
* @note    .
*
*/
u32 dexter_dsp_tx_buff_ovflws1_read(dexter_dsp_tx *InstancePtr);
#ifdef __cplusplus
}
#endif
#endif
