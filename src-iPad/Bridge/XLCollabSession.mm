#import "XLCollabSession.h"
#import "XLSequenceDocument.h"

#include "iPadRenderContext.h"
#include "collab/CollabController.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/SequenceElements.h"
#include "render/SequenceFile.h"
#include "models/Model.h"
#include "models/ModelManager.h"
#include "models/ViewObject.h"
#include "models/ViewObjectManager.h"
#include "xLightsVersion.h"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

namespace {

NSString* NS(const std::string& s) {
    return [NSString stringWithUTF8String:s.c_str()] ?: @"";
}

std::string Std(NSString* s) {
    return s ? std::string([s UTF8String] ?: "") : std::string();
}

uint32_t PackRGB(const xlColor& c) {
    return (uint32_t(c.red) << 16) | (uint32_t(c.green) << 8) | uint32_t(c.blue);
}

uint64_t NowMs() {
    using namespace std::chrono;
    return (uint64_t)duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

NSArray<NSString*>* NSStrings(const std::vector<std::string>& v) {
    NSMutableArray* out = [NSMutableArray arrayWithCapacity:v.size()];
    for (const auto& s : v) [out addObject:NS(s)];
    return out;
}

// Up to two letters: the first of the first two words.
std::string Initials(const std::string& name) {
    std::string out;
    bool atWordStart = true;
    for (char ch : name) {
        if (ch == ' ' || ch == '\t' || ch == '-' || ch == '.' || ch == '_') {
            atWordStart = true;
            continue;
        }
        if (atWordStart) {
            out += (char)std::toupper((unsigned char)ch);
            if (out.size() == 2) break;
            atWordStart = false;
        }
    }
    return out.empty() ? std::string("?") : out;
}

constexpr uint64_t kFlashMs = 1000;
// A layout change is sent once edits have paused this long (drags send a
// stream of small moves).
constexpr int kLayoutQuietMs = 300;

// Layout text fields with focus (uncommitted typing).
int g_layoutFieldsFocused = 0;
// Keeps ticking this long after the end so the goodbye gets out.
constexpr uint64_t kTickAfterEndMs = 5000;

} // namespace

// ---------------------------------------------------------------------------
// Value objects
// ---------------------------------------------------------------------------

@interface XLCollabParticipant ()
@property (nonatomic, readwrite) NSInteger participantId;
@property (nonatomic, readwrite) NSString* name;
@property (nonatomic, readwrite) NSString* deviceKind;
@property (nonatomic, readwrite) uint32_t colorRGB;
@property (nonatomic, readwrite) BOOL isHost;
@property (nonatomic, readwrite) BOOL isLocal;
@end
@implementation XLCollabParticipant
@end

@interface XLCollabAdvert ()
@property (nonatomic, readwrite) NSString* advertId;
@property (nonatomic, readwrite) NSString* sequenceName;
@property (nonatomic, readwrite) NSString* hostName;
@property (nonatomic, readwrite) NSString* deviceKind;
@end
@implementation XLCollabAdvert
@end

@interface XLCollabJoinReport ()
@property (nonatomic, readwrite) NSString* showFolder;
@property (nonatomic, readwrite) BOOL sessionCopy;
@property (nonatomic, readwrite) NSArray<NSString*>* fetched;
@property (nonatomic, readwrite) NSArray<NSString*>* missingMedia;
@property (nonatomic, readwrite) NSArray<NSString*>* differentMedia;
@property (nonatomic, readwrite) NSString* layoutDifference;
@property (nonatomic, readwrite) NSString* refusal;
@end
@implementation XLCollabJoinReport
@end

@interface XLCollabReject ()
@property (nonatomic, readwrite) uint64_t rejectId;
@property (nonatomic, readwrite) NSString* text;
@property (nonatomic, readwrite) BOOL offersNewLayer;
@end
@implementation XLCollabReject
@end

@interface XLCollabChange ()
@property (nonatomic, readwrite) XLCollabChangeCause cause;
@property (nonatomic, readwrite) NSString* authorName;
@property (nonatomic, readwrite) NSString* label;
@property (nonatomic, readwrite) NSArray<NSNumber*>* rows;
@property (nonatomic, readwrite) NSArray<NSString*>* modelNames;
@property (nonatomic, readwrite) XLCollabRenderKind renderKind;
@property (nonatomic, readwrite) NSInteger startMS;
@property (nonatomic, readwrite) NSInteger endMS;
@property (nonatomic, readwrite) BOOL rowsChanged;
@property (nonatomic, readwrite) BOOL mediaChanged;
@property (nonatomic, readwrite) BOOL everything;
@end
@implementation XLCollabChange
@end

@interface XLCollabBand ()
@property (nonatomic, readwrite) NSInteger startMS;
@property (nonatomic, readwrite) NSInteger endMS;
@property (nonatomic, readwrite) uint32_t colorRGB;
@property (nonatomic, readwrite) NSString* name;
@end
@implementation XLCollabBand
@end

@interface XLCollabEffectMark ()
@property (nonatomic, readwrite) NSInteger rowIndex;
@property (nonatomic, readwrite) NSInteger effectIndex;
@property (nonatomic, readwrite) NSArray<NSNumber*>* outlineColors;
@property (nonatomic, readwrite) NSString* lockInitials;
@property (nonatomic, readwrite) uint32_t lockColorRGB;
@property (nonatomic, readwrite) BOOL flash;
@end
@implementation XLCollabEffectMark
@end

@interface XLCollabGhost ()
@property (nonatomic, readwrite) XLCollabGhostKind kind;
@property (nonatomic, readwrite) NSInteger rowIndex;
@property (nonatomic, readwrite) NSInteger startMS;
@property (nonatomic, readwrite) NSInteger endMS;
@property (nonatomic, readwrite) NSString* effectName;
@end
@implementation XLCollabGhost
@end

@interface XLCollabOverlay ()
@property (nonatomic, readwrite) NSArray<XLCollabBand*>* bands;
@property (nonatomic, readwrite) NSDictionary<NSNumber*, NSArray<NSNumber*>*>* rowStripes;
@property (nonatomic, readwrite) NSArray<XLCollabEffectMark*>* effectMarks;
@property (nonatomic, readwrite) NSArray<XLCollabGhost*>* ghosts;
@property (nonatomic, readwrite) double flashSecondsLeft;
@end
@implementation XLCollabOverlay
@end

@interface XLCollabLayoutChange ()
@property (nonatomic, readwrite) BOOL layoutChanged;
@property (nonatomic, readwrite) BOOL controllersChanged;
@property (nonatomic, readwrite) NSArray<NSString*>* modelsChanged;
@property (nonatomic, readwrite) NSArray<NSString*>* modelsRemoved;
@property (nonatomic, readwrite) NSArray<NSArray<NSString*>*>* modelsRenamed;
@end
@implementation XLCollabLayoutChange
@end

@interface XLCollabLayoutMark ()
@property (nonatomic, readwrite) NSString* name;
@property (nonatomic, readwrite) NSArray<NSNumber*>* outlineColors;
@property (nonatomic, readwrite) NSString* lockHolder;
@property (nonatomic, readwrite) uint32_t lockColorRGB;
@property (nonatomic, readwrite) NSString* lockText;
@end
@implementation XLCollabLayoutMark
@end

@implementation XLCollabEffectRef
- (instancetype)initWithRow:(NSInteger)row effect:(NSInteger)effect {
    self = [super init];
    if (self) {
        _rowIndex = row;
        _effectIndex = effect;
    }
    return self;
}
@end

@interface XLCollabSession ()
- (void)deliver:(XLCollabEvent)event payload:(nullable id)payload;
- (void)updateTickTimer;
- (void)handleSequenceChange:(const Collab::SequenceChange&)change;
- (XLCollabJoinReport*)wrapJoinReport:(const Collab::JoinReport&)r;
- (void)syncSessionFlags;
- (BOOL)holdsLayoutLock;
@end

// ---------------------------------------------------------------------------
// The ICollabApp the controller drives
// ---------------------------------------------------------------------------

namespace {

class iPadCollabApp : public Collab::ICollabApp {
public:
    iPadCollabApp(XLCollabSession* owner, XLSequenceDocument* doc) : _owner(owner), _doc(doc) {}

    iPadRenderContext* Ctx() const { return (iPadRenderContext*)[_doc renderContext]; }

    // Where a joined sequence is written before it is opened: never the
    // user's own copy of the sequence, which the host may also be editing.
    std::string sessionSequenceFile;

    SequenceElements& Sequence() override { return Ctx()->GetSequenceElements(); }
    bool HasSequence() override { return Ctx()->IsSequenceLoaded(); }

    bool SaveXsq(std::string& out) override {
        auto* ctx = Ctx();
        auto* sf = ctx->GetSequenceFile();
        if (!ctx->IsSequenceLoaded() || !sf) return false;
        std::error_code ec;
        auto dir = std::filesystem::path(Std(NSTemporaryDirectory())) / "xLightsCollab";
        std::filesystem::create_directories(dir, ec);
        auto tmp = dir / ("snapshot-" + std::to_string(NowMs()) + ".xsq");
        // Same path swap as the autosave: Save only uses the path for the
        // final write.
        const std::string orig = sf->GetFullPath();
        sf->SetFullPath(tmp.string());
        const bool ok = sf->Save(ctx->GetSequenceElements());
        sf->SetFullPath(orig);
        if (!ok) return false;
        std::ifstream in(tmp, std::ios::binary);
        std::stringstream ss;
        ss << in.rdbuf();
        out = ss.str();
        in.close();
        std::filesystem::remove(tmp, ec);
        return !out.empty();
    }

    bool LoadXsq(const std::string& xsq) override {
        if (sessionSequenceFile.empty()) return false;
        std::error_code ec;
        std::filesystem::create_directories(std::filesystem::path(sessionSequenceFile).parent_path(), ec);
        {
            std::ofstream o(sessionSequenceFile, std::ios::binary | std::ios::trunc);
            o << xsq;
            if (!o.good()) return false;
        }
        // Lets the UI stop playback and its timers before the old
        // sequence goes away.
        [_owner deliver:XLCollabEventWillReplaceSequence payload:nil];
        return [_doc openSequence:NS(sessionSequenceFile)] ? true : false;
    }

    std::string ShowFolder() override { return Ctx()->GetShowDirectory(); }

    std::vector<std::string> AudioFiles() override {
        std::vector<std::string> out;
        auto* sf = Ctx()->GetSequenceFile();
        if (!sf) return out;
        if (!sf->GetMediaFile().empty()) out.push_back(sf->GetMediaFile());
        for (int i = 0; i < sf->GetAltTrackCount(); ++i) out.push_back(sf->GetAltTrack(i).path);
        return out;
    }

    bool UseShowFolder(const std::string& folder) override {
        NSArray<NSString*>* media = [_doc mediaFolderPaths];
        if (![_doc loadShowFolder:NS(folder) mediaFolders:media]) return false;
        [_owner deliver:XLCollabEventShowFolderChanged payload:NS(folder)];
        return true;
    }

    void AddMediaFolder(const std::string& folder) override { Ctx()->AddSessionMediaFolder(folder); }

    void SetCollaborationLock(bool locked, const std::string& reason) override {
        [_doc setCollaborationLock:locked ? YES : NO reason:NS(reason)];
    }

    void SetSequenceSettingsLock(bool locked, const std::string& reason) override {
        if (auto* ctx = Ctx()) {
            ctx->SetSequenceSettingsLock(locked, reason);
        }
    }

    void OnSessionStateChanged() override {
        [_owner syncSessionFlags];
        [_owner updateTickTimer];
        [_owner deliver:XLCollabEventStateChanged payload:nil];
    }
    void OnParticipantsChanged() override { [_owner deliver:XLCollabEventParticipantsChanged payload:nil]; }
    void OnPresenceChanged(Collab::ParticipantId) override { [_owner deliver:XLCollabEventPresenceChanged payload:nil]; }
    void OnSequenceChanged(const Collab::SequenceChange& change) override { [_owner handleSequenceChange:change]; }
    void MarkSequenceModified() override { [_owner deliver:XLCollabEventSequenceModified payload:nil]; }
    void OnRejected(const Collab::RejectNotice& n) override {
        XLCollabReject* r = [XLCollabReject new];
        r.rejectId = n.id;
        r.text = NS(n.text);
        r.offersNewLayer = n.action == Collab::RejectAction::PutOnNewLayer;
        [_owner deliver:XLCollabEventRejected payload:r];
    }
    void OnUndoStateChanged() override { [_owner deliver:XLCollabEventUndoStateChanged payload:nil]; }
    void OnJoinReport(const Collab::JoinReport& report) override {
        [_owner deliver:XLCollabEventJoinReport payload:[_owner wrapJoinReport:report]];
    }
    void OnSessionsFound(const std::vector<Collab::SessionAdvert>&) override {
        [_owner deliver:XLCollabEventSessionsFound payload:nil];
    }
    void OnBrowseError(const std::string& reason) override {
        spdlog::warn("Collaboration: browsing failed: {}", reason);
        [_owner deliver:XLCollabEventBrowseError payload:NS(reason)];
    }
    void OnLog(const std::string& line) override { spdlog::info("Collaboration: {}", line); }

    // ---- Layout ----
    bool SupportsLiveLayout() override { return true; }

    uint64_t LayoutGeneration() override { return Ctx()->LayoutGeneration(); }

    bool LayoutEditInProgress() override {
        return g_layoutFieldsFocused > 0 || [_owner holdsLayoutLock] || Ctx()->LayoutEditRecentlyActive(kLayoutQuietMs);
    }

    bool SerializeLayout(Collab::LayoutFiles& out) override {
        return Ctx()->SerializeLayoutFiles(out.rgbEffects, out.networks);
    }

    bool ApplyLayout(const Collab::LayoutFiles& current, const Collab::LayoutFiles& target, Collab::LayoutApplied& out) override {
        const bool rgbDiffers = current.rgbEffects != target.rgbEffects;
        const bool networksDiffer = current.networks != target.networks;
        ShowMergeApply::LayoutChanges changes;
        if (!Ctx()->ApplyLiveLayout(current.rgbEffects, target.rgbEffects, rgbDiffers, target.networks, networksDiffer, changes)) {
            return false;
        }
        out.layoutChanged = rgbDiffers;
        out.controllersChanged = networksDiffer;
        out.modelsChanged = changes.modelsAdded;
        out.modelsChanged.insert(out.modelsChanged.end(), changes.modelsChanged.begin(), changes.modelsChanged.end());
        out.modelsRemoved = changes.modelsRemoved;
        out.modelsRenamed = changes.modelsRenamed;

        XLCollabLayoutChange* c = [XLCollabLayoutChange new];
        c.layoutChanged = out.layoutChanged;
        c.controllersChanged = out.controllersChanged;
        c.modelsChanged = NSStrings(out.modelsChanged);
        c.modelsRemoved = NSStrings(out.modelsRemoved);
        NSMutableArray* renamed = [NSMutableArray array];
        for (const auto& [from, to] : out.modelsRenamed) [renamed addObject:@[ NS(from), NS(to) ]];
        c.modelsRenamed = renamed;
        [_owner deliver:XLCollabEventLayoutApplied payload:c];
        return true;
    }

    void OnLayoutPresenceChanged() override { [_owner deliver:XLCollabEventLayoutPresenceChanged payload:nil]; }
    void OnLayoutRejected(const std::string& text) override {
        spdlog::info("Collaboration: layout change not taken: {}", text);
        [_owner deliver:XLCollabEventLayoutRejected payload:NS(text)];
    }

private:
    __unsafe_unretained XLCollabSession* _owner;
    __unsafe_unretained XLSequenceDocument* _doc;
};

} // namespace

// ---------------------------------------------------------------------------
// XLCollabSession
// ---------------------------------------------------------------------------

@implementation XLCollabSession {
    XLSequenceDocument* _document;
    std::unique_ptr<iPadCollabApp> _app;
    std::unique_ptr<Collab::CollabController> _controller;
    dispatch_source_t _tickTimer;
    uint64_t _endedAtMs;
    std::string _pairingCode;
    // Effects someone else just changed, until when to highlight them.
    std::unordered_map<SeqObjectId, uint64_t> _flashUntil;
    Collab::PresenceInfo _localPresence;
    bool _holdsLayoutLock;
}

- (instancetype)initWithDocument:(XLSequenceDocument*)document {
    self = [super init];
    if (self) {
        _document = document;
        _app = std::make_unique<iPadCollabApp>(self, document);
        Collab::Dispatcher dispatch = [](std::function<void()> fn) {
            dispatch_async(dispatch_get_main_queue(), ^{
                fn();
            });
        };
        Collab::ControllerOptions opts;
        opts.appVersion = xlights_version_string;
        opts.deviceKind = "ipad";
        opts.sessionFolder = (std::filesystem::path(Std(NSTemporaryDirectory())) / "xLightsCollab" / "session").string();
        _controller = std::make_unique<Collab::CollabController>(*_app, Collab::CollabNetFactory::Lan(dispatch), dispatch, opts);
    }
    return self;
}

- (void)dealloc {
    if (_tickTimer) {
        dispatch_source_cancel(_tickTimer);
        _tickTimer = nil;
    }
    // The controller calls back into the app while it goes.
    _controller.reset();
    _app.reset();
}

- (void)deliver:(XLCollabEvent)event payload:(nullable id)payload {
    if (_eventHandler) _eventHandler(event, payload);
}

- (void)syncSessionFlags {
    using C = Collab::CollabController;
    const auto state = _controller->GetState();
    const bool inSession = _controller->GetRole() != C::Role::None && state != C::State::Idle && state != C::State::Ended;
    if (!inSession) _holdsLayoutLock = false;
    _app->Ctx()->SetCollabSession(inSession, inSession && _controller->LiveLayout(), _controller->GetRole() == C::Role::Host);
}

- (BOOL)holdsLayoutLock { return _holdsLayoutLock ? YES : NO; }

- (void)updateTickTimer {
    const bool ended = _controller->GetState() == Collab::CollabController::State::Ended;
    if (ended && _endedAtMs == 0) _endedAtMs = NowMs();
    if (!ended) _endedAtMs = 0;
    const bool want = _controller->GetRole() != Collab::CollabController::Role::None
        && (!ended || NowMs() - _endedAtMs < kTickAfterEndMs);
    if (want && !_tickTimer) {
        _tickTimer = dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0, dispatch_get_main_queue());
        dispatch_source_set_timer(_tickTimer, dispatch_time(DISPATCH_TIME_NOW, 50 * NSEC_PER_MSEC), 50 * NSEC_PER_MSEC, 10 * NSEC_PER_MSEC);
        __unsafe_unretained XLCollabSession* weakSelf = self;
        dispatch_source_set_event_handler(_tickTimer, ^{
            XLCollabSession* s = weakSelf;
            s->_controller->Tick(NowMs());
            [s syncSessionFlags];
            [s updateTickTimer];
        });
        dispatch_resume(_tickTimer);
    } else if (!want && _tickTimer) {
        dispatch_source_cancel(_tickTimer);
        _tickTimer = nil;
    }
}

// MARK: - State

- (BOOL)available { return _controller->Available() ? YES : NO; }

- (XLCollabRole)role {
    switch (_controller->GetRole()) {
    case Collab::CollabController::Role::Host: return XLCollabRoleHost;
    case Collab::CollabController::Role::Client: return XLCollabRoleClient;
    default: return XLCollabRoleNone;
    }
}

- (XLCollabState)state {
    switch (_controller->GetState()) {
    case Collab::CollabController::State::Idle: return XLCollabStateIdle;
    case Collab::CollabController::State::Connecting: return XLCollabStateConnecting;
    case Collab::CollabController::State::Joining: return XLCollabStateJoining;
    case Collab::CollabController::State::Syncing: return XLCollabStateSyncing;
    case Collab::CollabController::State::Active: return XLCollabStateActive;
    case Collab::CollabController::State::Ended: return XLCollabStateEnded;
    }
    return XLCollabStateIdle;
}

- (NSString*)statusText { return NS(_controller->StatusText()); }
- (NSString*)endText { return NS(_controller->EndText()); }
- (NSString*)endReason { return NS(_controller->EndReason()); }

- (NSString*)pairingCode {
    if (_controller->GetRole() != Collab::CollabController::Role::Host) return @"";
    return NS(_pairingCode);
}

- (NSArray<XLCollabParticipant*>*)participants {
    NSMutableArray* out = [NSMutableArray array];
    for (const auto& p : _controller->Participants()) {
        XLCollabParticipant* o = [XLCollabParticipant new];
        o.participantId = p.id;
        o.name = NS(p.name);
        o.deviceKind = NS(p.deviceKind);
        o.colorRGB = PackRGB(p.colour);
        o.isHost = p.isHost;
        o.isLocal = p.isLocal;
        [out addObject:o];
    }
    return out;
}

- (NSArray<XLCollabAdvert*>*)sessions {
    NSMutableArray* out = [NSMutableArray array];
    for (const auto& s : _controller->Sessions()) {
        XLCollabAdvert* a = [XLCollabAdvert new];
        a.advertId = NS(s.id);
        a.sequenceName = NS(s.sequenceName);
        a.hostName = NS(s.hostName);
        a.deviceKind = NS(s.deviceKind);
        [out addObject:a];
    }
    return out;
}

- (XLCollabJoinReport*)wrapJoinReport:(const Collab::JoinReport&)r {
    XLCollabJoinReport* o = [XLCollabJoinReport new];
    o.showFolder = NS(r.showFolder);
    o.sessionCopy = r.sessionCopy;
    o.fetched = NSStrings(r.fetched);
    o.missingMedia = NSStrings(r.missingMedia);
    o.differentMedia = NSStrings(r.differentMedia);
    o.layoutDifference = NS(r.layoutDifference);
    o.refusal = NS(r.refusal);
    return o;
}

- (nullable XLCollabJoinReport*)joinReport {
    const auto& r = _controller->GetJoinReport();
    return r ? [self wrapJoinReport:*r] : nil;
}

// MARK: - Lifecycle

- (nullable NSString*)hostWithName:(NSString*)displayName error:(NSString* _Nullable* _Nullable)error {
    auto* sf = _app->Ctx()->GetSequenceFile();
    if (!_app->Ctx()->IsSequenceLoaded() || !sf) {
        if (error) *error = @"Open a sequence first.";
        return nil;
    }
    std::string code, err;
    _flashUntil.clear();
    _localPresence = {};
    if (!_controller->Host(Std(displayName), sf->GetFullName(), code, err)) {
        if (error) *error = NS(err);
        [self updateTickTimer];
        return nil;
    }
    _pairingCode = code;
    [self syncSessionFlags];
    [self updateTickTimer];
    return NS(code);
}

- (void)startBrowsing { _controller->StartBrowsing(); }
- (void)stopBrowsing { _controller->StopBrowsing(); }

- (BOOL)joinWithName:(NSString*)displayName
            advertId:(NSString*)advertId
                code:(NSString*)code
          fetchFiles:(BOOL)fetchFiles
               error:(NSString* _Nullable* _Nullable)error {
    std::string seqName;
    for (const auto& s : _controller->Sessions()) {
        if (s.id == Std(advertId)) seqName = s.sequenceName;
    }
    seqName = std::filesystem::path(seqName).filename().string();
    if (seqName.empty()) seqName = "Session.xsq";
    // A fresh folder per join so a later session never opens a stale file.
    _app->sessionSequenceFile = (std::filesystem::path(Std(NSTemporaryDirectory())) / "xLightsCollab" /
                                 ("join-" + std::to_string(NowMs())) / seqName).string();
    _flashUntil.clear();
    _localPresence = {};
    _pairingCode.clear();
    std::string err;
    const bool ok = _controller->Join(Std(displayName), Std(advertId), Std(code), fetchFiles ? true : false, err);
    if (!ok && error) *error = NS(err);
    [self updateTickTimer];
    return ok ? YES : NO;
}

- (void)leave {
    _controller->Leave();
    [self syncSessionFlags];
    [self updateTickTimer];
}

- (void)end {
    _controller->End();
    [self syncSessionFlags];
    [self updateTickTimer];
}

// MARK: - Presence

- (EffectLayer*)layerForRow:(NSInteger)row {
    auto& se = _app->Ctx()->GetSequenceElements();
    if (row < 0 || row >= se.GetRowInformationSize()) return nullptr;
    auto* info = se.GetRowInformation((size_t)row);
    if (!info || !info->element) return nullptr;
    return info->element->GetEffectLayer(info->layerIndex);
}

- (Effect*)effectForRow:(NSInteger)row index:(NSInteger)index {
    EffectLayer* layer = [self layerForRow:row];
    if (!layer || index < 0 || index >= layer->GetEffectCount()) return nullptr;
    return layer->GetEffect((int)index);
}

- (void)setLocalPresenceStartMS:(NSInteger)startMS
                          endMS:(NSInteger)endMS
                    focusedRows:(NSArray<NSNumber*>*)focusedRows
                selectedEffects:(NSArray<XLCollabEffectRef*>*)selectedEffects
                       softLock:(nullable XLCollabEffectRef*)softLock {
    if (!_app->Ctx()->IsSequenceLoaded()) return;
    auto& se = _app->Ctx()->GetSequenceElements();
    Collab::PresenceInfo p;
    p.startMS = (int)startMS;
    p.endMS = (int)endMS;
    for (NSNumber* r in focusedRows) {
        const int row = r.intValue;
        if (row < 0 || row >= se.GetRowInformationSize()) continue;
        auto* info = se.GetRowInformation((size_t)row);
        if (!info || !info->element) continue;
        SeqObjectId uid = info->element->GetUid();
        if (std::find(p.focusedElements.begin(), p.focusedElements.end(), uid) == p.focusedElements.end()) {
            p.focusedElements.push_back(uid);
        }
    }
    for (XLCollabEffectRef* ref in selectedEffects) {
        if (Effect* e = [self effectForRow:ref.rowIndex index:ref.effectIndex]) {
            p.selectedEffects.push_back(e->GetUid());
        }
    }
    if (softLock) {
        if (Effect* e = [self effectForRow:softLock.rowIndex index:softLock.effectIndex]) {
            p.softLock = e->GetUid();
        }
    }
    _localPresence = p;
    _controller->SetLocalPresence(p);
}

- (NSArray<XLCollabEffectRef*>*)currentLocationsOfLocalSelection {
    NSMutableArray* out = [NSMutableArray array];
    if (_localPresence.selectedEffects.empty() || !_app->Ctx()->IsSequenceLoaded()) return out;
    std::unordered_set<SeqObjectId> want(_localPresence.selectedEffects.begin(), _localPresence.selectedEffects.end());
    auto& se = _app->Ctx()->GetSequenceElements();
    const int rows = se.GetRowInformationSize();
    for (int r = 0; r < rows && !want.empty(); ++r) {
        EffectLayer* layer = [self layerForRow:r];
        if (!layer) continue;
        for (int i = 0; i < layer->GetEffectCount(); ++i) {
            auto it = want.find(layer->GetEffect(i)->GetUid());
            if (it == want.end()) continue;
            want.erase(it);
            [out addObject:[[XLCollabEffectRef alloc] initWithRow:r effect:i]];
        }
    }
    return out;
}

- (XLCollabOverlay*)overlay {
    XLCollabOverlay* o = [XLCollabOverlay new];
    NSMutableArray* bands = [NSMutableArray array];
    NSMutableDictionary* stripes = [NSMutableDictionary dictionary];
    NSMutableArray* marks = [NSMutableArray array];
    NSMutableArray* ghosts = [NSMutableArray array];
    o.bands = bands;
    o.rowStripes = stripes;
    o.effectMarks = marks;
    o.ghosts = ghosts;
    o.flashSecondsLeft = 0;
    if (!_app->Ctx()->IsSequenceLoaded()) return o;

    const uint64_t now = NowMs();
    for (auto it = _flashUntil.begin(); it != _flashUntil.end();) {
        if (it->second <= now) it = _flashUntil.erase(it);
        else ++it;
    }

    // Others' focus and selection, keyed by object id.
    std::unordered_map<SeqObjectId, std::vector<uint32_t>> focused;
    std::unordered_map<SeqObjectId, std::vector<uint32_t>> selected;
    std::unordered_set<SeqObjectId> softLocked;
    const bool inSession = _controller->GetRole() != Collab::CollabController::Role::None
        && _controller->GetState() != Collab::CollabController::State::Ended;
    if (inSession) {
        for (const auto& part : _controller->Participants()) {
            if (part.isLocal) continue;
            const Collab::PresenceInfo* pi = _controller->PresenceOf(part.id);
            if (!pi) continue;
            const uint32_t col = PackRGB(part.colour);
            if (pi->startMS >= 0 && pi->endMS > pi->startMS) {
                XLCollabBand* b = [XLCollabBand new];
                b.startMS = pi->startMS;
                b.endMS = pi->endMS;
                b.colorRGB = col;
                b.name = NS(part.name);
                [bands addObject:b];
            }
            for (auto uid : pi->focusedElements) focused[uid].push_back(col);
            for (auto uid : pi->selectedEffects) selected[uid].push_back(col);
            if (pi->softLock != kNoSeqObjectId) softLocked.insert(pi->softLock);
        }
    }

    // Flash only what this user can see.
    const int visStart = _localPresence.startMS;
    const int visEnd = _localPresence.endMS;
    auto& se = _app->Ctx()->GetSequenceElements();
    const int rows = se.GetRowInformationSize();
    double flashLeft = 0;
    const bool anyEffects = !selected.empty() || !softLocked.empty() || !_flashUntil.empty();
    for (int r = 0; r < rows; ++r) {
        auto* info = se.GetRowInformation((size_t)r);
        if (!info || !info->element) continue;
        auto f = focused.find(info->element->GetUid());
        if (f != focused.end()) {
            NSMutableArray* cols = [NSMutableArray array];
            for (auto c : f->second) [cols addObject:@(c)];
            stripes[@(r)] = cols;
        }
        if (!anyEffects) continue;
        EffectLayer* layer = info->element->GetEffectLayer(info->layerIndex);
        if (!layer) continue;
        for (int i = 0; i < layer->GetEffectCount(); ++i) {
            Effect* e = layer->GetEffect(i);
            const SeqObjectId uid = e->GetUid();
            auto s = selected.find(uid);
            const bool locked = softLocked.count(uid) != 0;
            auto fl = _flashUntil.find(uid);
            bool flash = false;
            if (fl != _flashUntil.end()) {
                const bool visible = visStart < 0 || visEnd < 0
                    || (e->GetEndTimeMS() >= visStart && e->GetStartTimeMS() <= visEnd);
                if (visible) {
                    flash = true;
                    const double left = double(fl->second - now) / 1000.0;
                    if (flashLeft == 0 || left < flashLeft) flashLeft = left;
                }
            }
            if (s == selected.end() && !locked && !flash) continue;
            XLCollabEffectMark* m = [XLCollabEffectMark new];
            m.rowIndex = r;
            m.effectIndex = i;
            NSMutableArray* cols = [NSMutableArray array];
            if (s != selected.end()) {
                for (auto c : s->second) [cols addObject:@(c)];
            }
            m.outlineColors = cols;
            m.lockInitials = @"";
            if (locked) {
                Collab::ParticipantId holder = _controller->SoftLockHolder(uid);
                if (auto part = _controller->FindParticipant(holder)) {
                    m.lockInitials = NS(Initials(part->name));
                    m.lockColorRGB = PackRGB(part->colour);
                }
            }
            m.flash = flash;
            [marks addObject:m];
        }
    }
    o.flashSecondsLeft = flashLeft;

    if (_controller->GetRole() == Collab::CollabController::Role::Client) {
        for (const auto& g : _controller->PendingGhosts()) {
            int row = -1;
            for (int r = 0; r < rows && row < 0; ++r) {
                auto* info = se.GetRowInformation((size_t)r);
                if (!info || !info->element || info->element->GetUid() != g.effect.element) continue;
                EffectLayer* layer = info->element->GetEffectLayer(info->layerIndex);
                if (layer && layer->GetUid() == g.effect.layer) row = r;
            }
            if (row < 0) continue;
            XLCollabGhost* gh = [XLCollabGhost new];
            switch (g.kind) {
            case Collab::PendingGhost::Kind::Add: gh.kind = XLCollabGhostKindAdd; break;
            case Collab::PendingGhost::Kind::Move: gh.kind = XLCollabGhostKindMove; break;
            case Collab::PendingGhost::Kind::Delete: gh.kind = XLCollabGhostKindDelete; break;
            }
            gh.rowIndex = row;
            gh.startMS = g.startMS;
            gh.endMS = g.endMS;
            gh.effectName = NS(g.effectName);
            [ghosts addObject:gh];
        }
    }
    return o;
}

- (nullable NSString*)softLockHolderForRow:(NSInteger)row effect:(NSInteger)effect {
    Effect* e = [self effectForRow:row index:effect];
    if (!e) return nil;
    Collab::ParticipantId holder = _controller->SoftLockHolder(e->GetUid());
    if (holder == Collab::kNoParticipant) return nil;
    auto part = _controller->FindParticipant(holder);
    if (!part || part->isLocal) return nil;
    return NS(part->name);
}

- (BOOL)putOnNewLayer:(uint64_t)rejectId {
    return _controller->PutOnNewLayer(rejectId) ? YES : NO;
}

- (nullable NSString*)keepSessionCopy {
    std::string folder;
    if (!_controller->KeepSessionCopy(folder) || folder.empty()) return nil;
    return NS(folder);
}

// MARK: - Changes

- (void)handleSequenceChange:(const Collab::SequenceChange&)c {
    using Cause = Collab::SequenceChange::Cause;
    XLCollabChange* o = [XLCollabChange new];
    switch (c.cause) {
    case Cause::RemoteEdit: o.cause = XLCollabChangeCauseRemoteEdit; break;
    case Cause::OwnEdit: o.cause = XLCollabChangeCauseOwnEdit; break;
    case Cause::OwnUndo: o.cause = XLCollabChangeCauseOwnUndo; break;
    case Cause::OwnEditPending: o.cause = XLCollabChangeCauseOwnEditPending; break;
    case Cause::Resync: o.cause = XLCollabChangeCauseResync; break;
    case Cause::FileArrived: o.cause = XLCollabChangeCauseFileArrived; break;
    case Cause::OwnEditRejected: o.cause = XLCollabChangeCauseOwnEditRejected; break;
    }
    o.authorName = @"";
    if (auto part = _controller->FindParticipant(c.author)) o.authorName = NS(part->name);
    o.label = NS(c.label);
    o.startMS = c.startMS;
    o.endMS = c.endMS;
    o.rowsChanged = c.rowsAdded || c.rowsRemoved || c.rowsReordered || c.rowsRenamed;
    o.mediaChanged = c.mediaChanged;
    o.everything = c.everything;

    NSMutableArray* rowIds = [NSMutableArray arrayWithCapacity:c.rows.size()];
    for (SeqObjectId uid : c.rows) [rowIds addObject:@(uid)];
    o.rows = rowIds;
    NSMutableArray* models = [NSMutableArray array];
    if (_app->Ctx()->IsSequenceLoaded()) {
        auto& se = _app->Ctx()->GetSequenceElements();
        std::unordered_set<std::string> seen;
        for (SeqObjectId uid : c.rows) {
            Element* el = se.GetElementFromUid(uid);
            if (!el || el->GetType() == ElementType::ELEMENT_TYPE_TIMING) continue;
            const std::string& name = el->GetModelName();
            if (!name.empty() && seen.insert(name).second) [models addObject:NS(name)];
        }
    }
    o.modelNames = models;

    if (c.cause == Cause::RemoteEdit && !c.effects.empty()) {
        const uint64_t until = NowMs() + kFlashMs;
        for (SeqObjectId uid : c.effects) _flashUntil[uid] = until;
    }
    [self deliver:XLCollabEventSequenceChanged payload:o];
}

- (void)renderModelNamed:(NSString*)modelName startMS:(NSInteger)startMS endMS:(NSInteger)endMS {
    auto* ctx = _app->Ctx();
    if (!ctx->IsSequenceLoaded() || modelName.length == 0) return;
    int s = (int)startMS;
    int e = (int)endMS;
    if (s < 0 || e < 0) {
        s = 0;
        e = ctx->GetSequenceElements().GetSequenceEnd();
    }
    ctx->RenderEffectForModel(Std(modelName), s, e, true);
}

// MARK: - Layout

- (BOOL)liveLayout { return _controller->LiveLayout() ? YES : NO; }

// The layout id the session knows the object by (else its name).
- (std::string)layoutObjectIdFor:(const std::string&)name {
    auto* ctx = _app->Ctx();
    if (ctx->HasModelManager()) {
        if (Model* m = ctx->GetModelManager().GetModel(name)) {
            return m->GetObjectId().empty() ? name : m->GetObjectId();
        }
    }
    if (ViewObject* v = ctx->GetAllObjects().GetViewObject(name)) {
        return v->GetObjectId().empty() ? name : v->GetObjectId();
    }
    return name;
}

- (void)setLayoutSelectionModels:(NSArray<NSString*>*)models objects:(NSArray<NSString*>*)objects {
    std::vector<std::string> ids;
    for (NSString* n in models) ids.push_back([self layoutObjectIdFor:Std(n)]);
    for (NSString* n in objects) ids.push_back([self layoutObjectIdFor:Std(n)]);
    _controller->SetLayoutSelection(ids);
}

- (nullable NSString*)beginLayoutLockOn:(NSString*)name what:(NSString*)what {
    if (!_controller->LiveLayout()) return nil;
    Collab::LayoutLockInfo lock;
    lock.object = [self layoutObjectIdFor:Std(name)];
    lock.name = Std(name);
    lock.what = Std(what);
    std::string heldBy;
    if (!_controller->BeginLayoutLock(lock, heldBy)) {
        if (heldBy.empty()) heldBy = "Someone else";
        return NS(heldBy + " is editing " + lock.name + "'s " + lock.what);
    }
    _holdsLayoutLock = true;
    return nil;
}

- (void)endLayoutLock {
    if (!_holdsLayoutLock) return;
    _holdsLayoutLock = false;
    _controller->EndLayoutLock();
}

- (nullable NSString*)layoutReadOnlyReason:(NSString*)name {
    if (!_controller->LiveLayout()) return nil;
    std::string why;
    if (!_controller->LayoutObjectReadOnly([self layoutObjectIdFor:Std(name)], why)) return nil;
    return NS(why);
}

- (NSArray<XLCollabLayoutMark*>*)layoutMarks {
    NSMutableArray* out = [NSMutableArray array];
    if (!_controller->LiveLayout() || !_controller->InSession()) return out;
    auto* ctx = _app->Ctx();
    auto mark = [&](const std::string& name, const std::string& objectId) {
        const std::string key = objectId.empty() ? name : objectId;
        NSMutableArray* cols = [NSMutableArray array];
        for (Collab::ParticipantId who : _controller->LayoutSelectedBy(key)) {
            if (auto part = _controller->FindParticipant(who); part && !part->isLocal) [cols addObject:@(PackRGB(part->colour))];
        }
        std::string holderName;
        uint32_t holderColour = 0;
        const Collab::ParticipantId holder = _controller->LayoutLockHolder(key);
        if (holder != Collab::kNoParticipant) {
            if (auto part = _controller->FindParticipant(holder); part && !part->isLocal) {
                holderName = part->name;
                holderColour = PackRGB(part->colour);
            }
        }
        if (cols.count == 0 && holderName.empty()) return;
        XLCollabLayoutMark* m = [XLCollabLayoutMark new];
        m.name = NS(name);
        m.outlineColors = cols;
        m.lockHolder = NS(holderName);
        m.lockColorRGB = holderColour;
        std::string why;
        if (!holderName.empty()) _controller->LayoutObjectReadOnly(key, why);
        m.lockText = NS(why);
        [out addObject:m];
    };
    if (ctx->HasModelManager()) {
        for (const auto& [name, model] : ctx->GetModelManager().GetModels()) {
            if (model) mark(name, model->GetObjectId());
        }
    }
    for (const auto& [name, vo] : ctx->GetAllObjects()) {
        if (vo) mark(name, vo->GetObjectId());
    }
    return out;
}

+ (void)noteLayoutFieldFocused:(BOOL)focused {
    g_layoutFieldsFocused = std::max(0, g_layoutFieldsFocused + (focused ? 1 : -1));
}

@end
