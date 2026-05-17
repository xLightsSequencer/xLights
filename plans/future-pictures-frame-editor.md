# Future — Pictures Frame / GIF Timing Editor + Effect Assist panels

Not in the first-pass Phase C scope — the desktop implementation is
acknowledged to be rough and needs redesigning too. When the time
comes, the desktop and iPad versions should be designed together
rather than porting the current desktop UX one-for-one.

## Gap

**G6 — Pictures frame / GIF timing editor.** Desktop's
`PicturesAssistPanel` lets users scrub animated-GIF frames and set
per-frame timing. iPad has the filename picker only — no frame UI.

## Why deferred

- Rarely exercised workflow. Most users consume animated GIFs
  as-is; custom per-frame timing is a niche use.
- Desktop's current implementation is considered poor and due for
  a redesign. Porting the existing UX to iPad would entrench the
  weaknesses and then need to be redone on both platforms.

## When we come back to this

- Redesign the UX once, land it on both desktop and iPad as a
  shared schema-driven custom row where possible.
- Look at the underlying `AnimatedImageData` handling in
  `src-core/` to see whether the per-frame timing edit surface
  wants to live in core (which would naturally make the iPad
  side cheaper).

## Effect Assist sub-panels (gap analysis 2026-04-23 §2.11)

EA-1 SketchAssistPanel: partially shipped — endpoint drag, curve
reshaping, and path-list management on iPad via
`SketchPathEditorRowView.swift`. Cubic / quadratic creation, path
closing, SVG import / export remain desktop-only.

- **EA-2 PicturesAssistPanel** (XL) — multi-frame canvas,
  pencil/eraser/eyedropper/select/copy paint tools, palette
  manager (8-color swatches), color picker, image load / save /
  resize, pixel selection, multi-format support
  (PNG/BMP/JPG/GIF/WEBP). This file. Still open.
- **EA-3 MorphAssistPanel** (M) — 4-corner quadrilateral editor,
  start/end linking, region drag. Smallest of the three; could
  be pulled in independently. Still open.

EA-3 Morph is the only one likely to fit into a small follow-up
rather than a phase commitment.
