#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>

#include <flanterm/flanterm.h>
#include <flanterm/fb.h>
#include <cpu/cpu.h>
#include <apic/apicLocal.h>
#include <idt/idt.h>
#include <memory/memory.h>
#include <util/print.h>
#include <memory/paging.h>

__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

__attribute__((used, section(".limine_requests_start")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_paging_mode_request paging_mode_request = {
    .id = LIMINE_PAGING_MODE_REQUEST,
    .revision = 0
};

// Remplacement de limine_kernel_address_request par limine_executable_address_request
static volatile struct limine_executable_address_request kernel_address_request = {
    .id = LIMINE_EXECUTABLE_ADDRESS_REQUEST,
    .revision = 0,
    .response = NULL
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};
__attribute__((used, section(".limine_requests_end")))
static volatile LIMINE_REQUESTS_END_MARKER;

// Halt and catch fire function.
static void hcf(void) {
    asm ("hlt");
}

void kmain(void) {
    if (LIMINE_BASE_REVISION_SUPPORTED == false) {
        hcf();
    }


    // Vérification de la réponse du framebuffer.
    if (framebuffer_request.response == NULL
     || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    // Récupération du premier framebuffer.
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    struct limine_memmap_response *memmap = memmap_request.response;

    // Initialisation du contexte flanterm
    struct flanterm_context *ctx = flanterm_fb_init(
        NULL, NULL,
        (uint32_t *)framebuffer->address,
        framebuffer->width, framebuffer->height, framebuffer->pitch,
        8, 16, 8, 8, 8, 0,
        NULL, NULL, NULL, NULL,
        NULL, NULL, NULL, NULL,
        NULL, 0, 0, 0, 0, 0); // ← Remplace les deux derniers NULL par 0 (entiers)    
    flanterm_write(ctx, "Hello world --- Kernel Execution\n", 34);

    init(ctx); // Initialisation du CPU
    print_init(ctx); // Initialisation de flanterm    
    init_memory(memmap); // Initialisation de la mémoire
    init_paging(hhdm_request.response); // Initialisation de la pagination


    while (true)
    {
    }
    
        
    hcf();
}