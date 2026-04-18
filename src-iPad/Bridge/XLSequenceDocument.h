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

// Effect icons. Returns BGRA-premultiplied pixel data (width*height*4
// bytes) plus the chosen bucket size — parsed directly from the
// RenderableEffect's compiled-in XPM data. `desiredSize` is rounded
// up to the nearest {16,24,32,48,64} bucket. Returns nil if the effect
// name is unknown or the XPM couldn't be parsed.
- (NSData*)iconBGRAForEffectNamed:(NSString*)effectName
                      desiredSize:(int)desiredSize
                        outputSize:(int*)outputSize;

@end
