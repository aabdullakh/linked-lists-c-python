/* Demo of the singly and doubly linked lists. Run with: make run */
#include <stdio.h>

#include "doubly_list.h"
#include "singly_list.h"

int main(void) {
    puts("== Singly linked list ==");
    List l;
    list_init(&l);

    list_push_back(&l, 20);
    list_push_back(&l, 30);
    list_push_front(&l, 10);      /* 10 -> 20 -> 30 */
    list_insert_at(&l, 2, 25);    /* 10 -> 20 -> 25 -> 30 */
    list_print(&l);

    list_remove_value(&l, 20);    /* 10 -> 25 -> 30 */
    list_reverse(&l);             /* 30 -> 25 -> 10 */
    list_print(&l);

    printf("size=%zu middle=%d\n", l.size, list_middle(&l)->data);
    list_free(&l);                /* ALWAYS free what you malloc */

    puts("\n== Doubly linked list (with sentinel) ==");
    DList d;
    dlist_init(&d);
    DNode *a = dlist_push_back(&d, 1);
    dlist_push_back(&d, 2);
    DNode *c = dlist_push_back(&d, 3);
    dlist_push_front(&d, 0);      /* 0 <-> 1 <-> 2 <-> 3 */
    dlist_print_forward(&d);

    dlist_remove(&d, a);          /* O(1): we hold the node */
    dlist_move_to_front(&d, c);   /* 3 <-> 0 <-> 2 */
    dlist_print_forward(&d);
    dlist_free(&d);

    return 0;
}
