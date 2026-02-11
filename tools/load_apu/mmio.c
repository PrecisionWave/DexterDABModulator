// Memory mapped IO

#include "mmio.h"

#include <assert.h>
#include <string.h>

#ifdef SIM
static inline void MEMORY_BARRIER(void) {}
#else
static inline void MEMORY_BARRIER(void)
{
    asm volatile("dmb 0xf\ndsb " ::: "memory");
}
#endif

uint32_t ioread32(void* ptr, size_t reg)
{
    uint8_t* p8 = (uint8_t*)(ptr) + reg;
    MEMORY_BARRIER();
    uint32_t value;
    memcpy(&value, p8, 4);
    return value;
}

void iowrite32(void* ptr, size_t reg, uint32_t data)
{
    uint8_t* p8 = (uint8_t*)(ptr) + reg;
    memcpy(p8, &data, 4);
    MEMORY_BARRIER();
}

void iomemset(void* ptr, size_t reg, uint8_t pattern, size_t length)
{
    volatile uint8_t* p8 = (volatile uint8_t*)(ptr) + reg;
    volatile uint8_t* p8_end = (volatile uint8_t*)(ptr) + reg + length;

    while (p8 != p8_end) {
        *p8++ = pattern;
    }

    MEMORY_BARRIER();
}


void copytoio(void* ptr, size_t reg, const void* src, size_t length)
{
    volatile uint8_t* p8 = (volatile uint8_t*)(ptr) + reg;
    volatile uint8_t* p8_end = (volatile uint8_t*)(ptr) + reg + length;
    volatile uint8_t* s8 = (volatile uint8_t*)(src);

    while (p8 != p8_end) {
        *p8++ = *s8++;
    }

    MEMORY_BARRIER();
}
