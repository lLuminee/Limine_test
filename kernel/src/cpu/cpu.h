#pragma once
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <flanterm/flanterm.h>

void init(struct flanterm_context *ctx);
uint64_t read_cr3(void);
void write_cr3(uint64_t cr3);
void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
void *memmove(void *dest, const void *src, size_t n);
