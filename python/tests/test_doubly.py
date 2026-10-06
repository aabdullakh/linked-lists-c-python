import random

import pytest

from linked_lists import DoublyLinkedList


def assert_consistent(lst, expected):
    """Forward order, backward order, size, and prev/next symmetry all hold."""
    assert list(lst) == expected
    assert list(reversed(lst)) == expected[::-1]
    assert len(lst) == len(expected)
    s = lst._s
    cur = s.next
    while cur is not s:
        assert cur.next.prev is cur
        assert cur.prev.next is cur
        cur = cur.next
    if not expected:
        assert s.next is s and s.prev is s


def test_new_list_is_empty_with_self_pointing_sentinel():
    assert_consistent(DoublyLinkedList(), [])


def test_constructor_accepts_iterables():
    assert_consistent(DoublyLinkedList([1, 2, 3]), [1, 2, 3])
    assert_consistent(DoublyLinkedList(range(3)), [0, 1, 2])


def test_push_front_and_back_return_usable_handles():
    d = DoublyLinkedList()
    b = d.push_back(2)
    a = d.push_front(1)
    c = d.push_back(3)
    assert (a.data, b.data, c.data) == (1, 2, 3)
    assert_consistent(d, [1, 2, 3])


def test_push_front_reverses_push_back_preserves():
    d = DoublyLinkedList()
    for i in (1, 2, 3):
        d.push_front(i)
    assert_consistent(d, [3, 2, 1])
    d.push_back(0)
    assert_consistent(d, [3, 2, 1, 0])


def test_single_element_is_first_and_last():
    d = DoublyLinkedList()
    n = d.push_back(42)
    assert d._s.next is n and d._s.prev is n


def test_unlink_head_middle_tail_only_by_handle():
    d = DoublyLinkedList()
    a, b, c, e = (d.push_back(i) for i in (1, 2, 3, 4))
    assert d.unlink(b) == 2
    assert_consistent(d, [1, 3, 4])
    d.unlink(a)
    assert_consistent(d, [3, 4])
    d.unlink(e)
    assert_consistent(d, [3])
    d.unlink(c)
    assert_consistent(d, [])


def test_pop_front_and_back():
    d = DoublyLinkedList([1, 2, 3, 4])
    assert d.pop_front() == 1
    assert d.pop_back() == 4
    assert_consistent(d, [2, 3])


@pytest.mark.parametrize("method", ["pop_front", "pop_back"])
def test_pop_on_empty_raises(method):
    with pytest.raises(IndexError):
        getattr(DoublyLinkedList(), method)()


def test_reusable_after_emptied():
    d = DoublyLinkedList([1])
    d.pop_back()
    d.push_back(2)
    d.push_front(1)
    assert_consistent(d, [1, 2])


def test_move_to_front_from_back_middle_and_front():
    d = DoublyLinkedList()
    a, b, c = d.push_back(1), d.push_back(2), d.push_back(3)
    d.move_to_front(c)
    assert_consistent(d, [3, 1, 2])
    d.move_to_front(a)
    assert_consistent(d, [1, 3, 2])
    d.move_to_front(a)                      # already first: no-op
    assert_consistent(d, [1, 3, 2])
    d.move_to_front(b)
    assert_consistent(d, [2, 1, 3])


def test_move_to_front_single_element_and_size_unchanged():
    d = DoublyLinkedList()
    a = d.push_back(1)
    d.move_to_front(a)
    assert_consistent(d, [1])


def test_move_to_front_does_not_allocate_new_nodes():
    d = DoublyLinkedList()
    nodes = [d.push_back(i) for i in range(5)]
    d.move_to_front(nodes[3])
    assert d._s.next is nodes[3]


def test_node_uses_slots():
    n = DoublyLinkedList().push_back(1)
    assert not hasattr(n, "__dict__")


def test_stores_falsy_values():
    d = DoublyLinkedList([0, None, ""])
    assert list(d) == [0, None, ""]
    assert len(d) == 3


@pytest.mark.parametrize("seed", range(5))
def test_random_operations_match_a_python_list(seed):
    rng = random.Random(seed)
    d, model, handles = DoublyLinkedList(), [], []
    for _ in range(1500):
        op = rng.randrange(6)
        v = rng.randrange(1000)
        if op == 0:
            h = d.push_front(v); model.insert(0, v); handles.insert(0, h)
        elif op == 1:
            h = d.push_back(v); model.append(v); handles.append(h)
        elif op == 2 and model:
            assert d.pop_front() == model.pop(0); handles.pop(0)
        elif op == 3 and model:
            assert d.pop_back() == model.pop(); handles.pop()
        elif op == 4 and model:
            i = rng.randrange(len(model))
            assert d.unlink(handles[i]) == model.pop(i); handles.pop(i)
        elif op == 5 and model:
            i = rng.randrange(len(model))
            d.move_to_front(handles[i])
            model.insert(0, model.pop(i)); handles.insert(0, handles.pop(i))
        assert_consistent(d, model)
