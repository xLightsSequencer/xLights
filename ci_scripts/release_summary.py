#!/usr/bin/env python3
"""Extract one store's "What's new" summary from RELEASE_SUMMARIES.txt.

Usage: release_summary.py <store> [RELEASE_SUMMARIES.txt]

<store> is matched case-insensitively against the section headers, which look
like `=== Windows Store (limit 1500) ===`. The section body is written to
stdout. Exits 1 if the section is missing/empty or longer than the limit the
header declares, so the release workflow can skip the update rather than push
text the store would reject.
"""
import re
import sys
from pathlib import Path

HEADER_RE = re.compile(r"^===\s*(.+?)\s*(?:\(limit\s*(\d+)\)\s*)?===\s*$")


def extract_section(lines, store):
    """Return (body, limit) for the first header containing <store>."""
    want = store.lower()
    body = None
    limit = None
    for line in lines:
        m = HEADER_RE.match(line)
        if m:
            if body is not None:
                break  # hit the next section — stop
            if want in m.group(1).lower():
                body = []
                limit = int(m.group(2)) if m.group(2) else None
            continue
        if body is not None:
            body.append(line.rstrip())
    return ("\n".join(body).strip() if body is not None else None), limit


def main():
    if len(sys.argv) < 2:
        print("usage: release_summary.py <store> [RELEASE_SUMMARIES.txt]", file=sys.stderr)
        return 2
    store = sys.argv[1]
    path = Path(sys.argv[2] if len(sys.argv) > 2 else "RELEASE_SUMMARIES.txt")
    if not path.exists():
        print(f"{path} not found", file=sys.stderr)
        return 1
    text, limit = extract_section(path.read_text(encoding="utf-8").splitlines(), store)
    if not text:
        print(f"no '{store}' section in {path}", file=sys.stderr)
        return 1
    if limit is not None and len(text) > limit:
        print(f"'{store}' section is {len(text)} chars, over its {limit} limit", file=sys.stderr)
        return 1
    sys.stdout.write(text + "\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
