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
// projected touch point via `InitializeLocation`. Returns the
// final model name on success (the importer may uniquify it if
// the show already has a model by that name), nil on failure.
- (nullable NSString*)importXmodelFromPath:(NSString*)path
                              atScreenPoint:(CGPoint)point
                                   viewSize:(CGSize)viewSize
                                forDocument:(XLSequenceDocument*)doc;

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
