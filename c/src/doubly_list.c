#include "doubly_list.h"

#include <stdio.h>
#include <stdlib.h>

void dlist_init(DList *l) {
    l->sentinel.prev = l->sentinel.next = &l->sentinel;
    l->size = 0;
}

/* Insert `n` between two adjacent nodes. The one primitive everything uses. */
static void link_between(DNode *n, DNode *before, DNode *after) {
    n->prev = before;  n->next = after;
    before->next = n;  after->prev = n;
}

/* Unlink `n` in O(1) -- no traversal needed, because n knows both neighbours. */
static void unlink_node(DNode *n) {
    n->prev->next = n->next;
    n->next->prev = n->prev;
}

DNode *dlist_push_front(DList *l, int data) {
    DNode *n = malloc(sizeof *n);
    if (!n) return NULL;
    n->data = data;
    link_between(n, &l->sentinel, l->sentinel.next);
    l->size++;
    return n;
}

DNode *dlist_push_back(DList *l, int data) {
    DNode *n = malloc(sizeof *n);
    if (!n) return NULL;
    n->data = data;
    link_between(n, l->sentinel.prev, &l->sentinel);
    l->size++;
    return n;
}

void dlist_remove(DList *l, DNode *n) {          /* O(1) given the node */
    unlink_node(n);
    free(n);
    l->size--;
}

bool dlist_pop_front(DList *l, int *out) {
    if (l->size == 0) return false;
    DNode *n = l->sentinel.next;
    if (out) *out = n->data;
    dlist_remove(l, n);
    return true;
}

bool dlist_pop_back(DList *l, int *out) {
    if (l->size == 0) return false;
    DNode *n = l->sentinel.prev;
    if (out) *out = n->data;
    dlist_remove(l, n);
    return true;
}

/* Move an existing node to the front without any malloc/free (LRU trick). */
void dlist_move_to_front(DList *l, DNode *n) {
    unlink_node(n);
    link_between(n, &l->sentinel, l->sentinel.next);
}

void dlist_free(DList *l) {
    DNode *cur = l->sentinel.next;
    while (cur != &l->sentinel) {
        DNode *next = cur->next;
        free(cur);
        cur = next;
    }
    dlist_init(l);
}

void dlist_print_forward(const DList *l) {
    printf("[");
    for (const DNode *c = l->sentinel.next; c != &l->sentinel; c = c->next)
        printf("%d%s", c->data, c->next == &l->sentinel ? "" : " <-> ");
    printf("]\n");
}
