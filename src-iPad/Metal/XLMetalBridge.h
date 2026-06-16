#pragma once

#import <Foundation/Foundation.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

@class XLSequenceDocument;

// ObjC bridge for xlStandaloneMetalCanvas + iPadModelPreview.
// Owns the C++ canvas and preview, drives model rendering.

NS_ASSUME_NONNULL_BEGIN

@interface XLMetalBridge : NSObject

- (instancetype)initWithName:( NSString* )name;
- (void)attachLayer:(CAMetalLayer*)layer;
- (void)setDrawableSize:(CGSize)size scale:(CGFloat)scale;

// Draw all models at the given frame time using DisplayEffectOnWindow.
// When setPreviewModel: has set a non-empty name, only that model (plus view
// objects) is drawn — everything else is skipped. Empty/nil preview model =
// full house preview behavior.
- (void)drawModelsForDocument:(XLSequenceDocument*)doc atMS:(int)frameMS pointSize:(float)pointSize;

// Preview-mode model filter. Empty/nil string clears the filter.
- (void)setPreviewModel:(NSString*)modelName;

// Camera controls — route through the active PreviewCamera on iPadModelPreview.
// Zoom: 1.0 default, >1 zooms in. Pan is in world units. Rotate is in degrees.
- (void)setCameraZoom:(float)zoom;
- (float)cameraZoom;
- (void)setCameraPanX:(float)x panY:(float)y;
- (void)offsetCameraPanX:(float)dx panY:(float)dy;
- (float)cameraPanX;
- (float)cameraPanY;
- (void)setCameraAngleX:(float)ax angleY:(float)ay;
- (void)offsetCameraAngleX:(float)dx angleY:(float)dy;
- (float)cameraAngleX;
- (float)cameraAngleY;
- (void)resetCamera;

// 2D/3D mode. True = perspective (3D); false = orthographic (2D).
- (void)setIs3D:(BOOL)is3d;
- (BOOL)is3D;

// Coarse view-object (house mesh / terrain / gridlines / ground images)
// visibility toggle for the House Preview. No effect on Model Preview —
// that pane never draws view objects. Default: YES.
- (void)setShowViewObjects:(BOOL)show;
- (BOOL)showViewObjects;

// Drop the cached background texture. Called when the active layout
// group changes (possibly new background image path). Next draw
// re-loads whatever the render context's active-group path is.
- (void)invalidateBackgroundCache;

// Viewpoints (saved camera positions). List is filtered to the pane's
// current 2D/3D mode. Apply copies a saved PreviewCamera into the
// active camera; saveAs captures the active camera under a new name.
// Persistence for the add/delete mutations requires the document — the
// document triggers the rgbeffects.xml rewrite.
- (NSArray<NSString*>*)viewpointNamesForDocument:(XLSequenceDocument*)doc;
- (BOOL)applyViewpointNamed:(NSString*)name
                forDocument:(XLSequenceDocument*)doc;
- (BOOL)saveCurrentViewAs:(NSString*)name
              forDocument:(XLSequenceDocument*)doc;
- (BOOL)deleteViewpointNamed:(NSString*)name
                 forDocument:(XLSequenceDocument*)doc;
- (void)restoreDefaultViewpointForDocument:(XLSequenceDocument*)doc;

// Fit-to-window camera shortcuts. Fit All frames every model in the
// active layout group (matching the House Preview filter); Fit Model
// frames the single named model (or returns NO if it's missing or
// hidden by the current layout group). Adjusts zoom + pan (2D) or
// distance + pan (3D) without changing rotation angles.
- (BOOL)fitAllModelsForDocument:(XLSequenceDocument*)doc;
- (BOOL)fitModelNamed:(NSString*)name
          forDocument:(XLSequenceDocument*)doc;

// Phase J-2 — Layout Editor selection highlight. Empty / nil
// clears the highlight. The bridge does NOT auto-redraw; SwiftUI
// callers should `setNeedsDisplay` on the MTKView after setting
// the selection so the ring appears on the next frame.
- (void)setSelectedModel:(nullable NSString*)name;

// Phase J-4 (multi-select) — secondary selection set. The
// "primary" model is still driven by `setSelectedModel:` (it owns
// the gizmo, the action bar anchor, drag origin); these are the
// rest of the selection, drawn with the selection ring but
// without handles. Pass an empty array (or nil) to clear.
- (void)setExtraSelectedModels:(nullable NSArray<NSString*>*)names;

// Phase J-6 (sidebar canvas sync) — ModelGroup selection. Groups
// are NOT in `GetModelsForActivePreview` (their members are
// expanded), so they ride a parallel slot. When set, every
// member of the group renders in the "group selected" tint
// (cyan) so the user sees what's in the group without trying to
// draw a meaningful "group bounding box" (groups don't have a
// natural geometric extent). Empty / nil clears.
- (void)setSelectedGroup:(nullable NSString*)name;

// Phase J-31 — Controllers tab sidebar sync. Mirrors
// `setSelectedGroup:` but matches by each model's
// `GetControllerName()`. When set, every model assigned to the
// named controller renders in the group-member tint so the user
// sees which models live on the selected controller. Empty /
// nil clears.
- (void)setSelectedController:(nullable NSString*)name;

// Phase J-6 (sidebar canvas sync) — ViewObject selection. View
// objects (Mesh / Image / Gridlines / Terrain / Ruler) aren't
// Models, so they need their own selection slot. When set, the
// matching view object renders with `allowSelected=true` so its
// ScreenLocation handles appear. Empty / nil clears.
- (void)setSelectedViewObject:(nullable NSString*)name;

// J-13 — hit-test view objects under a touch point. Mirrors
// `pickModelAtScreenPoint` but searches `ViewObjectManager`
// rather than `ModelManager`. Returns the topmost (last-drawn)
// hit; nil for no hit. 2D and 3D supported.
- (nullable NSString*)pickViewObjectAtScreenPoint:(CGPoint)point
                                          viewSize:(CGSize)viewSize
                                       forDocument:(XLSequenceDocument*)doc;

// J-13 — 2D drag-to-move for view objects. Same delta-based
// math as `moveModel:byDeltaDX:dY:viewSize:forDocument:`. The
// caller passes incremental UI-point deltas; the bridge converts
// to world units via the active 2D camera zoom + virtual scale
// and writes through to `SetHcenterPos / SetVcenterPos`.
// Honors locked + isFromBase + snap-to-grid. Marks the VO
// dirty for save.
- (BOOL)moveViewObject:(NSString*)name
              byDeltaDX:(CGFloat)dx
                     dY:(CGFloat)dy
               viewSize:(CGSize)viewSize
            forDocument:(XLSequenceDocument*)doc;

// J-14 — handle-based endpoint drag for two-point view objects
// (Ruler). Mirrors `pickHandleAtScreenPoint:` but hit-tests the
// SELECTED VO's screen-location handles via the descriptor
// pipeline. Opens a `_dragSession` on the matching handle id;
// the existing `dragHandle:toScreenPoint:` routes touch updates
// through it. `endHandleDragForDocument:` commits + marks the VO
// dirty rather than a model.
//
// Returns the handle index (>= 0) on a hit, -1 on miss. Use the
// same `dragHandle:` and `endHandleDragForDocument:` methods as
// the model handle path — they auto-detect the VO target via
// the new `_dragSessionViewObjectName` slot.
- (NSInteger)pickViewObjectHandleAtScreenPoint:(CGPoint)point
                                       viewSize:(CGSize)viewSize
                                    forDocument:(XLSequenceDocument*)doc;

// J-15 — 3D body-drag for view objects. Same plane-anchor math
// as `beginBodyDrag3DForModel:` but targets a VO's screen
// location. The existing `dragBody3DToScreenPoint:` and
// `endBodyDrag3D` auto-route via a target-is-VO flag latched at
// .began; no parallel apply/end methods are needed.
- (BOOL)beginBodyDrag3DForViewObject:(NSString*)name
                        atScreenPoint:(CGPoint)point
                             viewSize:(CGSize)viewSize
                          forDocument:(XLSequenceDocument*)doc;

// J-15 — pinch-to-scale on a view object. Latches the saved
// scale matrix; subsequent `applyPinchScaleFactor:` calls
// multiply the saved matrix by the cumulative factor.
- (BOOL)beginPinchScaleForViewObject:(NSString*)name
                           forDocument:(XLSequenceDocument*)doc;

// J-15 — two-finger twist on a view object. Cumulative-radians
// gesture; the saved rotation is captured at .began and the
// applied rotation is `saved + degrees(radians)`.
- (BOOL)beginTwistRotateForViewObject:(NSString*)name
                            forDocument:(XLSequenceDocument*)doc;

// J-13 — Terrain heightmap edit. Maps a touch point onto the
// terrain's (u,v) grid, raises (delta > 0) or lowers
// (delta < 0) the nearest grid point by `delta` world units.
// `brushRadiusPoints` is in screen points; >0 applies a falloff
// to neighbouring grid points within that radius for a more
// natural deformation (0 = single-point edit). Returns YES if
// any point changed; NO for misses or non-Terrain targets.
- (BOOL)editTerrainHeight:(NSString*)terrainName
              atScreenPoint:(CGPoint)point
                  viewSize:(CGSize)viewSize
                     delta:(float)delta
        brushRadiusPoints:(CGFloat)brushRadiusPoints
               forDocument:(XLSequenceDocument*)doc;

// Phase J-2 — Layout Editor in-canvas overlays. Per-bridge state
// (not persisted to rgbeffects.xml in J-2). Initial values for
// the LayoutEditor pane are seeded from iPadRenderContext's
// `Display2DGrid` / `Display2DBoundingBox` flags on first
// `drawModelsForDocument`. Other panes default both to NO and
// ignore them. 2D-only — neither overlay draws in 3D.
- (void)setShowLayoutGrid:(BOOL)show;
- (BOOL)showLayoutGrid;
- (void)setShowLayoutBoundingBox:(BOOL)show;
- (BOOL)showLayoutBoundingBox;

// Phase J-2 — snap-to-grid for drag-to-move. When YES, the post-
// delta centre of the moved model snaps to the nearest grid
// spacing multiple (read from the iPadRenderContext's
// `Display2DGridSpacing`, with 2D-center0 origin honoured).
// Default: NO. Per-session — not persisted to rgbeffects.xml.
- (void)setSnapToGrid:(BOOL)snap;
- (BOOL)snapToGrid;

// Phase J-2 (touch UX) — persistent modifier state for handle
// drags. Replaces held-key modifiers in the desktop UI; the
// SwiftUI toolbar drives these, the bridge consults them on
// every drag Update().
//
// `uniformModifier` — ORs `handles::Modifier::Shift` into the
// drag modifier so existing session classes (which already
// interpret Shift as "uniform scale" / "aspect lock") need no
// change.
//
// `lockAxis` — pre-filters the drag's world ray onto an axis-
// aligned line through the active handle's saved position, so
// the underlying session sees a constrained cursor. 0=Free,
// 1=X, 2=Y, 3=Z; values match `ModelScreenLocation::MSLAXIS`
// for the X/Y/Z cases.
//
// Per-session — not persisted to rgbeffects.xml.
- (void)setUniformModifier:(BOOL)uniform;
- (BOOL)uniformModifier;
- (void)setLockAxis:(NSInteger)axis;  // 0=Free, 1=X, 2=Y, 3=Z
- (NSInteger)lockAxis;

// Phase J-2 — first-pixel highlight (`highlightFirst` arg to
// `DisplayModelOnWindow`). When YES, the first node of every
// model is rendered in cyan instead of its native colour, so the
// user can spot wiring origin while laying out.
// Default: NO. LayoutEditor pane only — other panes ignore.
- (void)setShowFirstPixel:(BOOL)show;
- (BOOL)showFirstPixel;

// Manipulation-handle size multiplier (1..10). Mirrors the desktop
// "Model Handle Size" view preference; larger values draw bigger
// selection handles, which makes them easier to grab by touch.
- (void)setHandleScale:(NSInteger)scale;
- (NSInteger)handleScale;

// Show Zone Indicator (desktop 'Show Zone Indicator in Preview').
// When on, DMX MovingHeadAdv models draw their position zones in
// the layout preview.
- (void)setShowZoneIndicator:(BOOL)show;
- (BOOL)showZoneIndicator;

// Phase J-2 — return the topmost model whose world bounding box
// contains `point` (in view-point coordinates relative to the
// MTKView's bounds), or nil if no model is hit. `viewSize` is the
// MTKView's bounds in points (not pixels) — the bridge multiplies
// by its stored scale factor internally.
//
// Today: 2D-only. In 3D mode the method returns nil; full 3D
// ray-cast hit testing lands alongside the gizmo work.
//
// Iterates the active layout group's models in reverse draw order
// (last drawn = on top), so a small model rendered on top of a
// larger backdrop wins the pick.
- (nullable NSString*)pickModelAtScreenPoint:(CGPoint)point
                                    viewSize:(CGSize)viewSize
                                 forDocument:(XLSequenceDocument*)doc;

// Phase J-2 (marquee) — pick every model whose 2D screen bounding
// box overlaps `rect` (iPad points, view coords). Submodels are
// excluded — they share their parent's screenLocation, matching
// the single-point `pickModel` filter. Returns names in
// `GetModelsForActivePreview` order (consistent draw order).
//
// 2D: pure axis-aligned overlap via
// `BaseObject::IsContained(preview, x1, y1, x2, y2)`.
// 3D: same containment plus a depth-cutoff pass (mirrors desktop's
// `LayoutPanel::SelectAllInBoundingRect` — gather depths, find
// the first > 40 %-of-nearest gap, drop everything past it) so a
// background model directly behind a foreground hit doesn't sneak
// in.
- (NSArray<NSString*>*)pickModelsInRect:(CGRect)rect
                                viewSize:(CGSize)viewSize
                             forDocument:(XLSequenceDocument*)doc;

// Phase J-2 — translate a screen-space drag delta (points) into a
// world-space move on the named model's centre. Returns YES if the
// model existed, was unlocked, and accepted the move (and was
// marked dirty); NO otherwise. 2D-only (3D drag follows the gizmo
// design). The caller is responsible for scoping this to the model
// it intends to drag — typically the LayoutEditor's currently-
// selected model.
- (BOOL)moveModel:(NSString*)name
       byDeltaDX:(CGFloat)dx
              dY:(CGFloat)dy
        viewSize:(CGSize)viewSize
     forDocument:(XLSequenceDocument*)doc;

// Phase J-2 (touch UX) — 3D body-drag (move-the-whole-model in
// 3D). Per-frame deltas don't map cleanly to world space when the
// camera can rotate; the drag is anchored to a plane through the
// model's start-of-drag centre Z, and the world point under the
// finger is recomputed each frame. The model translates so the
// finger stays on the same point of the underlying plane.
//
// Call on .began with the model name + touch screen point. The
// bridge latches the plane Z + the anchor world XY where the
// touch ray hit the plane. Returns NO if the model is locked, the
// view isn't in 3D, or the touch ray misses the plane.
- (BOOL)beginBodyDrag3DForModel:(NSString*)name
                   atScreenPoint:(CGPoint)point
                        viewSize:(CGSize)viewSize
                     forDocument:(XLSequenceDocument*)doc;
// Call on .changed with the touch screen point. Returns YES when
// the model moved (model marked dirty); NO when not active or
// when the ray missed the plane. Lock Axis / Snap toolbar state
// is honoured.
- (BOOL)dragBody3DToScreenPoint:(CGPoint)point
                        viewSize:(CGSize)viewSize
                     forDocument:(XLSequenceDocument*)doc;
// Call on .ended / .cancelled / .failed to clear the anchor.
- (void)endBodyDrag3D;

// Phase J-2 (touch UX) — hover-driven handle highlight, the
// cursor-equivalent for Apple Pencil hover (M2 iPad+) and
// trackpad pointers. Hit-tests under the hovered point against
// the selected model's handles and calls `MouseOverHandle` so
// `DrawAxisTool` colours the matched axis yellow. No-op if no
// model is selected. Returns YES when the highlight state
// actually changed (caller can use this to gate `setNeedsDisplay`
// and avoid per-event repaints when the pointer is stationary).
- (BOOL)setHoveredHandleAtScreenPoint:(CGPoint)point
                              viewSize:(CGSize)viewSize
                           forDocument:(XLSequenceDocument*)doc;
- (BOOL)clearHoveredHandleForDocument:(XLSequenceDocument*)doc;

// Phase J-2 (touch UX) — long-press / contextual-menu support.
// Hit-tests under the press for vertex / segment / curve-control
// handles on the selected model and returns a description the
// SwiftUI menu can read to decide which items to show. Returns
// nil for empty / model-body / non-actionable hits.
//
// Keys in the returned dictionary:
//   "type"          — NSString: "vertex" | "segment" | "curve_control"
//   "modelName"     — NSString
//   "vertexIndex"   — NSNumber (Int), 0-based, only for "vertex"
//   "segmentIndex"  — NSNumber (Int), 0-based, present for
//                     "segment" and "curve_control"
//   "hasCurve"      — NSNumber (Bool), only for "segment"
- (nullable NSDictionary*)inspectHandleAtScreenPoint:(CGPoint)point
                                            viewSize:(CGSize)viewSize
                                         forDocument:(XLSequenceDocument*)doc;

// Vertex / curve actions for the long-press menu live on
// `XLSequenceDocument` — they only need ModelManager access,
// not camera state, and SwiftUI reaches the document directly.

// Phase J-2 (touch UX) — project the top-centre of a model's
// bounding box to screen coords. The inline action bar anchors
// there. Returns NSValue wrapping CGPoint in UIKit (top-left
// origin) coordinates, or nil when the model is off-screen /
// behind the camera / unfound. Reads live camera state, so
// callers should re-query on every relevant change (pan, zoom,
// orbit, mode flip, drag-update).
- (nullable NSValue*)screenAnchorPointForModel:(NSString*)modelName
                                    forDocument:(XLSequenceDocument*)doc;

// Phase J-3 (touch UX) — create a new model of the given type
// at the touch point. Unprojects the touch to world coords (XY
// plane at z=0 in both 2D and 3D), calls
// `ModelManager::CreateDefaultModel` for the type, places at
// the unprojected world XY, sets layout group to the active
// group, adds to ModelManager. Returns the new model's
// generated name (e.g. "Arches-1") or nil on failure.
//
// Used by the SwiftUI Add-Model flow. The caller is expected to
// follow up by setting the selection and pushing a notification
// so the side panel refreshes.
- (nullable NSString*)createModelOfType:(NSString*)type
                           atScreenPoint:(CGPoint)point
                                viewSize:(CGSize)viewSize
                             forDocument:(XLSequenceDocument*)doc;

// Phase J-4 — import an .xmodel file at the touch point. Loads
// the XML, runs it through `Model::CreateDefaultModelFromSavedModelNode`
// so DMX / matrix / star / arch / etc. all deserialize through
// the same path desktop uses, then positions the model at the
// projected touch point via `InitializeLocation`.
//
// For multi-model xmodel files (`<models>` root with multiple
// `<model>` children — desktop PR #6365 added support for
// exporting these), the primary model goes at the touch point
// and additional siblings are placed to its right with a
// padding gap. Returns the final model names in order — first
// element is the primary, the rest are the additionals. Caller
// may uniquify since the show may already have models by those
// names. nil on failure.
//
// `targetLayoutGroup` overrides the active-layout-group default
// when non-nil/non-empty — used by the multi-model placement
// flow to let the user pick a destination group before
// committing the batch.
- (nullable NSArray<NSString*>*)importXmodelFromPath:(NSString*)path
                                        atScreenPoint:(CGPoint)point
                                             viewSize:(CGSize)viewSize
                                    targetLayoutGroup:(nullable NSString*)targetLayoutGroup
                                          forDocument:(XLSequenceDocument*)doc
    NS_SWIFT_NAME(importXmodel(fromPath:atScreenPoint:viewSize:targetLayoutGroup:for:));

// GDTF mode picker support. When a .gdtf fixture defines multiple
// DMX modes the SwiftUI side wants to chooser before placement
// (desktop prompts via ChooseFromList). This lists the DMX mode
// names in the fixture (sorted, as the parser sees them), or an
// empty array on parse failure / non-GDTF input. Class method —
// only opens the archive, no preview state needed.
+ (NSArray<NSString*>*)gdtfModesForFile:(NSString*)path
    NS_SWIFT_NAME(gdtfModesForFile(path:));

// GDTF import with an explicit DMX mode. Same as
// importXmodelFromPath for a .gdtf file but forces `gdtfMode` as
// the selected mode (skipping the auto-pick-first behaviour).
// Pass nil to auto-pick the first mode. Returns the placed model
// name (single element) or nil on failure.
- (nullable NSArray<NSString*>*)importGdtfFromPath:(NSString*)path
                                              mode:(nullable NSString*)gdtfMode
                                     atScreenPoint:(CGPoint)point
                                          viewSize:(CGSize)viewSize
                                 targetLayoutGroup:(nullable NSString*)targetLayoutGroup
                                       forDocument:(XLSequenceDocument*)doc
    NS_SWIFT_NAME(importGdtf(fromPath:mode:atScreenPoint:viewSize:targetLayoutGroup:for:));

// Phase J-4 (import) — peek at an .xmodel file to determine if
// it contains multiple models (root `<models>` element). Returns
// YES for multi-model files, NO for single-model files or on
// parse failure. Used to gate the layout-group picker sheet so
// single-model imports keep their zero-prompt UX. Class method —
// the peek only opens the XML; no preview/canvas state required.
+ (BOOL)xmodelFileIsMultiModel:(NSString*)path
    NS_SWIFT_NAME(xmodelFileIsMultiModel(path:));

// Phase J-3 (touch UX) — multi-vertex polyline create. Returns
// YES if `name` is a PolyPoint-style model (Poly Line / MultiPoint).
// Used by the SwiftUI gesture layer to decide whether the next tap
// should append a vertex or place a fresh model.
- (BOOL)modelUsesPolyPointLocation:(NSString*)name
                       forDocument:(XLSequenceDocument*)doc;

// Phase J-3 (touch UX) — append a new vertex to an in-progress
// polyline create. Commits any open drag session, calls
// `AddHandle` to push a new vertex at the projected touch point,
// then opens a `BeginExtend` session on that vertex so a follow-on
// drag (via `dragHandle:toScreenPoint:`) sizes the new segment.
// Returns NO if `name` is missing, not a PolyPoint model, locked,
// or the projection fails.
- (BOOL)appendVertexToPolyline:(NSString*)name
                  atScreenPoint:(CGPoint)point
                       viewSize:(CGSize)viewSize
                    forDocument:(XLSequenceDocument*)doc;

// Phase J-2 (touch UX) — batched name + anchor query for every
// model in the active layout group. Used by the SwiftUI label
// overlay to draw model-name text on the canvas. One bridge call
// per frame instead of one per model (saves 50–500 allocations
// per refresh on dense shows). Off-screen / behind-camera models
// are omitted. Returns array of NSDictionary { "name": NSString,
// "anchor": NSValue (CGPoint) }.
- (NSArray<NSDictionary*>*)modelLabelAnchorsForDocument:(XLSequenceDocument*)doc;

// Phase J-2 (touch UX) — pinch-on-model = uniform scale.
// Triggered when the user pinches with both fingers on the
// selected model's body (caller decides; the bridge just runs
// the math). `factor` is `UIPinchGestureRecognizer.scale` —
// cumulative from gesture start, so the resulting scale is
// `savedScale * factor`. Honours `IsLocked()`. Returns NO if
// the model is locked or not found.
- (BOOL)beginPinchScaleForModel:(NSString*)name forDocument:(XLSequenceDocument*)doc;
- (BOOL)applyPinchScaleFactor:(CGFloat)factor forDocument:(XLSequenceDocument*)doc;
- (void)endPinchScale;

// Phase J-2 (touch UX) — two-finger twist on model = rotate Z.
// `radians` is `UIRotationGestureRecognizer.rotation` —
// cumulative from gesture start, so resulting rotation is
// `savedRotateZ + degrees(radians)`. Honours `IsLocked()`.
- (BOOL)beginTwistRotateForModel:(NSString*)name forDocument:(XLSequenceDocument*)doc;
- (BOOL)applyTwistRotationRadians:(CGFloat)radians forDocument:(XLSequenceDocument*)doc;
- (void)endTwistRotate;

// Phase J-2 — resize handles. The bridge draws 4 corner handles
// around the selected model when the LayoutEditor selection ring
// is active. Hit-test returns 0..3 (corner index, see below) for a
// handle hit, or -1 for no handle. The drag method takes the
// touch's screen point and resizes the model so the dragged corner
// follows the touch and the OPPOSITE corner stays fixed.
//
// Corner indices, with ortho Y-up:
//   0 = top-left    (-x, +y)
//   1 = top-right   (+x, +y)
//   2 = bottom-right (+x, -y)
//   3 = bottom-left (-x, -y)
//
// Both methods return immediately for unselected / locked / empty
// preview / 3D mode. 3D resize handles ship with the gizmo work.
- (NSInteger)pickHandleAtScreenPoint:(CGPoint)point
                            viewSize:(CGSize)viewSize
                         forDocument:(XLSequenceDocument*)doc;

- (BOOL)dragHandle:(NSInteger)handleIndex
   toScreenPoint:(CGPoint)point
        viewSize:(CGSize)viewSize
     forDocument:(XLSequenceDocument*)doc;

// Phase J-2 — call when the gesture that started a handle drag
// ends, so per-drag state on the screen location (active_axis,
// latching) is cleared and the next gesture starts clean.
- (void)endHandleDragForDocument:(XLSequenceDocument*)doc;

// Phase J-2 — single-tap dispatch for the LayoutEditor pane in
// 3D. When a tap lands on the active centre handle of the
// currently-selected model, cycles `axis_tool` between
// translate / scale / rotate (mirrors desktop's
// LayoutPanel.cpp:3726). Returns YES if the tool was advanced
// (caller should repaint); NO if the tap should fall through
// to model-selection. 2D / no-selection returns NO immediately.
- (BOOL)handleCenterHandleTapAtScreenPoint:(CGPoint)point
                                  viewSize:(CGSize)viewSize
                               forDocument:(XLSequenceDocument*)doc;

// Phase J-3 (touch UX) — Pencil double-tap. Advance the axis tool
// (Translate → Scale → Rotate → …) on the selected model without
// needing a position. Returns YES if a tool change happened
// (caller should repaint); NO if no model is selected / model is
// locked / 2D.
- (BOOL)cycleAxisToolForSelectedModelForDocument:(XLSequenceDocument*)doc;

// Phase J-4 (multi-select) — bulk alignment. For each entry in
// `names`, shift its centre so the named edge / centre matches
// the leader's. Edges: @"left", @"right", @"top", @"bottom",
// @"centerH" (X-centre), @"centerV" (Y-centre), @"front",
// @"back", @"centerD" (Z-centre). Models in `names` that aren't
// editable (locked / fromBase) are skipped; the leader is never
// modified. Returns YES if at least one model moved. Caller
// should mark layout dirty + repaint.
- (BOOL)alignModels:(NSArray<NSString*>*)names
            toLeader:(NSString*)leader
                  by:(NSString*)edge
         forDocument:(XLSequenceDocument*)doc;

// Phase J-4 (multi-select) — equal-spacing distribution along an
// axis. Sorts the selection by centre along the chosen axis,
// keeps the two extreme models in place, and re-spaces the
// middle ones evenly between them. Needs at least 3 entries.
// `axis`: @"horizontal" (X), @"vertical" (Y), @"depth" (Z).
// Returns YES if anything moved.
- (BOOL)distributeModels:(NSArray<NSString*>*)names
                     axis:(NSString*)axis
              forDocument:(XLSequenceDocument*)doc;

// Phase J-4 (multi-select) — match the leader's dimensions.
// `dim`: @"width", @"height", @"depth", or @"all". Editable
// non-leader entries are resized; TwoPoint / PolyPoint subclasses
// honour the setter natively (rescales their points). Returns
// YES if at least one model resized.
- (BOOL)matchSizeOfModels:(NSArray<NSString*>*)names
                  toLeader:(NSString*)leader
                 dimension:(NSString*)dim
               forDocument:(XLSequenceDocument*)doc;

// Phase J-7 (multi-select) — flip the selection 180° about the
// chosen world axis. `axis`: @"horizontal" (about Y_AXIS) or
// @"vertical" (about X_AXIS). Mirrors desktop's
// `FlipHorizontal` / `FlipVertical` semantics. Each model
// flips in place (about its own origin); to flip an arrangement
// as a single rigid body, the user composes flip + align.
// Returns YES if at least one model changed.
- (BOOL)flipModels:(NSArray<NSString*>*)names
              axis:(NSString*)axis
       forDocument:(XLSequenceDocument*)doc;

// Swap the start/end of each named line model (Single Line / Poly
// Line). Mirrors desktop's ID_PREVIEW_SWAP_START_END — reverses
// endpoints, segment sizes, lead/trail offsets and curve control
// points via the core `Model::SwapStartEnd()`. Models that don't
// support it, are locked, or come from the base show are skipped.
// Returns YES if at least one model changed.
- (BOOL)swapStartEndForModels:(NSArray<NSString*>*)names
                  forDocument:(XLSequenceDocument*)doc;

// Bulk Edit Rotate (desktop ID_PREVIEW_BULKEDIT_ROTATEX/Y/Z) —
// set the absolute rotation about one axis on every editable model
// in `names`. `axis`: @"X", @"Y", or @"Z". `degrees` is clamped to
// [-180, 180] to match the rotation property grid; out-of-range
// callers should reject before invoking. Locked / from-base models
// are skipped. Returns YES if at least one model changed.
- (BOOL)rotateModels:(NSArray<NSString*>*)names
                axis:(NSString*)axis
             degrees:(double)degrees
         forDocument:(XLSequenceDocument*)doc;

// Replace each model in `targets` with a copy of the model named
// `source` (desktop ID_PREVIEW_REPLACEMODEL / ReplaceModelDialog,
// #4462). Each target is swapped for a fresh clone of the source
// that takes over the target's name. Option flags mirror the
// desktop dialog: `keepStartChannel` preserves each target's
// start-channel + controller/port/smart-remote assignment,
// `keepSubmodels` carries the target's submodels onto the clone,
// `keepSizePosition` preserves the target's centre / size /
// rotation. Models from the base show or named `source` are
// skipped. Returns the count of targets replaced.
- (NSInteger)replaceModels:(NSArray<NSString*>*)targets
                withSource:(NSString*)source
          keepStartChannel:(BOOL)keepStartChannel
             keepSubmodels:(BOOL)keepSubmodels
          keepSizePosition:(BOOL)keepSizePosition
               forDocument:(XLSequenceDocument*)doc;

// Phase J-7 (multi-select) — duplicate each named model. Each
// copy is offset by (+50, +50, 0) world units from the source so
// it doesn't overlap, gets a unique name via
// `ModelManager::GenerateModelName`, clears controller mapping,
// and unlocks. Returns the new names in source order — caller
// updates the multi-selection to the new set if desired.
- (NSArray<NSString*>*)duplicateModels:(NSArray<NSString*>*)names
                           forDocument:(XLSequenceDocument*)doc;

// Layout clipboard (desktop DoCopy / DoPaste). Serialize the named
// models to an XML string (a `<models>` document, same shape as a
// multi-model .xmodel export) for placing on UIPasteboard, or nil
// if nothing serializable. Groups are skipped (ambiguous member
// copy, as with Duplicate).
- (nullable NSString*)copyModelsToString:(NSArray<NSString*>*)names
                             forDocument:(XLSequenceDocument*)doc;

// Layout clipboard paste. Deserialize an XML string produced by
// copyModelsToString (or a multi-model .xmodel), uniquifying each
// model name, clearing its controller mapping (auto-assign), and
// offsetting by (+50, +50, 0) world units so the paste doesn't sit
// exactly on the source. Returns the new model names. Cross-
// sequence safe — the string can come from any document.
- (NSArray<NSString*>*)pasteModelsFromString:(NSString*)xml
                                 forDocument:(XLSequenceDocument*)doc;

// J-30 — Submodel editor support. Pick the node nearest `point`
// on the named model. Returns the 1-based node index (>=1) or 0
// for a miss. `point` is in UIKit view points relative to the
// MTKView bounds; the bridge scales to window pixels internally.
// Today: 2D-only. In 3D mode the method returns 0; the submodel
// editor pane is 2D ortho so this matches the desktop dialog.
- (NSInteger)nodeNearPoint:(CGPoint)point
                   onModel:(NSString*)modelName
                  viewSize:(CGSize)viewSize
               forDocument:(XLSequenceDocument*)doc;

// J-30 — Submodel editor support. Pick every node within `rect`
// on the named model. Returns 1-based node indices in
// ModelManager iteration order. Same 2D-only restriction as
// `nodeNearPoint`.
- (NSArray<NSNumber*>*)nodesInRect:(CGRect)rect
                           onModel:(NSString*)modelName
                          viewSize:(CGSize)viewSize
                       forDocument:(XLSequenceDocument*)doc;

// Color Dropper (desktop View ▸ Windows ▸ Color Dropper). Sample the
// current rendered colour of whichever model node sits under `point`.
// Walks every model in ModelManager iteration order and returns the
// `#RRGGBB` colour of the first node hit (so it works on the multi-
// model House preview as well as a single-model preview), or nil for
// a miss / 3D mode. The colour comes from the node's last-rendered
// value, not a GPU pixel read-back — no drawable stall.
- (nullable NSString*)sampledColorHexNearPoint:(CGPoint)point
                                      viewSize:(CGSize)viewSize
                                   forDocument:(XLSequenceDocument*)doc;

// Node inspect (desktop layout hover tooltip). Walk every model and
// return info about the node under `point`, or nil for a miss / 3D
// mode. Keys: "model" (NSString), "node" (1-based NSNumber),
// "channel" (absolute start channel, NSNumber), "controller"
// (NSString, may be empty), "port" (NSString port/connection range,
// may be empty). 2D-only, same restriction as `nodeNearPoint`.
- (nullable NSDictionary<NSString*, id>*)nodeInfoNearPoint:(CGPoint)point
                                                 viewSize:(CGSize)viewSize
                                              forDocument:(XLSequenceDocument*)doc;

// J-30 — Submodel editor support. Apply highlight colours to the
// named model's nodes: nodes in `highlighted` (1-based) are
// painted white, every other node is painted dark grey. Matches
// desktop SubModelsDialog::SelectRow visual treatment. Caller
// should `setNeedsDisplay` on the MTKView and have flipped
// `setSuppressChannelUpdate:YES` so the next frame doesn't
// repaint the channel-data colours over the overrides.
- (BOOL)setSubmodelHighlightedNodes:(NSArray<NSNumber*>*)highlighted
                            onModel:(NSString*)modelName
                        forDocument:(XLSequenceDocument*)doc;

// J-30 — Submodel editor support. Clear node colour overrides
// on the named model (the next frame with SuppressChannelUpdate
// off will repaint native pixel data).
- (BOOL)clearSubmodelHighlightsOnModel:(NSString*)modelName
                           forDocument:(XLSequenceDocument*)doc;

// SubModels Symmetrize (rotational generator) — mirrors desktop
// SubModelsDialog::Symmetrize. Generates `degree`-fold rotationally
// symmetric copies of `ranges` (the selected submodel's range
// strings) over the parent `modelName`'s node cloud. Returns the new
// full strand list (originals + generated copies, ordered per
// `bottomToTop`) or nil if matching failed / model lookup miss.
// `squarify` corrects a non-square node cloud before matching, the
// same option desktop offers interactively. Uses the bridge's 2D
// preview for node screen positions (`Model::GetScreenLocations`).
- (nullable NSArray<NSString*>*)symmetrizeRanges:(NSArray<NSString*>*)ranges
                                         onModel:(NSString*)modelName
                                 degreeOfSymmetry:(NSInteger)degree
                                       clockwise:(BOOL)clockwise
                                     bottomToTop:(BOOL)bottomToTop
                                        squarify:(BOOL)squarify
                                     forDocument:(XLSequenceDocument*)doc
    NS_SWIFT_NAME(symmetrizeRanges(_:onModel:degreeOfSymmetry:clockwise:bottomToTop:squarify:forDocument:));

// J-30 — Submodel editor support. When YES, drawModelsForDocument
// skips the per-frame `ctx->SetModelColors(frameMS)` step so any
// node colours set via `setSubmodelHighlightedNodes:` stay visible
// across redraws. Per-bridge state — only meaningful on the
// dedicated SubmodelEditor pane. Default: NO.
- (void)setSuppressChannelUpdate:(BOOL)suppress;
- (BOOL)suppressChannelUpdate;

// J-30 — Force the bridge into single-model fit-to-window mode
// (the path normally selected when the bridge is named
// "ModelPreview"). Combined with `setPreviewModel:` this lets a
// SubmodelEditor-named bridge draw the parent model centred and
// fit-to-window instead of the full house layout. Default: NO.
- (void)setSingleModelMode:(BOOL)single;
- (BOOL)singleModelMode;

// Diagnostic surface for the SwiftUI preview pane. `errorReason`
// returns the most recent silent-fail reason (no Metal layer, 0×0
// drawable, render context missing, StartDrawing failed, no models
// to draw, etc.) or nil when everything's fine. `hasRenderedSuccessfully`
// flips true after the first frame that completes through
// `EndDrawing`. Together they let the SwiftUI view distinguish
// "still warming up" (no error, hasRendered=false) from "actually
// broken" (errorReason set, hasRendered=false for ≥ a couple of
// seconds).
//
// Each unique error is also logged via spdlog at WARN level on first
// occurrence, so iPad → Tools → Package Logs captures the failure
// for tester reports.
- (nullable NSString*)errorReason;
- (BOOL)hasRenderedSuccessfully;

@end

NS_ASSUME_NONNULL_END
