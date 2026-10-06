# Linked Lists with C and Python
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#linked-lists-with-c-and-python`](#linked-lists-with-c-and-python)
<!-- paths:end -->

*A complete guide for a Data Structures & Algorithms course: theory, step-by-step illustrations, tested implementations, real performance numbers, use cases, and alternatives.*

> **How to use this guide.** Read Parts 1–3 for the mental model, work through Part 4 with pencil and paper while watching the diagrams, then type (don't paste) the code in Parts 5–6. Part 7 explains the performance reality that many textbooks gloss over. Parts 8–10 tell you when to use a linked list, which problems it solves, and what to use instead. Part 11 has exercises.
>
> Every code listing below is embedded from the repository's real source files (kept in sync by `scripts/sync_docs.py`) and is covered by a **Unity** test suite (C) and a **pytest** suite (Python). Each header carries a **Path** line (and a **Code** line where relevant) showing where it lives in the repository. The benchmark numbers in Part 7 were measured on one machine; treat them as *shape*, not gospel.

---

## Table of Contents
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#table-of-contents`](#table-of-contents)
<!-- paths:end -->

0. [Repository layout and quick start](#repository-layout-and-quick-start)
1. [What a linked list is](#1-what-a-linked-list-is)
2. [Anatomy: nodes, pointers, memory](#2-anatomy-nodes-pointers-memory)
3. [Variants](#3-variants)
4. [Step-by-step illustrations of core operations](#4-step-by-step-illustrations-of-core-operations)
5. [Implementation in C](#5-implementation-in-c)
6. [Implementation in Python](#6-implementation-in-python)
7. [Performance: theory and reality](#7-performance-theory-and-reality)
8. [When (and when not) to use a linked list](#8-when-and-when-not-to-use-a-linked-list)
9. [Problems solved with linked lists](#9-problems-solved-with-linked-lists)
10. [Alternatives: side-by-side comparison](#10-alternatives-side-by-side-comparison)
11. [Common bugs, checklist, and exercises](#11-common-bugs-checklist-and-exercises)
12. [References](#12-references)

---

## Repository layout and quick start
<!-- code: Makefile, c/Makefile, python/Makefile -->
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#repository-layout-and-quick-start`](#repository-layout-and-quick-start) · 💻 **Code:** [`Makefile`](../Makefile), [`c/Makefile`](../c/Makefile), [`python/Makefile`](../python/Makefile)
<!-- paths:end -->

```text
.
├── Makefile                  run everything: make test | run | bench | docs-check
├── README.md
├── LICENSE                   MIT
├── docs/
│   └── linked-list-with-c-and-python.md      <- this guide
├── c/
│   ├── Makefile              make test | run | bench | clean
│   ├── include/              singly_list.h, doubly_list.h
│   ├── src/                  singly_list.c, doubly_list.c
│   ├── examples/demo.c
│   ├── bench/bench.c
│   ├── tests/                Unity tests: test_singly_list.c, test_doubly_list.c
│   └── third_party/unity/    Unity test framework (bundled, MIT)
├── python/
│   ├── Makefile              make test | run | bench | clean
│   ├── pyproject.toml        pytest configuration
│   ├── src/linked_lists/     singly.py, doubly.py, algorithms.py, lru_cache.py
│   ├── examples/demo.py
│   ├── bench/bench.py
│   └── tests/                pytest tests
└── scripts/sync_docs.py      keeps code listings and Path lines in this guide up to date
```

Requirements: `make`, a C compiler (`gcc` or `clang`), and Python 3.10+. On Windows use WSL.

```bash
make test     # C (Unity) + Python (pytest) test suites
make run      # run both demos
make bench    # run both benchmarks
make -C c test       # only C
make -C python test  # only Python
```

---

## 1. What a linked list is
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#1-what-a-linked-list-is`](#1-what-a-linked-list-is)
<!-- paths:end -->

A **linked list** is a linear collection of **nodes**. Each node stores a value and a **reference (pointer) to the next node**. The order of the elements is defined by those links, **not** by where the nodes sit in memory.

```text
 head
  │
  ▼
┌──────┬───┐   ┌──────┬───┐   ┌──────┬───┐
│  10  │ ●─┼──►│  20  │ ●─┼──►│  30  │ ╳ │   ╳ = NULL / None (end of list)
└──────┴───┘   └──────┴───┘   └──────┴───┘
  data  next     data  next     data  next
```

The key trade made by this design:

| You gain | You pay |
|---|---|
| Insert/delete at a *known position* by rewiring a few pointers, with **no shifting** of other elements | **No random access**: reaching element *i* means walking *i* links, O(n) |
| Grows one node at a time; **no resizing, no copying** of existing elements | Extra memory for pointers (8 bytes per link on 64-bit) |
| **Stable addresses**: a node never moves when others are inserted or removed | **Poor cache locality**: nodes can be scattered across memory (see Part 7) |

If you remember one sentence: **a linked list makes *restructuring* cheap and *finding* expensive.**

---

## 2. Anatomy: nodes, pointers, memory
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#2-anatomy-nodes-pointers-memory`](#2-anatomy-nodes-pointers-memory)
<!-- paths:end -->

### 2.1 The three ideas you must hold
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#21-the-three-ideas-you-must-hold`](#21-the-three-ideas-you-must-hold)
<!-- paths:end -->

1. **Node**: a small struct/object: `data` + `next` (and `prev` for doubly linked lists).
2. **Head**: a variable that points at the first node. *Lose the head and you lose the list.* (In C that is a memory leak.)
3. **Terminator**: the last node's `next` is `NULL` (C) / `None` (Python). Every traversal loop is "follow `next` until you hit the terminator".

### 2.2 Memory picture: why it behaves the way it does
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#22-memory-picture-why-it-behaves-the-way-it-does`](#22-memory-picture-why-it-behaves-the-way-it-does)
<!-- paths:end -->

An array stores elements side by side. A linked list's nodes live wherever the allocator put them, and the `next` pointers stitch them together.

```text
ARRAY  (contiguous: address = base + i * sizeof(elem))
 0x1000   0x1004   0x1008   0x100C
┌───────┬───────┬───────┬───────┐
│  10   │  20   │  30   │  40   │
└───────┴───────┴───────┴───────┘

LINKED LIST  (scattered: you can only follow pointers)
 0x2A40                 0x7F10                 0x3C88
┌──────┬────────┐      ┌──────┬────────┐      ┌──────┬────────┐
│  10  │ 0x7F10 │─────►│  20  │ 0x3C88 │─────►│  30  │  NULL  │
└──────┴────────┘      └──────┴────────┘      └──────┴────────┘
```

On a 64-bit machine, a node holding a 4-byte `int` is **16 bytes** in C (4 data + 4 padding + 8 pointer), so 4× the memory of the bare `int`. Plus, each `malloc` carries allocator bookkeeping (often 8–16 more bytes).

### 2.3 Pointer-to-pointer: the C idiom that removes special cases
<!-- code: c/src/singly_list.c -->
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#23-pointer-to-pointer-the-c-idiom-that-removes-special-cases`](#23-pointer-to-pointer-the-c-idiom-that-removes-special-cases) · 💻 **Code:** [`c/src/singly_list.c`](../c/src/singly_list.c)
<!-- paths:end -->

Deleting the head and deleting a middle node look different if you track `prev`. With a **pointer to the link that points at the current node** (`Node **link`), both cases are the same code. You will see this in `list_remove_value` in Part 5.

---

## 3. Variants
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#3-variants`](#3-variants)
<!-- paths:end -->

```text
SINGLY LINKED            head ─► [A|•]─►[B|•]─►[C|╳]
  one pointer/node; forward-only; smallest.

DOUBLY LINKED            ╳◄─[•|A|•]⇄[•|B|•]⇄[•|C|•]─►╳
  next + prev; O(1) delete given a node; walk both ways; +8 bytes/node.

CIRCULAR (singly)        head ─► [A|•]─►[B|•]─►[C|•]─┐
                                  ▲                    │
                                  └────────────────────┘
  last points back to first; natural for round-robin schedulers.

CIRCULAR DOUBLY + SENTINEL    ┌──►[ S ]⇄[A]⇄[B]⇄[C]⇄┐ (S = dummy node)
                              └──────────────────────┘
  A dummy "sentinel" node means insert/remove NEVER hit a NULL or a
  head/tail special case. This is what the Linux kernel does.
```

| Variant | Forward walk | Backward walk | Delete a *given node* | Memory/node (64-bit, int) | Typical use |
|---|---|---|---|---|---|
| Singly | ✔ | ✘ | O(n) (need predecessor) | 16 B | Stacks, hash chains, adjacency lists |
| Doubly | ✔ | ✔ | **O(1)** | 24 B | LRU caches, editors, deques, OS queues |
| Circular | ✔ (wraps) | – | – | 16 B | Round-robin, ring/playlist loops |

**Advanced variants (know the names):**
- **Unrolled linked list**: each node holds a small *array* of elements. Fewer pointers and much better cache behavior. CPython's `collections.deque` uses exactly this idea: a doubly linked list of 64-slot blocks.
- **Intrusive linked list**: the link fields live *inside* your own struct (the Linux kernel's `struct list_head`), so there is no separate node allocation and one object can sit on several lists at once.
- **XOR linked list**: stores `prev ^ next` in one field to emulate a doubly linked list in one pointer's space. A curiosity; not safe in garbage-collected languages.
- **Skip list**: layered linked lists giving O(log n) search; an ordered-map alternative to balanced trees (used by Redis sorted sets).

---

## 4. Step-by-step illustrations of core operations
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#4-step-by-step-illustrations-of-core-operations`](#4-step-by-step-illustrations-of-core-operations)
<!-- paths:end -->

Read each one as a *film strip*. The numbers on arrows are the order in which the pointer writes happen. **Order matters**, and getting it wrong is the #1 source of lost nodes.

### 4.1 Traversal
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#41-traversal`](#41-traversal)
<!-- paths:end -->

```text
cur = head
 │
 ▼
[10]──►[20]──►[30]──►NULL        visit 10, cur = cur->next
      ▲
      cur                         visit 20, cur = cur->next
            ▲
            cur                   visit 30, cur = cur->next
                    ▲
                    cur = NULL → loop ends
```

### 4.2 Insert at the head: O(1)
<!-- code: c/src/singly_list.c, python/src/linked_lists/singly.py -->
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#42-insert-at-the-head-o1`](#42-insert-at-the-head-o1) · 💻 **Code:** [`c/src/singly_list.c`](../c/src/singly_list.c), [`python/src/linked_lists/singly.py`](../python/src/linked_lists/singly.py)
<!-- paths:end -->

Goal: add `5` in front of `10 → 20 → 30`.

```text
START
 head ─► [10]──►[20]──►[30]──►╳

STEP 1  create the new node (it points nowhere yet)
 new ─► [5|╳]
 head ─► [10]──►[20]──►[30]──►╳

STEP 2  new->next = head        (new node points at the OLD first node)
 new ─► [5]──────┐
                 ▼
 head ─────────►[10]──►[20]──►[30]──►╳

STEP 3  head = new              (head now points at the new node)
 head ─► [5]──►[10]──►[20]──►[30]──►╳
```

> **Why this order?** If you did `head = new` first, you'd lose the only reference to `10 → 20 → 30`.

### 4.3 Insert in the middle (after a node `prev`): O(1) once you hold `prev`
<!-- code: c/src/singly_list.c, python/src/linked_lists/singly.py -->
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#43-insert-in-the-middle-after-a-node-prev-o1-once-you-hold-prev`](#43-insert-in-the-middle-after-a-node-prev-o1-once-you-hold-prev) · 💻 **Code:** [`c/src/singly_list.c`](../c/src/singly_list.c), [`python/src/linked_lists/singly.py`](../python/src/linked_lists/singly.py)
<!-- paths:end -->

Goal: insert `15` between `10` and `20`.

```text
START
 prev
  ▼
 [10]──►[20]──►[30]──►╳

STEP 1  new = node(15)
 new ─► [15|╳]

STEP 2  new->next = prev->next          (connect new node FORWARD first)
 [10]──►[20]──►[30]
         ▲
 [15]────┘                                (both 10 and 15 now point at 20; list is still intact)

STEP 3  prev->next = new                (splice in)
 [10]──►[15]──►[20]──►[30]──►╳
```

> **Mnemonic: "connect forward, then splice."** Reverse the two steps and `20 → 30` is orphaned.

*Finding* `prev` is the O(n) part. The *splice itself* is O(1).

### 4.4 Insert at the tail: O(1) with a tail pointer, O(n) without
<!-- code: c/src/singly_list.c, python/src/linked_lists/singly.py -->
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#44-insert-at-the-tail-o1-with-a-tail-pointer-on-without`](#44-insert-at-the-tail-o1-with-a-tail-pointer-on-without) · 💻 **Code:** [`c/src/singly_list.c`](../c/src/singly_list.c), [`python/src/linked_lists/singly.py`](../python/src/linked_lists/singly.py)
<!-- paths:end -->

```text
START   head ─► [10]──►[20]──►╳        tail ─► [20]

STEP 1  new = node(30)
STEP 2  tail->next = new                 [10]──►[20]──►[30]
STEP 3  tail = new                       tail now ─► [30]
```

Without a `tail` pointer you'd walk the entire list to find the last node. **Keep a tail pointer if you append often.**

### 4.5 Delete a node: singly linked
<!-- code: c/src/singly_list.c, python/src/linked_lists/singly.py -->
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#45-delete-a-node-singly-linked`](#45-delete-a-node-singly-linked) · 💻 **Code:** [`c/src/singly_list.c`](../c/src/singly_list.c), [`python/src/linked_lists/singly.py`](../python/src/linked_lists/singly.py)
<!-- paths:end -->

Goal: delete `20` from `10 → 20 → 30`.

```text
START
 prev      cur
  ▼         ▼
 [10]─────►[20]─────►[30]──►╳

STEP 1  locate cur (value 20) while tracking prev
STEP 2  prev->next = cur->next           bypass `cur`
 [10]──────────────►[30]──►╳
        [20]  (orphaned, still allocated)

STEP 3  free(cur)                         C: return memory.  Python: GC reclaims it.
 [10]──►[30]──►╳
```

Special cases a naive implementation must handle: deleting the **head** (no `prev`), deleting the **tail** (update `tail`), deleting from a **one-element list** (head *and* tail become NULL). Part 5's pointer-to-pointer approach folds the first one away.

### 4.6 Delete a node: doubly linked (O(1) given the node)
<!-- code: c/src/doubly_list.c, python/src/linked_lists/doubly.py -->
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#46-delete-a-node-doubly-linked-o1-given-the-node`](#46-delete-a-node-doubly-linked-o1-given-the-node) · 💻 **Code:** [`c/src/doubly_list.c`](../c/src/doubly_list.c), [`python/src/linked_lists/doubly.py`](../python/src/linked_lists/doubly.py)
<!-- paths:end -->

```text
START   ⇄[A]⇄[B]⇄[C]⇄         delete B (we hold a pointer to B)

STEP 1  B->prev->next = B->next        A.next now skips B:   [A]──►[C]
STEP 2  B->next->prev = B->prev        C.prev now skips B:   [A]◄──[C]
STEP 3  free(B)

RESULT  ⇄[A]⇄[C]⇄
```

No traversal. This O(1) delete-by-handle is the reason doubly linked lists power LRU caches.

### 4.7 Reverse the list in place: O(n) time, O(1) space
<!-- code: c/src/singly_list.c, python/src/linked_lists/singly.py -->
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#47-reverse-the-list-in-place-on-time-o1-space`](#47-reverse-the-list-in-place-on-time-o1-space) · 💻 **Code:** [`c/src/singly_list.c`](../c/src/singly_list.c), [`python/src/linked_lists/singly.py`](../python/src/linked_lists/singly.py)
<!-- paths:end -->

Three pointers: `prev`, `cur`, `next`. Per node: **save next → flip arrow → advance both**.

```text
START
 prev=╳   cur
  ╳        ▼
          [1]──►[2]──►[3]──►╳

ITER 1  next=[2];  cur->next = prev(╳);  prev=[1];  cur=[2]
 ╳◄──[1]      [2]──►[3]──►╳
       ▲       ▲
      prev    cur

ITER 2  next=[3];  cur->next = prev([1]);  prev=[2];  cur=[3]
 ╳◄──[1]◄──[2]      [3]──►╳
             ▲       ▲
            prev    cur

ITER 3  next=╳;    cur->next = prev([2]);  prev=[3];  cur=╳
 ╳◄──[1]◄──[2]◄──[3]
                   ▲
                  prev   → head = prev

RESULT  head ─► [3]──►[2]──►[1]──►╳
```

### 4.8 Find the middle in one pass: slow/fast pointers
<!-- code: c/src/singly_list.c, python/src/linked_lists/singly.py -->
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#48-find-the-middle-in-one-pass-slowfast-pointers`](#48-find-the-middle-in-one-pass-slowfast-pointers) · 💻 **Code:** [`c/src/singly_list.c`](../c/src/singly_list.c), [`python/src/linked_lists/singly.py`](../python/src/linked_lists/singly.py)
<!-- paths:end -->

```text
[1]──►[2]──►[3]──►[4]──►[5]──►╳
 S,F                                 start: both at head
       S     F                       slow +1, fast +2
             S           F           slow +1, fast +2
                         S      F=╳  fast hit the end → slow is the middle: 3
```

### 4.9 Detect a cycle: Floyd's tortoise and hare
<!-- code: c/src/singly_list.c, python/src/linked_lists/algorithms.py -->
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#49-detect-a-cycle-floyds-tortoise-and-hare`](#49-detect-a-cycle-floyds-tortoise-and-hare) · 💻 **Code:** [`c/src/singly_list.c`](../c/src/singly_list.c), [`python/src/linked_lists/algorithms.py`](../python/src/linked_lists/algorithms.py)
<!-- paths:end -->

```text
[1]──►[2]──►[3]──►[4]
             ▲      │
             └──[6]◄[5]            a cycle: 6 points back to 3

slow moves 1 step, fast moves 2 steps. If there is a cycle, fast laps slow
inside the loop and they MUST meet. If there is none, fast reaches NULL.
O(n) time, O(1) space (no hash set of visited nodes).
```

---

## 5. Implementation in C
<!-- code: c/ -->
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#5-implementation-in-c`](#5-implementation-in-c) · 💻 **Code:** [`c/`](../c)
<!-- paths:end -->

Design choices (and why):

- The `List` struct holds `head`, `tail`, and `size`, so `push_back` and `size` are O(1).
- Every allocation is **checked**; every function that can fail returns `bool`.
- `list_free` saves `next` **before** `free`, the classic use-after-free trap.
- `list_remove_value` uses **pointer-to-pointer** to avoid special-casing the head.
- Interface (`include/*.h`) and implementation (`src/*.c`) are separate, as in real C projects, so tests and demos include only the header.

### 5.1 `singly_list.h` and `singly_list.c`: singly linked list + classic algorithms
<!-- code: c/include/singly_list.h, c/src/singly_list.c, c/tests/test_singly_list.c -->
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#51-singly_listh-and-singly_listc-singly-linked-list--classic-algorithms`](#51-singly_listh-and-singly_listc-singly-linked-list--classic-algorithms) · 💻 **Code:** [`c/include/singly_list.h`](../c/include/singly_list.h), [`c/src/singly_list.c`](../c/src/singly_list.c), [`c/tests/test_singly_list.c`](../c/tests/test_singly_list.c)
<!-- paths:end -->

**Header:** `c/include/singly_list.h`

<!-- source: c/include/singly_list.h -->
```c
#ifndef SINGLY_LIST_H
#define SINGLY_LIST_H

#include <stdbool.h>
#include <stddef.h>

/* ---------- Data layout ---------- */
typedef struct Node {
    int data;
    struct Node *next;
} Node;

typedef struct {
    Node  *head;   /* first node, or NULL when empty   */
    Node  *tail;   /* last node: makes push_back O(1)  */
    size_t size;   /* cached length: makes size O(1)   */
} List;

/* Lifecycle */
void list_init(List *l);
void list_free(List *l);

/* Insertion */
bool list_push_front(List *l, int data);               /* O(1)       */
bool list_push_back(List *l, int data);                /* O(1)       */
bool list_insert_at(List *l, size_t index, int data);  /* O(index)   */

/* Deletion */
bool list_pop_front(List *l, int *out);                /* O(1); out may be NULL */
bool list_remove_value(List *l, int value);            /* O(n)       */

/* Search / inspection */
Node *list_find(const List *l, int value);             /* O(n)       */
void  list_print(const List *l);

/* Classic algorithms */
void  list_reverse(List *l);                           /* O(n) time, O(1) space */
Node *list_middle(const List *l);                      /* slow/fast pointers    */
bool  has_cycle(const Node *head);                     /* Floyd's algorithm     */
Node *merge_sorted(Node *a, Node *b);                  /* relinks nodes; O(n+m) */

#endif /* SINGLY_LIST_H */
```

**Implementation:** `c/src/singly_list.c`

<!-- source: c/src/singly_list.c -->
```c
#include "singly_list.h"

#include <stdio.h>
#include <stdlib.h>

/* ---------- Lifecycle ---------- */
void list_init(List *l) { l->head = l->tail = NULL; l->size = 0; }

void list_free(List *l) {
    Node *cur = l->head;
    while (cur) {
        Node *next = cur->next;   /* save BEFORE freeing */
        free(cur);
        cur = next;
    }
    list_init(l);
}

static Node *node_new(int data) {
    Node *n = malloc(sizeof *n);
    if (!n) return NULL;          /* caller must check */
    n->data = data;
    n->next = NULL;
    return n;
}

/* ---------- Insertion ---------- */
bool list_push_front(List *l, int data) {          /* O(1) */
    Node *n = node_new(data);
    if (!n) return false;
    n->next = l->head;            /* 1. new node points at old head */
    l->head = n;                  /* 2. head points at new node     */
    if (!l->tail) l->tail = n;    /* list was empty                 */
    l->size++;
    return true;
}

bool list_push_back(List *l, int data) {           /* O(1) thanks to tail */
    Node *n = node_new(data);
    if (!n) return false;
    if (l->tail) l->tail->next = n; else l->head = n;
    l->tail = n;
    l->size++;
    return true;
}

/* Insert so that the new node ends up at position `index` (0-based). O(index) */
bool list_insert_at(List *l, size_t index, int data) {
    if (index > l->size) return false;
    if (index == 0)        return list_push_front(l, data);
    if (index == l->size)  return list_push_back(l, data);
    Node *prev = l->head;
    for (size_t i = 0; i < index - 1; i++) prev = prev->next;
    Node *n = node_new(data);
    if (!n) return false;
    n->next = prev->next;         /* 1. connect new node FORWARD first */
    prev->next = n;               /* 2. then splice it in              */
    l->size++;
    return true;
}

/* ---------- Deletion ---------- */
bool list_pop_front(List *l, int *out) {           /* O(1) */
    if (!l->head) return false;
    Node *old = l->head;
    if (out) *out = old->data;
    l->head = old->next;
    if (!l->head) l->tail = NULL;
    free(old);
    l->size--;
    return true;
}

/* Remove first node holding `value`. O(n). Uses a pointer-to-pointer so the
   head needs no special case. */
bool list_remove_value(List *l, int value) {
    Node **link = &l->head;       /* address of the pointer that points at cur */
    Node  *prev = NULL;
    while (*link && (*link)->data != value) {
        prev = *link;
        link = &(*link)->next;
    }
    if (!*link) return false;
    Node *victim = *link;
    *link = victim->next;         /* bypass victim */
    if (victim == l->tail) l->tail = prev;
    free(victim);
    l->size--;
    return true;
}

/* ---------- Search / access ---------- */
Node *list_find(const List *l, int value) {        /* O(n) */
    for (Node *cur = l->head; cur; cur = cur->next)
        if (cur->data == value) return cur;
    return NULL;
}

void list_print(const List *l) {
    for (Node *cur = l->head; cur; cur = cur->next)
        printf("%d -> ", cur->data);
    printf("NULL\n");
}

/* ---------- Classic algorithms ---------- */
void list_reverse(List *l) {                        /* O(n) time, O(1) space */
    Node *prev = NULL, *cur = l->head;
    l->tail = l->head;
    while (cur) {
        Node *next = cur->next;   /* 1. remember the rest      */
        cur->next = prev;         /* 2. flip this arrow        */
        prev = cur;               /* 3. advance prev           */
        cur = next;               /* 4. advance cur            */
    }
    l->head = prev;
}

Node *list_middle(const List *l) {                  /* slow/fast pointers */
    Node *slow = l->head, *fast = l->head;
    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
    }
    return slow;                  /* second middle if length is even */
}

/* Floyd's cycle detection on a raw node chain. O(n) time, O(1) space. */
bool has_cycle(const Node *head) {
    const Node *slow = head, *fast = head;
    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
        if (slow == fast) return true;
    }
    return false;
}

/* Merge two sorted chains into one (stable). Reuses nodes; O(n+m). */
Node *merge_sorted(Node *a, Node *b) {
    Node dummy = {0, NULL};       /* stack-allocated sentinel */
    Node *tail = &dummy;
    while (a && b) {
        if (a->data <= b->data) { tail->next = a; a = a->next; }
        else                    { tail->next = b; b = b->next; }
        tail = tail->next;
    }
    tail->next = a ? a : b;
    return dummy.next;
}
```

### 5.2 `doubly_list.h` and `doubly_list.c`: circular doubly linked list with a sentinel
<!-- code: c/include/doubly_list.h, c/src/doubly_list.c, c/tests/test_doubly_list.c -->
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#52-doubly_listh-and-doubly_listc-circular-doubly-linked-list-with-a-sentinel`](#52-doubly_listh-and-doubly_listc-circular-doubly-linked-list-with-a-sentinel) · 💻 **Code:** [`c/include/doubly_list.h`](../c/include/doubly_list.h), [`c/src/doubly_list.c`](../c/src/doubly_list.c), [`c/tests/test_doubly_list.c`](../c/tests/test_doubly_list.c)
<!-- paths:end -->

The sentinel is a node that lives *inside* the list struct and never holds user data. Because every real node always has a non-NULL `prev` and `next`, there are **no edge cases** in insertion or removal.

**Header:** `c/include/doubly_list.h`

<!-- source: c/include/doubly_list.h -->
```c
#ifndef DOUBLY_LIST_H
#define DOUBLY_LIST_H

#include <stdbool.h>
#include <stddef.h>

typedef struct DNode {
    int data;
    struct DNode *prev, *next;
} DNode;

/* Circular list with a SENTINEL node: sentinel.next is the first element,
   sentinel.prev is the last. An empty list is a sentinel pointing at itself.
   Result: insert/remove never need NULL checks or head/tail special cases. */
typedef struct {
    DNode  sentinel;
    size_t size;
} DList;

void   dlist_init(DList *l);
void   dlist_free(DList *l);

DNode *dlist_push_front(DList *l, int data);   /* O(1); returns the new node (handle) or NULL */
DNode *dlist_push_back(DList *l, int data);    /* O(1) */

void   dlist_remove(DList *l, DNode *n);       /* O(1) given the node */
bool   dlist_pop_front(DList *l, int *out);    /* O(1); out may be NULL */
bool   dlist_pop_back(DList *l, int *out);     /* O(1); out may be NULL */
void   dlist_move_to_front(DList *l, DNode *n);/* O(1), no malloc/free (LRU trick) */

void   dlist_print_forward(const DList *l);

#endif /* DOUBLY_LIST_H */
```

**Implementation:** `c/src/doubly_list.c`

<!-- source: c/src/doubly_list.c -->
```c
#include "doubly_list.h"

#include <stdio.h>
#include <stdlib.h>

void dlist_init(DList *l) {
    l->sentinel.prev = l->sentinel.next = &l->sentinel;
    l->size = 0;
}

/* Insert `n` between two adjacent nodes. The one primitive everything uses. */
static void link_between(DNode *n, DNode *before, DNode *after) {
    n->prev = before;  n->next = after;
    before->next = n;  after->prev = n;
}

/* Unlink `n` in O(1) -- no traversal needed, because n knows both neighbours. */
static void unlink_node(DNode *n) {
    n->prev->next = n->next;
    n->next->prev = n->prev;
}

DNode *dlist_push_front(DList *l, int data) {
    DNode *n = malloc(sizeof *n);
    if (!n) return NULL;
    n->data = data;
    link_between(n, &l->sentinel, l->sentinel.next);
    l->size++;
    return n;
}

DNode *dlist_push_back(DList *l, int data) {
    DNode *n = malloc(sizeof *n);
    if (!n) return NULL;
    n->data = data;
    link_between(n, l->sentinel.prev, &l->sentinel);
    l->size++;
    return n;
}

void dlist_remove(DList *l, DNode *n) {          /* O(1) given the node */
    unlink_node(n);
    free(n);
    l->size--;
}

bool dlist_pop_front(DList *l, int *out) {
    if (l->size == 0) return false;
    DNode *n = l->sentinel.next;
    if (out) *out = n->data;
    dlist_remove(l, n);
    return true;
}

bool dlist_pop_back(DList *l, int *out) {
    if (l->size == 0) return false;
    DNode *n = l->sentinel.prev;
    if (out) *out = n->data;
    dlist_remove(l, n);
    return true;
}

/* Move an existing node to the front without any malloc/free (LRU trick). */
void dlist_move_to_front(DList *l, DNode *n) {
    unlink_node(n);
    link_between(n, &l->sentinel, l->sentinel.next);
}

void dlist_free(DList *l) {
    DNode *cur = l->sentinel.next;
    while (cur != &l->sentinel) {
        DNode *next = cur->next;
        free(cur);
        cur = next;
    }
    dlist_init(l);
}

void dlist_print_forward(const DList *l) {
    printf("[");
    for (const DNode *c = l->sentinel.next; c != &l->sentinel; c = c->next)
        printf("%d%s", c->data, c->next == &l->sentinel ? "" : " <-> ");
    printf("]\n");
}
```

### 5.3 Using it: `examples/demo.c`
<!-- code: c/examples/demo.c -->
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#53-using-it-examplesdemoc`](#53-using-it-examplesdemoc) · 💻 **Code:** [`c/examples/demo.c`](../c/examples/demo.c)
<!-- paths:end -->

<!-- source: c/examples/demo.c -->
```c
/* Demo of the singly and doubly linked lists. Run with: make run */
#include <stdio.h>

#include "doubly_list.h"
#include "singly_list.h"

int main(void) {
    puts("== Singly linked list ==");
    List l;
    list_init(&l);

    list_push_back(&l, 20);
    list_push_back(&l, 30);
    list_push_front(&l, 10);      /* 10 -> 20 -> 30 */
    list_insert_at(&l, 2, 25);    /* 10 -> 20 -> 25 -> 30 */
    list_print(&l);

    list_remove_value(&l, 20);    /* 10 -> 25 -> 30 */
    list_reverse(&l);             /* 30 -> 25 -> 10 */
    list_print(&l);

    printf("size=%zu middle=%d\n", l.size, list_middle(&l)->data);
    list_free(&l);                /* ALWAYS free what you malloc */

    puts("\n== Doubly linked list (with sentinel) ==");
    DList d;
    dlist_init(&d);
    DNode *a = dlist_push_back(&d, 1);
    dlist_push_back(&d, 2);
    DNode *c = dlist_push_back(&d, 3);
    dlist_push_front(&d, 0);      /* 0 <-> 1 <-> 2 <-> 3 */
    dlist_print_forward(&d);

    dlist_remove(&d, a);          /* O(1): we hold the node */
    dlist_move_to_front(&d, c);   /* 3 <-> 0 <-> 2 */
    dlist_print_forward(&d);
    dlist_free(&d);

    return 0;
}
```

Run it (this compiles first, so there is nothing to set up):

```bash
make -C c run
```

Expected output:

```text
== Singly linked list ==
10 -> 20 -> 25 -> 30 -> NULL
30 -> 25 -> 10 -> NULL
size=3 middle=25

== Doubly linked list (with sentinel) ==
[0 <-> 1 <-> 2 <-> 3]
[3 <-> 0 <-> 2]
```

### 5.4 Testing with Unity
<!-- code: c/tests/, c/third_party/unity/ -->
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#54-testing-with-unity`](#54-testing-with-unity) · 💻 **Code:** [`c/tests/`](../c/tests), [`c/third_party/unity/`](../c/third_party/unity)
<!-- paths:end -->

[Unity](https://github.com/ThrowTheSwitch/Unity) is a tiny C unit-test framework (one `.c` file and two headers). It is **bundled in `c/third_party/unity/`**, so a plain `git clone` is enough: nothing to install.

How it works:

- Every `tests/test_*.c` file is its **own program** with a `main()` that registers tests via `RUN_TEST(...)`.
- `setUp()` runs before and `tearDown()` after **every** test, so each test starts from a fresh empty list.
- `TEST_ASSERT_*` macros check values; the first failure in a test stops that test and is reported with file and line.
- The Makefile compiles each test with **AddressSanitizer + UBSan**, so a leak, use-after-free, or double free fails the run even if the assertions pass.

An excerpt from `c/tests/test_singly_list.c`:

```c
static List l;

void setUp(void)    { list_init(&l); }
void tearDown(void) { list_free(&l); }

/* Verify size, element order, and that head/tail are consistent. */
static void assert_list(const List *list, const int *expected, size_t n);
#define ASSERT_LIST(list, ...) /* ... builds an int[] and calls assert_list ... */
#define ASSERT_EMPTY(list) assert_list((list), NULL, 0)

void test_remove_value_tail_updates_tail(void) {
    for (int i = 1; i <= 3; i++) list_push_back(&l, i);
    TEST_ASSERT_TRUE(list_remove_value(&l, 3));
    ASSERT_LIST(&l, 1, 2);
    TEST_ASSERT_TRUE(list_push_back(&l, 4));       /* proves tail is valid */
    ASSERT_LIST(&l, 1, 2, 4);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_remove_value_tail_updates_tail);
    /* ... more RUN_TEST lines ... */
    return UNITY_END();
}
```

Notice that the helper checks **every invariant at once** (length, order, and that `tail` really is the last node). That is why one assertion line can catch a forgotten `tail` update.

| Test file | What it covers |
|---|---|
| `tests/test_singly_list.c` | empty list; `push_front`/`push_back` on empty; insert at front/middle/back; out-of-range insert; `pop_front` (including `NULL` out-parameter and the last element); removing head/middle/tail/only/missing/duplicate values; `find`; `reverse` (0, 1, many); `middle` (odd and even); `has_cycle` (none, self-loop, loop in the middle); `merge_sorted` (interleaved, uneven, duplicates, empty sides); a **randomized test against a plain array** that checks all invariants after every one of 4,000 random operations |
| `tests/test_doubly_list.c` | sentinel invariants; handles returned by `push_*`; removing head/middle/tail/only by handle; `pop_front`/`pop_back` on empty; `move_to_front` from every position; an LRU-style scenario; 2,000-node stress test; prev/next symmetry checked in both directions |

Run the tests:

```bash
make -C c test
```

```text
=== build/test_doubly_list ===
...
13 Tests 0 Failures 0 Ignored
OK
=== build/test_singly_list ===
...
35 Tests 0 Failures 0 Ignored
OK

All C test suites passed.
```

> **Experiment (highly recommended):** deliberately break the code, e.g. delete the line `if (victim == l->tail) l->tail = prev;` in `list_remove_value`, and re-run `make -C c test`. Watching a test turn red is how you learn that the tests actually guard something. Try also moving `free(cur)` above `Node *next = cur->next;` in `list_free` and let AddressSanitizer catch the use-after-free.

### 5.5 Build, run, and memory checking
<!-- code: c/Makefile -->
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#55-build-run-and-memory-checking`](#55-build-run-and-memory-checking) · 💻 **Code:** [`c/Makefile`](../c/Makefile)
<!-- paths:end -->

All of this is driven by `c/Makefile`:

| Command | What it does |
|---|---|
| `make -C c test` | builds each `tests/test_*.c` with the library and Unity (sanitizers on), runs them all, stops on the first failure |
| `make -C c run` | builds and runs `examples/demo.c` |
| `make -C c bench` | builds `bench/bench.c` with `-O2` (no sanitizers, they would distort timings) and runs it |
| `make -C c clean` | removes `c/build/` |

Options: `CC=clang` to pick a compiler, `SANITIZE=0` to disable sanitizers, `V=1` to print the full compiler commands.

```bash
make -C c test CC=clang
make -C c test SANITIZE=0 V=1
valgrind --leak-check=full ./c/build/test_singly_list   # alternative to sanitizers (Linux), build with SANITIZE=0
```

**Ownership rule for `merge_sorted`:** it *relinks* existing nodes, so afterwards the two input lists no longer own their nodes. Reset the inputs (`list_init(&y)`) and free only the merged chain, or you will double-free. (A sanitizer caught exactly this mistake while this guide's first tests were being written.)

---

## 6. Implementation in Python
<!-- code: python/ -->
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#6-implementation-in-python`](#6-implementation-in-python) · 💻 **Code:** [`python/`](../python)
<!-- paths:end -->

Python has no pointers, but **object references behave the same way**: `node.next` is a reference to another `Node`. Garbage collection frees unreachable nodes automatically, so there is no `free()`.

Design choices:

- `__slots__` on the node classes removes the per-instance `__dict__` and keeps nodes small.
- Dunder methods (`__len__`, `__iter__`, `__contains__`, `__repr__`) make the list feel like a built-in.
- The doubly linked version uses a **sentinel**, as in C.
- An `LRUCache` shows *why* you would ever hand-write one (see Part 9).
- Code lives in a small package, `python/src/linked_lists/`, imported with `from linked_lists import ...`.

### 6.1 The `linked_lists` package
<!-- code: python/src/linked_lists/ -->
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#61-the-linked_lists-package`](#61-the-linked_lists-package) · 💻 **Code:** [`python/src/linked_lists/`](../python/src/linked_lists)
<!-- paths:end -->

`python/src/linked_lists/__init__.py` re-exports everything below, so you can write `from linked_lists import SinglyLinkedList, LRUCache`.

**Singly linked list:** `python/src/linked_lists/singly.py`

<!-- source: python/src/linked_lists/singly.py -->
```python
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
```

**Algorithms on raw node chains** (cycle detection, merge, remove *n*-th from end): `python/src/linked_lists/algorithms.py`

<!-- source: python/src/linked_lists/algorithms.py -->
```python
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
```

**Doubly linked list with sentinel:** `python/src/linked_lists/doubly.py`

<!-- source: python/src/linked_lists/doubly.py -->
```python
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
```

**LRU cache = hash map + doubly linked list, all operations O(1):** `python/src/linked_lists/lru_cache.py`

<!-- source: python/src/linked_lists/lru_cache.py -->
```python
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
```

### 6.2 Using it: `examples/demo.py`
<!-- code: python/examples/demo.py -->
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#62-using-it-examplesdemopy`](#62-using-it-examplesdemopy) · 💻 **Code:** [`python/examples/demo.py`](../python/examples/demo.py)
<!-- paths:end -->

<!-- source: python/examples/demo.py -->
```python
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
```

Run it (no dependencies needed):

```bash
make -C python run
```

Expected output:

```text
== Singly linked list ==
5 -> 10 -> 15 -> 20 -> 30 -> None
[30, 20, 15, 10, 5] 5

== Doubly linked list ==
[0 <-> 1 <-> 2 <-> 3] | reversed: [3, 2, 1, 0]
[1 <-> 2 <-> 3]

== LRU cache (capacity 2) ==
b -> None | keys (MRU first): ['c', 'a']
```

### 6.3 Testing with pytest
<!-- code: python/tests/, python/pyproject.toml -->
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#63-testing-with-pytest`](#63-testing-with-pytest) · 💻 **Code:** [`python/tests/`](../python/tests), [`python/pyproject.toml`](../python/pyproject.toml)
<!-- paths:end -->

[pytest](https://docs.pytest.org) finds every `tests/test_*.py` file and every function named `test_*`. You write plain `assert` statements, and on failure pytest shows the values involved. Configuration lives in `python/pyproject.toml` (it puts `src/` on the import path, so the package needn't be installed).

Three pytest features used in this repo:

```python
# 1. Plain asserts + pytest.raises for expected errors
def test_pop_front_on_empty_raises():
    with pytest.raises(IndexError):
        SinglyLinkedList().pop_front()

# 2. Parametrization: one test body, many cases (each shows up separately in the report)
@pytest.mark.parametrize(
    "index, expected",
    [(0, [9, 1, 2, 3]), (1, [1, 9, 2, 3]), (2, [1, 2, 9, 3]), (3, [1, 2, 3, 9])],
)
def test_insert_at_every_valid_position(index, expected):
    l = SinglyLinkedList([1, 2, 3])
    l.insert(index, 9)
    assert_consistent(l, expected)

# 3. Randomized "oracle" test: do the same random operations on a plain Python list
#    and on our linked list, and require identical results after every step.
@pytest.mark.parametrize("seed", range(5))
def test_random_operations_match_a_python_list(seed): ...
```

| Test file | What it covers |
|---|---|
| `tests/test_singly.py` | construction from any iterable; `__slots__`; push/insert at every valid and invalid index; `pop_front` and empty-list errors; removing head/middle/tail/only/missing/duplicate; equality vs identity; `reverse` (0, 1, many, twice); `middle` (even and odd); falsy values (`0`, `None`, `""`); 5 seeded randomized runs of 1,500 operations |
| `tests/test_doubly.py` | sentinel invariants; handles from `push_*`; unlink head/middle/tail/only; pops on empty; `move_to_front` from every position without allocating; forward/backward/prev-next symmetry after every step of the randomized runs |
| `tests/test_lru_cache.py` | invalid capacity; miss/default; eviction order; `get` and `put` refreshing recency; update without eviction; capacity 1; `keys()` ordering; `in` not counting as a use; stored `None`; map and list staying in sync under load |
| `tests/test_algorithms.py` | `has_cycle` (many loop shapes, long acyclic chain); `merge_sorted` (interleaved, duplicates, empty sides, **stability**, node reuse); `remove_nth_from_end` (head, tail, single node) |

Run them:

```bash
make -C python test                 # creates python/.venv on first run, installs pytest, runs everything
python/.venv/bin/python -m pytest python -k lru -v   # run a subset, verbosely
```

### 6.4 In real Python code, usually don't hand-roll one
<!-- code: python/src/linked_lists/ -->
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#64-in-real-python-code-usually-dont-hand-roll-one`](#64-in-real-python-code-usually-dont-hand-roll-one) · 💻 **Code:** [`python/src/linked_lists/`](../python/src/linked_lists)
<!-- paths:end -->

| You need… | Use this | Why |
|---|---|---|
| A queue / stack / sliding window | `collections.deque` | Doubly linked list of 64-slot blocks, written in C. O(1) at both ends. |
| An ordered sequence with index access | `list` | Dynamic array; contiguous; fastest overall for most work |
| LRU cache | `functools.lru_cache` or `collections.OrderedDict` (`move_to_end`) | Already optimized in C |

Write your own linked list to **learn** (and for interviews), or when you need O(1) *splice/unlink by node handle* semantics.

---

## 7. Performance: theory and reality
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#7-performance-theory-and-reality`](#7-performance-theory-and-reality)
<!-- paths:end -->

### 7.1 Big-O summary
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#71-big-o-summary`](#71-big-o-summary)
<!-- paths:end -->

| Operation | Singly (head only) | Singly (head + tail) | Doubly (head + tail) | Dynamic array (`list`/`vector`) |
|---|---|---|---|---|
| Access by index `a[i]` | O(n) | O(n) | O(n) (n/2 at best) | **O(1)** |
| Search by value | O(n) | O(n) | O(n) | O(n) (O(log n) if sorted) |
| Insert at front | **O(1)** | **O(1)** | **O(1)** | O(n) |
| Insert at back | O(n) | **O(1)** | **O(1)** | O(1) amortized |
| Insert in middle, *given position/node* | O(1) | O(1) | O(1) | O(n) |
| Insert in middle, *by index* | O(n) | O(n) | O(n) | O(n) |
| Delete front | **O(1)** | **O(1)** | **O(1)** | O(n) |
| Delete back | O(n) | O(n) | **O(1)** | O(1) |
| Delete a *given node* | O(n) | O(n) | **O(1)** | O(n) |
| Memory overhead per element | 1 pointer | 1 pointer | 2 pointers | ~0 (plus spare capacity) |

> ⚠️ The most common student mistake: "linked lists insert in O(1)". That's only true **once you already hold the position**. Finding the position is O(n). An array does the same *find* faster (it's cache-friendly) and then pays an O(n) shift, which is a very fast `memmove`.

### 7.2 Real hardware: cache locality dominates
<!-- code: c/bench/bench.c, python/bench/bench.py -->
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#72-real-hardware-cache-locality-dominates`](#72-real-hardware-cache-locality-dominates) · 💻 **Code:** [`c/bench/bench.c`](../c/bench/bench.c), [`python/bench/bench.py`](../python/bench/bench.py)
<!-- paths:end -->

Modern CPUs read memory in 64-byte **cache lines** and a main-memory access costs roughly a hundred-plus cycles, while an L1 hit costs a handful. An array traversal gets several elements per cache line and the hardware prefetcher can run ahead because addresses are predictable. A linked-list traversal is a **dependent chain**: you can't know where node *k+1* is until node *k* has arrived, so every cache miss stalls the loop. This is *pointer chasing*.

**Measured here** (C, `gcc -O2`, 5,000,000 elements, sum of all values, best of several runs):

| Traversal | Time | Relative |
|---|---|---|
| Array of `long` | 2.47 ms | 1× |
| Linked list, nodes allocated in order (neighbors in memory) | 27.42 ms | ~11× |
| Linked list, same nodes linked in **random memory order** | 684.07 ms | **~277×** |

Same Big-O (O(n)), wildly different wall-clock time. The middle row shows that a list whose nodes happen to be contiguous (e.g., from a pool/arena allocator) is far faster than the last row, which is what a long-lived, heavily-mutated list on a fragmented heap looks like.

**Measured here** (Python 3.13, 100,000 elements):

| Task | `list` | `deque` / linked list |
|---|---|---|
| Drain all items from the front | `pop(0)`: 831 ms | `deque.popleft()`: **2.5 ms** |
| 20,000 inserts at the front | `insert(0, x)`: 339 ms | `deque.appendleft()`: **0.9 ms** |
| 20,000 reads of the middle element | `l[50000]`: **0.5 ms** | `deque[50000]`: 31.6 ms |
| Iterate all items | built-in `list`: **0.66 ms** | hand-written linked list: 3.16 ms |

Takeaways: (1) `deque` crushes `list` for front operations (O(1) vs O(n)), (2) `deque`'s indexing in the middle is slow because it must walk blocks, (3) a hand-written Python linked list is ~5× slower to iterate than a built-in list because every step is interpreted bytecode plus pointer chasing.

> **Reproduce:** `make bench` (runs both). Numbers vary from run to run; across my runs the shuffled-list vs. array ratio ranged from roughly 270× to 315×. The shape (cache-friendly array ≪ ordered list ≪ scattered list) is what matters.

### 7.3 What the research says
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#73-what-the-research-says`](#73-what-the-research-says)
<!-- paths:end -->

Bjarne Stroustrup's well-known benchmark (GoingNative 2012, *"Why you should avoid linked lists"*) inserts random numbers into a sorted sequence. The theory says the list should win (O(1) insert vs. O(n) shift). In practice the vector wins, often by a large factor, because the **search** to find the insertion point is the dominant cost and it is brutally cache-unfriendly on a list. A 2023 study (*"RIP Linked List"*, arXiv:2306.06942) confirms the clear split between linked and array-based implementations and notes the cost lies in random access, not insertion/deletion, and that an index cache can help linked lists.

The nuance (from the practitioner discussions in the same sources): arrays win for **small elements**; as elements get large (the DZone write-up puts the break-even around 32+ bytes), the cost of *moving* elements grows and linked lists become competitive. Stable references and avoiding reallocation are the other genuine wins.

### 7.4 Making linked lists faster when you must use them
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#74-making-linked-lists-faster-when-you-must-use-them`](#74-making-linked-lists-faster-when-you-must-use-them)
<!-- paths:end -->

- **Allocate nodes from a pool/arena** so neighbors are adjacent in memory (the 11× vs 277× gap above).
- **Unroll**: store several elements per node (what `deque` does).
- **Intrusive design**: embed the link in your object; one allocation instead of two.
- **Keep `size` and `tail`** so `len` and `append` aren't O(n).
- **Sentinel nodes** remove branches from hot paths.

---

## 8. When (and when not) to use a linked list
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#8-when-and-when-not-to-use-a-linked-list`](#8-when-and-when-not-to-use-a-linked-list)
<!-- paths:end -->

### 8.1 Good fits
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#81-good-fits`](#81-good-fits)
<!-- paths:end -->

| Situation | Why a linked list fits |
|---|---|
| You hold **node handles** and splice/unlink them constantly (LRU, schedulers, undo history) | O(1) unlink/relink; handles stay valid while others change |
| **Stable addresses** required (other structures keep pointers to elements) | Nodes never move; arrays reallocate |
| **Large, expensive-to-move** elements, or non-copyable/non-movable objects | Relinking is cheaper than moving big payloads |
| **Frequent insert/delete at the ends only** (stack, queue, deque) | O(1) at ends, no resizing |
| **Hard real-time / no-reallocation** systems | Every insert costs the same small, bounded work: no occasional O(n) resize spike |
| **Memory-constrained, unknown size**, can't reserve a big block | Grows one node at a time |
| **Kernel/embedded code** with intrusive lists | No extra allocation; one object on many lists |
| Building blocks for other structures | Hash-table chains, adjacency lists, free lists, queues in allocators |

### 8.2 Poor fits
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#82-poor-fits`](#82-poor-fits)
<!-- paths:end -->

| Situation | Use instead |
|---|---|
| Random access by index | Array / `list` / `vector` |
| Mostly iterating over small elements | Array: cache-friendly, often 10×+ faster |
| Sorting a lot, binary search | Array (then sort/bisect) |
| Lookup by key | Hash table / balanced tree |
| Memory-tight with tiny elements | Array: 4-byte int vs. 16–24-byte node |
| Python for general sequences | `list`; `deque` for queues |

### 8.3 A decision flow
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#83-a-decision-flow`](#83-a-decision-flow)
<!-- paths:end -->

```text
Need lookup by KEY?  ───────────────────────► hash table / tree
        │ no
Need access by INDEX (a[i]) often?  ─────────► array / list / vector
        │ no
Only touch the ENDS (push/pop front/back)?  ─► deque (array-based or block-linked)
        │ no
Do you splice / unlink by HANDLE in O(1),
or need STABLE ADDRESSES, or huge elements? ─► LINKED LIST (doubly if you unlink by handle)
        │ no
                                              ─► default to an array
```

---

## 9. Problems solved with linked lists
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#9-problems-solved-with-linked-lists`](#9-problems-solved-with-linked-lists)
<!-- paths:end -->

### 9.1 Pattern catalog
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#91-pattern-catalog`](#91-pattern-catalog)
<!-- paths:end -->

| Pattern | Technique | Classic problems |
|---|---|---|
| **Pointer rewiring** | `prev/cur/next` | Reverse a list, reverse in groups of *k*, reverse a sublist, swap nodes in pairs |
| **Slow/fast pointers** | one pointer moves 2× | Find the middle, detect a cycle, find the cycle's start, check palindrome (middle + reverse half) |
| **Two pointers with a gap** | advance one pointer *n* steps first | Remove the *n*-th node from the end, find the *k*-th from last |
| **Dummy/sentinel head** | fake node before head | Merge two sorted lists, remove all nodes with value *x*, partition a list |
| **Merge** | stable pointer relinking | Merge sorted lists, **merge sort on a linked list** (O(n log n), O(1) extra space, the natural sort for lists) |
| **Hash map + list** | O(1) lookup + O(1) reorder | **LRU/LFU cache**, ordered set with insertion order |
| **Handle-based unlink** | doubly linked + node handles | Text-editor buffers, undo/redo, browser history, playlists |
| **Linked structure as component** | list as bucket/adjacency | Hash-table collision chains, graph adjacency lists, polynomial arithmetic, big-number digits |

### 9.2 Real-world uses
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#92-real-world-uses`](#92-real-world-uses)
<!-- paths:end -->

| Where | How |
|---|---|
| **Linux kernel** | `struct list_head`: a *circular, intrusive, doubly linked* list embedded in structs (tasks, devices, ...); one `list_head` field lets any struct join a list, and the same node can be on several lists |
| **Python `collections.deque`** | Doubly linked list of fixed-size blocks (64 slots in CPython) |
| **Java `LinkedList`, C++ `std::list`** | Doubly linked list containers |
| **LRU caches** | Hash map + doubly linked list (see `LRUCache` above) |
| **Hash tables** (separate chaining) | Each bucket is a list |
| **Memory allocators** | Free lists of available blocks |
| **OS schedulers / task queues** | Circular lists for round-robin; run queues |
| **Undo/redo, browser history, music playlists** | Doubly linked navigation |
| **Graphs** | Adjacency lists |
| **Blockchains (conceptually)** | Each block points to the previous block's hash |

### 9.3 Worked example: remove the *n*-th node from the end (one pass)
<!-- code: python/src/linked_lists/algorithms.py -->
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#93-worked-example-remove-the-n-th-node-from-the-end-one-pass`](#93-worked-example-remove-the-n-th-node-from-the-end-one-pass) · 💻 **Code:** [`python/src/linked_lists/algorithms.py`](../python/src/linked_lists/algorithms.py)
<!-- paths:end -->

Technique: **dummy head + gap pointers**. (The tested version is `remove_nth_from_end` in `python/src/linked_lists/algorithms.py`.)

```python
def remove_nth_from_end(head, n):
    dummy = Node(None, head)
    fast = slow = dummy
    for _ in range(n):            # fast gets an n-step head start
        fast = fast.next
    while fast.next:              # move both until fast is on the LAST node
        fast, slow = fast.next, slow.next
    slow.next = slow.next.next    # slow is just before the victim: bypass it
    return dummy.next
```

```text
list: 1→2→3→4→5, n=2        (remove 4)
dummy→1→2→3→4→5
 S,F
 gap of 2:   S         F(at 2)
 slide:      S=3       F=5 (last)      → slow.next is 4 → skip it
result: 1→2→3→5
```

The dummy node makes "remove the head" (n equals the length) work without a special case.

### 9.4 Worked example: LRU cache, why a *doubly* linked list
<!-- code: python/src/linked_lists/lru_cache.py -->
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#94-worked-example-lru-cache-why-a-doubly-linked-list`](#94-worked-example-lru-cache-why-a-doubly-linked-list) · 💻 **Code:** [`python/src/linked_lists/lru_cache.py`](../python/src/linked_lists/lru_cache.py)
<!-- paths:end -->

Requirements: `get` and `put` in **O(1)**, evict the least-recently-used item when full.

- A **hash map** gives O(1) key → node.
- A **doubly linked list** keeps recency order: front = newest, back = oldest.
- On access: `unlink(node)` + `push_front(node)`: O(1) because the map hands us the node and the node knows both neighbors. A singly linked list would need O(n) to find the predecessor. An array would need O(n) to shift.

```text
capacity 3, order shown front→back (MRU → LRU)

put A     [A]
put B     [B, A]
put C     [C, B, A]
get A     [A, C, B]        A unlinked from the back, re-linked at the front
put D     [D, A, C]        full → pop_back evicts B (the LRU)
```

The full implementation (tested) is `LRUCache` in Part 6.1.

---

## 10. Alternatives: side-by-side comparison
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#10-alternatives-side-by-side-comparison`](#10-alternatives-side-by-side-comparison)
<!-- paths:end -->

### 10.1 Data structure comparison
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#101-data-structure-comparison`](#101-data-structure-comparison)
<!-- paths:end -->

*n = number of elements. "Amortized" = averaged over many operations. Costs are for the typical implementation.*

| | **Singly linked list** | **Doubly linked list** | **Dynamic array** (`list`, `vector`) | **Fixed array** | **Deque** (block-linked / ring) | **Circular buffer** | **Skip list** | **Balanced BST** | **Hash table** |
|---|---|---|---|---|---|---|---|---|---|
| Index access | O(n) | O(n) | **O(1)** | **O(1)** | O(1) ring / O(n/64) blocks | **O(1)** | O(log n) | O(log n)* | – |
| Search by value | O(n) | O(n) | O(n) / O(log n) sorted | O(n) / O(log n) sorted | O(n) | O(n) | O(log n) | O(log n) | **O(1)** avg |
| Insert front | **O(1)** | **O(1)** | O(n) | O(n) | **O(1)** | O(1)† | O(log n) | O(log n) | – |
| Insert back | O(1) w/ tail | **O(1)** | O(1) amortized | O(1) if room | **O(1)** | O(1)† | O(log n) | O(log n) | – |
| Insert middle (have position) | O(1) | O(1) | O(n) | O(n) | O(n) | O(n) | O(log n) | O(log n) | – |
| Delete given node/handle | O(n) | **O(1)** | O(n) | O(n) | O(n) | O(n) | O(log n) | O(log n) | O(1) avg |
| Ordered iteration | ✔ insertion order | ✔ both ways | ✔ | ✔ | ✔ | ✔ | ✔ **sorted** | ✔ **sorted** | ✘ (unordered) |
| Memory overhead | +1 ptr/elem | +2 ptr/elem | spare capacity | none | small | none | several ptrs/elem | 2–3 ptrs/elem | load-factor slack |
| Cache friendliness | ✘ poor | ✘ poor | ✔ **excellent** | ✔ excellent | ✔ good | ✔ excellent | ✘ poor | ✘ poor | medium |
| Pointers/elements stay valid after insert | ✔ **yes** | ✔ **yes** | ✘ (may reallocate) | ✔ | mostly | ✔ | ✔ | ✔ | ✘ (on rehash) |
| Worst-case single op | O(1) front | O(1) ends | **O(n) on resize** | – | O(1) | O(1) | O(n) expected small | O(log n) | O(n) on rehash |
| Implementation complexity | low | medium | built in | trivial | built in | low | high | high | built in |

<sub>* if augmented with subtree sizes (order-statistic tree). † bounded capacity; overwrites or rejects when full.</sub>

### 10.2 Which one for which job?
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#102-which-one-for-which-job`](#102-which-one-for-which-job)
<!-- paths:end -->

| Your problem | Best choice | Why |
|---|---|---|
| General-purpose sequence | **Dynamic array** | Fastest in practice; random access; cache-friendly |
| Queue (FIFO) | **Deque / ring buffer** | O(1) at both ends without per-node allocation |
| Stack (LIFO) | **Dynamic array** (push/pop at the end) | Simplest, fastest. A list works but isn't needed |
| LRU / LFU cache | **Hash map + doubly linked list** | Needs O(1) handle-based unlink |
| Sorted data with inserts + lookups | **Balanced BST / skip list / B-tree** | O(log n) ordered operations |
| Fast lookup by key | **Hash table** | O(1) average |
| Frequent insert/delete in the middle of *large* records, handle-based | **Doubly linked list** | Relink, don't move |
| Streaming sliding window | **Deque** | O(1) both ends |
| Fixed-size recent history | **Circular buffer** | No allocation, perfect locality |
| Priority-based processing | **Heap** | O(log n) push/pop-min |
| Embedded/kernel, objects in many lists | **Intrusive linked list** | No extra allocation |

### 10.3 Linked list vs. array: the one-page version
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#103-linked-list-vs-array-the-one-page-version`](#103-linked-list-vs-array-the-one-page-version)
<!-- paths:end -->

| | Linked list | Array |
|---|---|---|
| Think of it as | A **scavenger hunt**: each clue points to the next | A **row of numbered lockers**: go straight to any |
| Strength | Cheap restructuring; stable addresses; smooth growth | Fast access & iteration; compact; cache-friendly |
| Weakness | Slow access; memory overhead; cache misses | Costly middle/front inserts; occasional resize spike |
| Real-world default? | **No**, specialized | **Yes** |

---

## 11. Common bugs, checklist, and exercises
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#11-common-bugs-checklist-and-exercises`](#11-common-bugs-checklist-and-exercises)
<!-- paths:end -->

### 11.1 Bug checklist (C)
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#111-bug-checklist-c`](#111-bug-checklist-c)
<!-- paths:end -->

- [ ] **Lost the head** (reassigned `head` before saving the old one) → leaked/orphaned nodes
- [ ] **Wrong pointer order** on insert (splice before connecting forward) → orphaned tail
- [ ] **Use-after-free** (`free(cur); cur = cur->next;`). Save `next` first.
- [ ] **Double free**, e.g., two lists that both still point at the same nodes after a merge
- [ ] **Forgot `NULL` check** on `malloc`, or on `head`/`next` before dereferencing
- [ ] **Forgot to update `tail`** when deleting the last node or emptying the list
- [ ] **Forgot to update `size`**
- [ ] **Off-by-one in index loops** (insert at `index` needs the node at `index-1`)
- [ ] **Memory leak**: no `list_free` on every exit path
- [ ] Always test: **empty list, one element, two elements, head, tail, middle**

### 11.2 Bug checklist (Python)
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#112-bug-checklist-python`](#112-bug-checklist-python)
<!-- paths:end -->

- [ ] `None` dereference: `cur.next.next` when `cur.next` is `None`
- [ ] Infinite loop from an accidental **cycle** after a bad relink
- [ ] Using `==` where you meant `is` when comparing nodes (use `is` for identity)
- [ ] Mutating while iterating (restructure carefully or iterate over a saved `next`)
- [ ] Reaching for a hand-written list when `deque`/`list` is faster and clearer

### 11.3 Debugging technique
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#113-debugging-technique`](#113-debugging-technique)
<!-- paths:end -->

Draw boxes and arrows for **before** and **after**, then write the pointer assignments in an order that never destroys information you still need. Test at the boundaries (empty, one node, head, tail).

### 11.4 Exercises (in rough order of difficulty)
<!-- code: c/tests/, python/tests/ -->
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#114-exercises-in-rough-order-of-difficulty`](#114-exercises-in-rough-order-of-difficulty) · 💻 **Code:** [`c/tests/`](../c/tests), [`python/tests/`](../python/tests)
<!-- paths:end -->

> **Write the test first.** Put C exercises in `c/tests/` (a new `test_*.c` file is picked up automatically by `make -C c test`) and Python exercises in `python/tests/` (any `test_*.py` file is picked up by pytest).

**Warm-up**
1. Add `list_get(l, i)` and `list_set(l, i, v)` to the C list. State their complexity.
2. Add `length` computed by traversal, then compare it with the cached `size`.
3. Implement `list_remove_all(l, value)`.

**Core**
4. Reverse a list **recursively**. What is the space complexity? (Answer: O(n) call stack.)
5. Merge two sorted lists (you have the iterative version; do it recursively).
6. Remove the *n*-th node from the end in one pass (Part 9.3; port it to C).
7. Detect a cycle, then **find where the cycle begins** (hint: after the pointers meet, reset one to head and move both 1 step at a time).
8. Check whether a list is a **palindrome** in O(n) time and O(1) space (middle + reverse the second half + compare + restore).

**Challenge**
9. **Merge sort** a linked list (split with slow/fast, sort halves, merge).
10. Add two numbers represented as digit lists (`2→4→3` + `5→6→4` = `7→0→8`).
11. **Reverse in groups of k**.
12. Implement a **deque** and a **stack** on top of your doubly linked list; benchmark against Python's `deque`.
13. Port `LRUCache` to C using `DList` + a hash table you write yourself.
14. Write an **unrolled** linked list (node = array of 8–16 items) and measure the iteration speedup vs. your plain list.
15. **Benchmark yourself:** reproduce Part 7's array-vs-list traversal test, and try allocating nodes from one big pool vs. individually with `malloc`.

---

## 12. References
<!-- paths:start -->
> 📍 **Path:** [`docs/linked-list-with-c-and-python.md#12-references`](#12-references)
<!-- paths:end -->

- Linux kernel documentation, *Linked Lists in Linux*: https://docs.kernel.org/core-api/list.html
- *Data Structures in the Linux Kernel: Doubly linked list* (linux-insides, 0xax): https://0xax.gitbooks.io/linux-insides/content/DataStructures/linux-datastructures-1.html
- CPython `collections.deque` (block-linked implementation, `Modules/_collectionsmodule.c`), and Laurent Luce, *Python deque implementation*: https://www.laurentluce.com/posts/python-deque-implementation/
- Stroustrup, B., *Why you should avoid linked lists* (GoingNative 2012 keynote)
- *RIP Linked List*, empirical study of linked vs. array-based lists: https://arxiv.org/abs/2306.06942
- *Performance of Array vs. Linked-List on Modern Computers* (DZone): https://dzone.com/articles/performance-of-array-vs-linked-list-on-modern-comp
- Unity, a unit test framework for C (ThrowTheSwitch): https://github.com/ThrowTheSwitch/Unity
- pytest documentation: https://docs.pytest.org
- Cormen, Leiserson, Rivest, Stein, *Introduction to Algorithms* (sentinels, §10.2)
- Sedgewick & Wayne, *Algorithms*, 4th ed. (linked lists, bags/stacks/queues)

---

*End of guide. The most important lesson isn't a pointer trick: it's that **Big-O tells you how a data structure scales, and the hardware tells you how fast it actually runs.** Know both.*
