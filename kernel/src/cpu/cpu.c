#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <flanterm/flanterm.h>


void *memcpy(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    for (size_t i = 0; i < n; i++) {
        pdest[i] = psrc[i];
    }

    return dest;
}

void *memset(void *s, int c, size_t n) {
    uint8_t *p = (uint8_t *)s;

    for (size_t i = 0; i < n; i++) {
        p[i] = (uint8_t)c;
    }

    return s;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = (uint8_t *)dest;
    const uint8_t *psrc = (const uint8_t *)src;

    if (src > dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if (src < dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return p1[i] < p2[i] ? -1 : 1;
        }
    }

    return 0;
}

uint64_t read_cr3(void) {
    uint64_t value;
    asm volatile("mov %%cr3, %0" : "=r"(value));
    return value;
}

uint64_t read_msr(uint32_t msr) {
    uint32_t low, high;
    asm volatile (
        "rdmsr"
        : "=a"(low), "=d"(high)
        : "c"(msr)
    );
    return ((uint64_t)high << 32) | low;
}

void cpuid(uint32_t code, uint32_t* eax, uint32_t* ebx, uint32_t* ecx, uint32_t* edx) {
    asm volatile("cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(code));
}

void GetConstructCpu(struct flanterm_context *ctx) {
    uint32_t eax, ebx, ecx, edx;
    char vendor[13]; 
    cpuid(0, &eax, &ebx, &ecx, &edx);
    memcpy(vendor, &ebx, 4);      
    memcpy(vendor + 4, &edx, 4);  
    memcpy(vendor + 8, &ecx, 4);  
    vendor[12] = '\0';           
    flanterm_write(ctx, vendor, 12);
    flanterm_write(ctx, "\n", 1);

}

void VerifyApicIsPresent(struct flanterm_context *ctx) {
    uint32_t eax, edx, ebx, ecx;
    cpuid(1, &eax, &ebx, &ecx, &edx);
    if (!(edx & (1 << 9))) {
        flanterm_write(ctx, "APIC not supported\n", 19);
    } else {
        flanterm_write(ctx, "APIC supported\n", 16);
    }

}

void GetphysicalApicAddr(struct flanterm_context *ctx) {
    #define IA32_APIC_BASE 0x1B
    uint64_t lapic_phys = read_msr(IA32_APIC_BASE) & 0xFFFFF000;
    if (lapic_phys == 0xFEE00000) {
        flanterm_write(ctx, "APIC Physical Address: 0xFEE00000\n", 36);
    }
    
}

void init(struct flanterm_context *ctx){
    flanterm_write(ctx, "Cpu Init\n", 10);
    GetConstructCpu(ctx);
    VerifyApicIsPresent(ctx);
    GetphysicalApicAddr(ctx);
}