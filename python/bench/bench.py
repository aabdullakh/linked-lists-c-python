"""Micro-benchmarks: list vs deque vs a hand-written linked list. Run: make bench"""
import sys
import timeit

from linked_lists import SinglyLinkedList

print("python", sys.version.split()[0])

N = 100_000

t_list = timeit.timeit("while l: l.pop(0)", setup=f"l=list(range({N}))", number=1)
t_deque = timeit.timeit("while d: d.popleft()", setup=f"from collections import deque; d=deque(range({N}))", number=1)
print(f"drain {N} items from the front: list.pop(0) {t_list*1e3:.1f} ms | deque.popleft {t_deque*1e3:.1f} ms")

t_list = timeit.timeit("for i in range(20000): l.insert(0, i)", setup=f"l=list(range({N}))", number=1)
t_deque = timeit.timeit("for i in range(20000): d.appendleft(i)", setup=f"from collections import deque; d=deque(range({N}))", number=1)
print(f"20000 front inserts into {N}: list.insert(0) {t_list*1e3:.1f} ms | deque.appendleft {t_deque*1e3:.1f} ms")

t_list = timeit.timeit("for i in range(20000): l[50000]", setup=f"l=list(range({N}))", number=1)
t_deque = timeit.timeit("for i in range(20000): d[50000]", setup=f"from collections import deque; d=deque(range({N}))", number=1)
print(f"20000 middle index reads: list {t_list*1e3:.1f} ms | deque {t_deque*1e3:.1f} ms")

sl = SinglyLinkedList()
for i in range(N):
    sl.push_front(i)
lst = list(range(N))
t_l = timeit.timeit("for x in sl: pass", globals={"sl": sl}, number=20) / 20
t_a = timeit.timeit("for x in lst: pass", globals={"lst": lst}, number=20) / 20
print(f"iterate {N}: hand-made linked list {t_l*1e3:.2f} ms | built-in list {t_a*1e3:.2f} ms")
