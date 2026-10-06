import pytest

from linked_lists import SinglyLinkedList, has_cycle, merge_sorted, remove_nth_from_end

from helpers import chain, to_list


# ---------------------------------------------------------------- has_cycle
def test_has_cycle_none_and_single_node():
    assert has_cycle(None) is False
    assert has_cycle(chain([1])) is False


def test_has_cycle_self_loop():
    head = chain([1])
    head.next = head
    assert has_cycle(head) is True


@pytest.mark.parametrize("n, back_to", [(2, 0), (6, 2), (10, 9), (10, 0)])
def test_has_cycle_detects_loops_of_many_shapes(n, back_to):
    head = chain(range(n))
    nodes = []
    cur = head
    while cur:
        nodes.append(cur)
        cur = cur.next
    assert has_cycle(head) is False
    nodes[-1].next = nodes[back_to]
    assert has_cycle(head) is True


def test_has_cycle_long_acyclic_chain():
    assert has_cycle(chain(range(10_000))) is False


# ------------------------------------------------------------- merge_sorted
def test_merge_interleaved():
    assert to_list(merge_sorted(chain([1, 3, 5, 7]), chain([2, 4, 6, 8]))) == list(range(1, 9))


def test_merge_different_lengths_and_duplicates():
    assert to_list(merge_sorted(chain([1, 1, 5]), chain([1, 2, 2, 9, 10]))) == [1, 1, 1, 2, 2, 5, 9, 10]


def test_merge_with_empty_sides():
    assert to_list(merge_sorted(chain([1, 2, 3]), None)) == [1, 2, 3]
    assert to_list(merge_sorted(None, chain([1, 2, 3]))) == [1, 2, 3]
    assert merge_sorted(None, None) is None


def test_merge_one_list_entirely_before_the_other():
    assert to_list(merge_sorted(chain([3, 4]), chain([1, 2]))) == [1, 2, 3, 4]


def test_merge_is_stable_left_list_wins_ties():
    class Keyed:
        """Orders by key only, so equal keys expose which list an item came from."""
        def __init__(self, key, tag): self.key, self.tag = key, tag
        def __le__(self, other): return self.key <= other.key

    left = chain([Keyed(1, "left"), Keyed(2, "left")])
    right = chain([Keyed(1, "right")])
    merged = to_list(merge_sorted(left, right))
    assert [(k.key, k.tag) for k in merged] == [(1, "left"), (1, "right"), (2, "left")]


def test_merge_reuses_nodes_instead_of_copying():
    a, b = chain([1, 3]), chain([2])
    a_second, b_first = a.next, b
    merged = merge_sorted(a, b)
    assert merged.next is b_first and b_first.next is a_second


# ------------------------------------------------------ remove_nth_from_end
@pytest.mark.parametrize(
    "items, n, expected",
    [
        ([1, 2, 3, 4, 5], 2, [1, 2, 3, 5]),
        ([1, 2, 3, 4, 5], 1, [1, 2, 3, 4]),     # remove the tail
        ([1, 2, 3, 4, 5], 5, [2, 3, 4, 5]),     # remove the head (dummy node earns its keep)
        ([1], 1, []),
        ([1, 2], 2, [2]),
        ([1, 2], 1, [1]),
    ],
)
def test_remove_nth_from_end(items, n, expected):
    assert to_list(remove_nth_from_end(chain(items), n)) == expected


# -------------------------------------------- algorithms agree with the class
def test_algorithms_work_on_a_linked_list_instance_chain():
    a, b = SinglyLinkedList([1, 4]), SinglyLinkedList([2, 3])
    assert to_list(merge_sorted(a.head, b.head)) == [1, 2, 3, 4]
