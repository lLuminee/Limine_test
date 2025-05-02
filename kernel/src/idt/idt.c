#include <stdint.h>
#include <flanterm/flanterm.h>
#include <util/print.h>

/* --- Structure d'une entrée de l'IDT en mode 64 bits --- */
struct InterruptDescriptor64 {
    uint16_t offset_1;        // bits 0..15 de l'adresse du handler
    uint16_t selector;        // sélecteur de segment (ex: 0x08 pour le code en flat mode)
    uint8_t  ist;             // bits 0..2 : index dans l'Interrupt Stack Table
    uint8_t  type_attributes; // type de porte, DPL, et bit present (P)
    uint16_t offset_2;        // bits 16..31 de l'adresse du handler
    uint32_t offset_3;        // bits 32..63 de l'adresse du handler
    uint32_t zero;            // réservé, mis à 0
} __attribute__((packed));

/* --- Déclaration de la table IDT (256 entrées) --- */
struct InterruptDescriptor64 idt[256];

/* --- Structure IDTR pour charger l'IDT avec la commande lidt --- */
struct IDTR {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

/* --- Fonction pour configurer une entrée dans l'IDT --- */
void set_idt_entry(int vector, void* handler, uint16_t selector, uint8_t type_attr, uint8_t ist) {
    uint64_t handler_addr = (uint64_t)handler;
    idt[vector].offset_1 = handler_addr & 0xFFFF;
    idt[vector].selector = selector;
    idt[vector].ist = ist & 0x7;  // seuls les 3 bits de poids faible sont utilisés
    idt[vector].type_attributes = type_attr;
    idt[vector].offset_2 = (handler_addr >> 16) & 0xFFFF;
    idt[vector].offset_3 = (handler_addr >> 32) & 0xFFFFFFFF;
    idt[vector].zero = 0;
}

/* --- Fonction de gestion de la page fault --- */
void page_fault_handler() {
    printl("Page Fault ! -----------------------------------------------------\n");
    while (1);  // Boucle infinie pour arrêter le système (exemple minimal)
}


extern void isr14();  // Déclaration de la fonction de gestion
void init_idt() {
    set_idt_entry(14, isr14, 0x08, 0x8E, 0);
    struct IDTR idtr;
    idtr.limit = sizeof(idt) - 1;
    idtr.base = (uint64_t)&idt;
    asm volatile("lidt %0" : : "m"(idtr));
    printl("IDT initialisée\n");

}
