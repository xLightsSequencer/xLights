# Phase H — App Store readiness

**Status: H-0 / H-1 / H-2 / H-3 / H-4 complete; H-5 remaining
(organizational).**

Apple Developer team: Kulp Lights LLC. iPad ships as an
additional platform on the existing `org.xlights` App Store
Connect record (Universal Purchase) — iOS platform added
2026-04-22.

## Done

- **H-0 unified bundle ID.** iPad target
  `PRODUCT_BUNDLE_IDENTIFIER = org.xlights` shared with the
  Mac target across Debug / Release / Archive. Universal
  Purchase gives customers cross-platform access under one
  record. Future IAP catalog will use per-platform availability
  flags so iPad-only IAPs can be scoped without affecting Mac.
- **H-1 app icon + launch screen.** `src-iPad/AppIcon.icon/`
  Icon Composer bundle (Liquid Glass) with Light / Dark /
  Tintable appearance variants and a 1024×1024 raster fallback
  for pre-iOS-26;
  `src-iPad/Assets.xcassets/LaunchBackground.colorset` light
  (white) / dark (#1A1A1A); `LaunchLogo.imageset` 1200×460
  @2x; `UILaunchScreen` dict in Info.plist (replacing
  `INFOPLIST_KEY_UILaunchScreen_Generation`).
- **H-2 privacy manifest.**
  `macOS/Assets/xLights-iPad/PrivacyInfo.xcprivacy` declares
  `NSPrivacyTracking = false`, no tracked domains, no
  collected data types, and only required-reason APIs that
  xLights actually uses (FileTimestamp C617.1, UserDefaults
  CA92.1, SystemBootTime 35F9.1).
- **H-3 network + encryption Info.plist keys.**
  `NSLocalNetworkUsageDescription`, `NSBonjourServices =
  ["_fppd._udp"]`, `ITSAppUsesNonExemptEncryption = false`.
- **H-4 TestFlight.** Internal group running on every Xcode
  Cloud archive via Automatic Distribution; external group
  cleared Beta App Review and went live 2026-04-28.
- **Xcode Cloud infrastructure.**
  `ci_scripts/ci_pre_xcodebuild.sh` runs the shared
  `macOS/scripts/download_deps` (with retry loop for
  mid-transfer GitHub failures on the ~500 MB tarball);
  `xLights-core` → `xLights-Apple-core` dependency edge so
  the downloader runs first; bundle version stamping moved
  into shared `macOS/scripts/set_bundle_version` (used by
  both Mac's `mac_fix_dylibs` and iPad's "Set Bundle Version"
  build phase); ANGLE XCFramework Info.plists patched at
  dep-download time to add missing `CFBundleShortVersionString`
  (fixes ITMS-90057, upstream fix in
  `xLights-macOS-dependencies/submodules/build_angle.sh`).
- **Controller output toggle pulled forward** after the first
  external feedback wave flagged the missing button — see
  the "Controller output" section of `iPad-xLights-Plan.md`.

## H-5 — Remaining work

All organizational. The binary is ready; the gate is App
Store Connect metadata.

### Screenshots

- iPad landscape and portrait, at the two device sizes Apple
  requires for new apps:
  - **12.9" / 13"** (Pro / Air M-series) — 2048×2732 portrait,
    2732×2048 landscape.
  - **11"** (Pro / Air) — 1668×2388 portrait, 2388×1668
    landscape.
- 3-5 frames per orientation per device covering the core
  flows: sequencer with effects grid + house preview, effect
  inspector, model preview detached, Display Elements editor,
  and Open via Files / iCloud Drive (highlights the `.xsqz`
  round-trip).
- Capture from a real device or Simulator, exported PNGs.
  No marketing chrome / device frames required (App Store
  Connect frames automatically).

### App Store Connect metadata

- **Description** — short pitch (<=170 chars promotional text)
  + long description focused on touch-first sequencing,
  Files / iCloud sync, parity with desktop, and Universal
  Purchase note.
- **Keywords** — 100-char comma-separated. Likely candidates:
  xlights, lights, christmas, halloween, dmx, sacn, artnet,
  led, sequencer, props.
- **Support URL** — pointing at the existing xLights support
  channel (manual / forum / GitHub issues — pick one canonical
  page).
- **Marketing URL** — optional; xlights.org if used.
- **Privacy policy URL** — required even with empty
  `NSPrivacyCollectedDataTypes`. Either reuse the desktop
  policy if one is published, or stand up a minimal
  iPad-applicable policy that matches the manifest.
- **Age rating questionnaire** — expect 4+ (no objectionable
  content). Walk through the App Store Connect form and save.
- **Category** — Primary: Entertainment or Music. Secondary:
  Utilities. Confirm with desktop's existing Mac App Store
  category if applicable.
- **Copyright** — `© Kulp Lights LLC` (year as appropriate).
- **What's New** text for the first iOS submission.

### Submission checklist

- Pin the TestFlight build to be promoted (the same archive
  external testers are exercising).
- Final metadata review pass, then submit for App Review.
- Have an answer ready for "how do testers verify controller
  output" — sACN multicast was approved 2026-05-01 and the
  entitlement is wired; ArtNet, DDP, and sACN unicast / multicast
  all work today.
