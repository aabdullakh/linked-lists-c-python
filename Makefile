# Top-level convenience Makefile: run everything from the repository root.
#
#   make test        run the C (Unity) and Python (pytest) test suites
#   make run         run both demos
#   make bench       run both benchmarks
#   make docs        refresh code listings and header paths in docs/
#   make docs-check  fail if docs/ is out of date (used by CI)
#   make clean       remove build output and the Python virtual environment

.DEFAULT_GOAL := help
.PHONY: help all test test-c test-python run run-c run-python bench bench-c bench-python docs docs-check clean

help:
	@echo "Targets:"
	@echo "  make test        run all tests (C with Unity, Python with pytest)"
	@echo "  make run         run both demos"
	@echo "  make bench       run both benchmarks"
	@echo "  make docs        refresh code listings and header paths in docs/"
	@echo "  make docs-check  fail if docs/ is out of date"
	@echo "  make clean       remove build output and the Python venv"
	@echo
	@echo "Per language:  make -C c <target>   |   make -C python <target>"

all: test

test: test-c test-python
test-c:
	$(MAKE) -C c test
test-python:
	$(MAKE) -C python test

run: run-c run-python
run-c:
	$(MAKE) -C c run
run-python:
	$(MAKE) -C python run

bench: bench-c bench-python
bench-c:
	$(MAKE) -C c bench
bench-python:
	$(MAKE) -C python bench

docs:
	python3 scripts/sync_docs.py

docs-check:
	python3 scripts/sync_docs.py --check

clean:
	$(MAKE) -C c clean
	$(MAKE) -C python clean
