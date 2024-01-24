// real simple heap-management code
//
// s.w. smith, dartmouth college
// october 2004

// Sep 2016: cleaned up TracePrintf
//
// Apr 2020: big clean-up`%


#ifndef ENTER
#define ENTER
#endif

#ifndef LEAVE
#define LEAVE
#endif




// prototypes so the compiler won't throw warnings
void *memcpy(void *restrict dst, const void *restrict src, size_t n);
void *memset(void *b, int c, size_t len);
void helper_maybort(char *msg);
void helper_check_heap_inside(char *msg,unsigned int ra);

//================================================

// globals
void *heap_start = 0;
void *heap_end   = 0;
u_long not_initialized = 1;


extern unsigned int lkra;


// for testing
void *fake_heap_start = 0;
void *fake_heap_end = 0;



//================================================

// the basic data structure
typedef struct mblock {
  u_long magic1;
  u_long size;  
  u_long size_used;
  struct mblock *next, *previous;
  u_long magic2;
} mblock_t;

//================================================

void init_heap();
mblock_t *grow_heap(u_long len);


// the first storage address in the block, given a ptr to the mblock struct
#define BLOCK_START(X)     ((void *)(((void *)(X)) + sizeof(mblock_t)))


// the first address where the NEXT block would start, 
// given a ptr to the mblock struct
#define BLOCK_END(X)       (BLOCK_START(X) + (X)->size)


// the first UNUSED address in the storage area, 
// given a ptr to the mblock struct
#define UBLOCK_END(X)      (BLOCK_START(X) + (X)->size_used)


// we use the magic number to help detect heap corruption
#define MAGIC 0xfeeddad  


// if we have a choice, don't bother creating heap block for 
// anything smaller than MINBLOCK
#define MINBLOCK (2 * sizeof(mblock_t))


// round to word len
#define WORDROUND(X)   ( (((u_long)(X)) + 3) & (~3) )



//================================================
// sanity check the block
// if there's a problem, print an error statement.
// but... always return.
void
sanity_check(mblock_t *bp,char *msg) {

  //  TracePrintf(8,"HEAP sanity_check\n");

  if (NULL == bp) {
    TracePrintf(0,"| %s HEAP Warning: sanity checking null heap block\n",WHOSE_HEAP);
    helper_maybort(msg);
    return;
  }

  if (heap_start == heap_end) {
    TracePrintf(1,"| %s HEAP empty heap\n",WHOSE_HEAP);
    return;
  }

  if ( (((void *)bp) < heap_start)  || ( ((void *)bp) >= heap_end) )  {
    TracePrintf(0,"| %s HEAP CORRUPTION WARNING: heap block at 0x%x lies outside heap [0x%x -> 0x%x]\n",WHOSE_HEAP,bp, heap_start,heap_end);
    helper_maybort(msg);
  }

  if ( (bp->magic1 != MAGIC) || (bp->magic2 != MAGIC) ) {
    TracePrintf(0,"| %s HEAP CORRUPTION WARNING: heap block at 0x%x has corrupted magic number.\n",WHOSE_HEAP,bp);
    helper_maybort(msg);;
  }

  if (bp->size_used > bp->size) {
    TracePrintf(0,"| %s HEAP CORRUPTION WARNING: heap block at 0x%x is overallocated.\n",WHOSE_HEAP,bp);
    helper_maybort(msg);
  }


  if (bp->next) {
    if (  BLOCK_END(bp)  !=  (void *)(bp->next) ) {
      TracePrintf(0,"| %s HEAP CORRUPTION WARNING: next heap block after 0x%x is not adjacent\n",WHOSE_HEAP,bp);
      helper_maybort(msg);
    }
  }

}




void
helper_check_heap(char *msg) {

  mblock_t *bp;
  int zero_out_lkra = 0;
  u_long
    blocks = 0,
    dirty = 0,
    free_block_bytes = 0,
    used_block_bytes = 0,
    ifrag_bytes = 0;

  if (0==msg)
    msg="";

  if (0 == lkra) {
    lkra = (unsigned int)(__builtin_return_address(0)); 
    zero_out_lkra = 1;
  }

  TracePrintf(2,"| %s HEAP check_heap(\"%s\") %x - %x\n",WHOSE_HEAP,msg,heap_start,heap_end);

  if (not_initialized) {
    TracePrintf(2,"| %s HEAP heap not initialized\n",WHOSE_HEAP);
    if (USER_LIB)
      init_heap();
    else {
      if (zero_out_lkra) lkra = 0;
      return;
    }
  }


  TracePrintf(2,"| %s HEAP total size = 0x%x\n", WHOSE_HEAP, heap_end - heap_start);

  for (bp = (mblock_t *)heap_start; bp; bp = bp->next) {
    sanity_check(bp,msg);
    blocks++;
    if (bp->size_used) {
      dirty++;
      used_block_bytes += bp->size_used;
      ifrag_bytes += bp->size - bp->size_used;
    } else {
      free_block_bytes += bp->size;
    }
  
  }

  TracePrintf(2,"| %s HEAP Blocks: 0x%x (0x%x dirty)\n", WHOSE_HEAP,blocks,dirty);
  TracePrintf(2,"| %s HEAP Bytes: 0x%x free, 0x%x used, 0x%x ifrag\n", WHOSE_HEAP,
	      free_block_bytes, used_block_bytes, ifrag_bytes);


  TracePrintf(2,"| %s HEAP check done\n",WHOSE_HEAP);


  if (zero_out_lkra) lkra = 0;  
}




//================================================
// assumption: bp is free, and its next block exists and is free too.
// action: we merge these two blocks into one block

// return address of bp, if we merged; o.w. return bp->next
// this makes garbage collection easier

mblock_t *
merge_free_pair(mblock_t *bp) {


  mblock_t *first, *second;
  

  first = bp;

  if (NULL == first)
    return (NULL);

  second = bp->next;
  if (NULL == second)
    return (NULL);

  sanity_check(first, (char *) __func__);
  sanity_check(second, (char *) __func__);

  if (first->size_used)
    return (second);

  if (second->size_used)
    return (second);

  // ok, both blocks exist and are indeed free.


  // absorb the bytes
  first->size = first->size + second->size + sizeof(mblock_t);

  // link it
  first->next = second->next;
  if (second->next) {
    sanity_check(second->next, (char *) __func__);
    second->next->previous = first;
  }

  // yee-haw! we're done
  return (first);
}		     


//==============================================
// assumption: bp is a big empty block, with at least len bytes 
// action: we'd like to split it into two.  the first one should be able
// to hold at least len bytes.  
// when we're done, bp can hold len bytes.

void
split_free_block(mblock_t *bp, u_long len) {

  mblock_t  *second;
  
  len = WORDROUND(len);

  sanity_check(bp, (char *) __func__);

  if (bp->size_used) {
    TracePrintf(2, "heap problem: splitting nonempty block\n");
    return;
  }

  if (bp->size < len) {
    TracePrintf(2, "heap problem: splitting block that's too small\n");
    return;
  }

  // don't bother splitting
  if (bp->size < (len + MINBLOCK) )
    return;

  //ok, split it off

  second = (mblock_t *)(  ((void *)bp) + sizeof(mblock_t) + len );
  second->magic1 = MAGIC;
  second->size = bp->size - len - sizeof(mblock_t);
  bp->size = len;
  second->size_used = 0;
  second->previous = bp;
  second->next = bp->next;
  bp->next = second;
  if (second->next)
    second->next->previous = second;
  second->magic2 = MAGIC;
}


//====================
void
garbage_collect(void) {

  mblock_t *bp,*np;;

  bp = ((mblock_t *) heap_start); 

  while(bp) {
    np = merge_free_pair(bp);
    bp = np;
  }
}

//==============================

void
free(void *addr) {


  mblock_t *bp;

  ENTER;

  lkra = (unsigned int)(__builtin_return_address(0));

  if (NULL == addr)
    return;

  TracePrintf(8,"| %s HEAP my_free(0x%x)\n", WHOSE_HEAP, addr);

  if (not_initialized) {
    TracePrintf(8, "%s HEAP unitialized!\n", WHOSE_HEAP);
    if (USER_LIB)
      init_heap();
    else {
      LEAVE;
      lkra = 0;
      return;
    }
  }



  for (bp = ((mblock_t *) heap_start); bp; bp = bp->next) {
    sanity_check(bp, (char *) __func__);

    TracePrintf(8,"| %s HEAP free locking at block %x\n", WHOSE_HEAP, bp);
    TracePrintf(8,"| block start %x, block end %x\n", BLOCK_START(bp), BLOCK_END(bp));

    if ( (BLOCK_START(bp) <= addr) && (addr < BLOCK_END(bp)) ) {

      TracePrintf(8,"| %s HEAP found the block\n",WHOSE_HEAP);
      
      if ( 0 == bp->size_used) {
	TracePrintf(0,"| %s HEAP WARNING: free(%x) was already free\n",WHOSE_HEAP,addr);
	helper_maybort( (char *) __func__);
	LEAVE;
	lkra = 0;
	return;
      }

      // mark this block as free
      bp->size_used = 0;

      // garbage collect
      garbage_collect();
      LEAVE;
      lkra = 0;
      return;
    }
  }

  TracePrintf(0,"| %s HEAP WARNING: free(%x): could not find the block\n",WHOSE_HEAP,addr);
  TracePrintf(8,"| %s HEAP heap %x to %x\n", WHOSE_HEAP, bp, heap_start, heap_end);
  helper_maybort( (char *) __func__);
  LEAVE;
  lkra = 0;
}

//========================
void *
malloc(size_t len) {


  mblock_t *bp, *last;

  ENTER;
  lkra = (unsigned int)(__builtin_return_address(0));
  
  TracePrintf(8,"| %s HEAP my_malloc(0x%x)\n", WHOSE_HEAP,len);

  if ( ((int)len) < 0) {
    LEAVE;
    lkra = 0;
    return NULL;
  }

  // increment by 4, to add slop space...and to keep a region from having len 0
  // (because len 0 regions cannot be freed, given how free does its containment test)
  len = len + 4;

  len = WORDROUND(len);

  TracePrintf(8,"| rounded to %x\n", len);
  
  if (not_initialized) {
    TracePrintf(8, "%s HEAP unitialized!\n",WHOSE_HEAP);
    if (USER_LIB)
      init_heap();
    else  {
      LEAVE;
      lkra = 0;
      return NULL;
    }
  }

  for (bp = (mblock_t *)heap_start; bp; bp = bp->next) {
    
    sanity_check(bp, (char *) __func__);
    if ( (0 == bp->size_used) && (len <= bp->size) ) {
      TracePrintf(10,"| %s HEAP found a block %x\n",WHOSE_HEAP,bp);
      split_free_block(bp,len);
      bp->size_used = len;
      TracePrintf(8,"| %s HEAP block %x, heap %x to %x\n",WHOSE_HEAP, bp, heap_start, heap_end); 
      LEAVE;
      lkra = 0;
      return BLOCK_START(bp);
    }

    last = bp;

  }

  TracePrintf(8,"| %s HEAP not enough space, so going to try to grow\n",WHOSE_HEAP);

  if (NULL == last) {
    TracePrintf(8,"| %s HEAP: no last block, oddly\n",WHOSE_HEAP);
    LEAVE;
    lkra = 0;
    return NULL;
  }

  // grow heap
  bp = grow_heap(len);

  if (NULL == bp) {
    TracePrintf(1,"| %s HEAP: grow heap failed\n",WHOSE_HEAP);
    LEAVE;
    lkra = 0;
    return NULL;
  }

  last->next = bp;
  bp->previous = last;

  // merge the new block with the last one, in case the last one had been
  // free tpp
  bp = merge_free_pair(last);

  if ( (NULL == bp) || (bp->size_used) || (bp->size < len) ) {
    TracePrintf(8,"| %s HEAP: something odd happened with the merge\n",WHOSE_HEAP);
    LEAVE;
    lkra = 0;
    return NULL;
  }

  split_free_block(bp,len);
  bp->size_used = len;

  LEAVE;
  lkra = 0;
  TracePrintf(8,"| %s HEAP block %x, heap %x to %x\n",WHOSE_HEAP, bp, heap_start, heap_end); 
  return BLOCK_START(bp);

}



void *calloc(size_t nmemb, size_t size) {

  void *rp;


  rp = malloc (nmemb * size);

  if (rp)
    memset(rp, 0x00, nmemb*size);


  return rp;

}


void *realloc(void *ptr, size_t size) {


  void *rp;
  mblock_t *bp,*np;
  size_t copied_size;
  

  TracePrintf(8,"| %s HEAP realloc\n",WHOSE_HEAP);


  if (0 == size) {
    free(ptr);
    return NULL;
  }

  if (0 == ptr) {
    return (malloc(size));
  }


  // not the best implementaiton


  // find the old block

  if (not_initialized) {
    TracePrintf(8, "%s HEAP unitialized\n", WHOSE_HEAP);
    if (USER_LIB)
      init_heap();
    else return NULL;
  }

  
  for (bp = ((mblock_t *) heap_start); bp; bp = bp->next) {
    sanity_check(bp, (char *) __func__);

    if (BLOCK_START(bp) == ptr) 
      break;
  }
  
  if (NULL == bp) {
    TracePrintf(0,"| %s HEAP WARNING: realloc %x could not find the block\n",WHOSE_HEAP, (unsigned int) ptr);
    helper_maybort( (char *) __func__);
    return NULL;
  }



  // easy case: can we fit within this block?
  if (size  <=  bp->size ) {
    bp->size_used = size;
    return (ptr);
  }

  // hard case: we have to grow

  // can we do it in place?
  np = bp->next;
  if (np && (0 == np->size_used) && (size <= bp->size + np->size + sizeof(mblock_t))) {
    
    bp->next = np->next;
    bp->size = bp->size + np->size + sizeof(mblock_t);
    bp->size_used = size;
    if (np->next) 
      np->next->previous = bp;

    return ptr;
  }


  // ok, we have to do it the hard way!


  rp = malloc(size);
  if (NULL == rp)
    return NULL;


  memcpy(rp,ptr,bp->size_used);

  free(ptr);

  return rp;

}
