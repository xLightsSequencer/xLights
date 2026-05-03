# Future — AI Palette Generate

## Status

**Shipped 2026-04-30 (or earlier).** Verified 2026-05-02:
`AIPaletteGenerationSheet.swift` + `XLAIServices.{h,mm}` cover the
prompt → palette flow. The "Generate Palette…" entry lives in the
existing palette-header overflow menu alongside Save / Load / Import.
Backed by the same iOS-Keychain-backed API key store
(`AIServicesSettingsSheet.swift` — PR-9) used by AI image generation
([`future-ai-image-generate.md`](future-ai-image-generate.md)).

## Cross-references (gap analysis 2026-04-23 §2.15)

- **AI-1** AIColorPaletteDialog (desktop) — iPad equivalent landed.
- Pairs with AI-3 / AI-4 / AI-5 in
  [`future-ai-image-generate.md`](future-ai-image-generate.md).
  PR-9 ServicesPanel infrastructure is shared.
