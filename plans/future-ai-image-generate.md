# Future — AI Image Generate

## Status

**Shipped 2026-04-30 (or earlier).** Verified 2026-05-02:
`AIImageGenerationSheet.swift` + `XLAIImageSession.{h,mm}` (bridge
that returns generated PNG bytes) + `AIServicesSettingsSheet.swift`
(PR-9 ServicesPanel — Keychain-backed API key storage) all live in
the iPad app. Pictures effect and Media Manager both surface
"Generate Image…" entry points.

## Cross-references

The vendor-side parts of this surface (ShaderDownload, VendorModel,
VendorMusic, MatrixFaceDownload) live in the AI / vendor-downloader
slice of Phase O — not yet split into a dedicated future plan, and
remain unshipped on iPad.

## Original gap-analysis cross-refs (2026-04-23 §2.15)

- **AI-2** AIImageDialog (desktop) — landed.
- **AI-3** AppleIntelligence service (Image Playground + Foundation
  Models LLM) — entitlement-gated; iPad's image-gen ships through
  AI-4 (ChatGPT) by default. Apple Intelligence path can be added
  alongside if/when the desktop side adds it for iPad parity.
- **AI-4** ChatGPT service (OpenAI API) — landed via PR-9
  ServicesPanel.
- **AI-5** ServicesPanel (PR-9 in [`future-preferences.md`](future-preferences.md))
  — landed (`AIServicesSettingsSheet.swift`).
