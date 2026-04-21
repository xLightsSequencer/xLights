# Future — AI Palette Generate

Not in the first-pass Phase C scope. Captured here so parity with
desktop's `Generate Palette` action isn't forgotten.

## Gap

**G17-AI — AI-generated palette.** Desktop's `ColorPanel` has a
"Generate Palette" menu item that opens `AIColorPaletteDialog`
(`src-ui-wx/ai/AIColorPaletteDialog.{h,cpp}`) — the user types a
prompt, the dialog calls xLights' AI service, and the returned
colour list is loaded into the 8 palette slots. iPad has no
equivalent.

## Why deferred

- Depends on porting the AI service client to iOS — no bridge yet,
  and the desktop client uses libcurl + provider-specific JSON
  endpoints. Separate infrastructure work.
- Niche workflow; palette creation by hand or from saved `.xpalette`
  files covers the common cases (G17 save / load / import / export
  is the first-pass work).
- `AIColorPaletteDialog` itself could benefit from a UX rethink
  when this lands — worth doing desktop + iPad together.

## Scope when we come back

- iOS bridge for the shared AI service client (currently wx-
  embedded on desktop).
- SwiftUI dialog analogue to `AIColorPaletteDialog`: prompt text,
  model / provider picker, preview of the returned colours, Apply.
- Entry point: add "Generate Palette…" to the existing palette-
  header overflow menu alongside Save / Load / Import.
- Same service can back an AI entry for the Media manager
  (G33 — already tracked separately in Phase C C5 / future).
