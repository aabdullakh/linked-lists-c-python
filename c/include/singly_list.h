#ifndef SINGLY_LIST_H
#define SINGLY_LIST_H

#include <stdbool.h>
#include <stddef.h>

/* ---------- Data layout ---------- */
typedef struct Node {
    int data;
    struct Node *next;
} Node;

typedef struct {
    Node  *head;   /* first node, or NULL when empty   */
    Node  *tail;   /* last node: makes push_back O(1)  */
    size_t size;   /* cached length: makes size O(1)   */
} List;

/* Lifecycle */
void list_init(List *l);
void list_free(List *l);

/* Insertion */
bool list_push_front(List *l, int data);               /* O(1)       */
bool list_push_back(List *l, int data);                /* O(1)       */
bool list_insert_at(List *l, size_t index, int data);  /* O(index)   */

/* Deletion */
bool list_pop_front(List *l, int *out);                /* O(1); out may be NULL */
bool list_remove_value(List *l, int value);            /* O(n)       */

/* Search / inspection */
Node *list_find(const List *l, int value);             /* O(n)       */
void  list_print(const List *l);

/* Classic algorithms */
void  list_reverse(List *l);                           /* O(n) time, O(1) space */
Node *list_middle(const List *l);                      /* slow/fast pointers    */
bool  has_cycle(const Node *head);                     /* Floyd's algorithm     */
Node *merge_sorted(Node *a, Node *b);                  /* relinks nodes; O(n+m) */

#endif /* SINGLY_LIST_H */
