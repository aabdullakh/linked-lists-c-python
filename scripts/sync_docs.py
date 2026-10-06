#!/usr/bin/env python3
"""Keep the guide in docs/ in sync with the repository.

What it does to every docs/*.md file:

1. Code listings.  A line  <!-- source: path/to/file -->  followed by a fenced
   code block gets that block's contents replaced with the file's contents.
2. Header paths.  Under EVERY header (#, ##, ###) it generates a "Path" line
   (the guide's own partial path + anchor).  If the line right after the header
   is  <!-- code: a, b, c -->  it also adds a "Code" line linking those repo paths.
3. Checks.  Every referenced repo path must exist, and every in-page
   [text](#anchor) link must point at a real header.

Usage:
    python3 scripts/sync_docs.py            # rewrite files in place
    python3 scripts/sync_docs.py --check    # exit 1 if anything is out of date (CI)
"""
from __future__ import annotations

import difflib
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DOCS = sorted((ROOT / "docs").glob("*.md"))

SOURCE_RE = re.compile(r"^<!-- source: (.+?) -->$")
CODE_RE = re.compile(r"^<!-- code: (.+?) -->$")
HEADING_RE = re.compile(r"^(#{1,3}) (.+?)\s*$")
START, END = "<!-- paths:start -->", "<!-- paths:end -->"


def slug(heading: str) -> str:
    """GitHub-style anchor: lowercase, drop punctuation, spaces -> hyphens."""
    s = heading.strip().lower()
    s = re.sub(r"[^\w\- ]", "", s)
    return s.replace(" ", "-")


def strip_generated(lines: list[str]) -> list[str]:
    out, skipping = [], False
    for ln in lines:
        if ln == START:
            skipping = True
        elif skipping:
            if ln == END:
                skipping = False
        else:
            out.append(ln)
    return out


def rel_from_doc(doc: Path, repo_path: str) -> str:
    return Path(__import__("os").path.relpath(ROOT / repo_path, doc.parent)).as_posix()


def process(doc: Path, errors: list[str]) -> str:
    text = doc.read_text(encoding="utf-8")
    lines = strip_generated(text.split("\n"))
    doc_rel = doc.relative_to(ROOT).as_posix()
    out: list[str] = []
    slugs: dict[str, str] = {}
    fence = False
    i = 0
    while i < len(lines):
        ln = lines[i]

        m = SOURCE_RE.match(ln)
        if m and not fence and i + 1 < len(lines) and lines[i + 1].startswith("```"):
            src = ROOT / m.group(1)
            j = i + 2
            while j < len(lines) and not lines[j].startswith("```"):
                j += 1
            if not src.is_file():
                errors.append(f"{doc_rel}: source file not found: {m.group(1)}")
                out.extend(lines[i : j + 1])
            else:
                out += [ln, lines[i + 1]]
                out += src.read_text(encoding="utf-8").rstrip("\n").split("\n")
                out.append("```")
            i = j + 1
            continue

        if ln.startswith("```"):
            fence = not fence
            out.append(ln)
            i += 1
            continue

        h = HEADING_RE.match(ln) if not fence else None
        if h:
            s = slug(h.group(2))
            if s in slugs:
                errors.append(f"{doc_rel}: duplicate anchor #{s} ({h.group(2)!r})")
            slugs[s] = h.group(2)
            out.append(ln)
            i += 1
            code_paths: list[str] = []
            if i < len(lines) and (cm := CODE_RE.match(lines[i])):
                out.append(lines[i])
                code_paths = [p.strip() for p in cm.group(1).split(",")]
                i += 1
            parts = [f"📍 **Path:** [`{doc_rel}#{s}`](#{s})"]
            if code_paths:
                links = []
                for p in code_paths:
                    if not (ROOT / p).exists():
                        errors.append(f"{doc_rel}: header {h.group(2)!r} references missing path {p}")
                    links.append(f"[`{p}`]({rel_from_doc(doc, p)})")
                parts.append("💻 **Code:** " + ", ".join(links))
            out += [START, "> " + " · ".join(parts), END]
            if i >= len(lines) or lines[i] != "":
                out.append("")
            continue

        out.append(ln)
        i += 1

    new = "\n".join(out)

    # in-page anchor links must resolve (ignore anything inside code fences)
    in_fence = False
    for ln in new.split("\n"):
        if ln.startswith("```"):
            in_fence = not in_fence
        elif not in_fence:
            for a in re.findall(r"\]\(#([^)]+)\)", ln):
                if a not in slugs:
                    errors.append(f"{doc_rel}: broken in-page link #{a}")
    return new


def main() -> int:
    check = "--check" in sys.argv[1:]
    errors: list[str] = []
    stale = False
    for doc in DOCS:
        new = process(doc, errors)
        old = doc.read_text(encoding="utf-8")
        if new != old:
            if check:
                stale = True
                print(f"{doc.relative_to(ROOT)} is out of date. Run: make docs")
                diff = difflib.unified_diff(old.split("\n"), new.split("\n"), lineterm="", n=0)
                for k, d in enumerate(diff):
                    if k > 40:
                        print("   ...")
                        break
                    print("   " + d)
            else:
                doc.write_text(new, encoding="utf-8")
                print(f"updated {doc.relative_to(ROOT)}")
        else:
            print(f"ok      {doc.relative_to(ROOT)}")
    for e in errors:
        print("ERROR:", e)
    return 1 if (errors or stale) else 0


if __name__ == "__main__":
    sys.exit(main())
