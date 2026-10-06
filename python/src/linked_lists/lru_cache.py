"""LRU cache = hash map + doubly linked list; get and put are both O(1)."""
from __future__ import annotations

from typing import Any, Dict, Iterator

from .doubly import DNode, DoublyLinkedList


class LRUCache:
    def __init__(self, capacity: int) -> None:
        if capacity < 1:
            raise ValueError("capacity must be at least 1")
        self.cap = capacity
        self.map: Dict[Any, DNode] = {}
        self.order = DoublyLinkedList()    # front = most recently used

    def __len__(self) -> int:
        return len(self.map)

    def __contains__(self, key: Any) -> bool:      # does NOT count as a "use"
        return key in self.map

    def keys(self) -> Iterator[Any]:               # most recently used first
        return (k for k, _ in self.order)

    def get(self, key: Any, default: Any = None) -> Any:
        node = self.map.get(key)
        if node is None:
            return default
        self.order.move_to_front(node)
        return node.data[1]

    def put(self, key: Any, value: Any) -> None:
        node = self.map.get(key)
        if node:
            node.data = (key, value)
            self.order.move_to_front(node)
            return
        if len(self.map) >= self.cap:
            old_key, _ = self.order.pop_back()   # evict least recently used
            del self.map[old_key]
        self.map[key] = self.order.push_front((key, value))
