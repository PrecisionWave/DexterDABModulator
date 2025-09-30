// Memory mapped IO

#include "mmio.h"

#include <stdatomic.h>
#include <assert.h>

uint32_t ioread32(void* ptr, size_t reg)
{
    volatile uint8_t* p8 = (volatile uint8_t*)(ptr) + reg;
    volatile uint32_t* p32 = (volatile uint32_t*)p8;
    return atomic_load(p32);
}

void iowrite32(void* ptr, size_t reg, uint32_t data)
{
    volatile uint8_t* p8 = (volatile uint8_t*)(ptr) + reg;
    volatile uint32_t* p32 = (volatile uint32_t*)p8;
    atomic_store(p32, data);
}

void iomemset(void* ptr, size_t reg, uint8_t pattern, size_t length)
{
    volatile uint8_t* p8 = (volatile uint8_t*)(ptr) + reg;
    volatile uint8_t* p8_end = (volatile uint8_t*)(ptr) + reg + length;

    while ((uintptr_t)p8 & 0x3) {
        atomic_store(p8++, pattern);
    }
    if (p8 == p8_end)
        return;

    volatile uint32_t* p32_end = (volatile uint32_t*)((uintptr_t)p8_end & ~0x3);
    volatile uint32_t* p32 = (volatile uint32_t*)p8;
    uint32_t pattern32 = (pattern << 24U) | (pattern << 16U) | (pattern << 8U) | pattern;
    while (p32 != p32_end) {
        atomic_store(p32++, pattern32);
    }

    p8 = (volatile uint8_t*)p32;
    while (p8 != p8_end) {
        atomic_store(p8++, pattern);
    }
}


void copytoio(void* ptr, size_t reg, const void* src, size_t length)
{
    volatile uint8_t* p8 = (volatile uint8_t*)(ptr) + reg;
    volatile uint8_t* p8_end = (volatile uint8_t*)(ptr) + reg + length;
    volatile uint8_t* s8 = (volatile uint8_t*)(src);

    while ((uintptr_t)p8 & 0x3) {
        atomic_store(p8++, *s8++);
    }
    if (p8 == p8_end)
        return;

    // copy words
    volatile uint32_t* p32 = (volatile uint32_t*)p8;
    volatile uint32_t* p32_end = (volatile uint32_t*)((uintptr_t)p8_end & ~0x3);
    volatile uint32_t* s32 = (volatile uint32_t*)s8;
    while (p32 != p32_end) {
        atomic_store(p32++, *s32++);
    }

    p8 = (volatile uint8_t*)p32;
    s8 = (volatile uint8_t*)s32;
    while (p8 != p8_end) {
        atomic_store(p8++, *s8++);
    }
}
