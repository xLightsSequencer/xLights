#pragma once

#import <Foundation/Foundation.h>

// ObjC bridge for iPadRenderContext — callable from Swift.
// Manages show folder loading and sequence access.

@interface XLSequenceDocument : NSObject

// Show folder
- (BOOL)loadShowFolder:(NSString*)path;
- (BOOL)loadShowFolder:(NSString*)path mediaFolders:(NSArray<NSString*>*)mediaFolders;

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

// Kick off a background re-render for the range of a single effect's model.
- (void)renderEffectForRow:(int)rowIndex atIndex:(int)effectIndex;

// Controller output
- (BOOL)startOutput;
- (void)stopOutput;
- (BOOL)isOutputting;
- (void)outputFrame:(int)frameMS;

// Rendering
- (void)renderAll;
- (BOOL)isRenderDone;

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

@end
