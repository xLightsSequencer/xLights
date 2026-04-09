#!/usr/bin/env python3
"""Convert an xLights README.txt release notes block to GitHub-flavored markdown.

Usage: release_notes_to_md.py <version> [README.txt]

Reads the release-notes block for <version> out of README.txt (defaults to
./README.txt) and writes markdown to stdout. Used by the release workflow
to format the body posted to the GitHub release.
"""
import re
import sys
from pathlib import Path

VERSION_RE = re.compile(r"^(\d{4}\.\d{2,})\s+(.+)$")
ENTRY_RE = re.compile(r"^\s*-\s*(enh|bug|change)\s*\(([^)]*)\)\s*(.*)$")

TYPE_LABELS = [
    ("enh",    "Enhancements"),
    ("bug",    "Bug Fixes"),
    ("change", "Changes"),
]


def extract_version_block(lines, version):
    """Return (date, body_lines) for the requested version, or (None, [])."""
    in_block = False
    date = None
    body = []
    for line in lines:
        m = VERSION_RE.match(line)
        if m:
            if in_block:
                break  # hit the next version header — stop
            if m.group(1) == version:
                in_block = True
                date = m.group(2).strip()
            continue
        if in_block:
            body.append(line.rstrip())
    return date, body


def parse_entries(body_lines):
    """Group entries by type, joining continuation lines and sub-bullets."""
    groups = {key: [] for key, _ in TYPE_LABELS}
    current_type = None
    current_entry = None  # {"author", "text", "subs": [str]}

    def flush():
        nonlocal current_entry, current_type
        if current_entry is not None and current_type is not None:
            groups[current_type].append(current_entry)
        current_entry = None

    for line in body_lines:
        stripped = line.strip()
        if not stripped:
            continue
        m = ENTRY_RE.match(line)
        if m:
            flush()
            current_type = m.group(1)
            current_entry = {
                "author": m.group(2).strip(),
                "text": m.group(3).strip(),
                "subs": [],
            }
        elif current_entry is not None:
            if stripped.startswith("- "):
                current_entry["subs"].append(stripped[2:].strip())
            elif current_entry["subs"]:
                # continuation of the most recent sub-bullet
                current_entry["subs"][-1] += " " + stripped
            else:
                # continuation of the entry's main text
                current_entry["text"] = (current_entry["text"] + " " + stripped).strip()
    flush()
    return groups


def render_markdown(version, date, groups):
    out = [f"## {version} — {date}", ""]
    for key, label in TYPE_LABELS:
        entries = groups[key]
        if not entries:
            continue
        out.append(f"### {label}")
        out.append("")
        for entry in entries:
            text = entry["text"] or "(no description)"
            out.append(f"- **{entry['author']}** — {text}")
            for sub in entry["subs"]:
                out.append(f"  - {sub}")
        out.append("")
    return "\n".join(out).rstrip() + "\n"


def main():
    if len(sys.argv) < 2:
        print("usage: release_notes_to_md.py <version> [README.txt]", file=sys.stderr)
        return 2
    version = sys.argv[1]
    readme = Path(sys.argv[2] if len(sys.argv) > 2 else "README.txt")
    lines = readme.read_text(encoding="utf-8").splitlines()
    date, body = extract_version_block(lines, version)
    if date is None:
        print(f"version {version} not found in {readme}", file=sys.stderr)
        return 1
    groups = parse_entries(body)
    sys.stdout.write(render_markdown(version, date, groups))
    return 0


if __name__ == "__main__":
    sys.exit(main())
