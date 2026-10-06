# Python part of the project. Run `make` (or `make help`) to see the targets.
#
#   make test    create .venv if needed, install pytest, run the test suite
#   make run     run the demo (no dependencies needed)
#   make bench   run the list/deque/linked-list benchmark
#   make clean   remove the virtual environment and caches
#
# Options:  PYTHON=python3.12   VENV=.venv

PYTHON ?= python3
VENV   ?= .venv
BIN    := $(VENV)/bin
STAMP  := $(VENV)/.deps-installed

.DEFAULT_GOAL := help
.PHONY: help test run bench clean

help:
	@echo "Targets:"
	@echo "  make test    create $(VENV) if needed, install pytest, run the tests"
	@echo "  make run     run the demo"
	@echo "  make bench   run the benchmark"
	@echo "  make clean   remove $(VENV) and caches"
	@echo "Options: PYTHON=python3.12  VENV=.venv"

# Re-creates the venv only when requirements-dev.txt changes.
$(STAMP): requirements-dev.txt
	$(PYTHON) -m venv $(VENV)
	$(BIN)/python -m pip install --quiet --upgrade pip
	$(BIN)/python -m pip install --quiet -r requirements-dev.txt
	@touch $(STAMP)

test: $(STAMP)
	$(BIN)/python -m pytest

run:
	PYTHONPATH=src $(PYTHON) examples/demo.py

bench:
	PYTHONPATH=src $(PYTHON) bench/bench.py

clean:
	rm -rf $(VENV) .pytest_cache src/*.egg-info
	find . -name __pycache__ -type d -prune -exec rm -rf {} +
