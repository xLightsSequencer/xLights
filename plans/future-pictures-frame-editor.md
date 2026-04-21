# Future — Pictures Frame / GIF Timing Editor

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
  shared schema-driven custom row where possible (or matching
  wxSmith / SwiftUI panels when a shared schema doesn't fit).
- Look at the underlying `AnimatedImageData` handling in
  `src-core/` to see whether the per-frame timing edit surface
  wants to live in core (which would naturally make the iPad
  side cheaper).
