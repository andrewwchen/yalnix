// Contains utility functions for managing allocated frames
//
// Andrew Chen
// 2/2024

#ifndef _frame_manager_h
#define _frame_manager_h

// initializes bit vector to track allocated frames
int InitializeFrames(int pmem_size);

// allocates a specific unallocated frame
int AllocateSpecificFrame(int frame);

// allocates a previously unallocated frame and returns it
int AllocateFrame();

// deallocates a previously allocated frame
int DeallocateFrame(int frame);

#endif