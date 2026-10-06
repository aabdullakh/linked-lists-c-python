import random
from collections import deque

import pytest

from linked_lists import Node, SinglyLinkedList


def assert_consistent(lst, expected):
    """Order, length, and head/tail invariants all agree with the model."""
    assert list(lst) == expected
    assert len(lst) == len(expected)
    if expected:
        assert lst.head.data == expected[0]
        assert lst.tail.data == expected[-1]
        assert lst.tail.next is None
    else:
        assert lst.head is None and lst.tail is None


# ---------------------------------------------------------------- construction
def test_new_list_is_empty():
    assert_consistent(SinglyLinkedList(), [])


def test_constructor_accepts_any_iterable():
    assert_consistent(SinglyLinkedList([1, 2, 3]), [1, 2, 3])
    assert_consistent(SinglyLinkedList(range(3)), [0, 1, 2])
    assert_consistent(SinglyLinkedList(x * 2 for x in range(3)), [0, 2, 4])


def test_node_uses_slots_so_no_instance_dict():
    n = Node(1)
    assert not hasattr(n, "__dict__")
    with pytest.raises(AttributeError):
        n.surprise = 1


# ------------------------------------------------------------------- push_*
def test_push_front_on_empty_sets_head_and_tail():
    l = SinglyLinkedList()
    l.push_front(5)
    assert_consistent(l, [5])
    assert l.head is l.tail


def test_push_back_on_empty_sets_head_and_tail():
    l = SinglyLinkedList()
    l.push_back(5)
    assert_consistent(l, [5])
    assert l.head is l.tail


def test_push_front_reverses_order_push_back_keeps_it():
    l = SinglyLinkedList()
    for i in (1, 2, 3):
        l.push_front(i)
    assert_consistent(l, [3, 2, 1])
    l.push_back(0)
    assert_consistent(l, [3, 2, 1, 0])


# ------------------------------------------------------------------- insert
@pytest.mark.parametrize(
    "index, expected",
    [(0, [9, 1, 2, 3]), (1, [1, 9, 2, 3]), (2, [1, 2, 9, 3]), (3, [1, 2, 3, 9])],
)
def test_insert_at_every_valid_position(index, expected):
    l = SinglyLinkedList([1, 2, 3])
    l.insert(index, 9)
    assert_consistent(l, expected)


def test_insert_into_empty_list_at_zero():
    l = SinglyLinkedList()
    l.insert(0, 1)
    assert_consistent(l, [1])


@pytest.mark.parametrize("bad", [-1, 4, 100])
def test_insert_out_of_range_raises_and_changes_nothing(bad):
    l = SinglyLinkedList([1, 2, 3])
    with pytest.raises(IndexError):
        l.insert(bad, 9)
    assert_consistent(l, [1, 2, 3])


# ---------------------------------------------------------------- pop_front
def test_pop_front_returns_in_order_until_empty():
    l = SinglyLinkedList([1, 2, 3])
    assert [l.pop_front(), l.pop_front()] == [1, 2]
    assert_consistent(l, [3])
    assert l.pop_front() == 3
    assert_consistent(l, [])


def test_pop_front_on_empty_raises():
    with pytest.raises(IndexError):
        SinglyLinkedList().pop_front()


def test_push_back_after_emptying_by_pop_works():
    l = SinglyLinkedList([1])
    l.pop_front()
    l.push_back(2)
    assert_consistent(l, [2])


# ------------------------------------------------------------------- remove
def test_remove_head_middle_tail_only():
    l = SinglyLinkedList([1, 2, 3, 4])
    assert l.remove(1) and l.remove(3)
    assert_consistent(l, [2, 4])
    assert l.remove(4)                       # tail
    assert_consistent(l, [2])
    l.push_back(5)                           # proves tail is still valid
    assert_consistent(l, [2, 5])
    assert l.remove(2) and l.remove(5)
    assert_consistent(l, [])


def test_remove_missing_returns_false():
    l = SinglyLinkedList([1, 2])
    assert l.remove(99) is False
    assert SinglyLinkedList().remove(1) is False
    assert_consistent(l, [1, 2])


def test_remove_only_first_duplicate():
    l = SinglyLinkedList([7, 7, 7])
    assert l.remove(7)
    assert_consistent(l, [7, 7])


def test_remove_uses_equality_not_identity():
    l = SinglyLinkedList([[1], [2]])
    assert l.remove([2])
    assert_consistent(l, [[1]])


# -------------------------------------------------------- dunder / container
def test_contains_len_iter_repr():
    l = SinglyLinkedList([1, 2, 3])
    assert 2 in l and 99 not in l
    assert len(l) == 3
    assert list(l) == [1, 2, 3]
    assert repr(l) == "1 -> 2 -> 3 -> None"
    assert repr(SinglyLinkedList()) == " -> None"


def test_iteration_is_repeatable_and_does_not_consume():
    l = SinglyLinkedList([1, 2])
    assert list(l) == list(l) == [1, 2]


def test_stores_none_and_falsy_values():
    l = SinglyLinkedList([0, None, "", False])
    assert len(l) == 4
    assert list(l) == [0, None, "", False]
    assert None in l


# ------------------------------------------------------------------ reverse
def test_reverse_empty_and_single():
    l = SinglyLinkedList()
    l.reverse()
    assert_consistent(l, [])
    l.push_back(1)
    l.reverse()
    assert_consistent(l, [1])


def test_reverse_updates_head_and_tail():
    l = SinglyLinkedList([1, 2, 3, 4, 5])
    l.reverse()
    assert_consistent(l, [5, 4, 3, 2, 1])
    l.push_back(0)
    assert_consistent(l, [5, 4, 3, 2, 1, 0])


def test_reverse_twice_is_identity():
    l = SinglyLinkedList(range(6))
    l.reverse()
    l.reverse()
    assert_consistent(l, list(range(6)))


# ------------------------------------------------------------------- middle
@pytest.mark.parametrize(
    "items, expected",
    [([], None), ([1], 1), ([1, 2], 2), ([1, 2, 3], 2), ([1, 2, 3, 4], 3), ([1, 2, 3, 4, 5], 3)],
)
def test_middle(items, expected):
    assert SinglyLinkedList(items).middle() == expected   # even length -> second middle


# ------------------------------------------------- randomized oracle (vs deque)
@pytest.mark.parametrize("seed", range(5))
def test_random_operations_match_a_python_list(seed):
    rng = random.Random(seed)
    lst, model = SinglyLinkedList(), []
    for _ in range(1500):
        op = rng.randrange(6)
        v = rng.randrange(20)
        if op == 0:
            lst.push_front(v); model.insert(0, v)
        elif op == 1:
            lst.push_back(v); model.append(v)
        elif op == 2:
            i = rng.randrange(len(model) + 1)
            lst.insert(i, v); model.insert(i, v)
        elif op == 3:
            if model:
                assert lst.pop_front() == model.pop(0)
            else:
                with pytest.raises(IndexError):
                    lst.pop_front()
        elif op == 4:
            expected = v in model
            assert lst.remove(v) is expected
            if expected:
                model.remove(v)
        else:
            lst.reverse(); model.reverse()
        assert_consistent(lst, model)
