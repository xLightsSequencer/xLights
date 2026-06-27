# 12 · AI, Automation & Scripting

> The AI *service* layer is fully shared (`src-core/ai/ServiceManager.cpp` registers chatGPT, claude, ollama, gemini, GenericClient, plus AppleIntelligence on Apple Silicon and OpenVINO only when `HAVE_OPENVINO_GENAI` is defined — i.e. Windows/Intel, so OpenVINO is on **neither** macOS-arm nor iPad). The iPad bridges that core through `XLAIServices` / `XLAIImageSession`, and the SwiftUI surface for AI is **substantially at parity** with desktop: per-service config + Test (`AIServicesSettingsSheet`), AI image generation (`AIImageGenerationSheet`), AI color palette (`AIPaletteGenerationSheet`), AI speech-to-text lyric tracks (`AddTimingTrackSheet` → `SequencerViewModel.generateAILyricTrack`), and LRCLIB lyric search/import (`AddTimingTrackSheet`). The genuine **AI** gaps on iPad are now very narrow: AI **model auto-mapping** (✅ landed 2026-06-11), AI image **crop/resize** (✅ landed 2026-06-12), and AI image **black-background removal** (✅ landed 2026-06-12) are all done. Remaining open items are the **OpenAI-style "Refresh model list"** action (which iPad has and desktop lacks — a desktop gap) and the **"Generate Lyrics From Data"** Papagayo phoneme→channel generator (a non-AI tool, desktop-only). The **Automation/Scripting** half is almost entirely desktop-only and largely **infeasible** on iOS: the REST/HTTP automation server (75-command dispatch in `xLightsAutomations.cpp`), the Lua/Python script runner (`ScriptsDialog` + `LuaRunner`/`PythonRunner`), and the `Generate 2D Path` tracing tool. These are blocked by App Store policy (no runtime code execution, no socket-server binding) and platform constraints, not by missing bridge work.

## Parity scorecard

| Feature | Surface | Desktop | iPad | Gap | Priority | Ease | Feasibility | Notes |
|---|---|---|---|---|---|---|---|---|
| AI Services configuration UI | preference | ✅ | ✅ | parity | P1 | easy | feasible | Desktop = Preferences→Services `wxPropertyGrid`; iPad = Tools→AI Services… `Form`. Both walk `aiBase::GetProperties()`. |
| Claude service config | preference | ✅ | ✅ | parity | P1 | easy | feasible | Shared `claude` service; PROMPT/COLORPALETTES/MAPPING. |
| ChatGPT/OpenAI service config | preference | ✅ | ✅ | parity | P1 | easy | feasible | Core file is `chatGPT.cpp`; `OpenAIAPI.cpp` is the shared HTTP impl. IMAGES/COLORPALETTES/SPEECH2TEXT. |
| Gemini service config | preference | ✅ | ✅ | parity | P2 | easy | feasible | Shared `gemini` service. |
| Ollama (local) service config | preference | ✅ | ✅ | parity | P2 | easy | feasible | Shared `ollama` service; user's own endpoint. |
| GenericClient (OpenAI-compatible) config | preference | ✅ | ✅ | parity | P2 | easy | feasible | Shared; vLLM/custom endpoints. |
| OpenVINO (local Intel) service config | preference | ❌ | ❌ | parity | P3 | hard | infeasible | Gated `#ifdef HAVE_OPENVINO_GENAI` — Windows/Intel only. Absent on macOS-arm **and** iPad. |
| Apple Intelligence service | preference | ✅ | ✅ | parity | P2 | easy | feasible | Registered `#if __APPLE__ && __arm64__`; capabilities gated by OS (FoundationModels 26+, ImagePlayground 15.4/18.4+, SFSpeech everywhere). |
| Service property: String | preference | ✅ | ✅ | parity | P1 | easy | feasible | `ServiceProperty::Kind::String` ↔ `XLAIPropertyKindString`. |
| Service property: Secret (API key) | preference | ✅ | ✅ | parity | P1 | easy | feasible | iPad renders `SecureField`. |
| Service property: Integer | preference | ✅ | ✅ | parity | P2 | easy | feasible | iPad `TextField` numberPad. |
| Service property: Boolean | preference | ✅ | ✅ | parity | P2 | easy | feasible | iPad `Toggle`. |
| Service property: Choice/Enum | preference | ✅ | ✅ | parity | P2 | easy | feasible | iPad `Picker`. |
| AI service Test/validate connection | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Both call `aiBase::TestLLM()`. Desktop modal msgbox; iPad inline status row. |
| Refresh model list (GET /v1/models) | dialog | ❌ | ✅ | desktop-missing | P3 | medium | feasible | iPad `AIServicesSettingsSheet.refreshModelsRow` → `refreshModels(forService:)`. Desktop ServicesPanel has no such button. |
| Generate AI Image from prompt | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop Tools→Generate AI Image (`AIImageDialog`); iPad `AIImageGenerationSheet`. |
| AI Image style/preset (Choice) props | dialog | ✅ | ✅ | parity | P2 | easy | feasible | iPad `styleSection` renders generator Choice props via `XLAIImageSession.imageProperties()`. |
| AI Image model picker (2026.11, server model list) | dialog | ✅ | ✅ | parity | P2 | easy | feasible | Core: `OpenAIAPI::createAIImageGenerator()` hands the server model list (`GetAvailableModels(false)`, cached) to the generator, which exposes it as a `OpenAIImage.Model` `Choice` prop when models are known (`OpenAIImageGenerator.cpp:25-45`, `:79-87`). iPad creates the generator via the *same* `createAIImageGenerator()` (`XLAIImageSession.mm:43`) and `imageProperties()` surfaces every Choice prop in `styleSection` — the model dropdown appears automatically with no extra wiring. Same cached-list semantics as desktop (free-text when no list cached); use the iPad-only "Refresh model list" in AI Services settings to populate it. |
| AI Image crop tool (drag-select) | dialog | ✅ | ✅ | parity | P3 | hard | feasible | Desktop `AIImageDialog.cpp` `CropPanel` (mouse paint/drag). iPad: `CropResizeSheet` (`AIImageGenerationSheet.swift`) — SwiftUI `DragGesture` crop overlay on the preview, applied via `CGImage.cropping(to:)` before commit. Opened via "Crop / Resize…" button when a generated image is present. |
| AI Image resize/reset | dialog | ✅ | ✅ | parity | P3 | medium | feasible | Desktop `ResizeImageDialog`. iPad: `CropResizeSheet` (`AIImageGenerationSheet.swift`) — W×H fields with aspect-lock toggle and quality picker, applied via `UIGraphicsImageRenderer` before commit. Same sheet as crop; combined UX. |
| AI Image save to file with black-background removal | dialog | ✅ | ✅ | parity | P2 | medium | feasible | Desktop `AIImageDialog.cpp:617` `OnSaveButtonClicked` runs `RemoveBlackBackground` flood-fill (`:430`, `:624-628`) before saving. iPad: `removeBlackBackgroundFromImage(_:)` in `AIImageGenerationSheet.swift` — same border-seeded BFS, tolerance 25/255 per channel, sets alpha=0 on keyed pixels. Applied before `commitImage()` when "Remove black background" toggle (default ON) is checked. |
| AI Image inline launch from Pictures effect | context-menu | 🟡 | ✅ | desktop-missing | P2 | easy | feasible | iPad: `EffectFilenameBlockView` shows an "AI" button on the Pictures filename row (`canShowAIButton`). Desktop reaches it only via Tools menu (PicturesPanel has no AI button). |
| Generate AI Color Palette | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop ColorPanel→Generate (`AIColorPaletteDialog`); iPad `AIPaletteGenerationSheet`. |
| AI Palette — Song mode (audio metadata seed) | dialog | ✅ | ✅ | parity | P2 | easy | feasible | Both prefill from title/artist and wrap "from the song …". |
| AI Palette — Free-Form mode | dialog | ✅ | ✅ | parity | P2 | easy | feasible | Arbitrary prompt. |
| AI Palette launch from Color Panel | context-menu | ✅ | ✅ | parity | P1 | easy | feasible | Desktop palette context menu; iPad `ColorPaletteView` "AI Generate Palette…". |
| AI Speech-to-Text lyric track | menu/context | ✅ | ✅ | parity | P1 | medium | feasible | **Both present.** Desktop: RowHeading "AI Speech 2 Lyrics" + NewTiming "AI Lyrics from Audio" → `GenerateAILyrics` (`tabSequencer.cpp`). iPad: `RowHeaderViews.swift:44-49,225-227` "Speech to Lyrics…" item (gate `SequencerViewModel.swift:4012`), `AddTimingTrackSheet` `.aiLyrics` → `generateAILyricTrack`. |
| AI lyrics — use vocals stem / alt track | dialog | ✅ | ✅ | parity | P2 | medium | feasible | Both point the recognizer at the selected waveform track / HTDemucs stem. |
| AI lyrics — multi-service picker | dialog | ✅ | ✅ | parity | P3 | easy | feasible | Desktop `wxSingleChoiceDialog`; iPad service Picker in `aiLyricsParamsSection`. |
| LRCLIB lyric search & import | dialog | ✅ | ✅ | parity | P1 | easy | feasible | Desktop `LRCLIBSearchDialog` (from RowHeading "Import Lyrics"); iPad `AddTimingTrackSheet` `.lrclib` + `LRCLIBClient.swift`. |
| AI structured model auto-mapping | dialog | ✅ | ✅ | parity | P2 | medium | feasible | Desktop import "AI Map" → `GenerateModelMapping` (xLightsImportChannelMapDialog.cpp:3543 DoStructuredAIMapping). iPad: `XLImportSession.runAIMap(completion:)` (XLImportSession.mm — MappingModelInfo from sources/targets, existing mappings as examples, validated apply) + gated "AI Map" button beside Auto Map in `ImportEffectsView.swift`. |
| Heuristic Auto-Map (non-AI) | dialog | ✅ | ✅ | parity | P2 | easy | feasible | Shared `AutoMapper`; iPad `ImportEffectsView` "Auto Map". Listed for contrast with AI map above. |
| "Generate Lyrics From Data" (Papagayo phoneme→channel) | menu | ✅ | ❌ | ipad-missing | P3 | medium | feasible | Desktop Tools→Generate &Lyrics From Data (`GenerateLyricsDialog`) maps phonemes to coro-face DMX channels from typed text. **Not AI.** No iPad equivalent. |
| Generate 2D Path from image | menu | ✅ | ❌ | ipad-missing | P3 | hard | hard | Desktop Tools→Generate 2D Path (`PathGenerationDialog`): interactive image-tracing canvas. iPad lacks the wx canvas infra; would need a bespoke Metal/SwiftUI tracer. |
| Lua script execution | menu/dialog | ✅ | ❌ | ipad-missing | P2 | hard | infeasible | Desktop Tools→Run Scripts (`ScriptsDialog` + `LuaRunner`). iOS/App Store forbids runtime code execution. |
| Lua/REST batch sequence importing (#6511) | other | ✅ | ❌ | ipad-missing | P3 | hard | infeasible | Desktop #6511: `importSequence` automation command (`LuaRunner.cpp`, `xLightsAutomations.cpp`, sample `resources/scripts/BatchImportSequences.lua`) scripting `xLightsImportChannelMapDialog` mappings. Doubly blocked on iPad (code exec + no HTTP listener). iPad's interactive `ImportEffectsView` remains the manual path. |
| Python script execution | menu/dialog | ✅ | ❌ | ipad-missing | P3 | hard | infeasible | Desktop conditional (`PYTHON_RUNNER`, pybind11). No interpreter allowed on iOS. |
| Scripts Dialog (browse/download/run/log) | dialog | ✅ | ❌ | ipad-missing | P2 | hard | infeasible | Enumerates show `scripts/`, downloads from repos, runs with log capture. Sandbox + code-exec policy block it. |
| REST/HTTP automation server | other | ✅ | ❌ | ipad-missing | P1 | hard | infeasible | `xLightsAutomations.cpp` (`ProcessHttpRequest`, `StartAutomationListener`); 75-command dispatch. iOS app-sandbox blocks listening-socket servers. |
| REST API: sequence management | other | ✅ | ❌ | ipad-missing | P1 | hard | infeasible | openSequence/closeSequence/saveSequence/renderAll. Requires the HTTP listener. |
| REST API: model/effect ops | other | ✅ | ❌ | ipad-missing | P2 | hard | infeasible | getModels/addEffect/setEffectSettings. |
| REST API: controller ops | other | ✅ | ❌ | ipad-missing | P2 | hard | infeasible | getControllers/uploadController. (Closed-firmware uploads separately restricted.) |
| REST API: layout/view ops | other | ✅ | ❌ | ipad-missing | P2 | hard | infeasible | saveLayout/getViews/exportLayoutImage. |
| REST API: runScript | other | ✅ | ❌ | ipad-missing | P2 | hard | infeasible | Runs Lua — doubly blocked (server + code exec). |
| REST API: media/audio control | other | ✅ | ❌ | ipad-missing | P2 | hard | infeasible | setAudioSpeed/playSequence via HTTP. |
| REST API: jukebox | other | ✅ | ❌ | ipad-missing | P3 | hard | infeasible | playJukebox; iPad has no jukebox today. |
| REST API: utility (purge cache, package logs) | other | ✅ | ❌ | ipad-missing | P3 | hard | infeasible | purgeRenderCache/packageLogFiles via HTTP. iPad exposes Package Logs as a UI button instead. |
| Command-line / `xlDo` automation | other | ✅ | ❌ | ipad-missing | P3 | hard | infeasible | `automation.cpp` CLI dispatch; no CLI on iOS. |
| AI plugin loading (dylib/DLL) | other | ✅ | ❌ | ipad-missing | P3 | hard | infeasible | `ServiceManager::loadPlugins`. iPad passes empty `pluginDir` (App Store forbids loading external executable code). |
| Bridge: service snapshot (ObjC value types) | other | n/a | ✅ | n/a | P3 | easy | feasible | `XLAIServiceInfo`/`XLAIServiceProperty`/`XLAIPaletteColor` Sendable snapshots. Bridge plumbing, not a user feature; no desktop analog needed. |
| Bridge: async image/palette/lyric marshalling | other | n/a | ✅ | n/a | P3 | easy | feasible | `XLAIImageSession`/`XLAIServices` hop completions to MainActor. Bridge plumbing only. |

## iPad gaps (desktop has, iPad missing)

### P2

- ~~**AI structured model auto-mapping**~~ — **landed 2026-06-11.** `XLImportSession.runAIMap(completion:)` gathers `MappingModelInfo` for sources (names/types — the iPad source list carries less structure than desktop's ImportChannel; names dominate the signal) and unmapped targets (full node metadata incl. submodel names), passes mapped rows as `existingMappings` examples, calls `ServiceManager::findService(aiType::MAPPING)->GenerateModelMapping` on a utility queue, and applies validated results on the main queue. "AI Map" button beside Auto Map in `ImportEffectsView.swift`, gated on `XLAICapabilityMapping`, with progress + result/error alert.

- **"Generate Lyrics From Data" (Papagayo phoneme→channel)** — Desktop Tools→Generate &Lyrics From Data (`src-ui-wx/xLightsMain.cpp:7233`, `GenerateLyricsDialog`) maps phonemes to coro-face DMX channels from typed text (NOT AI; distinct from the AI Speech-to-Text track, which iPad already has). **Work:** wrap the phoneme→channel logic in a bridge call and add a small SwiftUI sheet. Niche (coro-face authoring); ease: medium. Priority pinned P3 in the scorecard given low frequency.

- ~~**AI image save with black-background removal**~~ — **landed 2026-06-12.** `removeBlackBackgroundFromImage(_:)` in `AIImageGenerationSheet.swift` — border-seeded BFS, tolerance 25/255, alpha=0 on keyed pixels, applied before `commitImage()` when the "Remove black background" toggle (default ON) is active. Mirrors `RemoveBlackBackground` in `AIImageDialog.cpp:430`.

- ~~**AI image crop & resize**~~ — **landed 2026-06-12.** `CropResizeSheet` in `AIImageGenerationSheet.swift` — SwiftUI `DragGesture` overlay on the preview for crop (applied via `CGImage.cropping(to:)`), plus W×H fields with aspect-lock and quality picker for resize (applied via `UIGraphicsImageRenderer`). Opened via "Crop / Resize…" button when a generated image is present.

### P3

## Desktop gaps (iPad has, desktop missing)

### P2

- **Inline AI-image launch from the Pictures effect** — iPad `EffectFilenameBlockView.swift:83` shows an "AI" wand button right on the Pictures filename row (`canShowAIButton`, gated on `hasEnabledService(forCapability:XLAICapabilityImages)`), opening `AIImageGenerationSheet` in embedded mode and writing the path straight back to `E_TEXTCTRL_Pictures_Filename`. Desktop only offers AI image generation through Tools→Generate AI Image — `PicturesPanel.cpp` has no AI button. **Work:** add an "AI…" button to the desktop Pictures panel that opens `AIImageDialog` in embedded mode and writes the chosen path back. Ease: easy.

### P3

- **"Refresh model list" (OpenAI-style GET /v1/models)** — iPad `AIServicesSettingsSheet.refreshModelsRow` (`:141`) → `XLAIServices.refreshModels(forService:)` re-fetches the model list and flips the model property from free text to a Choice picker. The desktop `ServicesPanel.cpp` has no equivalent action; users type the model name. **Work:** add a "Refresh models" button to the desktop Services panel that calls the same core listing path. Ease: medium.

## Infeasible / restricted on iPad

- **REST/HTTP automation server + all 75 endpoints** — `xLightsAutomations.cpp` binds a listening socket (`StartAutomationListener`). The iOS app sandbox prohibits background server sockets; no Background Modes entitlement covers an arbitrary REST listener. *Infeasible.*
- **Lua / Python script execution + Scripts Dialog** — App Store Guideline 2.5.2 forbids downloading and executing code (Lua VM, Python interpreter). The `runScript` REST endpoint is doubly blocked, as is the #6511 Lua/REST `importSequence` batch-import command. *Infeasible.*
- **AI plugin loading (dylib)** — `ServiceManager::loadPlugins` `dlopen`s `AIPlugin*.dylib`; iPad deliberately passes an empty `pluginDir` (`XLAIServices.mm`). Loading external executable code is prohibited. *Infeasible.*
- **Command-line / `xlDo` automation** — No CLI/argv automation surface on iOS. *Infeasible.*
- **OpenVINO local AI service** — Compiled only with `HAVE_OPENVINO_GENAI` (Windows/Intel). Not built for macOS-arm or iOS; no OpenVINO GenAI runtime on iOS. *Infeasible* (and also absent on macOS-arm desktop).
- **Generate 2D Path tracer** — Relies on the wx interactive image canvas; would need a complete bespoke Metal/SwiftUI re-implementation. *Hard / low value.*
- **Closed-firmware controller uploads via REST** — even if a REST surface existed, proprietary-firmware controller config/upload is IAP-gated and out of scope (P3, restricted). Open-firmware (FPP/WLED/ESPixelStick/DDP/Kulp) controller automation would be in scope only once a non-socket automation channel existed — which it doesn't on iOS.

## Recommended sequencing

1. **Desktop Pictures-panel "AI…" button (P2, easy)** — closes the one genuinely user-facing desktop gap and reuses the existing embedded `AIImageDialog`. Cheap parity win.
2. ~~**iPad AI structured model-mapping (P2, medium)**~~ — ✅ **done 2026-06-11** (`runAIMap` bridge + AI Map button).
3. ~~**iPad AI image black-background removal (P2, medium)**~~ — ✅ **done 2026-06-12** (`removeBlackBackgroundFromImage` + toggle in `AIImageGenerationSheet`).
4. ~~**iPad AI image crop/resize (P3, hard/medium)**~~ — ✅ **done 2026-06-12** (`CropResizeSheet` in `AIImageGenerationSheet`).
5. **Desktop "Refresh models" button (P3, medium)** — small reciprocal parity item; mirror the iPad model-listing action into ServicesPanel.
6. **Papagayo "Generate Lyrics From Data" (P3)** — only if user demand surfaces; niche coro-face authoring tool.
7. **Do not invest** in REST server / Lua-Python / 2D-Path / OpenVINO on iPad — they are infeasible under App Store + platform constraints; record and move on.
