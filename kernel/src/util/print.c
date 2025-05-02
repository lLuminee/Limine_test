#include <flanterm/flanterm.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

struct flanterm_context *print_ctx;


static void print_number(struct flanterm_context *ctx, int num, int base) {
    char buf[32];
    const char *digits = "0123456789abcdef";
    int i = 0;

    if (num == 0) {
        flanterm_write(ctx, "0", 1);
        return;
    }

    int is_negative = 0;
    if (base == 10 && num < 0) {
        is_negative = 1;
        num = -num;
    }

    while (num > 0 && i < 31) {
        buf[i++] = digits[num % base];
        num /= base;
    }

    if (is_negative) {
        buf[i++] = '-';
    }

    // reverse
    for (int j = i - 1; j >= 0; j--) {
        flanterm_write(ctx, &buf[j], 1);
    }
}

static void print_u64(struct flanterm_context *ctx, uint64_t num, int base) {
    char buf[32];
    const char *digits = "0123456789abcdef";
    int i = 0;

    if (num == 0) {
        flanterm_write(ctx, "0", 1);
        return;
    }

    while (num > 0 && i < 31) {
        buf[i++] = digits[num % base];
        num /= base;
    }

    // reverse
    for (int j = i - 1; j >= 0; j--) {
        flanterm_write(ctx, &buf[j], 1);
    }
}


void printl(const char *str, ...) {
    va_list args;
    va_start(args, str);

    for (size_t i = 0; str[i]; i++) {
        // Traitement spécial pour %lx
        if (str[i] == '%' && str[i + 1] == 'l' && str[i + 2] == 'x') {
            uint64_t val = va_arg(args, uint64_t);
            flanterm_write(print_ctx, "0x", 2);
            print_u64(print_ctx, val, 16);
            i += 2; // on consomme 'l' et 'x'
            continue;
        }

        // Traitement normal des autres cas
        if (str[i] == '%' && str[i + 1]) {
            i++;
            switch (str[i]) {
                case 's': {
                    const char *s = va_arg(args, const char*);
                    if (s) {
                        for (size_t j = 0; s[j]; j++) {
                            flanterm_write(print_ctx, &s[j], 1);
                        }
                    }
                    break;
                }
                case 'd': {
                    int val = va_arg(args, int);
                    print_number(print_ctx, val, 10);
                    break;
                }
                case 'x': {
                    int val = va_arg(args, int);
                    print_number(print_ctx, val, 16);
                    break;
                }
                case '%': {
                    flanterm_write(print_ctx, "%", 1);
                    break;
                }
                default:
                    flanterm_write(print_ctx, &str[i - 1], 2);
                    break;
            }
        } else {
            flanterm_write(print_ctx, &str[i], 1);
        }
    }

    va_end(args);
}

void print_init(struct flanterm_context *ctx) {
    print_ctx = ctx;
    flanterm_write(ctx, "Print initialized\n", 18);
}