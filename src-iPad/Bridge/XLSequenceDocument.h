#pragma once

#import <Foundation/Foundation.h>

// ObjC bridge for iPadRenderContext — callable from Swift.
// Manages show folder loading and sequence access.

@interface XLSequenceDocument : NSObject

// Show folder
- (BOOL)loadShowFolder:(NSString*)path;
- (BOOL)loadShowFolder:(NSString*)path mediaFolders:(NSArray<NSString*>*)mediaFolders;

// Register/refresh a persistent security-scoped bookmark for a folder path.
// Call this whenever the user picks a folder via UIDocumentPicker so the
// bookmark is stored in UserDefaults and access survives app restart.
+ (BOOL)obtainAccessToPath:(NSString*)path enforceWritable:(BOOL)enforceWritable;

// Show / media folder accessors — needed by the file-relocation logic
// so Swift can tell whether a picked URL is already inside the
// enforced roots.
- (NSString*)showFolderPath;
- (NSArray<NSString*>*)mediaFolderPaths;

// Copy `sourcePath` into `<showFolder>/<subdirectory>`, appending `_N`
// to the basename on collision. Returns the destination absolute path
// on success, nil on failure (no show folder loaded, copy error).
- (NSString*)moveFileToShowFolder:(NSString*)sourcePath
                        subdirectory:(NSString*)subdirectory;

// Copy `sourcePath` into `<mediaFolderPath>/<subdirectory>`.
// `mediaFolderPath` must already be in `mediaFolderPaths`; unknown
// paths are rejected so the "media always lives in a configured root"
// invariant isn't broken.
- (NSString*)copyFileToMediaFolder:(NSString*)sourcePath
                       mediaFolderPath:(NSString*)mediaFolderPath
                        subdirectory:(NSString*)subdirectory;

// True iff `path` is under the show folder or any configured media
// folder. Used to decide whether a picked file needs copying.
- (BOOL)pathIsInShowOrMediaFolder:(NSString*)path;

// Compute a show-folder-relative path (e.g. `Images/foo.png`). Absolute
// paths outside the show folder round-trip unchanged so media-folder
// files aren't clobbered.
- (NSString*)makeRelativePath:(NSString*)path;

// Sequence
- (BOOL)openSequence:(NSString*)path;
- (void)closeSequence;
- (BOOL)isSequenceLoaded;

// Sequence metadata
- (int)sequenceDurationMS;
- (int)frameIntervalMS;
- (NSString*)sequenceName;

// Elements & rows
- (int)visibleRowCount;
- (NSString*)rowDisplayNameAtIndex:(int)index;
- (int)rowLayerIndexAtIndex:(int)index;
- (BOOL)rowIsCollapsedAtIndex:(int)index;
// Model name for a row (element->GetName()). Empty for non-model rows (e.g. timings).
- (NSString*)rowModelNameAtIndex:(int)index;

// Timing-row queries (rows whose Element is TIMING). Returns indices into
// the visible-row list used by effectCountForRow: and friends.
- (NSArray<NSNumber*>*)timingRowIndices;
- (BOOL)timingRowIsActiveAtIndex:(int)rowIndex;
- (void)setTimingRowActive:(BOOL)active atIndex:(int)rowIndex;
// Color index assigned sequentially to each timing element (0..4, cycles).
// Layers within the same timing element share the same color index.
- (int)timingRowColorIndexAtIndex:(int)rowIndex;
// Name of the timing element the row belongs to (stable across layers).
- (NSString*)timingRowElementNameAtIndex:(int)rowIndex;
// Effect layer's own label, e.g. "Phrases", "Words", "Phonemes" for
// lyric tracks. Empty if the layer has no explicit name.
- (NSString*)rowLayerNameAtIndex:(int)rowIndex;

// Model-row queries used by row headers.
- (BOOL)rowIsModelGroupAtIndex:(int)rowIndex;
- (int)rowLayerCountAtIndex:(int)rowIndex;
- (BOOL)rowIsElementCollapsedAtIndex:(int)rowIndex;
- (void)toggleElementCollapsedAtIndex:(int)rowIndex;

// Views (view picker).
- (NSArray<NSString*>*)availableViews;
- (int)currentViewIndex;
- (void)setCurrentViewIndex:(int)viewIndex;

// dynamicOptions sources for JSON `choice` properties. Mirrors the desktop
// repopulate lambdas in JsonEffectPanel (file:1777-1884). Empty arrays on
// lookup failure — never nil. See EffectPropertyView for dispatch.

// All timing tracks in the sequence with <= 1 effect layer (i.e. not lyric).
- (NSArray<NSString*>*)timingTrackNames;
// Timing tracks with exactly 3 layers (phrase / word / phoneme).
- (NSArray<NSString*>*)lyricTimingTrackNames;

// Model-scoped sources. Uses the effect's parent element's ModelName to
// resolve a Model; ModelGroups are unwrapped to their first contained
// model, matching desktop (JsonEffectPanel.cpp:1815-1818). Empty on
// unresolvable model.
- (NSArray<NSString*>*)statesForRow:(int)rowIndex atIndex:(int)effectIndex;
- (NSArray<NSString*>*)facesForRow:(int)rowIndex atIndex:(int)effectIndex;
- (NSArray<NSString*>*)modelNodeNamesForRow:(int)rowIndex atIndex:(int)effectIndex;

// Effect-scoped: RenderableEffect::GetSettingOptions(settingId). Returns
// {} for most effects; SingleStrand overrides it to return the WLED FX /
// palette name lists (SingleStrandEffect.cpp:100-131). Matches desktop's
// "effect" dynamicOptions source.
- (NSArray<NSString*>*)effectSettingOptionsForRow:(int)rowIndex
                                          atIndex:(int)effectIndex
                                         settingId:(NSString*)settingId;

// Effects for a given row
- (int)effectCountForRow:(int)rowIndex;

// Effect data — returns arrays parallel to each other
- (NSArray<NSString*>*)effectNamesForRow:(int)rowIndex;
- (NSArray<NSNumber*>*)effectStartTimesForRow:(int)rowIndex;
- (NSArray<NSNumber*>*)effectEndTimesForRow:(int)rowIndex;

// Model preview — sets channel data on models for a given frame
- (void)setModelColorsAtMS:(int)frameMS;

// Opaque pointer to iPadRenderContext for Metal bridge
- (void*)renderContext;

// Effect editing
- (BOOL)addEffectToRow:(int)rowIndex
                  name:(NSString*)effectName
               startMS:(int)startMS
                 endMS:(int)endMS;
- (BOOL)deleteEffectInRow:(int)rowIndex atIndex:(int)effectIndex;
- (BOOL)moveEffectInRow:(int)rowIndex
                atIndex:(int)effectIndex
              toStartMS:(int)newStartMS
                toEndMS:(int)newEndMS;
- (NSArray<NSString*>*)availableEffectNames;

// Effect settings for selected effect
- (NSDictionary<NSString*, NSString*>*)effectSettingsForRow:(int)rowIndex atIndex:(int)effectIndex;
- (NSDictionary<NSString*, NSString*>*)effectPaletteForRow:(int)rowIndex atIndex:(int)effectIndex;

// Effect metadata — returns JSON string as loaded from resources/effectmetadata/<Name>.json
// Empty string if no metadata is available for the effect.
- (NSString*)metadataJsonForEffectNamed:(NSString*)effectName;

// Shader dynamic properties — parses the .fs file at `shaderPath` and
// returns a JSON-encoded array of property entries that drop into
// `EffectPropertyView` using the same schema as static metadata. Empty
// string if the file isn't a parseable shader or the path is empty.
- (NSString*)shaderDynamicPropertiesJsonForPath:(NSString*)shaderPath;

// Shared metadata — returns JSON string for Buffer/Color/Timing shared panels.
// name must be one of "Buffer", "Color", "Timing".
- (NSString*)sharedMetadataJsonNamed:(NSString*)name;

// Read a single effect setting value by key.
// Keys with C_ prefix come from palette map; everything else from settings map.
- (NSString*)effectSettingValueForKey:(NSString*)key
                                inRow:(int)rowIndex
                              atIndex:(int)effectIndex;

// Write a single effect setting value. Returns YES if the value changed.
// Does NOT trigger a re-render — caller should invoke renderEffectForRow after
// batching setting changes.
- (BOOL)setEffectSettingValue:(NSString*)value
                       forKey:(NSString*)key
                        inRow:(int)rowIndex
                      atIndex:(int)effectIndex;

// Remove a setting from the effect (used for properties with
// suppressIfDefault=true when the new value equals the default, so the
// settings map doesn't persist a redundant default). Returns YES if the
// key was present and was removed.
- (BOOL)removeEffectSettingForKey:(NSString*)key
                            inRow:(int)rowIndex
                          atIndex:(int)effectIndex;

// Fade in/out (seconds, stored as T_TEXTCTRL_Fadein / T_TEXTCTRL_Fadeout).
- (float)effectFadeInSecondsForRow:(int)rowIndex atIndex:(int)effectIndex;
- (float)effectFadeOutSecondsForRow:(int)rowIndex atIndex:(int)effectIndex;
- (BOOL)setEffectFadeInSeconds:(float)seconds
                          forRow:(int)rowIndex
                         atIndex:(int)effectIndex;
- (BOOL)setEffectFadeOutSeconds:(float)seconds
                           forRow:(int)rowIndex
                          atIndex:(int)effectIndex;

// Resize one edge of an effect. edge: 0 = left/start, 1 = right/end.
// Clamps to neighbors; returns NO if clamp would invert the range.
- (BOOL)resizeEffectEdgeInRow:(int)rowIndex
                       atIndex:(int)effectIndex
                          edge:(int)edge
                        toMS:(int)newMS;

// Lock / render-disable state.
- (BOOL)effectIsLockedInRow:(int)rowIndex atIndex:(int)effectIndex;
- (void)setEffectLocked:(BOOL)locked inRow:(int)rowIndex atIndex:(int)effectIndex;
- (BOOL)effectIsRenderDisabledInRow:(int)rowIndex atIndex:(int)effectIndex;
- (void)setEffectRenderDisabled:(BOOL)disabled inRow:(int)rowIndex atIndex:(int)effectIndex;

// Copy: returns full settings string (xLights legacy format) and the effect's
// palette as a separate string. Empty strings on failure.
- (NSString*)effectSettingsStringForRow:(int)rowIndex atIndex:(int)effectIndex;
- (NSString*)effectPaletteStringForRow:(int)rowIndex atIndex:(int)effectIndex;
- (NSString*)effectNameForRow:(int)rowIndex atIndex:(int)effectIndex;

// Paste / scripted add: insert a new effect with settings+palette pre-populated.
// Returns the index of the new effect, or -1 on failure.
- (int)addEffectToRow:(int)rowIndex
                 name:(NSString*)effectName
             settings:(NSString*)settings
              palette:(NSString*)palette
              startMS:(int)startMS
                endMS:(int)endMS;

// Kick off a background re-render for the range of a single effect's model.
- (void)renderEffectForRow:(int)rowIndex atIndex:(int)effectIndex;

// Re-render an arbitrary time range on a row's model (used after delete so
// the cleared output is refreshed).
- (void)renderRangeForRow:(int)rowIndex
                  startMS:(int)startMS
                    endMS:(int)endMS
                    clear:(BOOL)clear;

// Controller output
- (BOOL)startOutput;
- (void)stopOutput;
- (BOOL)isOutputting;
- (void)outputFrame:(int)frameMS;

// Rendering
- (void)renderAll;
- (BOOL)isRenderDone;
// Signal all in-flight render jobs to abort and block until they've
// completed (or `timeoutSeconds` elapses). Returns YES if the render
// is fully quiesced by the time the call returns. Call on shutdown /
// sequence-close paths before tearing down `SequenceElements` et al —
// the render workers hold pointers into those structures and would
// crash otherwise.
- (BOOL)abortRenderAndWait:(NSTimeInterval)timeoutSeconds;

// Memory pressure
- (void)handleMemoryWarning;
- (void)handleMemoryCritical;
+ (int64_t)availableMemoryMB;

// House preview pixel data at a given time
// Returns NSData containing packed float x, y and uint8 r, g, b per pixel
- (int)pixelCountAtMS:(int)frameMS;
- (NSData*)pixelDataAtMS:(int)frameMS;

// Audio playback
- (BOOL)hasAudio;
- (void)audioPlay;
- (void)audioPause;
- (void)audioStop;
- (void)audioSeekToMS:(long)positionMS;
- (long)audioTellMS;
- (int)audioPlayingState;  // 0=PLAYING, 1=PAUSED, 2=STOPPED
- (void)setAudioVolume:(int)volume;
- (int)audioVolume;

// Waveform data — returns downsampled peaks for display
// Returns array of alternating min/max float values for the given time range
- (NSData*)waveformDataFromMS:(long)startMS
                         toMS:(long)endMS
                   numSamples:(int)numSamples;

// Effect-background batch append. Mirrors desktop's
// `EffectsGrid::DrawEffectBackground` helper — resolves the
// RenderableEffect + color mask, then calls
// `RenderableEffect::DrawEffectBackground` with the bridge's current
// effect-background accumulator. Caller must have wrapped the visible-
// effects loop in `-beginEffectBackgroundBatch` / `-flushEffectBackgroundBatch`
// on the bridge. Coordinates are in logical pixel space, top-left
// origin (matches the grid's coord system — desktop uses bottom-left
// but the accumulator just stores the numbers, so we stay consistent
// with the grid's other calls).
//
// Returns the draw-icon hint desktop uses:
//   0 — effect drew a full background, skip the icon
//   1 — show the normal-size icon
//   2 — show a smaller icon (leaves room for partial background)
// Bridge is declared `id` in the header so this file doesn't have to
// import `XLGridMetalBridge.h`; the .mm casts it back.
- (int)appendEffectBackgroundForRow:(int)rowIndex
                            atIndex:(int)effectIndex
                                 x1:(float)x1
                                 y1:(float)y1
                                 x2:(float)x2
                                 y2:(float)y2
                             bridge:(id)bridge
                          drawRamps:(BOOL)drawRamps;

// Effect icons. Returns BGRA-premultiplied pixel data (width*height*4
// bytes) plus the chosen bucket size — parsed directly from the
// RenderableEffect's compiled-in XPM data. `desiredSize` is rounded
// up to the nearest {16,24,32,48,64} bucket. Returns nil if the effect
// name is unknown or the XPM couldn't be parsed.
- (NSData*)iconBGRAForEffectNamed:(NSString*)effectName
                      desiredSize:(int)desiredSize
                        outputSize:(int*)outputSize;

@end
