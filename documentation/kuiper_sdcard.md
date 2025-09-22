# Dexter Linux

## ADI Kuiper Linux
One of the easiest ways to get started with Dexter is to use ADI Kuiper Linux as a base
and modify it for use with the Dexter platform.  

Baiscally there are three steps to do:
1. Download the ADI Kuiper Image
  - [Download Linux Image on analog.com wiki](https://wiki.analog.com/resources/tools-software/linux-software/embedded_arm_images#download_linux_image)

2. Flashing the ADI Kuiper Image to the SD-Cards
  - [Formatting and Flashing SD Cards using Linux on analog.com wiki](https://wiki.analog.com/resources/tools-software/linux-software/zynq_images/linux_hosts)
  - [Formatting and Flashing SD Cards using Windows on analog.com wiki](https://wiki.analog.com/resources/tools-software/linux-software/zynq_images/windows_hosts)

3. Replace the files on the BOOT partition with the Dexter files
  - [Start here](build_bsp_sdboot.md) and follow the dependencies to build and copy all relevant files

### Tested Kuiper Release
2 April 2023 release (2021_r2)
  
| File                                  | Checksum                          |
| ------------------------------------- | --------------------------------- |
| image_2023-04-02-ADI-Kuiper-full.zip  | 0cdcf6e131318113a137cf54335b9614  |
| 2023-04-02-ADI-Kuiper-full.img        | aeff476b577b45cc6ce6ce02403a57c2  |

https://swdownloads.analog.com/cse/kuiper/image_2023-04-02-ADI-Kuiper-full.zip

### Notes
#### Users and Passwords  
| User      | Password  |
| --------- | --------- |
| root      | analog    |
| analog    | analog    |