// Contains utility functions for managing allocated frames
//
// Andrew Chen
// 2/2024

#include <ykernel.h>

// bit vector which tracks allocated frames
// "allocated_frames[frame] == 1" means that the frame is allocated
int *allocated_frames;

// number of frames in allocated_frames
int num_frames;

// number of allocated frames in allocated_frames
int num_allocated_frames;

// the lowest frame above PMEM_BASE
int min_frame = UP_TO_PAGE(PMEM_BASE)/PAGESIZE;

// initializes bit vector to track allocated frames
int InitializeFrames(int pmem_size) {
  num_frames = (pmem_size / PAGESIZE);
  allocated_frames = malloc(num_frames * sizeof(int));
  if (allocated_frames == NULL) {
    TracePrintf(1, "InitializeFrames: failed to malloc allocated_frames\n");
    return -1;
  }
  bzero(allocated_frames, (pmem_size / PAGESIZE) * sizeof(int));
  return 0;
}

// allocates a specific unallocated frame
int AllocateSpecificFrame(int frame) {
  if (frame < min_frame) {
    TracePrintf(1, "AllocateSpecificFrame: Failed to allocate frame %d: below minimum frame %d\n", frame, min_frame);
    return -1;
  }
  if (frame >= num_frames) {
    TracePrintf(1, "AllocateSpecificFrame: Failed to allocate frame %d: above maximum frame %d\n", frame, num_frames-1);
    return -1;
  }
  if (allocated_frames[frame] == 1) {
    TracePrintf(1, "AllocateSpecificFrame: Failed to allocate frame %d: frame is already allocated\n", frame);
    return -1;
  }
  num_allocated_frames++;
  allocated_frames[frame] = 1;
  return 0;
}

// allocates a previously unallocated frame and returns it
int AllocateFrame() {
  // do not allocate frames that are below PMEM_BASE 
  for (int frame = min_frame; frame < num_frames; frame++ ) {
    if (allocated_frames[frame] == 0) {
      allocated_frames[frame] = 1;
      num_allocated_frames++;
      return frame;
    }
  }
  TracePrintf(1, "AllocateFrame: Failed to allocate a frame\n");
  return -1;
}

// deallocates a previously allocated frame
int DeallocateFrame(int frame) {
  if (frame < min_frame) {
    TracePrintf(1, "DeallocateFrame: Failed to deallocate frame %d: below minimum frame %d\n", frame, min_frame);
    return -1;
  }
  if (frame >= num_frames) {
    TracePrintf(1, "DeallocateFrame: Failed to deallocate frame %d: above maximum frame %d\n", frame, num_frames-1);
    return -1;
  }
  if (allocated_frames[frame] == 0) {
    TracePrintf(1, "DeallocateFrame: Failed to deallocate frame %d: frame is already deallocated\n", frame);
    return -1;
  }
  allocated_frames[frame] = 0;

  num_allocated_frames--;
  return 0;
}