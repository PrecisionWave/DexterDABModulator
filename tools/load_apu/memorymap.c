#include "memorymap.h"

struct memory_map_entry* mm_lookup(struct memory_map* mm, uint32_t address)
{
    for (size_t i = 0; i < mm->count; i++) {
        if (address >= mm->entries[i].apu_linked && address < mm->entries[i].apu_linked + mm->entries[i].length) {
            return &mm->entries[i];
        }
    }
    return NULL;
}


