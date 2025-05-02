#include "memory.h"
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <limine.h>

uint64_t BiggestMemoryPart_Base;
uint64_t BiggestMemoryPart_Size;
uint64_t FirstPage_Base;
uint64_t FirstPage_Limit; // ✅ Ajouté
uint16_t PageSize = 0x1000;
int PageAvailable = 0;

void GetBiggestMemoryMap(struct limine_memmap_response *memmap, uint64_t *biggest_size, uint64_t *biggest_base) {
    *biggest_size = 0;
    *biggest_base = 0;
    for (size_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE && entry->length > *biggest_size) {
            *biggest_size = entry->length;
            *biggest_base = entry->base;
        }
    }

    BiggestMemoryPart_Size = *biggest_size;
    BiggestMemoryPart_Base = *biggest_base;

    // ✅ Commencer au début du bloc, aligné à 4K
    FirstPage_Base = (*biggest_base + 0xFFF) & ~0xFFF;
    FirstPage_Limit = *biggest_base + *biggest_size;
    PageAvailable = (FirstPage_Limit - FirstPage_Base) / PageSize;
}

void* allocPage() {
    if (PageAvailable <= 0 || FirstPage_Base + PageSize > FirstPage_Limit) {
        printl("No more pages available or limit exceeded\n");
        asm ("hlt");
        return NULL;
    }
    uint64_t page = FirstPage_Base;
    FirstPage_Base += PageSize;
    PageAvailable--;
    return (void*)page;
}

void init_memory(struct limine_memmap_response *memmap) {
    uint64_t biggest_size = 0;
    uint64_t biggest_base = 0;
    GetBiggestMemoryMap(memmap, &biggest_size, &biggest_base);
    printl("Biggest memory part: base=0x%lx, size=0x%lx\n", biggest_base, biggest_size);
    printl("First page base: 0x%lx\n", FirstPage_Base);
    printl("Page size: %d bytes\n", PageSize);
    printl("Pages available: %d\n", PageAvailable);
}
