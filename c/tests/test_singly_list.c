#include <stdlib.h>
#include <string.h>

#include "singly_list.h"
#include "unity.h"

static List l;

void setUp(void)    { list_init(&l); }
void tearDown(void) { list_free(&l); }

/* ---------- helpers ---------- */

/* Verify size, element order, and that head/tail are consistent. */
static void assert_list(const List *list, const int *expected, size_t n) {
    TEST_ASSERT_EQUAL_UINT(n, list->size);
    size_t i = 0;
    const Node *last = NULL;
    for (const Node *c = list->head; c; c = c->next) {
        TEST_ASSERT_TRUE_MESSAGE(i < n, "list has more nodes than expected");
        TEST_ASSERT_EQUAL_INT(expected[i], c->data);
        last = c;
        i++;
    }
    TEST_ASSERT_EQUAL_UINT(n, i);
    TEST_ASSERT_EQUAL_PTR(last, list->tail);   /* NULL == NULL when empty */
    if (n == 0) TEST_ASSERT_NULL(list->head);
}
#define ASSERT_LIST(list, ...) do { int e_[] = {__VA_ARGS__}; \
    assert_list((list), e_, sizeof e_ / sizeof e_[0]); } while (0)
#define ASSERT_EMPTY(list) assert_list((list), NULL, 0)

/* Build a standalone chain (no List wrapper) from an array. */
static Node *chain_from(const int *v, size_t n) {
    Node *head = NULL, **link = &head;
    for (size_t i = 0; i < n; i++) {
        Node *nd = malloc(sizeof *nd);
        TEST_ASSERT_NOT_NULL(nd);
        nd->data = v[i];
        nd->next = NULL;
        *link = nd;
        link = &nd->next;
    }
    return head;
}
static void chain_free(Node *h) {
    while (h) { Node *n = h->next; free(h); h = n; }
}
static void assert_chain(const Node *h, const int *expected, size_t n) {
    size_t i = 0;
    for (; h; h = h->next, i++) {
        TEST_ASSERT_TRUE_MESSAGE(i < n, "chain longer than expected");
        TEST_ASSERT_EQUAL_INT(expected[i], h->data);
    }
    TEST_ASSERT_EQUAL_UINT(n, i);
}

/* ---------- init / free ---------- */

void test_init_makes_empty_list(void) {
    ASSERT_EMPTY(&l);
}

void test_free_on_empty_list_is_safe(void) {
    list_free(&l);
    list_free(&l);              /* twice: still safe */
    ASSERT_EMPTY(&l);
}

void test_free_resets_list_to_empty_and_reusable(void) {
    list_push_back(&l, 1);
    list_push_back(&l, 2);
    list_free(&l);
    ASSERT_EMPTY(&l);
    TEST_ASSERT_TRUE(list_push_back(&l, 7));
    ASSERT_LIST(&l, 7);
}

/* ---------- push_front / push_back ---------- */

void test_push_front_on_empty_sets_head_and_tail(void) {
    TEST_ASSERT_TRUE(list_push_front(&l, 5));
    ASSERT_LIST(&l, 5);
    TEST_ASSERT_EQUAL_PTR(l.head, l.tail);
}

void test_push_back_on_empty_sets_head_and_tail(void) {
    TEST_ASSERT_TRUE(list_push_back(&l, 5));
    ASSERT_LIST(&l, 5);
    TEST_ASSERT_EQUAL_PTR(l.head, l.tail);
}

void test_push_front_reverses_insertion_order(void) {
    for (int i = 1; i <= 4; i++) list_push_front(&l, i);
    ASSERT_LIST(&l, 4, 3, 2, 1);
}

void test_push_back_keeps_insertion_order(void) {
    for (int i = 1; i <= 4; i++) list_push_back(&l, i);
    ASSERT_LIST(&l, 1, 2, 3, 4);
}

void test_push_front_and_back_mix(void) {
    list_push_back(&l, 2);
    list_push_front(&l, 1);
    list_push_back(&l, 3);
    ASSERT_LIST(&l, 1, 2, 3);
}

/* ---------- insert_at ---------- */

void test_insert_at_zero_on_empty(void) {
    TEST_ASSERT_TRUE(list_insert_at(&l, 0, 9));
    ASSERT_LIST(&l, 9);
}

void test_insert_at_front_middle_back(void) {
    list_push_back(&l, 10);
    list_push_back(&l, 30);
    TEST_ASSERT_TRUE(list_insert_at(&l, 1, 20));   /* middle */
    ASSERT_LIST(&l, 10, 20, 30);
    TEST_ASSERT_TRUE(list_insert_at(&l, 0, 5));    /* front  */
    ASSERT_LIST(&l, 5, 10, 20, 30);
    TEST_ASSERT_TRUE(list_insert_at(&l, 4, 40));   /* back == size */
    ASSERT_LIST(&l, 5, 10, 20, 30, 40);
    TEST_ASSERT_EQUAL_INT(40, l.tail->data);
}

void test_insert_at_out_of_range_fails_and_changes_nothing(void) {
    list_push_back(&l, 1);
    TEST_ASSERT_FALSE(list_insert_at(&l, 2, 99));
    ASSERT_LIST(&l, 1);
    TEST_ASSERT_FALSE(list_insert_at(&l, (size_t)-1, 99));
    ASSERT_LIST(&l, 1);
}

/* ---------- pop_front ---------- */

void test_pop_front_returns_values_in_order_and_empties(void) {
    for (int i = 1; i <= 3; i++) list_push_back(&l, i);
    int v = 0;
    TEST_ASSERT_TRUE(list_pop_front(&l, &v)); TEST_ASSERT_EQUAL_INT(1, v);
    TEST_ASSERT_TRUE(list_pop_front(&l, &v)); TEST_ASSERT_EQUAL_INT(2, v);
    ASSERT_LIST(&l, 3);
    TEST_ASSERT_TRUE(list_pop_front(&l, &v)); TEST_ASSERT_EQUAL_INT(3, v);
    ASSERT_EMPTY(&l);               /* head AND tail reset to NULL */
}

void test_pop_front_on_empty_returns_false_and_leaves_out_untouched(void) {
    int v = 1234;
    TEST_ASSERT_FALSE(list_pop_front(&l, &v));
    TEST_ASSERT_EQUAL_INT(1234, v);
}

void test_pop_front_accepts_null_out(void) {
    list_push_back(&l, 1);
    list_push_back(&l, 2);
    TEST_ASSERT_TRUE(list_pop_front(&l, NULL));
    ASSERT_LIST(&l, 2);
}

void test_push_back_works_after_list_was_emptied_by_pop(void) {
    list_push_back(&l, 1);
    list_pop_front(&l, NULL);
    list_push_back(&l, 2);
    ASSERT_LIST(&l, 2);
}

/* ---------- remove_value ---------- */

void test_remove_value_head(void) {
    for (int i = 1; i <= 3; i++) list_push_back(&l, i);
    TEST_ASSERT_TRUE(list_remove_value(&l, 1));
    ASSERT_LIST(&l, 2, 3);
}

void test_remove_value_middle(void) {
    for (int i = 1; i <= 3; i++) list_push_back(&l, i);
    TEST_ASSERT_TRUE(list_remove_value(&l, 2));
    ASSERT_LIST(&l, 1, 3);
}

void test_remove_value_tail_updates_tail(void) {
    for (int i = 1; i <= 3; i++) list_push_back(&l, i);
    TEST_ASSERT_TRUE(list_remove_value(&l, 3));
    ASSERT_LIST(&l, 1, 2);
    TEST_ASSERT_TRUE(list_push_back(&l, 4));       /* proves tail is valid */
    ASSERT_LIST(&l, 1, 2, 4);
}

void test_remove_value_only_element(void) {
    list_push_back(&l, 1);
    TEST_ASSERT_TRUE(list_remove_value(&l, 1));
    ASSERT_EMPTY(&l);
}

void test_remove_value_missing_or_empty_returns_false(void) {
    TEST_ASSERT_FALSE(list_remove_value(&l, 1));
    list_push_back(&l, 1);
    TEST_ASSERT_FALSE(list_remove_value(&l, 2));
    ASSERT_LIST(&l, 1);
}

void test_remove_value_removes_only_first_duplicate(void) {
    list_push_back(&l, 7);
    list_push_back(&l, 7);
    list_push_back(&l, 7);
    TEST_ASSERT_TRUE(list_remove_value(&l, 7));
    ASSERT_LIST(&l, 7, 7);
}

/* ---------- find ---------- */

void test_find_hit_and_miss(void) {
    TEST_ASSERT_NULL(list_find(&l, 1));            /* empty */
    for (int i = 1; i <= 3; i++) list_push_back(&l, i);
    Node *n = list_find(&l, 2);
    TEST_ASSERT_NOT_NULL(n);
    TEST_ASSERT_EQUAL_INT(2, n->data);
    TEST_ASSERT_EQUAL_PTR(l.head->next, n);
    TEST_ASSERT_NULL(list_find(&l, 99));
}

/* ---------- reverse ---------- */

void test_reverse_empty_and_single(void) {
    list_reverse(&l);
    ASSERT_EMPTY(&l);
    list_push_back(&l, 1);
    list_reverse(&l);
    ASSERT_LIST(&l, 1);
}

void test_reverse_many_updates_head_and_tail(void) {
    for (int i = 1; i <= 5; i++) list_push_back(&l, i);
    list_reverse(&l);
    ASSERT_LIST(&l, 5, 4, 3, 2, 1);
    TEST_ASSERT_TRUE(list_push_back(&l, 0));       /* tail still valid */
    ASSERT_LIST(&l, 5, 4, 3, 2, 1, 0);
}

void test_reverse_twice_restores_original(void) {
    for (int i = 1; i <= 4; i++) list_push_back(&l, i);
    list_reverse(&l);
    list_reverse(&l);
    ASSERT_LIST(&l, 1, 2, 3, 4);
}

/* ---------- middle ---------- */

void test_middle_empty_is_null(void) {
    TEST_ASSERT_NULL(list_middle(&l));
}

void test_middle_single_odd_and_even(void) {
    list_push_back(&l, 1);
    TEST_ASSERT_EQUAL_INT(1, list_middle(&l)->data);
    list_push_back(&l, 2);
    list_push_back(&l, 3);
    TEST_ASSERT_EQUAL_INT(2, list_middle(&l)->data);          /* odd  */
    list_push_back(&l, 4);
    TEST_ASSERT_EQUAL_INT(3, list_middle(&l)->data);          /* even: second middle */
}

/* ---------- has_cycle ---------- */

void test_has_cycle_null_and_single_node(void) {
    TEST_ASSERT_FALSE(has_cycle(NULL));
    list_push_back(&l, 1);
    TEST_ASSERT_FALSE(has_cycle(l.head));
}

void test_has_cycle_self_loop(void) {
    list_push_back(&l, 1);
    l.head->next = l.head;
    TEST_ASSERT_TRUE(has_cycle(l.head));
    l.head->next = NULL;                           /* restore so free terminates */
}

void test_has_cycle_loop_in_the_middle(void) {
    for (int i = 1; i <= 6; i++) list_push_back(&l, i);
    TEST_ASSERT_FALSE(has_cycle(l.head));
    l.tail->next = l.head->next->next;             /* 6 -> 3 */
    TEST_ASSERT_TRUE(has_cycle(l.head));
    l.tail->next = NULL;                           /* restore so free terminates */
    TEST_ASSERT_FALSE(has_cycle(l.head));
}

/* ---------- merge_sorted ---------- */

void test_merge_sorted_interleaved(void) {
    int a[] = {1, 3, 5, 7}, b[] = {2, 4, 6, 8};
    Node *m = merge_sorted(chain_from(a, 4), chain_from(b, 4));
    int e[] = {1, 2, 3, 4, 5, 6, 7, 8};
    assert_chain(m, e, 8);
    chain_free(m);
}

void test_merge_sorted_different_lengths_and_duplicates(void) {
    int a[] = {1, 1, 5}, b[] = {1, 2, 2, 9, 10};
    Node *m = merge_sorted(chain_from(a, 3), chain_from(b, 5));
    int e[] = {1, 1, 1, 2, 2, 5, 9, 10};
    assert_chain(m, e, 8);
    chain_free(m);
}

void test_merge_sorted_with_empty_sides(void) {
    int a[] = {1, 2, 3};
    Node *m = merge_sorted(chain_from(a, 3), NULL);
    assert_chain(m, a, 3);
    chain_free(m);

    m = merge_sorted(NULL, chain_from(a, 3));
    assert_chain(m, a, 3);
    chain_free(m);

    TEST_ASSERT_NULL(merge_sorted(NULL, NULL));
}

void test_merge_sorted_all_of_one_list_before_the_other(void) {
    int a[] = {1, 2}, b[] = {3, 4};
    Node *m = merge_sorted(chain_from(b, 2), chain_from(a, 2));
    int e[] = {1, 2, 3, 4};
    assert_chain(m, e, 4);
    chain_free(m);
}

/* ---------- randomized check against a plain array (oracle) ---------- */

static unsigned long long rng = 88172645463325252ULL;
static unsigned rnd(unsigned bound) {
    rng ^= rng << 13; rng ^= rng >> 7; rng ^= rng << 17;
    return (unsigned)(rng % bound);
}

void test_random_operations_match_array_model(void) {
    enum { CAP = 512 };
    int model[CAP];
    size_t n = 0;

    for (int step = 0; step < 4000; step++) {
        switch (rnd(5)) {
        case 0:                                             /* push_front */
            if (n < CAP) {
                int v = (int)rnd(100);
                memmove(model + 1, model, n * sizeof *model);
                model[0] = v; n++;
                TEST_ASSERT_TRUE(list_push_front(&l, v));
            }
            break;
        case 1:                                             /* push_back */
            if (n < CAP) {
                int v = (int)rnd(100);
                model[n++] = v;
                TEST_ASSERT_TRUE(list_push_back(&l, v));
            }
            break;
        case 2:                                             /* insert_at */
            if (n < CAP) {
                size_t idx = rnd((unsigned)n + 1);
                int v = (int)rnd(100);
                memmove(model + idx + 1, model + idx, (n - idx) * sizeof *model);
                model[idx] = v; n++;
                TEST_ASSERT_TRUE(list_insert_at(&l, idx, v));
            }
            break;
        case 3:                                             /* pop_front */
            if (n > 0) {
                int out = -1;
                TEST_ASSERT_TRUE(list_pop_front(&l, &out));
                TEST_ASSERT_EQUAL_INT(model[0], out);
                memmove(model, model + 1, (n - 1) * sizeof *model);
                n--;
            } else {
                TEST_ASSERT_FALSE(list_pop_front(&l, NULL));
            }
            break;
        default: {                                          /* remove_value */
            int v = (int)rnd(100);
            size_t at = n;
            for (size_t i = 0; i < n; i++) if (model[i] == v) { at = i; break; }
            if (at < n) {
                memmove(model + at, model + at + 1, (n - at - 1) * sizeof *model);
                n--;
                TEST_ASSERT_TRUE(list_remove_value(&l, v));
            } else {
                TEST_ASSERT_FALSE(list_remove_value(&l, v));
            }
        }
        }
        assert_list(&l, model, n);
    }
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_init_makes_empty_list);
    RUN_TEST(test_free_on_empty_list_is_safe);
    RUN_TEST(test_free_resets_list_to_empty_and_reusable);
    RUN_TEST(test_push_front_on_empty_sets_head_and_tail);
    RUN_TEST(test_push_back_on_empty_sets_head_and_tail);
    RUN_TEST(test_push_front_reverses_insertion_order);
    RUN_TEST(test_push_back_keeps_insertion_order);
    RUN_TEST(test_push_front_and_back_mix);
    RUN_TEST(test_insert_at_zero_on_empty);
    RUN_TEST(test_insert_at_front_middle_back);
    RUN_TEST(test_insert_at_out_of_range_fails_and_changes_nothing);
    RUN_TEST(test_pop_front_returns_values_in_order_and_empties);
    RUN_TEST(test_pop_front_on_empty_returns_false_and_leaves_out_untouched);
    RUN_TEST(test_pop_front_accepts_null_out);
    RUN_TEST(test_push_back_works_after_list_was_emptied_by_pop);
    RUN_TEST(test_remove_value_head);
    RUN_TEST(test_remove_value_middle);
    RUN_TEST(test_remove_value_tail_updates_tail);
    RUN_TEST(test_remove_value_only_element);
    RUN_TEST(test_remove_value_missing_or_empty_returns_false);
    RUN_TEST(test_remove_value_removes_only_first_duplicate);
    RUN_TEST(test_find_hit_and_miss);
    RUN_TEST(test_reverse_empty_and_single);
    RUN_TEST(test_reverse_many_updates_head_and_tail);
    RUN_TEST(test_reverse_twice_restores_original);
    RUN_TEST(test_middle_empty_is_null);
    RUN_TEST(test_middle_single_odd_and_even);
    RUN_TEST(test_has_cycle_null_and_single_node);
    RUN_TEST(test_has_cycle_self_loop);
    RUN_TEST(test_has_cycle_loop_in_the_middle);
    RUN_TEST(test_merge_sorted_interleaved);
    RUN_TEST(test_merge_sorted_different_lengths_and_duplicates);
    RUN_TEST(test_merge_sorted_with_empty_sides);
    RUN_TEST(test_merge_sorted_all_of_one_list_before_the_other);
    RUN_TEST(test_random_operations_match_array_model);
    return UNITY_END();
}
