#include "memorymap.h"

struct memory_map_entry* mm_lookup(struct memory_map* mm, uint32_t address)
{
    for (size_t i = 0; i < mm->count; i++) {
        if (address >= mm->entries[i].apu_linked && address <= mm->entries[i].apu_linked + mm->entries[i].length) {
            // Need to include end of address (<=) as well
            // else the this code will not fixup correctly:
            //   for (p = start; p < end; ++p);
            return &mm->entries[i];
        }
    }
    return NULL;
}

struct memory_map_entry* mm_lookup_loaded(struct memory_map* mm, uint32_t address)
{
    for (size_t i = 0; i < mm->count; i++) {
        if (address >= mm->entries[i].apu_loaded && address <= mm->entries[i].apu_loaded + mm->entries[i].length) {
            // Need to include end of address (<=) as well
            // else the this code will not fixup correctly:
            //   for (p = start; p < end; ++p);
            return &mm->entries[i];
        }
    }
    return NULL;
}
