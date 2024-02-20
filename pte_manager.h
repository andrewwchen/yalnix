// Contains functions for editing and creating page table entries
//
// Andrew Chen
// 2/2024

#include <ykernel.h>

// populates an existing PTE with the specified data
int PopulatePTE(pte_t* pte, int prot, int pfn);

// makes an existing pte invalid, sets prot to 0, and frees the corresponding pfn
int ClearPTE(pte_t* pte);

// creates multiple new PTE with the specified prot, allocates pfn for each, and enters them into a page table between the specified pages
// for use with user page tables
int PopulatePTERegion(pte_t* pt, int start_page, int end_page, int prot);


pte_t* CreateUserPTE(int prot);

int FreeUserPTE(pte_t* pte);

int ClearPT(pte_t* pt);