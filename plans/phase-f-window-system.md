# Phase F — Window system + Display Elements

Consumes finished previews from Phase D and the document lifecycle
from Phase E. Focus is the Scene-level layout, menu bar, multi-window
/ external display routing, and the Display Elements dialog.

## F-1. Scene-level layout

Split `ContentView` into a `Scene`-level layout that supports multiple
`Window` scenes for previews and inspector tabs:

```swift
var body: some Scene {
    WindowGroup("xLights", id: "sequencer") { SequencerScene() }
    Window("House Preview", id: "house-preview") { HousePreviewWindow() }
        .defaultWindowPlacement { _, ctx in
            if let ext = ctx.displays.first(where: { !$0.isMain }) {
                return WindowPlacement(ext)
            }
            return WindowPlacement(.trailing(ctx.windows.first!))
        }
    Window("Model Preview", id: "model-preview") { ModelPreviewWindow() }
    WindowGroup(id: "inspector", for: InspectorTab.self) { $tab in
        InspectorTabWindow(tab: tab ?? .effect)
    }
}
```

Both previews detach-able; House prefers the external display when
present; Model stays with the main window by default. Inspector tabs
detach on demand — one `WindowGroup` keyed by `InspectorTab` so each
tab opens as its own scene window.

## F-2. Size-class responsive layout

Main sequencer window adapts via `@Environment(\.horizontalSizeClass)`:

- **Compact** (narrow split, Slide Over) — inspector stays as a
  four-tab notebook; previews collapse to a single toggleable inline
  strip. Detach actions are hidden.
- **Regular** on a single display — inspector shows the notebook but
  surfaces the per-tab "Open in new window" action; on 12.9"+ both
  previews dock side-by-side at the top, on 11" one preview docks and
  the other can detach.
- **External display attached** — House Preview auto-routes to the
  external display on first open; inspector tabs and Model Preview
  remain offerable to either stage.

## F-3. Docked layout

Inline strip mode houses both previews. On 12.9"+ iPads a side-by-side
docked pane at the top of the sequencer makes sense; on 11" the user
toggles which preview is docked (House default) and the other lives in
a detached window or slide-over.

## F-4. Menu bar + keyboard shortcuts

`Commands { CommandMenu(...) }` for File / Edit / View / Playback —
Cmd+Z, Cmd+C, Cmd+V, Cmd+S, Space discoverable via the iPadOS 26 menu
bar. View menu entries follow the desktop menu (see F-7) — each panel
toggle lives here, including "Open Effect in New Window" / "Open Colors
in New Window" / etc. for the four inspector tabs. The File menu
entries (New / Open / Open Recent / Save / Save As / Close) bind to
the Phase E document-lifecycle commands.

## F-5. Persistence

Persist open sequence + play-head + which panels are visible + which
are docked vs detached + which inspector tabs are detached (and on
which display) + per-preview 2D/3D mode (D-4), via `SceneStorage` /
`@AppStorage`. Currently only folder bookmarks survive.

## F-6. Display Elements dialog

The desktop "Display Elements" panel is how users create and edit the
named Views that the view picker switches between. Without it, the
view picker is read-only. Implement as a modal sheet (`.sheet`) rather
than a dockable panel — used infrequently and doesn't need to stay on
screen.

- Lists current views and their member models / submodels / groups.
- Add / rename / delete view.
- Add / remove / reorder models within a view.
- Persists back through `SequenceElements::GetViews()` serialization
  path already used by desktop; no new on-disk schema.
- Accessible from the View menu (F-4) and from a toolbar button next
  to the view picker.

## F-7. Desktop panels not yet planned for

The desktop View menu lists these panels beyond the six main ones
Phases B-F cover (Grid, Model Preview, House Preview, Effect Settings
with Colors / Layer Blending / Layer Settings tabs). None block MVP;
tracking for a future milestone:

- **Effect Dropper** — quick effect-type picker for drag-to-place.
  Probably subsumed into our bottom effect palette strip.
- **Value Curves** — curve editor. The Phase C editor covers the
  modal case; a dockable panel is post-MVP.
- **Color Dropper** — eyedrop a colour from any preview into the
  palette.
- **Effect Assist** — effect-specific helper panel (varies per effect;
  not all effects define one).
- **Select Effect / Search Effects / Find Effect Data** — three
  related search / filter UIs over the effect set in the sequence.
- **Video Preview** — plays the sequence's media (video) track aligned
  to the play-head.
- **Jukebox** — quick-jump buttons to timing marks.

Post-MVP milestone "Panel parity" takes a pass at these in priority
order once the core UI is stable.
