#pragma once

#import <Foundation/Foundation.h>

// ObjC bridge for live collaboration (Collab::CollabController) — one per
// app, owned by SequencerViewModel. Every method must be called on the main
// thread; every event is delivered on the main thread.

@class XLSequenceDocument;

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSInteger, XLCollabRole) {
    XLCollabRoleNone = 0,
    XLCollabRoleHost = 1,
    XLCollabRoleClient = 2,
};

typedef NS_ENUM(NSInteger, XLCollabState) {
    XLCollabStateIdle = 0,
    XLCollabStateConnecting = 1,
    XLCollabStateJoining = 2,
    XLCollabStateSyncing = 3,
    XLCollabStateActive = 4,
    XLCollabStateEnded = 5,
    // The host was lost; someone is taking over (edits keep working).
    XLCollabStateMigrating = 6,
};

typedef NS_ENUM(NSInteger, XLCollabEvent) {
    XLCollabEventStateChanged = 0,
    XLCollabEventParticipantsChanged = 1,
    XLCollabEventPresenceChanged = 2,
    XLCollabEventSequenceChanged = 3,   // payload: XLCollabChange
    XLCollabEventSequenceModified = 4,  // host only
    XLCollabEventRejected = 5,          // payload: XLCollabReject
    XLCollabEventUndoStateChanged = 6,
    XLCollabEventJoinReport = 7,        // payload: XLCollabJoinReport
    XLCollabEventSessionsFound = 8,
    XLCollabEventBrowseError = 9,       // payload: NSString (readable)
    XLCollabEventShowFolderChanged = 10, // payload: NSString (the session's show folder)
    // Sent synchronously just before a joined sequence replaces the open
    // one: stop playback and anything else reading the old sequence.
    XLCollabEventWillReplaceSequence = 11,
    // The session's layout reached memory: models, groups, view objects or
    // controllers changed. payload: XLCollabLayoutChange.
    XLCollabEventLayoutApplied = 12,
    // Someone's layout selection or modal-editor lock changed.
    XLCollabEventLayoutPresenceChanged = 13,
    // A layout change of this user was not (fully) taken. payload: NSString.
    XLCollabEventLayoutRejected = 14,
    // The SharePlay session on offer, or whether a FaceTime call is
    // active, changed (offeredGroupSession / groupInCall).
    XLCollabEventGroupChanged = 15,
    // Something from the host to read: this user's role changed, or a row
    // assignment it asked for was refused. payload: NSString.
    XLCollabEventNotice = 16,
    // The host was lost and someone else took over; `role` may have
    // changed. payload: XLCollabHostChange.
    XLCollabEventHostChanged = 17,
};

// Which network a session runs on: the local network (pairing code) or a
// SharePlay group (FaceTime call or Messages conversation, no code).
typedef NS_ENUM(NSInteger, XLCollabNetwork) {
    XLCollabNetworkLan = 0,
    XLCollabNetworkSharePlay = 1,
};

typedef NS_ENUM(NSInteger, XLCollabChangeCause) {
    XLCollabChangeCauseRemoteEdit = 0,
    XLCollabChangeCauseOwnEdit = 1,
    XLCollabChangeCauseOwnUndo = 2,
    XLCollabChangeCauseOwnEditPending = 3,
    XLCollabChangeCauseResync = 4,
    XLCollabChangeCauseFileArrived = 5,
    XLCollabChangeCauseOwnEditRejected = 6,
};

// What the change re-renders (Collab::PlanRender, the rule both apps follow).
typedef NS_ENUM(NSInteger, XLCollabRenderKind) {
    XLCollabRenderKindNothing = 0,
    XLCollabRenderKindRows = 1,       // `rows`, through the visible-range-first scheduler
    XLCollabRenderKindModels = 2,     // `modelNames`, the whole sequence (a layout change)
    XLCollabRenderKindEverything = 3, // every row
};

typedef NS_ENUM(NSInteger, XLCollabGhostKind) {
    XLCollabGhostKindAdd = 0,
    XLCollabGhostKindMove = 1,
    XLCollabGhostKindDelete = 2,
};

@interface XLCollabParticipant : NSObject
@property (nonatomic, readonly) NSInteger participantId;
@property (nonatomic, readonly) NSString* name;
@property (nonatomic, readonly) NSString* deviceKind;
// 0xRRGGBB
@property (nonatomic, readonly) uint32_t colorRGB;
@property (nonatomic, readonly) BOOL isHost;
@property (nonatomic, readonly) BOOL isLocal;
// Sees everything but can't change the sequence or the layout.
@property (nonatomic, readonly) BOOL isViewer;
// Names of the rows assigned to them.
@property (nonatomic, readonly) NSArray<NSString*>* assignedRows;
@end

// A row only one participant may edit.
@interface XLCollabRowAssignment : NSObject
@property (nonatomic, readonly) NSInteger ownerId;
@property (nonatomic, readonly) NSString* ownerName;
@property (nonatomic, readonly) uint32_t colorRGB;
// Assigned to this user.
@property (nonatomic, readonly) BOOL mine;
// "Assigned to Sam" / "Assigned to you".
@property (nonatomic, readonly) NSString* text;
// This user may release it (the owner or the host).
@property (nonatomic, readonly) BOOL canRelease;
@end

@interface XLCollabAdvert : NSObject
@property (nonatomic, readonly) NSString* advertId;
@property (nonatomic, readonly) NSString* sequenceName;
@property (nonatomic, readonly) NSString* hostName;
@property (nonatomic, readonly) NSString* deviceKind;
@end

@interface XLCollabJoinReport : NSObject
@property (nonatomic, readonly) NSString* showFolder;
@property (nonatomic, readonly) BOOL sessionCopy;
@property (nonatomic, readonly) NSArray<NSString*>* fetched;
@property (nonatomic, readonly) NSArray<NSString*>* missingMedia;
@property (nonatomic, readonly) NSArray<NSString*>* differentMedia;
@property (nonatomic, readonly) NSString* layoutDifference;
// Empty when the join went ahead.
@property (nonatomic, readonly) NSString* refusal;
@end

@interface XLCollabReject : NSObject
@property (nonatomic, readonly) uint64_t rejectId;
@property (nonatomic, readonly) NSString* text;
@property (nonatomic, readonly) BOOL offersNewLayer;
@end

@interface XLCollabHostChange : NSObject
@property (nonatomic, readonly) NSString* previousHostName;
@property (nonatomic, readonly) NSString* hostName;
// This iPad took over: it saves from now on (Save As, since the sequence is
// the session's copy).
@property (nonatomic, readonly) BOOL becameHost;
@property (nonatomic, readonly) BOOL saveAsOnly;
// "Sam left; Alex is now hosting"
@property (nonatomic, readonly) NSString* text;
@end

@interface XLCollabChange : NSObject
@property (nonatomic, readonly) XLCollabChangeCause cause;
@property (nonatomic, readonly) NSString* authorName;
@property (nonatomic, readonly) NSString* label;
// Top-level rows (SeqObjectId values) the change touched.
@property (nonatomic, readonly) NSArray<NSNumber*>* rows;
// Models (by name) to render for XLCollabRenderKindModels; for the other
// kinds, the models of `rows`, resolved when the change arrived.
@property (nonatomic, readonly) NSArray<NSString*>* modelNames;
@property (nonatomic, readonly) XLCollabRenderKind renderKind;
// -1 / -1: the whole sequence.
@property (nonatomic, readonly) NSInteger startMS;
@property (nonatomic, readonly) NSInteger endMS;
@property (nonatomic, readonly) BOOL rowsChanged; // added / removed / reordered / renamed
@property (nonatomic, readonly) BOOL mediaChanged;
@property (nonatomic, readonly) BOOL everything;
@end

// Another participant's visible time range (ruler band).
@interface XLCollabBand : NSObject
@property (nonatomic, readonly) NSInteger startMS;
@property (nonatomic, readonly) NSInteger endMS;
@property (nonatomic, readonly) uint32_t colorRGB;
@property (nonatomic, readonly) NSString* name;
@end

// What to draw on one effect of the grid.
@interface XLCollabEffectMark : NSObject
@property (nonatomic, readonly) NSInteger rowIndex;
@property (nonatomic, readonly) NSInteger effectIndex;
// Colours (0xRRGGBB) of the others who have it selected.
@property (nonatomic, readonly) NSArray<NSNumber*>* outlineColors;
// Initials of whoever has it open in their inspector; empty for nobody.
@property (nonatomic, readonly) NSString* lockInitials;
@property (nonatomic, readonly) uint32_t lockColorRGB;
// Recently changed by someone else.
@property (nonatomic, readonly) BOOL flash;
@end

// One of this device's structural edits waiting for the host.
@interface XLCollabGhost : NSObject
@property (nonatomic, readonly) XLCollabGhostKind kind;
@property (nonatomic, readonly) NSInteger rowIndex;
@property (nonatomic, readonly) NSInteger startMS;
@property (nonatomic, readonly) NSInteger endMS;
@property (nonatomic, readonly) NSString* effectName;
@end

@interface XLCollabOverlay : NSObject
@property (nonatomic, readonly) NSArray<XLCollabBand*>* bands;
// Display row index -> colours (0xRRGGBB) of the others focused on it.
@property (nonatomic, readonly) NSDictionary<NSNumber*, NSArray<NSNumber*>*>* rowStripes;
@property (nonatomic, readonly) NSArray<XLCollabEffectMark*>* effectMarks;
@property (nonatomic, readonly) NSArray<XLCollabGhost*>* ghosts;
// Display row index -> who holds that row.
@property (nonatomic, readonly) NSDictionary<NSNumber*, XLCollabRowAssignment*>* rowAssignments;
// Seconds until the earliest flash ends; 0 when nothing flashes.
@property (nonatomic, readonly) double flashSecondsLeft;
@end

@interface XLCollabLayoutChange : NSObject
@property (nonatomic, readonly) BOOL layoutChanged;
@property (nonatomic, readonly) BOOL controllersChanged;
// Names after the change.
@property (nonatomic, readonly) NSArray<NSString*>* modelsChanged;
@property (nonatomic, readonly) NSArray<NSString*>* modelsRemoved;
// Pairs of old name, new name.
@property (nonatomic, readonly) NSArray<NSArray<NSString*>*>* modelsRenamed;
@end

// What the layout editor shows for one model, group or view object others
// are working on.
@interface XLCollabLayoutMark : NSObject
@property (nonatomic, readonly) NSString* name;
// Colours of the others who have it selected.
@property (nonatomic, readonly) NSArray<NSNumber*>* outlineColors;
// Who has it open in a modal editor (empty: nobody), and in which.
@property (nonatomic, readonly) NSString* lockHolder;
@property (nonatomic, readonly) uint32_t lockColorRGB;
// "Sam is editing Arch 1's submodels".
@property (nonatomic, readonly) NSString* lockText;
@end

// (row, effect) pair for presence input.
@interface XLCollabEffectRef : NSObject
- (instancetype)initWithRow:(NSInteger)row effect:(NSInteger)effect;
@property (nonatomic, readonly) NSInteger rowIndex;
@property (nonatomic, readonly) NSInteger effectIndex;
@end

typedef void (^XLCollabEventHandler)(XLCollabEvent event, id _Nullable payload);

@interface XLCollabSession : NSObject

- (instancetype)initWithDocument:(XLSequenceDocument*)document;

@property (nonatomic, copy, nullable) XLCollabEventHandler eventHandler;

// A transport exists on this platform.
@property (nonatomic, readonly) BOOL available;
@property (nonatomic, readonly) XLCollabRole role;
@property (nonatomic, readonly) XLCollabState state;
@property (nonatomic, readonly) NSString* statusText;
@property (nonatomic, readonly) NSString* endText;
@property (nonatomic, readonly) NSString* endReason;
@property (nonatomic, readonly) NSArray<XLCollabParticipant*>* participants;
@property (nonatomic, readonly) NSArray<XLCollabAdvert*>* sessions;
@property (nonatomic, readonly, nullable) XLCollabJoinReport* joinReport;
// The pairing code while hosting (also after taking over); empty otherwise.
@property (nonatomic, readonly) NSString* pairingCode;

// ---- SharePlay ----
// SharePlay sessions can be hosted and joined on this iPad.
@property (nonatomic, readonly) BOOL groupAvailable;
// A FaceTime call is active, so hosting reaches its members at once
// (otherwise the system asks whom to invite).
@property (nonatomic, readonly) BOOL groupInCall;
// The SharePlay session someone in the call started, or nil. `sessions`
// may list it too; tell it apart by advertId.
@property (nonatomic, readonly, nullable) XLCollabAdvert* offeredGroupSession;
// The network the current (or last) session used.
@property (nonatomic, readonly) XLCollabNetwork sessionNetwork;

// Starts hosting the open sequence. Returns the pairing code, or nil with
// `error` set.
- (nullable NSString*)hostWithName:(NSString*)displayName error:(NSString* _Nullable* _Nullable)error
    NS_SWIFT_NAME(host(name:error:));
// Starts hosting the open sequence over SharePlay. Asynchronous: watch
// `state`; it is Active once the system has started the activity.
- (BOOL)hostGroupWithName:(NSString*)displayName error:(NSString* _Nullable* _Nullable)error
    NS_SWIFT_NAME(hostGroup(name:error:));
// Joins `offeredGroupSession`. Asynchronous: watch `state`.
- (BOOL)joinGroupWithName:(NSString*)displayName
                 advertId:(NSString*)advertId
               fetchFiles:(BOOL)fetchFiles
                    error:(NSString* _Nullable* _Nullable)error
    NS_SWIFT_NAME(joinGroup(name:advertId:fetchFiles:error:));
- (void)startBrowsing NS_SWIFT_NAME(startBrowsing());
- (void)stopBrowsing NS_SWIFT_NAME(stopBrowsing());
// Asynchronous: watch `state`.
- (BOOL)joinWithName:(NSString*)displayName
            advertId:(NSString*)advertId
                code:(NSString*)code
          fetchFiles:(BOOL)fetchFiles
               error:(NSString* _Nullable* _Nullable)error
    NS_SWIFT_NAME(join(name:advertId:code:fetchFiles:error:));
- (void)leave NS_SWIFT_NAME(leave());
- (void)end NS_SWIFT_NAME(end());

// What this user is looking at / working on. `softLock` is the effect shown
// in the inspector (nil for none).
- (void)setLocalPresenceStartMS:(NSInteger)startMS
                          endMS:(NSInteger)endMS
                    focusedRows:(NSArray<NSNumber*>*)focusedRows
                selectedEffects:(NSArray<XLCollabEffectRef*>*)selectedEffects
                       softLock:(nullable XLCollabEffectRef*)softLock
    NS_SWIFT_NAME(setLocalPresence(startMS:endMS:focusedRows:selectedEffects:softLock:));

// Where the effects last passed as selected are now (after an edit moved
// them between rows or reordered a layer). Missing ones are left out.
- (NSArray<XLCollabEffectRef*>*)currentLocationsOfLocalSelection NS_SWIFT_NAME(currentLocationsOfLocalSelection());

// Everything the grid draws for the others.
- (XLCollabOverlay*)overlay NS_SWIFT_NAME(overlay());

// Name of whoever else has this effect open for editing, or nil.
- (nullable NSString*)softLockHolderForRow:(NSInteger)row effect:(NSInteger)effect
    NS_SWIFT_NAME(softLockHolder(row:effect:));

- (BOOL)putOnNewLayer:(uint64_t)rejectId NS_SWIFT_NAME(putOnNewLayer(_:));

// ---- Roles and row assignments ----
// Not a viewer (YES when not in a session).
@property (nonatomic, readonly) BOOL canEditSequence;
// Host: people who join from now on are viewers.
@property (nonatomic) BOOL joinAsViewers;
// Host: makes a participant a viewer, or an editor again. Nil on success,
// else why not.
- (nullable NSString*)setViewer:(BOOL)viewer forParticipant:(NSInteger)participantId
    NS_SWIFT_NAME(setViewer(_:participant:));
// Assigns the display row's row (its model for a submodel or strand row) to
// a participant; the host may pick anyone who edits, others only
// themselves. Nil when done (or sent to the host), else why not.
- (nullable NSString*)assignRow:(NSInteger)row toParticipant:(NSInteger)participantId
    NS_SWIFT_NAME(assignRow(_:to:));
// Releases the claim covering the display row. Nil when done, else why not.
- (nullable NSString*)releaseRow:(NSInteger)row NS_SWIFT_NAME(releaseRow(_:));
- (nullable XLCollabRowAssignment*)assignmentForRow:(NSInteger)row NS_SWIFT_NAME(assignment(forRow:));
// Why this user can't edit the display row now ("Assigned to Sam"), or nil.
- (nullable NSString*)editRefusalForRow:(NSInteger)row NS_SWIFT_NAME(editRefusal(forRow:));

// Wording both apps use (Collab::Labels).
@property (class, nonatomic, readonly) NSString* labelEditor;
@property (class, nonatomic, readonly) NSString* labelViewer;
@property (class, nonatomic, readonly) NSString* labelViewOnly;
@property (class, nonatomic, readonly) NSString* labelMakeViewer;
@property (class, nonatomic, readonly) NSString* labelMakeEditor;
@property (class, nonatomic, readonly) NSString* labelJoinAsViewers;
@property (class, nonatomic, readonly) NSString* labelAssignTo;
@property (class, nonatomic, readonly) NSString* labelAssignToMe;
@property (class, nonatomic, readonly) NSString* labelRelease;
@property (class, nonatomic, readonly) NSString* labelRows;

// Client, after the session: keep the session copy of the host's show and
// return its folder (nil when the join didn't use one).
- (nullable NSString*)keepSessionCopy NS_SWIFT_NAME(keepSessionCopy());

// Re-render one model's output over a time range (-1 / -1: the whole
// sequence), through the same render entry point a local edit uses.
- (void)renderModelNamed:(NSString*)modelName startMS:(NSInteger)startMS endMS:(NSInteger)endMS
    NS_SWIFT_NAME(renderModel(named:startMS:endMS:));

// ---- Layout ----
// The session edits the layout live (the layout editor is open to everyone).
@property (nonatomic, readonly) BOOL liveLayout;
// The models / groups and view objects selected in this user's layout editor.
- (void)setLayoutSelectionModels:(NSArray<NSString*>*)models objects:(NSArray<NSString*>*)objects
    NS_SWIFT_NAME(setLayoutSelection(models:objects:));
// Before opening a modal editor on one model, group or view object
// (`what`: "submodels", "faces", ...): nil when this user now holds it,
// else the readable reason ("Sam is editing Arch 1's submodels"). Call
// endLayoutLock when the editor closes, whichever way.
- (nullable NSString*)beginLayoutLockOn:(NSString*)name what:(NSString*)what
    NS_SWIFT_NAME(beginLayoutLock(on:what:));
- (void)endLayoutLock NS_SWIFT_NAME(endLayoutLock());
// Why `name` is read-only here (someone else has it open in a modal
// editor), or nil.
- (nullable NSString*)layoutReadOnlyReason:(NSString*)name NS_SWIFT_NAME(layoutReadOnlyReason(_:));
// Every layout object others have selected or locked.
- (NSArray<XLCollabLayoutMark*>*)layoutMarks NS_SWIFT_NAME(layoutMarks());
// A layout text field gained or lost focus: proposals wait while one holds
// uncommitted typing.
+ (void)noteLayoutFieldFocused:(BOOL)focused NS_SWIFT_NAME(noteLayoutFieldFocused(_:));

@end

NS_ASSUME_NONNULL_END
