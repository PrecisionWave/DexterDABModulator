#ifndef MEMORY_MAP_H__INCLUDED
#define MEMORY_MAP_H__INCLUDED

#include <stdio.h>
#include <stdint.h>

struct memory_map_entry {
    const char* name;
    int index;
    void* cpu_virtual;
    uint32_t length;
    uint32_t apu_linked;
    uint32_t apu_loaded;
};

struct memory_map {
    size_t count;
    struct memory_map_entry entries[];
};

struct memory_map_entry* mm_lookup(struct memory_map* mm, uint32_t address);

#endif // MEMORY_MAP_H__INCLUDED
