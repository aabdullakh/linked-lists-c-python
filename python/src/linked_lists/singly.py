"""Singly linked list: each node knows only its successor."""
from __future__ import annotations

from typing import Any, Iterable, Iterator, Optional


class Node:
    __slots__ = ("data", "next")          # no per-instance __dict__: less memory

    def __init__(self, data: Any, next: Optional["Node"] = None) -> None:
        self.data = data
        self.next = next


class SinglyLinkedList:
    def __init__(self, items: Iterable[Any] = ()) -> None:
        self.head: Optional[Node] = None
        self.tail: Optional[Node] = None
        self._size = 0
        for x in items:
            self.push_back(x)

    # -- dunder helpers make it feel like a built-in container ------------
    def __len__(self) -> int:
        return self._size

    def __iter__(self) -> Iterator[Any]:
        cur = self.head
        while cur:
            yield cur.data
            cur = cur.next

    def __contains__(self, value: Any) -> bool:
        return any(x == value for x in self)

    def __repr__(self) -> str:
        return " -> ".join(map(repr, self)) + " -> None"

    # -- insertion ---------------------------------------------------------
    def push_front(self, data: Any) -> None:               # O(1)
        self.head = Node(data, self.head)
        if self.tail is None:
            self.tail = self.head
        self._size += 1

    def push_back(self, data: Any) -> None:                # O(1) via tail
        node = Node(data)
        if self.tail:
            self.tail.next = node
        else:
            self.head = node
        self.tail = node
        self._size += 1

    def insert(self, index: int, data: Any) -> None:       # O(index)
        if not 0 <= index <= self._size:
            raise IndexError("index out of range")
        if index == 0:
            return self.push_front(data)
        if index == self._size:
            return self.push_back(data)
        prev = self.head
        for _ in range(index - 1):
            prev = prev.next
        prev.next = Node(data, prev.next)                   # connect forward, then splice
        self._size += 1

    # -- deletion ----------------------------------------------------------
    def pop_front(self) -> Any:                             # O(1)
        if self.head is None:
            raise IndexError("pop from empty list")
        node = self.head
        self.head = node.next
        if self.head is None:
            self.tail = None
        self._size -= 1
        return node.data

    def remove(self, value: Any) -> bool:                   # O(n)
        prev, cur = None, self.head
        while cur and cur.data != value:
            prev, cur = cur, cur.next
        if cur is None:
            return False
        if prev is None:
            self.head = cur.next
        else:
            prev.next = cur.next
        if cur is self.tail:
            self.tail = prev
        self._size -= 1
        return True

    # -- algorithms --------------------------------------------------------
    def reverse(self) -> None:                              # O(n) time, O(1) space
        prev, cur = None, self.head
        self.tail = self.head
        while cur:
            cur.next, prev, cur = prev, cur, cur.next       # tuple assignment: flip + advance
        self.head = prev

    def middle(self) -> Any:
        slow = fast = self.head
        while fast and fast.next:
            slow, fast = slow.next, fast.next.next
        return slow.data if slow else None
