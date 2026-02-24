proc generate {drv_handle} {
    xdefine_include_file $drv_handle "xparameters.h" "dexter_dsp_tx" "NUM_INSTANCES" "DEVICE_ID" "C_DEXTER_DSP_TX_S_AXI_BASEADDR" "C_DEXTER_DSP_TX_S_AXI_HIGHADDR" 
    xdefine_config_file $drv_handle "dexter_dsp_tx_g.c" "dexter_dsp_tx" "DEVICE_ID" "C_DEXTER_DSP_TX_S_AXI_BASEADDR" 
    xdefine_canonical_xpars $drv_handle "xparameters.h" "dexter_dsp_tx" "DEVICE_ID" "C_DEXTER_DSP_TX_S_AXI_BASEADDR" "C_DEXTER_DSP_TX_S_AXI_HIGHADDR" 

}