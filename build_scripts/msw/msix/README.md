# xLights MSIX packaging

This directory builds an **MSIX** package of xLights for Windows, in addition to
the classic Inno Setup `.exe` installer (`../xLights_4_64bit_VS.iss`). MSIX is
**additive** — the GitHub `.exe` download, winget, and the in-app updater all
remain the primary distribution path. MSIX exists to enable two things the
classic installer can't:

1. **TestFlight-like nightly auto-update** for testers, via an `.appinstaller`
   file (Windows App Installer checks on launch and updates a sideloaded MSIX).
2. **Microsoft Store** distribution (Store handles updates automatically).

## Files

| File | Purpose |
|---|---|
| `AppxManifest.template.xml` | Manifest with `{{TOKENS}}` substituted at build time. |
| `GenerateAssets.ps1` | Generates the tile/logo/splash PNGs from `include/xlights.png`. |
| `BuildMSIX.ps1` | Builds (and optionally signs) the `.msix` from existing build outputs. |
| `Assets/` | Generated PNG asset set (regenerated on demand by `BuildMSIX.ps1`). |

## Prerequisites

- **Windows SDK** — provides `makeappx.exe` and `signtool.exe` (auto-detected;
  override with `-SdkBinDir`).
- **Visual Studio** — for the VC++ runtime redist DLLs that are bundled
  app-locally (`VC_redist.exe` can't run inside a package). Override with
  `-VCRedistDir`, or skip with `-SkipVCRuntime`.
- A completed VS/MSBuild Release build (the `.msix` packages those outputs;
  nothing is recompiled here). Pass `-Layout CMake` to package a Ninja `bin\`
  tree instead.

## Identity — two flavours

The MSIX `Publisher` **must exactly match the signing certificate subject**, so
there are two identity sets:

### Sideload / nightly (self-distributed)
- `Publisher` = the **Azure Trusted Signing** certificate subject for the
  `kulplights-public` profile:
  `CN=Kulp Lights LLC, O=Kulp Lights LLC, L=Framingham, S=Massachusetts, C=US`
  (this is the `MSIX_PUBLISHER` CI variable and the `BuildMSIX.ps1` default).
  To re-derive it from a signed binary:
  `(Get-AuthenticodeSignature signed.exe).SignerCertificate.Subject`.
- `Name` (identity) can be anything stable, default `KulpLightsLLC.xLights`.

### Microsoft Store
`Name`, `Publisher` (`CN=<GUID>`), and `PublisherDisplayName` come from the app's
**Partner Center** reservation. The Store **re-signs** the upload, so the package
is built with the Store identity and left **unsigned** (`BuildMSIX.ps1 -Store`
sets these and skips signing). The reserved values:

| Field | Value |
|---|---|
| Identity Name | `KulpLightsLLC.xLights` |
| Publisher | `CN=1FED2C12-F7F6-45DF-A649-E647058AAA5F` |
| PublisherDisplayName | `Kulp Lights LLC` |
| Package Family Name | `KulpLightsLLC.xLights_yy7kem98vg5zc` |
| Store ID | `9PGTCKZNKJ2G` |

The version's revision (4th) component **must be 0** for Store uploads (`-Store`
forces this).

## Build locally (sideload test)

Mint a self-signed cert matching `-Publisher`, sign, and trust it so the package
installs (the cert-trust step needs an **elevated** shell):

```powershell
# from this directory, in an elevated PowerShell:
pwsh -File BuildMSIX.ps1 -SelfSign -InstallCert
# -> ..\output\xLights-<version>-x64.msix  (double-click to install)
```

Without `-InstallCert` the package is signed but won't install until the cert is
trusted in `LocalMachine\TrustedPeople`.

## Build in CI (Trusted Signing)

Build unsigned, then sign the `.msix` with the existing
`azure/artifact-signing-action@v2` step (same account/profile that signs the
exes today — it supports MSIX):

```powershell
pwsh -File BuildMSIX.ps1 -Version 2026.11.1.<nightly> -OutFile xLights-x64.msix
# CI then: artifact-signing-action over ..\output\xLights-x64.msix
```

### Nightly workflow (`.github/workflows/win_nightly.yml`)

The nightly builds the MSIX, Trusted-Signs it, generates `xLights.appinstaller`,
and uploads both (stable filenames) to the reused `nightly` GitHub release so the
App Installer auto-update URLs stay constant. This path is **gated** and only runs
when these **repo variables** are set (otherwise the nightly behaves exactly as
before):

| Repo variable | Required | Purpose |
|---|---|---|
| `WINDOWS_SIGNING_ENABLED` | yes (`true`) | Master switch; also gates exe/installer signing. |
| `MSIX_PUBLISHER` | yes | **Exact** Trusted Signing cert subject DN (Publisher must match it). MSIX steps are skipped until this is set. |
| `MSIX_IDENTITY_NAME` | optional | Package identity Name (defaults to `KulpLightsLLC.xLights`). |
| `MSIX_PUBLISHER_DISPLAY_NAME` | optional | Defaults to `Kulp Lights LLC`. |

Nightly MSIX version is `Year.Minor.0.<github.run_number>` (monotonic). Testers
install once from
`https://github.com/<org>/<repo>/releases/download/nightly/xLights.appinstaller`
and Windows App Installer updates them on launch thereafter.

## Microsoft Store

Build the Store package (uses the reserved identity above, unsigned):

```powershell
pwsh -File BuildMSIX.ps1 -Store -OutDir ..\store-output
# -> ..\store-output\xLights-Store-x64.msix
```

The **release workflow** (`.github/workflows/Ubuntu_Window_Release.yml`) does this
on every release tag and uploads `xLights-Store-x64.msix` as the **`xLights_Store_MSIX`
workflow artifact** (deliberately *not* a public release asset — an unsigned
Store-identity package isn't user-installable). Download it from the workflow run
to submit.

### Submission checklist (Partner Center, manual)

Before the package can go live, the submission needs:

- [ ] **Custom license terms** — the default Store license terms conflict with
      GPLv3; supply your own GPL-aligned EULA in the submission (the real gate).
- [ ] **Privacy policy URL** — required (the app declares `webcam`/`microphone`/
      network + `runFullTrust`).
- [ ] **`runFullTrust` justification** — short note; it's a restricted capability.
- [ ] **WACK pass** — run the Windows App Certification Kit on the `.msix` first.
- [ ] **Listing** — description, ≥1 screenshot, store logo, category, search terms.
- [ ] **Age rating** (IARC), **pricing** (Free) + markets.

Store updates are automatic for users once a new submission is live — no client
code (the in-app updater is already disabled when packaged). Automating submission
from CI (Store submission API / StoreBroker) needs an Azure AD app linked to
Partner Center (Tenant/Client ID + secret) and the Store ID `9PGTCKZNKJ2G`; defer
until after the first manual submission.

## Versioning

MSIX requires a 4-part numeric `Major.Minor.Build.Revision`. With no `-Version`,
`BuildMSIX.ps1` derives it from `../xLights_common.iss` (`Year.Version.<Other
digits>.0`). For **nightlies**, pass a monotonically increasing `Revision`
(e.g. a build/run number) so Windows App Installer detects each nightly as newer
— the `nightly-<gitsha>` qualifier is not numeric and can't be used here.

## What is / isn't in the package

- **Bundled:** xLights.exe, xlDo.exe, fseq_convert.exe, all runtime DLLs the
  Inno installer ships, the VC++ runtime (app-local), resources, and the MSIX
  assets. File associations (`.xsq`/`.xsqz`) and the app icon are declared in
  the manifest (not the registry).
- **Not bundled:** the **Vamp Plugin Pack** (GPL/AGPL). xLights directs the user
  to install the official pack on their own machine and finds it via `VAMP_PATH`
  (see `ConfigureVampPath`/`DownloadVamp` in `src-ui-wx/shared/utils/wxUtilities.cpp`).
  Keeping GPL/AGPL plugin binaries out of the package also keeps it Store-clean.

## Behavior changes when packaged

xLights detects a packaged run via `IsRunningPackaged()` and:
- **Skips the in-app GitHub/Inno self-updater** (Store / App Installer updates it
  instead) — both the startup check and the manual "Check for Updates" menu.
- **Installs Vamp plugins to a per-user dir** (`%LOCALAPPDATA%\xLights\VampPlugins`)
  since the package dir is read-only and a Store user may lack admin rights.

## ⚠️ runFullTrust verification checklist (do before investing in the Store)

The package declares `runFullTrust`, which should make xLights behave like the
classic build, but this **must be verified on a clean sideload install** — it's
the go/no-go gate for the Store. Confirm on real hardware:

- [ ] **Loopback automation** — `xlDo.exe` (standalone) drives the packaged
      xLights over `127.0.0.1`; `xFade` connects to the automation listener.
- [ ] **Hardware output** — USB/serial DMX, sACN/ArtNet/DDP (UDP
      multicast/broadcast), controller discovery, FPP discovery, controller upload.
- [ ] **Camera** — KLightMapper scan (Settings → Privacy → Camera must allow it).
- [ ] **Printing** — any print path.
- [ ] **Vamp** — install the official pack, confirm beat/onset detection works
      and the "Download…" offer disappears.
- [ ] **Lua scripting** — scripts run; confirm none assume a writable install dir
      or a specific working directory (packaged CWD defaults to `System32`).
