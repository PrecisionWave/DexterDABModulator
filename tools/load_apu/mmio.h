#ifndef MMIO_H__INCLUDED
#define MMIO_H__INCLUDED

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdint.h>
#include <stdio.h>

uint32_t ioread32(void* ptr, size_t reg);
void iowrite32(void* ptr, size_t reg, uint32_t data);

uint16_t ioread16(void* ptr, size_t reg);
void iowrite16(void* ptr, size_t reg, uint16_t data);

void iomemset(void* ptr, size_t reg, uint8_t pattern, size_t length);
void copytoio(void* ptr, size_t reg, const void* src, size_t length);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // MMIO_H__INCLUDED
