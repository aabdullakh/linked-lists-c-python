# C part of the project. Run `make` (or `make help`) to see the targets.
#
#   make test    build and run all Unity tests (with AddressSanitizer + UBSan)
#   make run     build and run the demo
#   make bench   build (optimized) and run the array-vs-list benchmark
#   make clean   remove build output
#
# Options:  CC=clang   SANITIZE=0 (disable sanitizers)   V=1 (show commands)

CC       ?= cc
SANITIZE ?= 1
V        ?= 0

CSTD   := -std=c11
WARN   := -Wall -Wextra -Wpedantic
INC    := -Iinclude -Ithird_party/unity
BUILD  := build

ifeq ($(SANITIZE),1)
SAN := -fsanitize=address,undefined -fno-omit-frame-pointer
endif

ifeq ($(V),1)
Q :=
else
Q := @
endif

LIB_SRC   := src/singly_list.c src/doubly_list.c
UNITY_SRC := third_party/unity/unity.c
HEADERS   := $(wildcard include/*.h)
TEST_SRC  := $(wildcard tests/test_*.c)
TEST_BINS := $(patsubst tests/%.c,$(BUILD)/%,$(TEST_SRC))

.DEFAULT_GOAL := help
.PHONY: help test run bench clean

help:
	@echo "Targets:"
	@echo "  make test    build and run all Unity tests (sanitizers on by default)"
	@echo "  make run     build and run the demo"
	@echo "  make bench   build (-O2) and run the benchmark"
	@echo "  make clean   remove build output"
	@echo "Options: CC=clang  SANITIZE=0  V=1"

$(BUILD):
	$(Q)mkdir -p $(BUILD)

# One test binary per tests/test_*.c file, linked with the library and Unity.
$(BUILD)/test_%: tests/test_%.c $(LIB_SRC) $(UNITY_SRC) $(HEADERS) | $(BUILD)
	$(Q)$(CC) $(CSTD) $(WARN) $(SAN) -g -O0 $(INC) tests/test_$*.c $(LIB_SRC) $(UNITY_SRC) -o $@

test: $(TEST_BINS)
	@set -e; for t in $(TEST_BINS); do \
		echo "=== $$t ==="; \
		./$$t; \
	done; \
	echo; echo "All C test suites passed."

$(BUILD)/demo: examples/demo.c $(LIB_SRC) $(HEADERS) | $(BUILD)
	$(Q)$(CC) $(CSTD) $(WARN) $(SAN) -g $(INC) examples/demo.c $(LIB_SRC) -o $@

run: $(BUILD)/demo
	@./$(BUILD)/demo

# The benchmark is built WITHOUT sanitizers: they would distort the timings.
$(BUILD)/bench: bench/bench.c | $(BUILD)
	$(Q)$(CC) $(CSTD) $(WARN) -O2 -D_POSIX_C_SOURCE=199309L bench/bench.c -o $@

bench: $(BUILD)/bench
	@./$(BUILD)/bench

clean:
	rm -rf $(BUILD)
