// Contains Process Control Block datastructure
//
// Andrew Chen
// 2/2024

#include <pcb.h>
#include <frame_manager.h>
#include <pte_manager.h>

// add a child pid to a pcb
void PCBAddChild(pcb_t* parent_pcb, int child_pid) {
  if (parent_pcb->child_pids_size == parent_pcb->child_pids_count) {
    int *child_pids_new = malloc(parent_pcb->child_pids_size * 2 * sizeof(int));
    for (int i = 0; i < parent_pcb->child_pids_count; i++) {
      child_pids_new[i] = parent_pcb->child_pids[i];
    }
    parent_pcb->child_pids_size *= 2;
    free(parent_pcb->child_pids);
    parent_pcb->child_pids = child_pids_new;
  }
  parent_pcb->child_pids[parent_pcb->child_pids_count] = child_pid;
  parent_pcb->child_pids_count += 1;
}

// check if a pcb has a child with the specified pid
int PCBHasChild(pcb_t* parent_pcb, int child_pid) {
  for (int i = 0; i < parent_pcb->child_pids_count; i++) {
    if (parent_pcb->child_pids[i] == child_pid) {
      return 1;
    }
  }
  return 0;
}

// Create a new pcb for a user process
pcb_t* NewPCB()
{
  pcb_t *pcb = malloc(sizeof(pcb_t));
  if (pcb == NULL) {
    TracePrintf(1, "CreateRegion1PCB: failed to malloc pcb \n");
    return NULL;
  }
  bzero(pcb, sizeof(pcb_t));

  // Create pcb kernel stack frames
  int kernel_stack_frame_1 = AllocateFrame();
  if (kernel_stack_frame_1 == -1) {
    TracePrintf(1, "CreateRegion1PCB: failed to allocate kernel_stack_frame_1 \n");
    return NULL;
  }
  int kernel_stack_frame_2 = AllocateFrame();
  if (kernel_stack_frame_2 == -1) {
    TracePrintf(1, "CreateRegion1PCB: failed to allocate kernel_stack_frame_2 \n");
    return NULL;
  }
  PopulatePTE(&pcb->kernel_stack_pages[0], PROT_READ | PROT_WRITE, kernel_stack_frame_1);
  PopulatePTE(&pcb->kernel_stack_pages[1], PROT_READ | PROT_WRITE, kernel_stack_frame_2);

  // Create pcb page table
  pte_t *pt = malloc(sizeof(pte_t) * MAX_PT_LEN);
  if (pt == NULL) {
    TracePrintf(1, "CreateRegion1PCB: failed to malloc pt \n");
    return NULL;
  }
  bzero(pt, sizeof(pte_t) * MAX_PT_LEN);
  pcb->pt_addr = pt;

  // Set pcb contents
  pcb->pid = helper_new_pid(pt);
  pcb->parent_pid = -1;
  pcb->delay_ticks = 0;
  pcb->child_pids_size = 4;
  pcb->child_pids_count = 0;
  pcb->child_pids = malloc(pcb->child_pids_size * sizeof(int));

  return pcb;
}