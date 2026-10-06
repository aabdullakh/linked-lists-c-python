#include "singly_list.h"

#include <stdio.h>
#include <stdlib.h>

/* ---------- Lifecycle ---------- */
void list_init(List *l) { l->head = l->tail = NULL; l->size = 0; }

void list_free(List *l) {
    Node *cur = l->head;
    while (cur) {
        Node *next = cur->next;   /* save BEFORE freeing */
        free(cur);
        cur = next;
    }
    list_init(l);
}

static Node *node_new(int data) {
    Node *n = malloc(sizeof *n);
    if (!n) return NULL;          /* caller must check */
    n->data = data;
    n->next = NULL;
    return n;
}

/* ---------- Insertion ---------- */
bool list_push_front(List *l, int data) {          /* O(1) */
    Node *n = node_new(data);
    if (!n) return false;
    n->next = l->head;            /* 1. new node points at old head */
    l->head = n;                  /* 2. head points at new node     */
    if (!l->tail) l->tail = n;    /* list was empty                 */
    l->size++;
    return true;
}

bool list_push_back(List *l, int data) {           /* O(1) thanks to tail */
    Node *n = node_new(data);
    if (!n) return false;
    if (l->tail) l->tail->next = n; else l->head = n;
    l->tail = n;
    l->size++;
    return true;
}

/* Insert so that the new node ends up at position `index` (0-based). O(index) */
bool list_insert_at(List *l, size_t index, int data) {
    if (index > l->size) return false;
    if (index == 0)        return list_push_front(l, data);
    if (index == l->size)  return list_push_back(l, data);
    Node *prev = l->head;
    for (size_t i = 0; i < index - 1; i++) prev = prev->next;
    Node *n = node_new(data);
    if (!n) return false;
    n->next = prev->next;         /* 1. connect new node FORWARD first */
    prev->next = n;               /* 2. then splice it in              */
    l->size++;
    return true;
}

/* ---------- Deletion ---------- */
bool list_pop_front(List *l, int *out) {           /* O(1) */
    if (!l->head) return false;
    Node *old = l->head;
    if (out) *out = old->data;
    l->head = old->next;
    if (!l->head) l->tail = NULL;
    free(old);
    l->size--;
    return true;
}

/* Remove first node holding `value`. O(n). Uses a pointer-to-pointer so the
   head needs no special case. */
bool list_remove_value(List *l, int value) {
    Node **link = &l->head;       /* address of the pointer that points at cur */
    Node  *prev = NULL;
    while (*link && (*link)->data != value) {
        prev = *link;
        link = &(*link)->next;
    }
    if (!*link) return false;
    Node *victim = *link;
    *link = victim->next;         /* bypass victim */
    if (victim == l->tail) l->tail = prev;
    free(victim);
    l->size--;
    return true;
}

/* ---------- Search / access ---------- */
Node *list_find(const List *l, int value) {        /* O(n) */
    for (Node *cur = l->head; cur; cur = cur->next)
        if (cur->data == value) return cur;
    return NULL;
}

void list_print(const List *l) {
    for (Node *cur = l->head; cur; cur = cur->next)
        printf("%d -> ", cur->data);
    printf("NULL\n");
}

/* ---------- Classic algorithms ---------- */
void list_reverse(List *l) {                        /* O(n) time, O(1) space */
    Node *prev = NULL, *cur = l->head;
    l->tail = l->head;
    while (cur) {
        Node *next = cur->next;   /* 1. remember the rest      */
        cur->next = prev;         /* 2. flip this arrow        */
        prev = cur;               /* 3. advance prev           */
        cur = next;               /* 4. advance cur            */
    }
    l->head = prev;
}

Node *list_middle(const List *l) {                  /* slow/fast pointers */
    Node *slow = l->head, *fast = l->head;
    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
    }
    return slow;                  /* second middle if length is even */
}

/* Floyd's cycle detection on a raw node chain. O(n) time, O(1) space. */
bool has_cycle(const Node *head) {
    const Node *slow = head, *fast = head;
    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
        if (slow == fast) return true;
    }
    return false;
}

/* Merge two sorted chains into one (stable). Reuses nodes; O(n+m). */
Node *merge_sorted(Node *a, Node *b) {
    Node dummy = {0, NULL};       /* stack-allocated sentinel */
    Node *tail = &dummy;
    while (a && b) {
        if (a->data <= b->data) { tail->next = a; a = a->next; }
        else                    { tail->next = b; b = b->next; }
        tail = tail->next;
    }
    tail->next = a ? a : b;
    return dummy.next;
}
