#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"

struct bst_node {
  int addr;
  int size;
  struct bst_node *left, *right;
};

void inorder(struct bst_node *root) {
  if (root != NULL) {
      inorder(root->left);
      printf("%d ", root->addr);
      inorder(root->right);
    }
}

struct bst_node *new_bst_node(int addr) {
  struct bst_node *root = (struct bst_node *)malloc(sizeof(struct bst_node));
  root->size = 1;
  root->addr = addr;
  root->left = root->right = NULL;
  return root;
}

struct bst_node *ins_bst_node(struct bst_node *root, int addr) {
  int size_before, size_after;
  if (root == NULL) root = new_bst_node(addr);
  else if (root->addr > addr) {
    size_before = root->left == NULL ? 0 : root->left->size;
    root->left = ins_bst_node(root->left, addr);
    size_after = root->left == NULL ? 0 : root->left->size;
    if (size_after > size_before)
      root->size = root->size + 1;
  }
  else if (root->addr < addr) {
    size_before = root->right == NULL ? 0 : root->right->size;
    root->right = ins_bst_node(root->right, addr);
    size_after = root->right == NULL ? 0 : root->right->size;
    if (size_after > size_before)
      root->size = root->size + 1;
  }
  return root;
}

struct bst_node *del_bst_node(struct bst_node *root, int addr) {
  int size_before, size_after;
  if (root == NULL) {
    return root;
  }
  else if (root->addr > addr) {
    size_before = root->left == NULL ? 0 : root->left->size;
    root->left = del_bst_node(root->left, addr);
    size_after = root->left == NULL ? 0 : root->left->size;
    if (size_after < size_before)
      root->size = root->size - 1;
  }
  else if (root->addr < addr) {
    size_before = root->right == NULL ? 0 : root->right->size;
    root->right = del_bst_node(root->right, addr);
    size_after = root->right == NULL ? 0 : root->right->size;
    if (size_after < size_before)
      root->size = root->size - 1;
  }
  else {
    if (root->left == NULL) {
      struct bst_node *temp = root->right;
      free(root);
      return temp;
    }
    else if (root->right == NULL) {
      struct bst_node *temp = root->left;
      free(root);
      return temp;
    }
    else {
      struct bst_node *temp = root->right;
      while (temp->left != NULL) {
        temp = temp->left;
      }
      root->addr = temp->addr;
      size_before = root->right == NULL ? 0 : root->right->size;
      root->right = del_bst_node(root->right, temp->addr);
      size_after = root->right == NULL ? 0 : root->right->size;
      if (size_after < size_before)
        root->size = root->size - 1;
    }
  }
  return root;
}

struct bst_node *clone_bst(struct bst_node *root) {
  if (root == NULL) return NULL;
  struct bst_node *temp = (struct bst_node *)malloc(sizeof(struct bst_node));
  temp->addr = root->addr;
  temp->size = root->size;
  temp->left = clone_bst(root->left);
  temp->right = clone_bst(root->right);
  return temp;
}

void wipe_bst(struct bst_node *root) {
  if (root->left) wipe_bst(root->left);
  if (root->right) wipe_bst(root->right);
  free(root);
}

extern char *tracefile;

extern int memsize;

extern int debug;

extern struct frame *coremap;

struct bst_node *tree;

unsigned long referenced;

/* Page to evict is chosen using the optimal (aka MIN) algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int opt_evict() {
  int cnum;
  int lnum;
  int addr;
  struct bst_node *temp = clone_bst(tree);
  int size_before;
  int size_after;

  FILE *file = fopen(tracefile, "r");
  if (file != NULL) {
    char line [128];
    lnum = 0;
    while (temp->size != 1 && (fgets(line, sizeof(line), file) != NULL)) {
      if (lnum > referenced) {
        // shift string left by 2
        cnum = 2;
        while(line[cnum] != '\0') {
          line[cnum - 2] = line[cnum];
          cnum = cnum + 1;
        }
        line[cnum - 2] = '\0';
        addr = (strtol(line, NULL, 16) >> PAGE_SHIFT);
        size_before = temp->size;
        temp = del_bst_node(temp, addr);
        size_after = temp->size;
      }
      lnum = lnum + 1;
    }
    fclose (file);
  } else {
    perror (tracefile);
    swap_destroy();
    exit(1);
  }
  addr = temp->addr;
  wipe_bst(temp);
  tree = del_bst_node(tree, addr);
  return addr;
}

/* This function is called on each access to a page to update any information
 * needed by the opt algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void opt_ref(pgtbl_entry_t *p) {
  referenced = referenced + 1;
  int addr = p->frame >> PAGE_SHIFT;
  tree = ins_bst_node(tree, addr);
}

/* size any data structures needed for this
 * replacement algorithm.
 */
void opt_init() {
  referenced = 0;
  tree = NULL;
}
