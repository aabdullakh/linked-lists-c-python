#include <stdlib.h>

#include "doubly_list.h"
#include "unity.h"

static DList l;

void setUp(void)    { dlist_init(&l); }
void tearDown(void) { dlist_free(&l); }

/* Verify size, forward order, backward order, and prev/next symmetry. */
static void assert_dlist(const DList *list, const int *expected, size_t n) {
    TEST_ASSERT_EQUAL_UINT(n, list->size);

    size_t i = 0;
    for (const DNode *c = list->sentinel.next; c != &list->sentinel; c = c->next) {
        TEST_ASSERT_TRUE_MESSAGE(i < n, "forward walk longer than expected");
        TEST_ASSERT_EQUAL_INT(expected[i], c->data);
        TEST_ASSERT_EQUAL_PTR_MESSAGE(c, c->next->prev, "next->prev must point back");
        i++;
    }
    TEST_ASSERT_EQUAL_UINT(n, i);

    i = n;
    for (const DNode *c = list->sentinel.prev; c != &list->sentinel; c = c->prev) {
        TEST_ASSERT_TRUE_MESSAGE(i > 0, "backward walk longer than expected");
        i--;
        TEST_ASSERT_EQUAL_INT(expected[i], c->data);
    }
    TEST_ASSERT_EQUAL_UINT(0, i);
}
#define ASSERT_DLIST(list, ...) do { int e_[] = {__VA_ARGS__}; \
    assert_dlist((list), e_, sizeof e_ / sizeof e_[0]); } while (0)
#define ASSERT_EMPTY(list) assert_dlist((list), NULL, 0)

void test_init_makes_sentinel_point_at_itself(void) {
    ASSERT_EMPTY(&l);
    TEST_ASSERT_EQUAL_PTR(&l.sentinel, l.sentinel.next);
    TEST_ASSERT_EQUAL_PTR(&l.sentinel, l.sentinel.prev);
}

void test_free_on_empty_list_is_safe_and_idempotent(void) {
    dlist_free(&l);
    dlist_free(&l);
    ASSERT_EMPTY(&l);
}

void test_push_front_and_back_return_handles(void) {
    DNode *a = dlist_push_back(&l, 2);
    DNode *b = dlist_push_front(&l, 1);
    DNode *c = dlist_push_back(&l, 3);
    TEST_ASSERT_NOT_NULL(a); TEST_ASSERT_NOT_NULL(b); TEST_ASSERT_NOT_NULL(c);
    TEST_ASSERT_EQUAL_INT(2, a->data);
    TEST_ASSERT_EQUAL_INT(1, b->data);
    TEST_ASSERT_EQUAL_INT(3, c->data);
    ASSERT_DLIST(&l, 1, 2, 3);
}

void test_push_front_reverses_and_push_back_preserves_order(void) {
    for (int i = 1; i <= 3; i++) dlist_push_front(&l, i);
    ASSERT_DLIST(&l, 3, 2, 1);
    for (int i = 4; i <= 5; i++) dlist_push_back(&l, i);
    ASSERT_DLIST(&l, 3, 2, 1, 4, 5);
}

void test_single_element_is_both_first_and_last(void) {
    DNode *n = dlist_push_back(&l, 42);
    TEST_ASSERT_EQUAL_PTR(n, l.sentinel.next);
    TEST_ASSERT_EQUAL_PTR(n, l.sentinel.prev);
}

void test_remove_head_middle_tail_by_handle(void) {
    DNode *a = dlist_push_back(&l, 1);
    DNode *b = dlist_push_back(&l, 2);
    DNode *c = dlist_push_back(&l, 3);
    DNode *d = dlist_push_back(&l, 4);
    dlist_remove(&l, b);                    /* middle */
    ASSERT_DLIST(&l, 1, 3, 4);
    TEST_ASSERT_EQUAL_PTR(c, a->next);
    TEST_ASSERT_EQUAL_PTR(a, c->prev);
    dlist_remove(&l, a);                    /* head   */
    ASSERT_DLIST(&l, 3, 4);
    dlist_remove(&l, d);                    /* tail   */
    ASSERT_DLIST(&l, 3);
    dlist_remove(&l, c);                    /* only   */
    ASSERT_EMPTY(&l);
}

void test_pop_front_and_back(void) {
    for (int i = 1; i <= 4; i++) dlist_push_back(&l, i);
    int v = 0;
    TEST_ASSERT_TRUE(dlist_pop_front(&l, &v)); TEST_ASSERT_EQUAL_INT(1, v);
    TEST_ASSERT_TRUE(dlist_pop_back(&l, &v));  TEST_ASSERT_EQUAL_INT(4, v);
    ASSERT_DLIST(&l, 2, 3);
    TEST_ASSERT_TRUE(dlist_pop_back(&l, NULL));
    TEST_ASSERT_TRUE(dlist_pop_front(&l, NULL));
    ASSERT_EMPTY(&l);
}

void test_pop_on_empty_returns_false_and_leaves_out_untouched(void) {
    int v = 777;
    TEST_ASSERT_FALSE(dlist_pop_front(&l, &v));
    TEST_ASSERT_FALSE(dlist_pop_back(&l, &v));
    TEST_ASSERT_EQUAL_INT(777, v);
    ASSERT_EMPTY(&l);
}

void test_list_is_reusable_after_being_emptied(void) {
    dlist_push_back(&l, 1);
    dlist_pop_front(&l, NULL);
    dlist_push_back(&l, 2);
    dlist_push_front(&l, 1);
    ASSERT_DLIST(&l, 1, 2);
}

void test_move_to_front_from_back_middle_and_front(void) {
    DNode *a = dlist_push_back(&l, 1);
    DNode *b = dlist_push_back(&l, 2);
    DNode *c = dlist_push_back(&l, 3);
    dlist_move_to_front(&l, c);             /* from back   */
    ASSERT_DLIST(&l, 3, 1, 2);
    dlist_move_to_front(&l, a);             /* from middle */
    ASSERT_DLIST(&l, 1, 3, 2);
    dlist_move_to_front(&l, a);             /* already first: no-op */
    ASSERT_DLIST(&l, 1, 3, 2);
    dlist_move_to_front(&l, b);
    ASSERT_DLIST(&l, 2, 1, 3);
    TEST_ASSERT_EQUAL_UINT(3, l.size);      /* size unchanged by moves */
}

void test_move_to_front_on_single_element(void) {
    DNode *a = dlist_push_back(&l, 1);
    dlist_move_to_front(&l, a);
    ASSERT_DLIST(&l, 1);
}

void test_lru_style_usage_keeps_most_recent_first(void) {
    DNode *h[4];
    for (int i = 0; i < 4; i++) h[i] = dlist_push_front(&l, i);   /* 3 2 1 0 */
    dlist_move_to_front(&l, h[0]);                                /* 0 3 2 1 */
    int evicted = -1;
    TEST_ASSERT_TRUE(dlist_pop_back(&l, &evicted));               /* evict LRU */
    TEST_ASSERT_EQUAL_INT(1, evicted);
    ASSERT_DLIST(&l, 0, 3, 2);
}

void test_many_nodes(void) {
    for (int i = 0; i < 2000; i++) dlist_push_back(&l, i);
    TEST_ASSERT_EQUAL_UINT(2000, l.size);
    long sum = 0;
    for (const DNode *c = l.sentinel.next; c != &l.sentinel; c = c->next) sum += c->data;
    TEST_ASSERT_EQUAL_INT64(1999000L, sum);
    for (int i = 0; i < 1000; i++) dlist_pop_front(&l, NULL);
    TEST_ASSERT_EQUAL_UINT(1000, l.size);
    TEST_ASSERT_EQUAL_INT(1000, l.sentinel.next->data);
    TEST_ASSERT_EQUAL_INT(1999, l.sentinel.prev->data);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_init_makes_sentinel_point_at_itself);
    RUN_TEST(test_free_on_empty_list_is_safe_and_idempotent);
    RUN_TEST(test_push_front_and_back_return_handles);
    RUN_TEST(test_push_front_reverses_and_push_back_preserves_order);
    RUN_TEST(test_single_element_is_both_first_and_last);
    RUN_TEST(test_remove_head_middle_tail_by_handle);
    RUN_TEST(test_pop_front_and_back);
    RUN_TEST(test_pop_on_empty_returns_false_and_leaves_out_untouched);
    RUN_TEST(test_list_is_reusable_after_being_emptied);
    RUN_TEST(test_move_to_front_from_back_middle_and_front);
    RUN_TEST(test_move_to_front_on_single_element);
    RUN_TEST(test_lru_style_usage_keeps_most_recent_first);
    RUN_TEST(test_many_nodes);
    return UNITY_END();
}
