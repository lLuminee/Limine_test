#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

void init_memory(struct limine_memmap_response *memmap);
void* allocPage();
void GetBiggestMemoryMap(struct limine_memmap_response *memmap, uint64_t *biggest_size, uint64_t *biggest_base);
