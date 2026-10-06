# Linked Lists with C and Python

A hands-on, tested guide to **linked lists** for a Data Structures & Algorithms course:
theory, step-by-step diagrams, working implementations in **C** and **Python**, real
performance measurements, use cases, and a side-by-side comparison with alternatives.

**Start here: [`docs/linked-list-with-c-and-python.md`](docs/linked-list-with-c-and-python.md)**

## Quick start

You need `make`, a C compiler (`gcc` or `clang`) and Python 3.10+. On Windows, use WSL.

```bash
git clone <this-repository-url>
cd <repository-folder>

make test     # run all tests: C (Unity) + Python (pytest)
make run      # run both demos
make bench    # run both benchmarks (array vs. linked list, list vs. deque)
```

Nothing else to install: the Unity test framework is bundled in `c/third_party/unity/`,
and the Python Makefile creates its own virtual environment and installs pytest the first
time you run `make test`.

| From the repo root | Same thing, one language |
|---|---|
| `make test` | `make -C c test` · `make -C python test` |
| `make run` | `make -C c run` · `make -C python run` |
| `make bench` | `make -C c bench` · `make -C python bench` |
| `make clean` | `make -C c clean` · `make -C python clean` |

The C tests are compiled with AddressSanitizer and UBSan, so memory leaks,
use-after-free and double frees fail the test run.
Options: `make -C c test CC=clang`, `make -C c test SANITIZE=0`, `make -C c test V=1`.

## What is inside

```text
.
├── docs/linked-list-with-c-and-python.md    the guide
├── c/
│   ├── include/ src/                        singly + doubly linked lists
│   ├── tests/                               Unity tests
│   ├── examples/demo.c  bench/bench.c
│   ├── third_party/unity/                   Unity (MIT), bundled
│   └── Makefile
├── python/
│   ├── src/linked_lists/                    singly, doubly, algorithms, LRU cache
│   ├── tests/                               pytest tests
│   ├── examples/demo.py  bench/bench.py
│   └── Makefile
├── scripts/sync_docs.py                     keeps the guide's code listings in sync with the code
└── Makefile                                 run everything from here
```

| Implemented | C | Python |
|---|---|---|
| Singly linked list (head, tail, size) | `c/src/singly_list.c` | `python/src/linked_lists/singly.py` |
| Cycle detection, sorted merge | `c/src/singly_list.c` | `python/src/linked_lists/algorithms.py` |
| Doubly linked list with sentinel | `c/src/doubly_list.c` | `python/src/linked_lists/doubly.py` |
| LRU cache (hash map + doubly linked list) | (exercise 13 in the guide) | `python/src/linked_lists/lru_cache.py` |

## Keeping the guide and code in sync

Every code listing in the guide is embedded from the real source files, and every header
has a **Path** line. After changing code or headings, run:

```bash
make docs         # refresh listings and header paths
make docs-check   # fail if anything is stale (this is what CI runs)
```

## Contributing

Issues and pull requests are welcome. Please add or update tests (`c/tests/` for C,
`python/tests/` for Python), make sure `make test` and `make docs-check` pass, and keep
the code readable: this is a teaching repository.

## License

[MIT](LICENSE). The bundled Unity framework is MIT-licensed too; see
[`c/third_party/unity/LICENSE.txt`](c/third_party/unity/LICENSE.txt).
