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

// Write the current mapping state out to a `.xmaphint` file. `path`
// must be writable. Returns NO on write failure.
- (BOOL)saveMapHintsToPath:(NSString*)path;

// Apply the current mappings to the host document's sequence. Runs
// EffectMapper for each mapped row, registers undo on the document,
// triggers a grid reload. `eraseExisting` and `lock` are the
// import-options-sheet toggles. Returns NO if no mappings exist or
// the source sequence wasn't loaded.
- (BOOL)applyImportWithEraseExisting:(BOOL)eraseExisting
                                lock:(BOOL)lock
                                error:(NSError**)error;

@end

NS_ASSUME_NONNULL_END
