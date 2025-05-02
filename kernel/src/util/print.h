#pragma once
#include <flanterm/flanterm.h>
#include <stdarg.h>

void print_init(struct flanterm_context *ctx);
void printl(const char *str, ...);
