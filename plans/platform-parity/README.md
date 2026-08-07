# Platform Feature Parity — audit docs

A from-scratch, code-derived audit of feature parity across xLights platforms:
primarily **iPad ↔ Desktop**, plus the **macOS ↔ Windows ↔ Linux** differences within the
desktop build. Generated 2026-07-31/08-01 by walking the code directly — the previous
`plans/ipad-parity/` docs were deliberately **not** consulted during generation; this set
replaces them (the old directory has been removed).

**Verification pass (2026-08-01):** after generation, every theme was adversarially
cross-checked against the prior `plans/ipad-parity/` docs, with each disagreement settled by
reading current code. ~27 wrong statuses were corrected and ~75 rows added; every major
finding survived. The dominant error mode in generated rows was a ❌ "proven" by a search
that could never have matched the other platform's identifiers — when updating rows, make
absence claims with searches demonstrated against the counterpart's real symbol names (and
mind zsh's `grep -E "a\|b"` literal-pipe trap; use `-e` alternation). Product scope decisions
(controller-upload scope, jukebox, the settings redo, AC deferral) are recorded in
`00-overview.md` §Decisions with their dates.

Start with [00-overview.md](00-overview.md): headline tallies, the severity-grouped gap
inventory, the P1/P2/P3 roadmap, the cross-OS summary, and the reverse-parity shortlist.
Themes 01–14 are the iPad axis; theme 15 is the desktop cross-OS matrix.

## How to read a scorecard

Every theme doc has a `## Features` table (one row per feature — deliberately fine-grained:
a menu entry, a dialog field, a keyboard/touch behavior each get their own row), a
`## Desktop platform differences` section (mac/win/linux notes encountered in that theme's
code), and a `## Notes` section (structural observations plus a suggested porting order —
the roadmap in `00-overview.md` was synthesized from these).

Status legend:

| Mark | Meaning |
|---|---|
| ✅ | Parity — the iPad UI actually reaches the behavior |
| 🟡 | Partial — present but weaker, or core-shared with no iPad UI exposure |
| ❌ | Missing on iPad |
| 🚫 | Infeasible/restricted on iPad — the row states the platform reason |
| 🔵 | iPad-only (desktop lacks it) |
| ➖ | Not applicable to iPad (theme 11 only) |

## Evidence rules used

- Every status claim carries `file:line` evidence; ❌ rows also record the iPad-side
  search that came up empty, so an absence claim can be re-verified (or falsified) later.
- A feature implemented in shared `src-core/` counts ✅ only if the iPad UI actually
  reaches it; otherwise it is 🟡 "core-shared, no iPad UI".
- Line numbers are accurate as of the generation date and will drift with the code —
  treat them as anchors for `grep`, not gospel.

## Tallies are machine-derived — use `tally.sh`

The per-theme counts in `00-overview.md` are **derived, not hand-maintained**. After editing
any scorecard rows:

```bash
plans/platform-parity/tally.sh        # recompute + diff against the overview (CI-able; exits 1 on drift)
plans/platform-parity/tally.sh fix    # rewrite the overview's numeric cells + parity-index numbers in place
plans/platform-parity/tally.sh lint   # flag status emoji outside status cells
```

The counting rule (also documented in the script): **a table row contributes 1 to each
distinct status emoji it contains.** Single-status tables contribute one mark per row; the
multi-status matrices (04 per-effect: render+settings, 06 model-type: create/edit/renders,
07 output-type: configure/live) contribute one mark per distinct status in the row. Two
conventions make this work: status emoji are **reserved for status cells** — in prose or
evidence cells write the status in words ("marked missing", "iPad-only"), never the emoji —
and new multi-status tables must be added to the linter's allowance list in `tally.sh`.
Never adjust the overview numbers by hand or by delta; run `fix`.

## Maintenance

These docs are snapshots, not living state — but keep the *rows* honest: when a PR closes
a gap, flip that row's status in the theme doc (per AGENTS.md §3 the parity accounting is
part of change completeness). If a theme has drifted too far, regenerate it the same way
this set was made: enumerate the desktop feature superset from `src-ui-wx/` + `src-core/`,
check each feature against `src-iPad/`, require file:line on both sides, and do not seed
from existing parity docs.
