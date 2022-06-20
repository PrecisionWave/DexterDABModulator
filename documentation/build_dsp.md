# Dexter DSP

## Tools
| Matlab Version |  Vivado Version | Vivado Environment                                 | Sysgen Command   |
|----------------|-----------------|----------------------------------------------------|------------------|
| R2020a         | 2021.1          | `source /tools/Xilinx/Vivado/2021.1/settings64.sh` | `model_composer` |

## DSP Generation
Check Matlab version and change to correct version before trying this.
1. Open console
2. Source Vivado environment  
   `$` `source /tools/Xilinx/Vivado/2021.1/settings64.sh`
4. Open System generator  
   `$` `model_composer`
6. Wait until Matlab opens
7. Set Current folder to `<dexter git repo>/pcw_hdl/dsp`
8. Open the Matlab environment initialization file (e.g. `dexter_dsp_init.m`)
9. Run the script
10. Open the Simulink file (e.g. `dexter_dsp_tx.slx`)
11. Wait until all Simulink window open  
   This takes a long time and multiple windows open during the process.
   Wait until the main Simulink windows is opened and ready (e.g. `dexter_dsp_tx - Simulink` in title)
13. Select the top level (dexter_dsp_tx) in Model Browser
14. Press the "Run" button
15. Wait until "Running" appears in Simulink status bar  
   Order of occurence: "Compiling" -> "Initializing" -> "Running"
16. Stop the Simulation or wait until it is done
17. The Schematic is now fully colored
18. Find the Xilinx "System Generator" icon in the Toplevel and double-click on it
19. Check target directory  
   `../library/dexter_dsp_tx`  
   Attention: Everything in the target directory gets deleted!
21. Press "Generate" to generate the DSP
22. Wait until "Generation Completed" dialog box appears and Press "OK"
23. Check generated files
24. Add the generated zip file to git  
   (e.g. `<dexter git repo>/pcw_hdl/library/dexter_dsp_tx/ip/PrecisionWave_AG_pcwlib_dexter_dsp_tx_v1_0.zip`)
25. Close the System Generator by pressing "OK"
26. Close Simulink
27. Close Matlab

### Notes
You may need to delete the precompiled PrecisionWave_AG_pcwlib_dexter_dsp_tx_v1_0 directory to use the generated DSP.
