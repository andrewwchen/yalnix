// Contains functions for editing and creating page table entries
//
// Andrew Chen
// 2/2024

#include <ykernel.h>

// create a new PTE with the specified prot and allocates a pfn
// for use with user page tables
pte_t* CreateUserPTE(int prot);

// populates an existing PTE with the specified data
// for use with the kernel page table
int PopulateKernelPTE(pte_t* pte, int prot, int pfn);

// makes an existing pte invalid, sets prot to 0, and frees the corresponding pfn
// for use with the kernel page table
int ClearKernelPTE(pte_t* pte);

// frees an existing pte and frees the corresponding pfn
// for use with user page tables
int FreeUserPTE(pte_t* pte);

// creates multiple new PTE with the specified prot, allocates pfn for each, and enters them into a page table between the specified pages
// for use with user page tables
int CreateUserPTERegion(pte_t* pt, int start_page, int end_page, int prot);