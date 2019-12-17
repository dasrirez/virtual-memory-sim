#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

unsigned long timestamp;

extern int debug;

extern struct frame *coremap;

/* Page to evict is chosen using the accurate LRU algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */

int lru_evict() {
  int i;
  int frame;
  unsigned long min = timestamp + 1;
  for (i = 0; i < memsize; i++) {
    if (coremap[i].timestamp < min) {
      frame = i;
      min = coremap[i].timestamp;
    }
  }
	return frame;
}

/* This function is called on each access to a page to update any information
 * needed by the lru algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void lru_ref(pgtbl_entry_t *p) {
  coremap[p->frame >> PAGE_SHIFT].timestamp = timestamp;
  timestamp = timestamp + 1;
	return;
}


/* Initialize any data structures needed for this 
 * replacement algorithm 
 */
void lru_init() {
  timestamp = 0;

  int i;
  for (i = 0; i < memsize; i++) {
    coremap[i].timestamp = timestamp;
    timestamp++;
  }
}
