#ifndef __CHAR_APU_DEXTER_H
#define __CHAR_APU_DEXTER_H

#include <linux/ioctl.h>

#define DEXTER_APU_DEV_MAX                  16

// APU controls
#define DEXTER_APU_SRAM_SIZE                0x04000U
#define DEXTER_APU_DDR_SIZE_DEFAULT         (1 * 1024 * 1024)

// mmap special page offsets
#define DEXTER_APU_MMAP_REGS                0
#define DEXTER_APU_MMAP_DDR                 1
#define DEXTER_APU_MMAP_SRAM                2
#define DEXTER_APU_MMAP_REGS2               3

// IOCTL calls
#define DEXTER_APU_IOCTL_APU_RESET          _IOW(0, 1, int)
#define DEXTER_APU_IOCTL_GET_SRAM_SIZE      _IOR(0, 2, uint32_t)
#define DEXTER_APU_IOCTL_GET_DDR_SIZE       _IOR(0, 3, uint32_t)
#define DEXTER_APU_IOCTL_GET_DDR_PHYS       _IOR(0, 4, uint32_t)
#define DEXTER_APU_IOCTL_SYNC_FOR_CPU       _IOW(0, 5, int)
#define DEXTER_APU_IOCTL_SYNC_FOR_DEVICE    _IOW(0, 6, int)
#define DEXTER_APU_IOCTL_APU_START          _IOW(0, 7, uint32_t)

#define DEXTER_APU_DMA_FROM_DEVICE          0
#define DEXTER_APU_DMA_TO_DEVICE            1
#define DEXTER_APU_DMA_BIDIR                2

#endif  // #define __CHAR_APU_DEXTER_H
