"""Circular doubly linked list with a sentinel node."""
from __future__ import annotations

from typing import Any, Iterable, Iterator


class DNode:
    __slots__ = ("data", "prev", "next")

    def __init__(self, data: Any = None) -> None:
        self.data = data
        self.prev: "DNode" = self
        self.next: "DNode" = self


class DoublyLinkedList:
    def __init__(self, items: Iterable[Any] = ()) -> None:
        self._s = DNode()                  # sentinel: _s.next is first, _s.prev is last
        self._size = 0
        for x in items:
            self.push_back(x)

    def __len__(self) -> int:
        return self._size

    def __iter__(self) -> Iterator[Any]:
        cur = self._s.next
        while cur is not self._s:
            yield cur.data
            cur = cur.next

    def __reversed__(self) -> Iterator[Any]:
        cur = self._s.prev
        while cur is not self._s:
            yield cur.data
            cur = cur.prev

    def __repr__(self) -> str:
        return "[" + " <-> ".join(map(repr, self)) + "]"

    @staticmethod
    def _link(node: DNode, before: DNode, after: DNode) -> None:
        node.prev, node.next = before, after
        before.next = after.prev = node

    def push_front(self, data: Any) -> DNode:
        n = DNode(data)
        self._link(n, self._s, self._s.next)
        self._size += 1
        return n

    def push_back(self, data: Any) -> DNode:
        n = DNode(data)
        self._link(n, self._s.prev, self._s)
        self._size += 1
        return n

    def unlink(self, node: DNode) -> Any:                   # O(1) given the node
        node.prev.next = node.next
        node.next.prev = node.prev
        self._size -= 1
        return node.data

    def pop_front(self) -> Any:
        if not self._size:
            raise IndexError("pop from empty list")
        return self.unlink(self._s.next)

    def pop_back(self) -> Any:
        if not self._size:
            raise IndexError("pop from empty list")
        return self.unlink(self._s.prev)

    def move_to_front(self, node: DNode) -> None:           # no allocation
        node.prev.next = node.next
        node.next.prev = node.prev
        self._link(node, self._s, self._s.next)
