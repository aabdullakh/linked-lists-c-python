"""Classic linked-list algorithms that operate on raw `Node` chains."""
from __future__ import annotations

from typing import Optional

from .singly import Node


def has_cycle(head: Optional[Node]) -> bool:               # Floyd's tortoise and hare
    slow = fast = head
    while fast and fast.next:
        slow, fast = slow.next, fast.next.next
        if slow is fast:
            return True
    return False


def merge_sorted(a: Optional[Node], b: Optional[Node]) -> Optional[Node]:
    """Merge two sorted chains by relinking nodes (stable). O(n + m)."""
    dummy = tail = Node(None)
    while a and b:
        if a.data <= b.data:
            tail.next, a = a, a.next
        else:
            tail.next, b = b, b.next
        tail = tail.next
    tail.next = a or b
    return dummy.next


def remove_nth_from_end(head: Optional[Node], n: int) -> Optional[Node]:
    """Remove the n-th node from the end in ONE pass (dummy head + gap pointers)."""
    dummy = Node(None, head)
    fast = slow = dummy
    for _ in range(n):            # fast gets an n-step head start
        fast = fast.next
    while fast.next:              # move both until fast is on the LAST node
        fast, slow = fast.next, slow.next
    slow.next = slow.next.next    # slow is just before the victim: bypass it
    return dummy.next
