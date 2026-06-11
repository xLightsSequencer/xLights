#pragma once

#import <Foundation/Foundation.h>

@class XLSequenceDocument;

NS_ASSUME_NONNULL_BEGIN

// A single source-candidate row exposed to Swift — what the user picks
// from when assigning a destination row's mapping. Fields mirror the
// core C++ AvailableSource struct.
@interface XLImportAvailableSource : NSObject
@property (nonatomic, copy, readonly) NSString* displayName;
@property (nonatomic, copy, readonly) NSString* canonicalName;
@property (nonatomic, copy, readonly) NSString* modelType;
@end

// A timing track from the source sequence that the user can choose
// to copy across. `alreadyExists` is true when the active sequence
// already has a timing element with the same name and existing
// effects — the SwiftUI view defaults `selected` to `!alreadyExists`
// to match desktop behaviour.
@interface XLImportTimingTrack : NSObject
@property (nonatomic, copy, readonly) NSString* name;
@property (nonatomic, assign, readonly) BOOL alreadyExists;
@property (nonatomic, assign, readonly) BOOL selected;
@end

// One destination tree row exposed to Swift. Mirrors the
// ImportMappingNode + BasicImportMappingNode core layout. Tree depth
// is at most three (model → strand/submodel → node) — Swift renders
// it via a List/OutlineGroup keyed on `nodeID`.
@interface XLImportMappingRow : NSObject
@property (nonatomic, assign, readonly) intptr_t nodeID;       // opaque pointer-as-int for stable identity
@property (nonatomic, copy, readonly) NSString* model;
@property (nonatomic, copy, readonly) NSString* strand;
@property (nonatomic, copy, readonly) NSString* node;
@property (nonatomic, copy, readonly) NSString* mapping;       // empty if unmapped
@property (nonatomic, copy, readonly) NSString* mappingModelType;
@property (nonatomic, assign, readonly) BOOL isGroup;
@property (nonatomic, assign, readonly) BOOL isSubmodel;
@property (nonatomic, assign, readonly) NSInteger effectCount;
@property (nonatomic, copy, readonly) NSArray<XLImportMappingRow*>* children;
@end

// Bridge for the iPad Import Effects flow. Holds:
//   - the source `.xsq` parsed once (available list + element map)
//   - the destination mapping tree (built from the user's current
//     loaded sequence's models)
//   - mutating ops the SwiftUI mapping view calls (assign / unassign,
//     auto-map, save hints, apply).
//
// Lifetime: created when the user taps Tools → Import Effects… and
// destroyed when the sheet is dismissed. The bridge holds a weak
// reference to the host `XLSequenceDocument` so it can read the
// active sequence and apply mappings against it when the user
// confirms.
@interface XLImportSession : NSObject

// `document` is the active sequence being edited; the import targets
// its loaded models and elements.
- (instancetype)initWithDocument:(XLSequenceDocument*)document NS_DESIGNATED_INITIALIZER;
- (instancetype)init NS_UNAVAILABLE;

// Parse a source `.xsq` (loose) at `path`. Builds the available list
// (every model/strand/node element in the source that has effects)
// AND the destination tree (every model in the active sequence).
// Returns a non-nil error on parse failure.
- (BOOL)loadSourceSequenceAtPath:(NSString*)path
                            error:(NSError**)error NS_SWIFT_NAME(loadSourceSequence(atPath:));

// Number of available source rows + number of destination roots —
// SwiftUI surfaces these as a quick "loaded N source elements,
// M destination models" header.
@property (nonatomic, readonly) NSInteger availableSourceCount;
@property (nonatomic, readonly) NSInteger destinationRootCount;
@property (nonatomic, readonly) NSInteger mappedDestinationCount;

// SwiftUI consumes these as a snapshot — call after every mutating op
// to refresh the displayed lists.
- (NSArray<XLImportAvailableSource*>*)availableSources;
- (NSArray<XLImportMappingRow*>*)destinationRows;
- (NSArray<XLImportTimingTrack*>*)timingTracks;

// Toggle whether a timing track from the source should be imported.
// `name` matches XLImportTimingTrack.name. Unknown names are ignored.
- (void)setTimingTrackImport:(NSString*)name enabled:(BOOL)enabled;

// Set / clear the mapping on a destination row. `nodeID` is the
// `intptr_t` from XLImportMappingRow.nodeID; `sourceDisplayName` is
// the source row's display string (or nil to clear).
- (void)setMappingForRow:(intptr_t)nodeID
            sourceDisplayName:(nullable NSString*)sourceDisplayName
                    modelType:(nullable NSString*)modelType;

// Run the desktop's Auto Map button: norm pass + aggressive pass +
// every regex hint in `<showdir>/maphints/*.xmaphint`.
- (void)runAutoMap;

// Run the "Auto Map Selected" variant — same passes but limited to
// the destination rows in `selectedNodeIDs` and / or the source rows
// in `selectedSourceDisplayNames`.
- (void)runAutoMapSelectedTargets:(NSArray<NSNumber*>*)selectedNodeIDs
                          sources:(NSArray<NSString*>*)selectedSourceDisplayNames;

// Desktop "AI Map": asks the MAPPING-capable AI service (e.g. Claude
// GenerateModelMapping) for target→source suggestions over the
// currently-unmapped destination rows, applying validated results
// like Auto Map. The network call runs on a utility queue; the
// completion fires on the main queue with the applied-mapping count
// or an error message.
- (void)runAIMapWithCompletion:(void (^)(NSInteger applied, NSString* _Nullable error))completion
    NS_SWIFT_NAME(runAIMap(completion:));

// Write the current mapping state out to a `.xmaphint` file. `path`
// must be writable. Returns NO on write failure.
- (BOOL)saveMapHintsToPath:(NSString*)path;

// IE-3: load a specific `.xmaphint` file and apply its regex hints to
// the destination tree — the same `MatchRegex` AutoMapper pass that the
// show-dir scan runs in `runAutoMap`, but for one user-picked file.
// Returns the number of hint entries applied (0 if none / load failed).
// (The legacy `.xmap` / `.xjmap` formats are wx-bound and would need a
// new core MappingIO module — tracked separately, not here.)
- (int)loadMapHintsFromPath:(NSString*)path
    NS_SWIFT_NAME(loadMapHints(fromPath:));

// IE-12: add each mapped source model name as an alias on its
// destination model so future imports auto-map without re-mapping.
// Mirrors the desktop "Update Aliases". `Model::AddAlias` is idempotent
// (dedups / lowercases / rejects self-name), so this can be called
// repeatedly. Marks affected models dirty so `saveLayoutChanges`
// persists the aliases. Returns the number of aliases added.
- (int)updateModelAliasesFromMapping
    NS_SWIFT_NAME(updateModelAliasesFromMapping());

// Apply the current mappings to the host document's sequence. Runs
// EffectMapper for each mapped row, registers undo on the document,
// triggers a grid reload. `eraseExisting`, `lock`, and
// `convertRenderStyle` are the import-options-sheet toggles
// (`convertRenderStyle` maps to the `convertRender` arg of
// `MapXLightsEffects` — converts per-model render style on import).
// Returns NO if no mappings exist or the source sequence wasn't loaded.
- (BOOL)applyImportWithEraseExisting:(BOOL)eraseExisting
                                lock:(BOOL)lock
                  convertRenderStyle:(BOOL)convertRenderStyle
                                error:(NSError**)error;

// Parse a LOR S5 `.loredit` source at `path` using the wx-free core
// LOREdit reader (the same reader the desktop's ImportS5 uses). Builds
// the available source list (props with effects + per-node/strand
// channels) and the importable timing tracks, then reuses the shared
// destination tree + AutoMapper / MapHints mapping flow. Returns a
// non-nil error on parse failure. Discovery, mapping, AND the effect-synthesis
// apply path (the iPad analogue of desktop MapS5*) are all wired.
- (BOOL)loadLOREditSourceAtPath:(NSString*)path
                          error:(NSError**)error NS_SWIFT_NAME(loadLOREditSource(atPath:));

// Load a Vixen 3 `.tim` file as an EFFECT-import source (IE-25 — the iPad
// analogue of desktop xLightsFrame::ImportVixen3). Parses the .tim + sibling
// SystemConfig.xml via the wx-free core Vixen3 reader and populates the shared
// available/destination tree; apply replays through core MapVixen3*. Returns a
// non-nil error if SystemConfig.xml is missing or the file can't be parsed.
// (Distinct from the timing-only `.tim` path in XLSequenceDocument used by the
// Settings → Timings tab.)
- (BOOL)loadVixen3SourceAtPath:(NSString*)path
                         error:(NSError**)error NS_SWIFT_NAME(loadVixen3Source(atPath:));

// Load a Light-O-Rama Music `.lms` or Animation `.las` file as an EFFECT-import
// source (the iPad analogue of desktop xLightsFrame::ImportLMS — both extensions
// share one XML schema and reader). Parses the document with the wx-free core
// LORMusic reader and populates the shared available/destination tree; apply
// synthesizes On / Color Wash / Twinkle effects (and per-pixel CCR fan-out)
// onto the mapped layers. Returns a non-nil error on parse failure.
- (BOOL)loadLMSSourceAtPath:(NSString*)path
                      error:(NSError**)error NS_SWIFT_NAME(loadLMSSource(atPath:));

// IE-7 — source-sequence metadata for pre-import warnings. Only meaningful for
// an `.xsq` / package source (nil / 0 / empty for `.loredit` / `.tim`).
// `sourceFrequency` is frames-per-second. `sourceMissingMedia` is only
// populated after the media walk that runs during apply, so query it
// post-apply (it is empty at discovery time).
- (nullable NSString*)sourceVersion;
- (NSInteger)sourceFrequency;
- (NSInteger)targetFrequency;
- (NSArray<NSString*>*)sourceMissingMedia;

@end

NS_ASSUME_NONNULL_END
