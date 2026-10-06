"""Demo of the linked lists. Run with: make run"""
from linked_lists import DoublyLinkedList, LRUCache, SinglyLinkedList

print("== Singly linked list ==")
l = SinglyLinkedList([10, 20, 30])
l.push_front(5)
l.insert(2, 15)
print(l)                 # 5 -> 10 -> 15 -> 20 -> 30 -> None
l.reverse()
print(list(l), len(l))   # [30, 20, 15, 10, 5] 5

print("\n== Doubly linked list ==")
d = DoublyLinkedList([1, 2, 3])
handle = d.push_front(0)
print(d, "| reversed:", list(reversed(d)))
d.unlink(handle)         # O(1): we hold the node
print(d)

print("\n== LRU cache (capacity 2) ==")
cache = LRUCache(2)
cache.put("a", 1)
cache.put("b", 2)
cache.get("a")           # "a" becomes most recently used
cache.put("c", 3)        # evicts "b"
print("b ->", cache.get("b"), "| keys (MRU first):", list(cache.keys()))
