// Contains functions for editing and creating page table entries
//
// Andrew Chen
// 2/2024

#include <ykernel.h>
#include <frame_manager.h>

// create a new PTE with the specified prot and allocates a pfn
// for use with user page tables
pte_t* CreateUserPTE(int prot) {
  pte_t* pte = malloc(sizeof(pte_t));
  if (pte == NULL) {
    TracePrintf(1, "CreateUserPTE: failed to malloc pte\n");
    return NULL;
  }
  bzero(pte, sizeof(pte_t));

  int pfn = AllocateFrame();
  if (pfn == -1) {
    TracePrintf(1, "CreateUserPTE: failed to allocate pfn \n");
    return NULL;
  }

  pte->valid = 1;
  pte->prot = prot;
  pte->pfn = pfn;
  return pte;
}

// populates an existing PTE with the specified data
// for use with the kernel page table
int PopulateKernelPTE(pte_t* pte, int prot, int pfn) {
  if (pte->valid == 1) {
    TracePrintf(1, "PopulateKernelPTE: pte is already valid\n");
    return -1;
  }
  pte->valid = 1;
  pte->prot = prot;
  pte->pfn = pfn;
  return 0;
}

// makes an existing pte invalid, sets prot to 0, and frees the corresponding pfn
// for use with the kernel page table
int ClearKernelPTE(pte_t* pte) {
  if (pte->valid == 0) {
    return 0;
  }
  if (DeallocateFrame(pte->pfn) == -1) {
    TracePrintf(1, "ClearKernelPTE: failed to deallocate pfn\n");
    return -1;
  }
  pte->prot = 0;
  pte->valid = 0;
  return 0;
}

// frees an existing pte and frees the corresponding pfn
// for use with user page tables
int FreeUserPTE(pte_t* pte) {
  if (DeallocateFrame(pte->pfn) == -1) {
    TracePrintf(1, "FreeUserPTE: failed to deallocate pfn\n");
    return -1;
  }
  free(pte);
  return 0;
}

// creates multiple new PTE with the specified prot, allocates pfn for each, and enters them into a page table between the specified pages
// for use with user page tables
int CreateUserPTERegion(pte_t* pt, int start_page, int end_page, int prot) {
  for (int page = start_page; page < end_page; page++)
  {
    pte_t* pte = CreateUserPTE(prot);
    if (pte == NULL) {
      TracePrintf(1, "CreateUserPTERegion: failed create PTE for page number %d\n", page);
      return -1;
    }
    pt[page] = *pte;
  }
  return 0;
}