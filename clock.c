#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

int clockarm;

/* Page to evict is chosen using the clock algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */

 //http://www.mathcs.emory.edu/~cheung/Courses/355/Syllabus/9-virtual-mem/SC-replace.html

int clock_evict() {
  int evictee = -1;
  // loop around the pages like a clock
  while(evictee == -1) {
    // check current page/clockarm
    // if bit is 0/false, evict that page
    if (coremap[clockarm].referenced == 0) {
      evictee = clockarm;
    } else {
      // it's 1/true
      // set the bit to 0 and continue
      coremap[clockarm].referenced = 0;
      clockarm = (clockarm + 1) % memsize;
    }
  }

  return clockarm;
}

/* This function is called on each access to a page to update any information
 * needed by the clock algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void clock_ref(pgtbl_entry_t *p) {
  // this function being called means the pte was referenced
  // must set to 1/true

  // get physical address and shift it
  int frame = (p->frame) >> PAGE_SHIFT;
  // store the timestamp
  coremap[frame].referenced = 1;
  return;
}

/* Initialize any data structures needed for this replacement
 * algorithm.
 */
void clock_init() {
  clockarm = 0;

  // for CLOCK, the second chance bit is initialzied to 0/false for all incoming pages
  int i;
  for (i = 0 ; i < memsize; i++) {
    coremap[i].referenced =  0;
  }
}
