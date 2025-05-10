#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limine.h>
#include <cpu/cpu.h>
#include <util/print.h>
#include <memory/memory.h>

#define PAGESIZE 0x1000


struct page_directory_entry {
    uint64_t present                   :1;
    uint64_t writeable                 :1;
    uint64_t user_access               :1;
    uint64_t write_through             :1;
    uint64_t cache_disabled            :1;
    uint64_t accessed                  :1;
    uint64_t ignored_3                 :1;
    uint64_t size                      :1; // 0 means page directory mapped
    uint64_t ignored_2                 :4;
    uint64_t page_ppn                  :28;
    uint64_t reserved_1                :12; // must be 0
    uint64_t ignored_1                 :11;
    uint64_t execution_disabled        :1;
} __attribute__((packed));

struct page_table_entry{
    uint64_t present            : 1;
    uint64_t writeable          : 1;
    uint64_t user_access        : 1;
    uint64_t write_through      : 1;
    uint64_t cache_disabled     : 1;
    uint64_t accessed           : 1;
    uint64_t dirty              : 1;
    uint64_t size               : 1;
    uint64_t global             : 1;
    uint64_t ignored_2          : 3;
    uint64_t page_ppn           : 28;
    uint64_t reserved_1         : 12; // must be 0
    uint64_t ignored_1          : 11;
    uint64_t execution_disabled : 1;
} __attribute__((__packed__));

struct page_table {
    struct page_table_entry entries[512];
} __attribute__((aligned(PAGESIZE)));

struct page_directory {
    struct page_directory_entry entries[512];
} __attribute__((aligned(PAGESIZE)));


// THIS CODE NOT CRACH
void init_paging(struct limine_hhdm_response * hhdm_reponse) {
    Breakpoint();
    // Creat new page directory
    struct page_directory *pml4_new = (struct page_directory*)((uint64_t)allocPage() + hhdm_reponse->offset);
    // Secure the new page directory
    memset(pml4_new, 0, sizeof(struct page_directory));
    printl("pml4 aligement : %lx \n", (uint64_t)pml4_new - hhdm_reponse->offset);
    printl("pml4 new : %lx \n", (uint64_t)pml4_new);
    printl("pml4 new havent hddm : %lx \n", (uint64_t)pml4_new - hhdm_reponse->offset);
    printl("pml4 old : %lx \n", (uint64_t)read_cr3() & ~0xFFFULL);

    // get the old page directory
    struct page_directory *pml4_old = (struct page_directory *)((read_cr3() & ~0xFFFULL) + hhdm_reponse->offset);

    // Copy the old PML4 to the new one
    for (uint64_t pml4_entry  = 0; pml4_entry  < 512; pml4_entry ++) {
        struct page_directory_entry actual_pml4_page = pml4_old->entries[pml4_entry];
        if (actual_pml4_page.present) {
            struct page_directory * PDPT_old = (struct page_directory*)(((uint64_t)actual_pml4_page.page_ppn << 12) + hhdm_reponse->offset);
            struct page_directory * PDPT_new = (struct page_directory*)((uint64_t)allocPage() + hhdm_reponse->offset);
            if (PDPT_old == NULL || PDPT_new == NULL) {printl("PDPT = NULLE");  asm ("hlt"); }

            memcpy(&pml4_new->entries[pml4_entry], &pml4_old->entries[pml4_entry], sizeof(struct page_directory_entry));
            pml4_new->entries[pml4_entry].page_ppn = ((uint64_t)PDPT_new - hhdm_reponse->offset) >> 12;

            for (uint64_t pdpt_entry = 0; pdpt_entry < 512; pdpt_entry++){
                struct page_directory_entry actual_PDPT_page = PDPT_old->entries[pdpt_entry];
                if (actual_PDPT_page.present) {
                    struct page_directory * PD_old = (struct page_directory*)(((uint64_t)actual_PDPT_page.page_ppn << 12) + hhdm_reponse->offset);
                    struct page_directory * PD_new = (struct page_directory*)((uint64_t)allocPage() + hhdm_reponse->offset);
                    if (PD_old == NULL || PD_new == NULL) { printl("PD = NULL");  asm ("hlt");}

                    memcpy(&PDPT_new->entries[pdpt_entry], &PDPT_old->entries[pdpt_entry],sizeof(struct page_directory_entry));
                    PDPT_new->entries[pdpt_entry].page_ppn = ((uint64_t)PD_new - hhdm_reponse->offset) >> 12;

                    for (uint64_t pd_entry = 0; pd_entry < 512; pd_entry++){
                        struct page_directory_entry acutal_PD_page = PD_old->entries[pd_entry];
                        if (acutal_PD_page.present){
                            struct page_table * PT_old = (struct page_table*)(((uint64_t)acutal_PD_page.page_ppn << 12) + hhdm_reponse->offset); 
                            struct page_table * PT_new = (struct page_table*)((uint64_t)allocPage() + hhdm_reponse->offset);
                            if (PT_old == NULL || PT_new == NULL) { printl("PT = NULL");  asm ("hlt");}

                            memcpy(&PD_new->entries[pd_entry], &PD_old->entries[pd_entry],sizeof(struct page_directory_entry));
                            PD_new->entries[pd_entry].page_ppn  = ((uint64_t)PT_new - hhdm_reponse->offset) >> 12;

                            for (uint64_t pt_entry = 0; pt_entry < 512; pt_entry++){
                                struct page_table_entry actual_pt_page = PT_old->entries[pt_entry];
                                if(actual_pt_page.present){
                                    memcpy(&PT_new->entries[pt_entry], &PT_old->entries[pt_entry],sizeof(struct page_table_entry) );
                                }
                            }
                        }
                    }
                }
            }
        } 
    }
    printl("Before mov \n");
    Breakpoint();
    uint64_t pml4_physical = (uint64_t)pml4_new - hhdm_reponse->offset;    
    asm volatile("mov %0, %%cr3" :: "r"(pml4_physical) : "memory");
    printl("Paging is set");

}




void Breakpoint() {
    int i = 1;
}