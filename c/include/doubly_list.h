#ifndef DOUBLY_LIST_H
#define DOUBLY_LIST_H

#include <stdbool.h>
#include <stddef.h>

typedef struct DNode {
    int data;
    struct DNode *prev, *next;
} DNode;

/* Circular list with a SENTINEL node: sentinel.next is the first element,
   sentinel.prev is the last. An empty list is a sentinel pointing at itself.
   Result: insert/remove never need NULL checks or head/tail special cases. */
typedef struct {
    DNode  sentinel;
    size_t size;
} DList;

void   dlist_init(DList *l);
void   dlist_free(DList *l);

DNode *dlist_push_front(DList *l, int data);   /* O(1); returns the new node (handle) or NULL */
DNode *dlist_push_back(DList *l, int data);    /* O(1) */

void   dlist_remove(DList *l, DNode *n);       /* O(1) given the node */
bool   dlist_pop_front(DList *l, int *out);    /* O(1); out may be NULL */
bool   dlist_pop_back(DList *l, int *out);     /* O(1); out may be NULL */
void   dlist_move_to_front(DList *l, DNode *n);/* O(1), no malloc/free (LRU trick) */

void   dlist_print_forward(const DList *l);

#endif /* DOUBLY_LIST_H */
