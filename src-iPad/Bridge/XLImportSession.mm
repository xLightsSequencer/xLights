#import "XLImportSession.h"
#import "XLSequenceDocument.h"
#import "XLAIServices.h"

#include "iPadRenderContext.h"

#include "ai/ServiceManager.h"
#include "ai/aiBase.h"
#include "import_export/AutoMapper.h"
#include "import_export/BasicImportMappingNode.h"
#include "import_export/EffectMapper.h"
#include "import_export/HLSFile.h"
#include "import_export/LOREdit.h"
#include "import_export/LORMusic.h"
#include "import_export/LORPixelEditor.h"
#include "import_export/Vixen3.h"
#include "import_export/MapHintsIO.h"
#include "models/Model.h"
#include "models/ModelManager.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/SequenceElements.h"
#include "render/SequenceFile.h"
#include "render/SequencePackage.h"
#include "utils/string_utils.h"

#include <pugixml.hpp>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <filesystem>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <unordered_set>
#include <vector>

// XLSequenceDocument already exposes the iPadRenderContext via its
// `-(void*)renderContext` accessor (see XLSequenceDocument.mm:2365).
// We just cast it. The cast is safe because the document owns the
// iPadRenderContext as a unique_ptr<iPadRenderContext>.
static iPadRenderContext* RawRenderContext(XLSequenceDocument* doc) {
    return doc != nil ? (iPadRenderContext*)[doc renderContext] : nullptr;
}

@implementation XLImportAvailableSource {
@public
    NSString* _displayName;
    NSString* _canonicalName;
    NSString* _modelType;
}
- (instancetype)initWithDisplay:(NSString*)display
                       canonical:(NSString*)canonical
                       modelType:(NSString*)modelType {
    self = [super init];
    if (self) {
        _displayName = [display copy];
        _canonicalName = [canonical copy];
        _modelType = [modelType copy];
    }
    return self;
}
- (NSString*)displayName { return _displayName; }
- (NSString*)canonicalName { return _canonicalName; }
- (NSString*)modelType { return _modelType; }
@end

@implementation XLImportTimingTrack {
@public
    NSString* _name;
    BOOL _alreadyExists;
    BOOL _selected;
}
- (NSString*)name { return _name; }
- (BOOL)alreadyExists { return _alreadyExists; }
- (BOOL)selected { return _selected; }
@end

@implementation XLImportMappingRow {
@public
    intptr_t _nodeID;
    NSString* _model;
    NSString* _strand;
    NSString* _node;
    NSString* _mapping;
    NSString* _mappingModelType;
    NSArray<NSString*>* _stackedMappings;
    BOOL _isGroup;
    BOOL _isSubmodel;
    NSInteger _effectCount;
    NSArray<XLImportMappingRow*>* _children;
}
- (intptr_t)nodeID { return _nodeID; }
- (NSString*)model { return _model; }
- (NSString*)strand { return _strand; }
- (NSString*)node { return _node; }
- (NSString*)mapping { return _mapping; }
- (NSString*)mappingModelType { return _mappingModelType; }
- (NSArray<NSString*>*)stackedMappings { return _stackedMappings ?: @[]; }
- (BOOL)isGroup { return _isGroup; }
- (BOOL)isSubmodel { return _isSubmodel; }
- (NSInteger)effectCount { return _effectCount; }
- (NSArray<XLImportMappingRow*>*)children { return _children; }
@end

@implementation XLImportSession {
    // Unsafe-unretained: the import sheet that owns this session is
    // strictly shorter-lived than the host XLSequenceDocument, so
    // there's no retain-cycle risk and no need for __weak.
    __unsafe_unretained XLSequenceDocument* _document;

    // Source-side state (the incoming sequence). Built lazily by
    // -loadSourceSequenceAtPath:. The SequencePackage owns the temp
    // extraction directory for `.xsqz` packages — keep it alive for
    // the lifetime of the session so the inner `.xsq` path stays
    // valid through apply.
    std::unique_ptr<SequencePackage> _sourcePackage;
    std::unique_ptr<SequenceFile> _sourceFile;
    std::unique_ptr<SequenceElements> _sourceElements;
    std::vector<AvailableSource> _availableSources;

    // Source elements indexed by their flat name ("Model" or
    // "Model/Strand/Node") for fast lookup during apply. Filled
    // alongside _availableSources.
    std::map<std::string, Element*> _sourceElementMap;
    std::map<std::string, EffectLayer*> _sourceLayerMap;

    // Timing tracks from the source — name + selected flag + whether
    // the active sequence already has a timing element with the same
    // name and existing effects. Default selected = !alreadyExists.
    struct TimingTrackEntry {
        std::string name;
        TimingElement* sourceElement{ nullptr };
        bool alreadyExists{ false };
        bool selected{ false };
    };
    std::vector<TimingTrackEntry> _timingTracks;

    // Destination tree — owned root holders. Each root corresponds to
    // a top-level model on the active sequence.
    std::vector<std::unique_ptr<BasicImportMappingNode>> _destinationRoots;

    // LOR S5 `.loredit` source state. When the picked file is a
    // `.loredit` the source isn't an xLights SequenceElements tree —
    // it's an effect-level LOR document parsed by the wx-free core
    // LOREdit reader. We keep the parsed document + reader alive so the
    // discovery list (and a future apply path mirroring desktop
    // ImportS5 / MapS5*) can query it. `_loreditMode` flips the bridge
    // into this branch; the destination tree + AutoMapper / MapHints
    // flow is shared with the `.xsq` path unchanged.
    bool _loreditMode;
    pugi::xml_document _loreditDoc;
    std::unique_ptr<LOREdit> _loredit;
    // Timing-track name -> begin/end pairs, prefetched at load so the
    // (deferred) apply path doesn't need to re-walk the document.
    std::map<std::string, std::vector<std::pair<uint32_t, uint32_t>>> _loreditTimings;

    // Vixen 3 `.tim` EFFECT source state (IE-25). Like `.loredit`, the source
    // is a wx-free core reader (Vixen3) rather than an xLights tree; kept alive
    // so discovery + the apply branch can query its effects / timings.
    // `_vixen3Mode` selects the Vixen3 effect branch — distinct from the
    // timing-only `.tim` path used by Settings → Timings (XLSequenceDocument).
    bool _vixen3Mode;
    std::unique_ptr<Vixen3> _vixen3;

    // LOR Music / Animation (.lms / .las) EFFECT source state. Like .loredit /
    // .tim the source is a wx-free core reader (LORMusic) rather than an
    // xLights tree. `_lmsMode` selects the LMS apply branch. `_lmsDoc` owns the
    // parsed document the reader references for the session's lifetime.
    bool _lmsMode;
    pugi::xml_document _lmsDoc;
    std::unique_ptr<LORMusic> _lms;
    std::map<std::string, std::vector<std::pair<uint32_t, uint32_t>>> _lmsTimings;
    // CCR prop prefix names (per-pixel rgbChannels collapsed to one source row);
    // apply fans these across a model's node layers.
    std::unordered_set<std::string> _lmsCCRNames;

    // LOR Pixel Editor (.lpe) EFFECT source state. Like the other legacy effect
    // formats the source is a wx-free core reader (LORPixelEditor) over the owned
    // _lpeDoc. `_lpeMode` selects the LPE apply branch.
    bool _lpeMode;
    pugi::xml_document _lpeDoc;
    std::unique_ptr<LORPixelEditor> _lpe;

    // HLS (.hlsIdata) EFFECT source state. wx-free core reader (HLSFile) over the
    // owned _hlsDoc. `_hlsMode` selects the HLS apply branch; `_hlsCCRNames` are
    // the strand prefixes fanned across node layers at apply time.
    bool _hlsMode;
    pugi::xml_document _hlsDoc;
    std::unique_ptr<HLSFile> _hls;
    std::unordered_set<std::string> _hlsCCRNames;

    // Model-row nodeIDs whose submodel children are sorted-by-name in the
    // SwiftUI snapshot (#4636). Display-only; the live child order is intact.
    NSMutableSet<NSNumber*>* _sortedSubmodelNodeIDs;
}

- (instancetype)initWithDocument:(XLSequenceDocument*)document {
    self = [super init];
    if (self) {
        _document = document;
        _sortedSubmodelNodeIDs = [[NSMutableSet alloc] init];
        [self buildDestinationTree];
    }
    return self;
}

#pragma mark - Public counts

- (NSInteger)availableSourceCount {
    return (NSInteger)_availableSources.size();
}

- (NSInteger)destinationRootCount {
    return (NSInteger)_destinationRoots.size();
}

- (NSInteger)mappedDestinationCount {
    NSInteger n = 0;
    for (const auto& root : _destinationRoots) {
        if (root->HasMapping()) ++n;
    }
    return n;
}

#pragma mark - Source loading

- (BOOL)loadSourceSequenceAtPath:(NSString*)path
                            error:(NSError**)outError {
    iPadRenderContext* rc = RawRenderContext(_document);
    if (rc == nullptr) {
        if (outError) {
            *outError = [NSError errorWithDomain:@"XLImportSession" code:1
                          userInfo:@{ NSLocalizedDescriptionKey:
                                      @"No active sequence loaded." }];
        }
        return NO;
    }
    if (path == nil || path.length == 0) {
        if (outError) {
            *outError = [NSError errorWithDomain:@"XLImportSession" code:2
                          userInfo:@{ NSLocalizedDescriptionKey:
                                      @"No source file specified." }];
        }
        return NO;
    }

    std::string pathStr = path.UTF8String;
    std::string showDir = rc->GetShowDirectory();

    // Build a SequencePackage for both `.xsq` (loose) and `.xsqz` /
    // `.zip` / `.piz` (packaged) sources — the same shape the
    // desktop's xLightsFrame::ImportXLights uses. For packaged
    // sources Extract() unpacks to a temp dir and GetXsqFile()
    // returns the inner `.xsq`.
    auto pkg = std::make_unique<SequencePackage>(
        std::filesystem::path(pathStr), showDir, pathStr,
        &rc->GetModelManager());
    if (pkg->IsPkg()) {
        pkg->Extract();
        if (!pkg->IsValid()) {
            if (outError) {
                *outError = [NSError errorWithDomain:@"XLImportSession" code:4
                              userInfo:@{ NSLocalizedDescriptionKey:
                                          @"Source package could not be extracted — file may be corrupt or not an xLights sequence package." }];
            }
            return NO;
        }
    } else {
        pkg->FindRGBEffectsFile();
    }

    std::string xsqPath = pkg->GetXsqFile().string();
    auto file = std::make_unique<SequenceFile>(xsqPath);
    auto doc = file->Open(showDir, /*ignore_audio=*/true, xsqPath);
    if (!doc.has_value()) {
        if (outError) {
            *outError = [NSError errorWithDomain:@"XLImportSession" code:3
                          userInfo:@{ NSLocalizedDescriptionKey:
                                      @"Could not parse source sequence file." }];
        }
        return NO;
    }

    auto elements = std::make_unique<SequenceElements>(rc);
    elements->SetFrequency(rc->GetSequenceElements().GetFrequency());
    elements->LoadSequencerFile(*file, *doc, showDir, /*importMode=*/true);
    file->AdjustEffectSettingsForVersion(*elements, rc);
    pkg->SetSequenceElements(elements.get());

    _sourcePackage = std::move(pkg);
    _sourceFile = std::move(file);
    _sourceElements = std::move(elements);

    // This is an `.xsq` source — clear any prior `.loredit` / `.tim` mode so a
    // source switch within one session doesn't apply through the wrong branch.
    _loreditMode = false;
    _loredit.reset();
    _loreditTimings.clear();
    _vixen3Mode = false;
    _vixen3.reset();
    _lmsMode = false;
    _lms.reset();
    _lmsTimings.clear();
    _lmsCCRNames.clear();

    [self rebuildAvailableSources];
    [self rebuildTimingTracks];
    return YES;
}

- (void)rebuildTimingTracks {
    _timingTracks.clear();
    if (_sourceElements == nullptr) return;
    iPadRenderContext* rc = RawRenderContext(_document);
    if (rc == nullptr) return;

    SequenceElements& targetSE = rc->GetSequenceElements();
    for (size_t e = 0; e < _sourceElements->GetElementCount(); ++e) {
        Element* el = _sourceElements->GetElement(e);
        if (el == nullptr) continue;
        if (el->GetType() != ElementType::ELEMENT_TYPE_TIMING) continue;
        TimingElement* tel = dynamic_cast<TimingElement*>(el);
        if (tel == nullptr) continue;

        bool hasEffects = false;
        for (int n = 0; n < (int)tel->GetEffectLayerCount(); ++n) {
            if (tel->GetEffectLayer(n) && tel->GetEffectLayer(n)->GetEffectCount() > 0) {
                hasEffects = true;
                break;
            }
        }
        if (!hasEffects) continue;

        TimingTrackEntry entry;
        entry.name = tel->GetName();
        entry.sourceElement = tel;
        TimingElement* existing = targetSE.GetTimingElement(entry.name);
        entry.alreadyExists = (existing != nullptr) && existing->HasEffects();
        // Mirrors desktop: preselect when not already in target.
        entry.selected = !entry.alreadyExists;
        _timingTracks.push_back(entry);
    }
}

#pragma mark - LOR S5 (.loredit) source loading

- (BOOL)loadLOREditSourceAtPath:(NSString*)path
                          error:(NSError**)outError {
    iPadRenderContext* rc = RawRenderContext(_document);
    if (rc == nullptr) {
        if (outError) {
            *outError = [NSError errorWithDomain:@"XLImportSession" code:1
                          userInfo:@{ NSLocalizedDescriptionKey:
                                      @"No active sequence loaded." }];
        }
        return NO;
    }
    if (path == nil || path.length == 0) {
        if (outError) {
            *outError = [NSError errorWithDomain:@"XLImportSession" code:2
                          userInfo:@{ NSLocalizedDescriptionKey:
                                      @"No source file specified." }];
        }
        return NO;
    }

    std::string pathStr = path.UTF8String;
    // Mirrors desktop xLightsFrame::ImportS5 — load the raw LOR document
    // with pugixml and hand it to the core reader at the active
    // sequence's frequency.
    pugi::xml_document doc;
    if (!doc.load_file(pathStr.c_str())) {
        if (outError) {
            *outError = [NSError errorWithDomain:@"XLImportSession" code:3
                          userInfo:@{ NSLocalizedDescriptionKey:
                                      @"Could not parse LOR S5 .loredit file." }];
        }
        return NO;
    }

    // Reset any prior `.xsq` source state — the two modes are mutually
    // exclusive within one session.
    _sourcePackage.reset();
    _sourceFile.reset();
    _sourceElements.reset();
    _sourceElementMap.clear();
    _sourceLayerMap.clear();

    _loreditDoc.reset();
    _loreditDoc = std::move(doc);
    _loredit = std::make_unique<LOREdit>(_loreditDoc, rc->GetSequenceElements().GetFrequency());
    _vixen3Mode = false;
    _vixen3.reset();
    _lmsMode = false;
    _lms.reset();
    _lmsTimings.clear();
    _lmsCCRNames.clear();
    _loreditMode = true;

    [self rebuildAvailableSourcesFromLOREdit];
    [self rebuildTimingTracksFromLOREdit];
    return YES;
}

- (void)rebuildAvailableSourcesFromLOREdit {
    _availableSources.clear();
    _sourceElementMap.clear();
    _sourceLayerMap.clear();
    if (_loredit == nullptr) return;

    auto addSource = [&](const std::string& name, const std::string& type) {
        AvailableSource src;
        src.displayName = name;
        src.canonicalName = Lower(Trim(name));
        src.modelType = type;
        _availableSources.push_back(src);
    };

    // Whole-prop (track / channel) sources — these map onto a
    // destination model and replay through the desktop MapS5* path.
    for (const auto& m : _loredit->GetModelsWithEffects()) {
        addSource(m, "Model");
    }
    // Per-node strand/channel sources (`Name[r,c,col][color]`) — these
    // map onto a single node layer (IsNodeStrandMapping == true).
    for (const auto& n : _loredit->GetNodesWithEffects()) {
        addSource(n, "Node");
    }
}

- (void)rebuildTimingTracksFromLOREdit {
    _timingTracks.clear();
    _loreditTimings.clear();
    if (_loredit == nullptr) return;
    iPadRenderContext* rc = RawRenderContext(_document);
    if (rc == nullptr) return;

    SequenceElements& targetSE = rc->GetSequenceElements();
    for (const auto& name : _loredit->GetTimingTracks()) {
        auto timings = _loredit->GetTimings(name, 0);
        if (timings.empty()) continue;
        _loreditTimings[name] = timings;

        TimingTrackEntry entry;
        entry.name = name;
        entry.sourceElement = nullptr; // synthesized on apply, not copied from a source element
        TimingElement* existing = targetSE.GetTimingElement(name);
        entry.alreadyExists = (existing != nullptr) && existing->HasEffects();
        entry.selected = !entry.alreadyExists;
        _timingTracks.push_back(entry);
    }
}

#pragma mark - Vixen 3 (.tim) effect source loading

- (BOOL)loadVixen3SourceAtPath:(NSString*)path
                         error:(NSError**)outError {
    iPadRenderContext* rc = RawRenderContext(_document);
    if (rc == nullptr) {
        if (outError) {
            *outError = [NSError errorWithDomain:@"XLImportSession" code:1
                          userInfo:@{ NSLocalizedDescriptionKey:
                                      @"No active sequence loaded." }];
        }
        return NO;
    }
    if (path == nil || path.length == 0) {
        if (outError) {
            *outError = [NSError errorWithDomain:@"XLImportSession" code:2
                          userInfo:@{ NSLocalizedDescriptionKey:
                                      @"No source file specified." }];
        }
        return NO;
    }

    std::string pathStr = path.UTF8String;
    // Mirrors desktop xLightsFrame::ImportVixen3 — the Vixen3 reader parses the
    // .tim XML and resolves the sibling SystemConfig.xml for the effect data.
    auto vixen = std::make_unique<Vixen3>(pathStr);
    if (!vixen->IsSystemFound()) {
        if (outError) {
            *outError = [NSError errorWithDomain:@"XLImportSession" code:14
                          userInfo:@{ NSLocalizedDescriptionKey:
                                      @"SystemConfig.xml could not be found next to the .tim file — Vixen 3 effect import is not possible without it." }];
        }
        return NO;
    }

    // Reset any prior `.xsq` and `.loredit` source state — the modes are
    // mutually exclusive within one session.
    _sourcePackage.reset();
    _sourceFile.reset();
    _sourceElements.reset();
    _sourceElementMap.clear();
    _sourceLayerMap.clear();
    _loreditMode = false;
    _loredit.reset();
    _loreditTimings.clear();

    _lmsMode = false;
    _lms.reset();
    _lmsTimings.clear();
    _lmsCCRNames.clear();

    _vixen3 = std::move(vixen);
    _vixen3Mode = true;

    [self rebuildAvailableSourcesFromVixen3];
    [self rebuildTimingTracksFromVixen3];
    return YES;
}

- (void)rebuildAvailableSourcesFromVixen3 {
    _availableSources.clear();
    _sourceElementMap.clear();
    _sourceLayerMap.clear();
    if (_vixen3 == nullptr) return;

    // Whole-model effect sources. A model whose first effect is a "Data"
    // marker is a timing track (handled in rebuildTimingTracksFromVixen3), not
    // an effect source — mirror desktop ImportVixen3's split.
    for (const auto& m : _vixen3->GetModelsWithEffects()) {
        auto effects = _vixen3->GetEffects(m.first);
        if (!effects.empty() && effects.front().type == "Data") continue;
        AvailableSource src;
        src.displayName = m.first;
        src.canonicalName = Lower(Trim(m.first));
        src.modelType = "Model";
        _availableSources.push_back(src);
    }
}

- (void)rebuildTimingTracksFromVixen3 {
    _timingTracks.clear();
    if (_vixen3 == nullptr) return;
    iPadRenderContext* rc = RawRenderContext(_document);
    if (rc == nullptr) return;
    SequenceElements& targetSE = rc->GetSequenceElements();

    auto addTimingTrack = [&](const std::string& name) {
        TimingTrackEntry entry;
        entry.name = name;
        entry.sourceElement = nullptr; // marks synthesized from _vixen3 on apply
        TimingElement* existing = targetSE.GetTimingElement(name);
        entry.alreadyExists = (existing != nullptr) && existing->HasEffects();
        entry.selected = !entry.alreadyExists;
        _timingTracks.push_back(entry);
    };

    // Models whose first effect is a "Data" marker are timing tracks.
    for (const auto& m : _vixen3->GetModelsWithEffects()) {
        auto effects = _vixen3->GetEffects(m.first);
        if (!effects.empty() && effects.front().type == "Data") {
            addTimingTrack(m.first);
        }
    }
    // Phrase / word / phoneme timing tracks.
    for (const auto& name : _vixen3->GetTimings()) {
        addTimingTrack(name);
    }
}

#pragma mark - LOR Music / Animation (.lms / .las) effect source loading

- (BOOL)loadLMSSourceAtPath:(NSString*)path
                      error:(NSError**)outError {
    iPadRenderContext* rc = RawRenderContext(_document);
    if (rc == nullptr) {
        if (outError) {
            *outError = [NSError errorWithDomain:@"XLImportSession" code:1
                          userInfo:@{ NSLocalizedDescriptionKey:
                                      @"No active sequence loaded." }];
        }
        return NO;
    }
    if (path == nil || path.length == 0) {
        if (outError) {
            *outError = [NSError errorWithDomain:@"XLImportSession" code:2
                          userInfo:@{ NSLocalizedDescriptionKey:
                                      @"No source file specified." }];
        }
        return NO;
    }

    std::string pathStr = path.UTF8String;
    // Mirrors desktop xLightsFrame::ImportLMS — load the raw LOR document with
    // pugixml and hand it to the core reader at the active sequence's frequency.
    pugi::xml_document doc;
    if (!doc.load_file(pathStr.c_str())) {
        if (outError) {
            *outError = [NSError errorWithDomain:@"XLImportSession" code:3
                          userInfo:@{ NSLocalizedDescriptionKey:
                                      @"Could not parse LOR .lms / .las file." }];
        }
        return NO;
    }

    // Reset any prior `.xsq` / `.loredit` / `.tim` source state — modes are
    // mutually exclusive within one session.
    _sourcePackage.reset();
    _sourceFile.reset();
    _sourceElements.reset();
    _sourceElementMap.clear();
    _sourceLayerMap.clear();
    _loreditMode = false;
    _loredit.reset();
    _loreditTimings.clear();
    _vixen3Mode = false;
    _vixen3.reset();

    _lmsDoc.reset();
    _lmsDoc = std::move(doc);
    _lms = std::make_unique<LORMusic>(_lmsDoc, rc->GetSequenceElements().GetFrequency());
    _lmsMode = true;

    [self rebuildAvailableSourcesFromLMS];
    [self rebuildTimingTracksFromLMS];
    return YES;
}

- (void)rebuildAvailableSourcesFromLMS {
    _availableSources.clear();
    _sourceElementMap.clear();
    _sourceLayerMap.clear();
    _lmsCCRNames.clear();
    if (_lms == nullptr) return;

    auto addSource = [&](const std::string& name, const std::string& type) {
        AvailableSource src;
        src.displayName = name;
        src.canonicalName = Lower(Trim(name));
        src.modelType = type;
        _availableSources.push_back(src);
    };

    for (const auto& n : _lms->GetChannelNames()) {
        addSource(n, "Channel");
    }
    for (const auto& c : _lms->GetCCRNames()) {
        _lmsCCRNames.insert(c);
        addSource(c, "Model");
    }
}

- (void)rebuildTimingTracksFromLMS {
    _timingTracks.clear();
    _lmsTimings.clear();
    if (_lms == nullptr) return;
    iPadRenderContext* rc = RawRenderContext(_document);
    if (rc == nullptr) return;

    SequenceElements& targetSE = rc->GetSequenceElements();
    for (const auto& name : _lms->GetTimingTracks()) {
        auto timings = _lms->GetTimings(name, 0);
        if (timings.empty()) continue;
        _lmsTimings[name] = timings;

        TimingTrackEntry entry;
        entry.name = name;
        entry.sourceElement = nullptr; // synthesized on apply from _lmsTimings
        TimingElement* existing = targetSE.GetTimingElement(name);
        entry.alreadyExists = (existing != nullptr) && existing->HasEffects();
        entry.selected = !entry.alreadyExists;
        _timingTracks.push_back(entry);
    }
}

- (BOOL)loadLPESourceAtPath:(NSString*)path
                      error:(NSError**)outError {
    iPadRenderContext* rc = RawRenderContext(_document);
    if (rc == nullptr) {
        if (outError) {
            *outError = [NSError errorWithDomain:@"XLImportSession" code:1
                          userInfo:@{ NSLocalizedDescriptionKey:
                                      @"No active sequence loaded." }];
        }
        return NO;
    }
    if (path == nil || path.length == 0) {
        if (outError) {
            *outError = [NSError errorWithDomain:@"XLImportSession" code:2
                          userInfo:@{ NSLocalizedDescriptionKey:
                                      @"No source file specified." }];
        }
        return NO;
    }

    std::string pathStr = path.UTF8String;
    pugi::xml_document doc;
    if (!doc.load_file(pathStr.c_str())) {
        if (outError) {
            *outError = [NSError errorWithDomain:@"XLImportSession" code:3
                          userInfo:@{ NSLocalizedDescriptionKey:
                                      @"Could not parse LOR Pixel Editor .lpe file." }];
        }
        return NO;
    }

    _sourcePackage.reset();
    _sourceFile.reset();
    _sourceElements.reset();
    _sourceElementMap.clear();
    _sourceLayerMap.clear();
    _loreditMode = false;
    _loredit.reset();
    _loreditTimings.clear();
    _vixen3Mode = false;
    _vixen3.reset();
    _lmsMode = false;
    _lms.reset();
    _lmsTimings.clear();
    _lmsCCRNames.clear();
    _hlsMode = false;
    _hls.reset();
    _hlsCCRNames.clear();

    _lpeDoc.reset();
    _lpeDoc = std::move(doc);
    _lpe = std::make_unique<LORPixelEditor>(_lpeDoc, rc->GetSequenceElements().GetFrequency());
    _lpeMode = true;

    [self rebuildAvailableSourcesFromLPE];
    _timingTracks.clear();
    return YES;
}

- (void)rebuildAvailableSourcesFromLPE {
    _availableSources.clear();
    _sourceElementMap.clear();
    _sourceLayerMap.clear();
    if (_lpe == nullptr) return;

    for (const auto& n : _lpe->GetChannelNames()) {
        AvailableSource src;
        src.displayName = n;
        src.canonicalName = Lower(Trim(n));
        src.modelType = "Model";
        _availableSources.push_back(src);
    }
}

- (BOOL)loadHLSSourceAtPath:(NSString*)path
                      error:(NSError**)outError {
    iPadRenderContext* rc = RawRenderContext(_document);
    if (rc == nullptr) {
        if (outError) {
            *outError = [NSError errorWithDomain:@"XLImportSession" code:1
                          userInfo:@{ NSLocalizedDescriptionKey:
                                      @"No active sequence loaded." }];
        }
        return NO;
    }
    if (path == nil || path.length == 0) {
        if (outError) {
            *outError = [NSError errorWithDomain:@"XLImportSession" code:2
                          userInfo:@{ NSLocalizedDescriptionKey:
                                      @"No source file specified." }];
        }
        return NO;
    }

    std::string pathStr = path.UTF8String;
    pugi::xml_document doc;
    if (!doc.load_file(pathStr.c_str())) {
        if (outError) {
            *outError = [NSError errorWithDomain:@"XLImportSession" code:3
                          userInfo:@{ NSLocalizedDescriptionKey:
                                      @"Could not parse HLS .hlsIdata file." }];
        }
        return NO;
    }

    _sourcePackage.reset();
    _sourceFile.reset();
    _sourceElements.reset();
    _sourceElementMap.clear();
    _sourceLayerMap.clear();
    _loreditMode = false;
    _loredit.reset();
    _loreditTimings.clear();
    _vixen3Mode = false;
    _vixen3.reset();
    _lmsMode = false;
    _lms.reset();
    _lmsTimings.clear();
    _lmsCCRNames.clear();
    _lpeMode = false;
    _lpe.reset();

    _hlsDoc.reset();
    _hlsDoc = std::move(doc);
    _hls = std::make_unique<HLSFile>(_hlsDoc);
    _hlsMode = true;

    [self rebuildAvailableSourcesFromHLS];
    _timingTracks.clear();
    return YES;
}

- (void)rebuildAvailableSourcesFromHLS {
    _availableSources.clear();
    _sourceElementMap.clear();
    _sourceLayerMap.clear();
    _hlsCCRNames.clear();
    if (_hls == nullptr) return;

    auto addSource = [&](const std::string& name, const std::string& type) {
        AvailableSource src;
        src.displayName = name;
        src.canonicalName = Lower(Trim(name));
        src.modelType = type;
        _availableSources.push_back(src);
    };

    for (const auto& n : _hls->GetChannelNames()) {
        addSource(n, "Channel");
    }
    for (const auto& c : _hls->GetCCRNames()) {
        _hlsCCRNames.insert(c);
        addSource(c, "Model");
    }
}

#pragma mark - Tree builders

- (void)buildDestinationTree {
    _destinationRoots.clear();

    iPadRenderContext* rc = RawRenderContext(_document);
    if (rc == nullptr) return;

    SequenceElements& se = rc->GetSequenceElements();
    for (size_t i = 0; i < se.GetElementCount(); ++i) {
        Element* el = se.GetElement(i);
        if (el == nullptr) continue;
        if (el->GetType() != ElementType::ELEMENT_TYPE_MODEL) continue;

        Model* m = rc->GetModel(el->GetName());
        std::list<std::string> aliases;
        std::string modelType;
        bool isGroup = false;
        if (m != nullptr) {
            const auto& a = m->GetAliases();
            aliases.assign(a.begin(), a.end());
            modelType = m->GetDisplayAsString();
            isGroup = (modelType == "ModelGroup");
        }
        auto node = std::make_unique<BasicImportMappingNode>(
            el->GetName(), "", "", isGroup, aliases,
            modelType, false);
        // Per-layer effect totals — same convention as desktop AddModel
        // (sums across layers).
        int totalEffects = 0;
        for (size_t l = 0; l < el->GetEffectLayerCount(); ++l) {
            EffectLayer* layer = el->GetEffectLayer(l);
            if (layer) totalEffects += layer->GetEffectCount();
        }
        node->_effectCount = totalEffects;
        if (m != nullptr) node->_strandCount = m->GetNumStrands();

        // Submodels — child nodes flagged isSubmodel=true. Source aliases
        // come from each submodel itself so AutoMapper can match a
        // submodel-level vendor name against the user's submodel
        // aliases.
        if (m != nullptr) {
            ModelElement* modelEl = dynamic_cast<ModelElement*>(el);
            for (int s = 0; s < m->GetNumSubModels(); ++s) {
                Model* sm = m->GetSubModel(s);
                if (sm == nullptr) continue;
                std::list<std::string> smAliases;
                const auto& sa = sm->GetAliases();
                smAliases.assign(sa.begin(), sa.end());
                int smEffects = 0;
                if (modelEl) {
                    SubModelElement* sme = modelEl->GetSubModel(s);
                    if (sme) {
                        for (auto* layer : sme->GetEffectLayers()) {
                            if (layer) smEffects += layer->GetEffectCount();
                        }
                    }
                }
                auto child = std::make_unique<BasicImportMappingNode>(
                    el->GetName(), sm->GetName(), "", false,
                    std::move(smAliases),
                    sm->GetDisplayAsString(),
                    /*isSubmodel=*/true);
                child->_effectCount = smEffects;
                node->AddChild(std::move(child));
            }

            // Strands (and per-strand nodes). Strand entries get the
            // parent model's aliases — desktop convention so a strand
            // alias like "Roof Outline/Strand 1" routes via the model
            // alias.
            for (int s = 0; s < m->GetNumStrands(); ++s) {
                std::string strandName = m->GetStrandName(s, /*def=*/true);
                auto strandChild = std::make_unique<BasicImportMappingNode>(
                    el->GetName(), strandName, "", false,
                    aliases,
                    modelType,
                    /*isSubmodel=*/false);
                strandChild->_strandCount = 0;

                // Per-strand nodes
                int strandLen = m->GetStrandLength(s);
                for (int n = 0; n < strandLen; ++n) {
                    std::string nodeName = m->GetNodeName(n, /*def=*/true);
                    auto nodeChild = std::make_unique<BasicImportMappingNode>(
                        el->GetName(), strandName, nodeName, false,
                        aliases,
                        modelType,
                        /*isSubmodel=*/false);
                    strandChild->AddChild(std::move(nodeChild));
                }
                node->AddChild(std::move(strandChild));
            }
        }
        _destinationRoots.push_back(std::move(node));
    }
}

- (void)rebuildAvailableSources {
    _availableSources.clear();
    _sourceElementMap.clear();
    _sourceLayerMap.clear();
    if (_sourceElements == nullptr) return;

    // Flat list of source entries (model + submodel/strand + node) using
    // slash-delimited names — same shape the desktop's
    // ImportXLights/AddChannel pass produces. AutoMapper splits on '/'
    // to descend into the destination tree.
    auto addSource = [&](const std::string& name, const std::string& type) {
        AvailableSource src;
        src.displayName = name;
        src.canonicalName = Lower(Trim(name));
        src.modelType = type;
        _availableSources.push_back(src);
    };

    for (size_t e = 0; e < _sourceElements->GetElementCount(); ++e) {
        Element* el = _sourceElements->GetElement(e);
        if (el == nullptr) continue;
        if (el->GetType() != ElementType::ELEMENT_TYPE_MODEL) continue;

        ModelElement* modelEl = dynamic_cast<ModelElement*>(el);
        bool hasEffects = false;
        for (size_t l = 0; l < el->GetEffectLayerCount(); ++l) {
            if (el->GetEffectLayer(l)->GetEffectCount() > 0) {
                hasEffects = true;
                break;
            }
        }
        if (hasEffects) {
            addSource(el->GetName(), "Model");
            _sourceElementMap[el->GetName()] = el;
        }

        if (modelEl == nullptr) continue;

        int strandCounter = 0;
        for (int sm = 0; sm < modelEl->GetSubModelAndStrandCount(); ++sm) {
            SubModelElement* sme = modelEl->GetSubModel(sm);
            if (sme == nullptr) continue;
            StrandElement* ste = dynamic_cast<StrandElement*>(sme);
            std::string smName = sme->GetName();
            if (ste != nullptr) {
                ++strandCounter;
                if (smName.empty()) smName = "Strand " + std::to_string(strandCounter);
            }
            std::string flatName = el->GetName() + "/" + smName;
            if (sme->HasEffects()) {
                addSource(flatName, ste != nullptr ? "Strand" : "SubModel");
                _sourceElementMap[flatName] = sme;
            }
            if (ste != nullptr) {
                for (int n = 0; n < ste->GetNodeLayerCount(); ++n) {
                    NodeLayer* nl = ste->GetNodeLayer(n, true);
                    if (nl == nullptr) continue;
                    if (nl->GetEffectCount() == 0) continue;
                    std::string nodeName = nl->GetNodeName();
                    if (nodeName.empty()) nodeName = "Node " + std::to_string(n + 1);
                    std::string nodeFlat = flatName + "/" + nodeName;
                    addSource(nodeFlat, "Node");
                    _sourceLayerMap[nodeFlat] = nl;
                }
            }
        }
    }
}

#pragma mark - Snapshots for SwiftUI

- (NSArray<XLImportAvailableSource*>*)availableSources {
    NSMutableArray* arr = [[NSMutableArray alloc] initWithCapacity:_availableSources.size()];
    for (const auto& src : _availableSources) {
        NSString* d = [[NSString alloc] initWithUTF8String:src.displayName.c_str()];
        NSString* c = [[NSString alloc] initWithUTF8String:src.canonicalName.c_str()];
        NSString* t = [[NSString alloc] initWithUTF8String:src.modelType.c_str()];
        XLImportAvailableSource* row = [[XLImportAvailableSource alloc]
            initWithDisplay:d canonical:c modelType:t];
        [arr addObject:row];
    }
    return arr;
}

- (XLImportMappingRow*)snapshotRow:(BasicImportMappingNode*)node {
    XLImportMappingRow* row = [[XLImportMappingRow alloc] init];
    row->_nodeID = (intptr_t)node;
    row->_model = [[NSString alloc] initWithUTF8String:node->_model.c_str()];
    row->_strand = [[NSString alloc] initWithUTF8String:node->_strand.c_str()];
    row->_node = [[NSString alloc] initWithUTF8String:node->_node.c_str()];
    row->_mapping = [[NSString alloc] initWithUTF8String:node->_mapping.c_str()];
    row->_mappingModelType = [[NSString alloc] initWithUTF8String:node->_mappingModelType.c_str()];
    if (!node->_stackedMappings.empty()) {
        NSMutableArray* stacked = [[NSMutableArray alloc] initWithCapacity:node->_stackedMappings.size()];
        for (const auto& s : node->_stackedMappings) {
            [stacked addObject:[[NSString alloc] initWithUTF8String:s.c_str()]];
        }
        row->_stackedMappings = stacked;
    }
    row->_isGroup = node->_group ? YES : NO;
    row->_isSubmodel = node->_isSubmodel ? YES : NO;
    row->_effectCount = node->_effectCount;
    NSMutableArray* kids = [[NSMutableArray alloc] initWithCapacity:node->GetChildCount()];
    for (unsigned int i = 0; i < node->GetChildCount(); ++i) {
        BasicImportMappingNode* child = node->GetNthChild(i);
        if (child) [kids addObject:[self snapshotRow:child]];
    }
    if ([_sortedSubmodelNodeIDs containsObject:@(row->_nodeID)] && kids.count > 1) {
        // Display-only submodel sort (#4636): submodel children first, ordered
        // by name; strands keep their original order after.
        NSArray* submodels = [[kids filteredArrayUsingPredicate:
            [NSPredicate predicateWithBlock:^BOOL(XLImportMappingRow* r, NSDictionary* b) { return r->_isSubmodel; }]]
            sortedArrayUsingComparator:^NSComparisonResult(XLImportMappingRow* a, XLImportMappingRow* b) {
                return [a->_strand caseInsensitiveCompare:b->_strand];
            }];
        NSArray* strands = [kids filteredArrayUsingPredicate:
            [NSPredicate predicateWithBlock:^BOOL(XLImportMappingRow* r, NSDictionary* b) { return !r->_isSubmodel; }]];
        kids = [[submodels arrayByAddingObjectsFromArray:strands] mutableCopy];
    }
    row->_children = kids;
    return row;
}

- (NSArray<XLImportMappingRow*>*)destinationRows {
    NSMutableArray* arr = [[NSMutableArray alloc] initWithCapacity:_destinationRoots.size()];
    for (const auto& root : _destinationRoots) {
        [arr addObject:[self snapshotRow:root.get()]];
    }
    return arr;
}

- (NSArray<XLImportTimingTrack*>*)timingTracks {
    NSMutableArray* arr = [[NSMutableArray alloc] initWithCapacity:_timingTracks.size()];
    for (const auto& t : _timingTracks) {
        XLImportTimingTrack* row = [[XLImportTimingTrack alloc] init];
        row->_name = [[NSString alloc] initWithUTF8String:t.name.c_str()];
        row->_alreadyExists = t.alreadyExists ? YES : NO;
        row->_selected = t.selected ? YES : NO;
        [arr addObject:row];
    }
    return arr;
}

- (void)setTimingTrackImport:(NSString*)name enabled:(BOOL)enabled {
    if (name == nil) return;
    std::string n = name.UTF8String;
    for (auto& t : _timingTracks) {
        if (t.name == n) { t.selected = enabled ? true : false; return; }
    }
}

#pragma mark - Mutations

static BasicImportMappingNode* FindNodeByIDRecursive(BasicImportMappingNode* n, intptr_t nodeID) {
    if (n == nullptr) return nullptr;
    if ((intptr_t)n == nodeID) return n;
    for (unsigned int i = 0; i < n->GetChildCount(); ++i) {
        BasicImportMappingNode* found = FindNodeByIDRecursive(n->GetNthChild(i), nodeID);
        if (found) return found;
    }
    return nullptr;
}

- (BasicImportMappingNode*)findNodeByID:(intptr_t)nodeID {
    for (const auto& root : _destinationRoots) {
        BasicImportMappingNode* found = FindNodeByIDRecursive(root.get(), nodeID);
        if (found) return found;
    }
    return nullptr;
}

- (void)setMappingForRow:(intptr_t)nodeID
            sourceDisplayName:(nullable NSString*)sourceDisplayName
                    modelType:(nullable NSString*)modelType {
    BasicImportMappingNode* node = [self findNodeByID:nodeID];
    if (node == nullptr) return;

    if (sourceDisplayName == nil || sourceDisplayName.length == 0) {
        node->ClearMapping();
        return;
    }
    std::string mt = modelType ? std::string(modelType.UTF8String) : std::string("Model");
    node->Map(sourceDisplayName.UTF8String, mt);
}

- (void)addStackedMappingForRow:(intptr_t)nodeID
              sourceDisplayName:(NSString*)sourceDisplayName
                      modelType:(nullable NSString*)modelType {
    if (sourceDisplayName == nil || sourceDisplayName.length == 0) return;
    BasicImportMappingNode* node = [self findNodeByID:nodeID];
    if (node == nullptr) return;
    std::string mt = modelType ? std::string(modelType.UTF8String) : std::string("Model");
    if (node->_mapping.empty()) {
        // Nothing to stack onto yet — behave like a plain map.
        node->Map(sourceDisplayName.UTF8String, mt);
        return;
    }
    node->AddStackedMapping(sourceDisplayName.UTF8String, mt);
}

- (BOOL)sortSubmodelsForRow:(intptr_t)nodeID {
    BasicImportMappingNode* node = [self findNodeByID:nodeID];
    if (node == nullptr) return NO;
    // Display-only toggle, mirroring desktop's wxDataView Compare override:
    // the underlying child order (which the apply path routes through via
    // ModelElement::GetSubModel(index)) is left untouched; only the SwiftUI
    // snapshot reorders the submodel children alphabetically.
    int submodelCount = 0;
    for (unsigned int i = 0; i < node->GetChildCount(); ++i) {
        if (node->GetNthChild(i)->_isSubmodel) ++submodelCount;
    }
    if (submodelCount < 2) return NO;
    [_sortedSubmodelNodeIDs containsObject:@(nodeID)]
        ? [_sortedSubmodelNodeIDs removeObject:@(nodeID)]
        : [_sortedSubmodelNodeIDs addObject:@(nodeID)];
    return YES;
}

- (void)clearAllMappings {
    for (const auto& root : _destinationRoots) {
        root->ClearMapping();
    }
}

- (void)rebuildDestinationTree {
    // Snapshot the current mappings keyed by (model, strand, node) so we can
    // re-apply them to surviving rows after the tree is rebuilt from the
    // (possibly enlarged) active sequence.
    struct SavedMapping {
        std::string mapping;
        std::string mappingModelType;
        std::vector<std::string> stacked;
        std::vector<std::string> stackedTypes;
    };
    std::map<std::string, SavedMapping> saved;
    std::function<void(BasicImportMappingNode*)> collect = [&](BasicImportMappingNode* n) {
        if (n == nullptr) return;
        if (!n->_mapping.empty() || !n->_stackedMappings.empty()) {
            saved[n->GetModelName()] = { n->_mapping, n->_mappingModelType,
                                         n->_stackedMappings, n->_stackedMappingModelTypes };
        }
        for (unsigned int i = 0; i < n->GetChildCount(); ++i) collect(n->GetNthChild(i));
    };
    for (const auto& root : _destinationRoots) collect(root.get());

    // nodeIDs are raw pointers; the rebuild invalidates every old one, so any
    // submodel-sort flags keyed on them no longer apply.
    [_sortedSubmodelNodeIDs removeAllObjects];
    [self buildDestinationTree];

    std::function<void(BasicImportMappingNode*)> restore = [&](BasicImportMappingNode* n) {
        if (n == nullptr) return;
        auto it = saved.find(n->GetModelName());
        if (it != saved.end()) {
            if (!it->second.mapping.empty()) n->Map(it->second.mapping, it->second.mappingModelType);
            for (size_t i = 0; i < it->second.stacked.size(); ++i) {
                n->AddStackedMapping(it->second.stacked[i],
                                     i < it->second.stackedTypes.size() ? it->second.stackedTypes[i] : std::string("Model"));
            }
        }
        for (unsigned int i = 0; i < n->GetChildCount(); ++i) restore(n->GetNthChild(i));
    };
    for (const auto& root : _destinationRoots) restore(root.get());
}

- (void)runAutoMap {
    iPadRenderContext* rc = RawRenderContext(_document);
    if (rc == nullptr) return;

    std::vector<ImportMappingNode*> roots;
    roots.reserve(_destinationRoots.size());
    for (const auto& r : _destinationRoots) roots.push_back(r.get());

    std::unordered_set<const ImportMappingNode*> noSelection;

    AutoMapper::Run(roots, _availableSources, *rc,
                    AutoMapper::MatchNorm, AutoMapper::MatchNorm, AutoMapper::MatchNorm,
                    "", "", "B", /*selectOnly=*/false, noSelection);
    AutoMapper::Run(roots, _availableSources, *rc,
                    AutoMapper::MatchAggressive, AutoMapper::MatchAggressive, AutoMapper::MatchAggressive,
                    "", "", "B", /*selectOnly=*/false, noSelection);
    AutoMapper::RunSubModelFallback(roots, _availableSources, *rc, /*selectOnly=*/false, noSelection);

    // .xmaphint regex pass — pulls every hint file under
    // <showdir>/maphints/*.xmaphint and runs MatchRegex for each.
    auto hints = LoadMapHintsFromShowDir(rc->GetShowDirectory());
    for (const auto& h : hints) {
        AutoMapper::Run(roots, _availableSources, *rc,
                        AutoMapper::MatchRegex, AutoMapper::MatchRegex, AutoMapper::MatchNorm,
                        h.toRegex, h.fromModel, h.applyTo,
                        /*selectOnly=*/false, noSelection);
    }
}

- (void)runAIMapWithCompletion:(void (^)(NSInteger, NSString* _Nullable))completion {
    ServiceManager* mgr = [[XLAIServices shared] serviceManager];
    aiBase* ai = mgr ? mgr->findService(aiType::MAPPING) : nullptr;
    if (ai == nullptr) {
        if (completion) completion(0, @"No AI service with model-mapping support is enabled.");
        return;
    }

    // Source side: the iPad's AvailableSource list carries less
    // structure than desktop's ImportChannel (names + model type);
    // names dominate the mapping signal so that's acceptable.
    std::vector<aiBase::MappingModelInfo> sourceInfo;
    std::set<std::string> validSources;
    std::map<std::string, std::string> sourceTypes;
    sourceInfo.reserve(_availableSources.size());
    for (const auto& s : _availableSources) {
        aiBase::MappingModelInfo info;
        info.name = s.displayName;
        info.type = s.modelType;
        info.isSubModel = s.displayName.find('/') != std::string::npos;
        sourceInfo.push_back(std::move(info));
        validSources.insert(s.displayName);
        sourceTypes[s.displayName] = s.modelType;
    }

    // Target side mirrors desktop DoStructuredAIMapping: unmapped rows
    // become targets, mapped rows feed existingMappings as examples.
    std::vector<aiBase::MappingModelInfo> targetInfo;
    std::map<std::string, std::string> existingMappings;
    std::vector<BasicImportMappingNode*> targets;
    auto addTarget = [&](BasicImportMappingNode* n) {
        std::string name = n->GetModelName();
        if (!n->_mapping.empty()) {
            existingMappings[name] = n->_mapping;
            return;
        }
        aiBase::MappingModelInfo info;
        info.name = name;
        info.type = n->_isSubmodel ? std::string("SubModel") : n->_modelType;
        info.isSubModel = n->_isSubmodel;
        info.modelClass = n->_modelClass;
        info.groupModels = n->_groupModels;
        info.nodeCount = n->_nodeCount;
        info.strandCount = n->_strandCount;
        info.effectCount = n->_effectCount;
        info.aliases.assign(n->_aliases.begin(), n->_aliases.end());
        for (unsigned int c = 0; c < n->GetChildCount(); ++c) {
            auto* child = n->GetNthChild(c);
            if (child != nullptr && child->_isSubmodel) {
                info.subModelNames.push_back(child->_strand);
            }
        }
        targetInfo.push_back(std::move(info));
        targets.push_back(n);
    };
    for (const auto& root : _destinationRoots) {
        addTarget(root.get());
        for (unsigned int i = 0; i < root->GetChildCount(); ++i) {
            if (auto* child = root->GetNthChild(i)) addTarget(child);
        }
    }
    if (targetInfo.empty()) {
        if (completion) completion(0, nil);
        return;
    }

    XLImportSession* strongSelf = self;
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0), ^{
        auto result = ai->GenerateModelMapping(sourceInfo, targetInfo, existingMappings);
        dispatch_async(dispatch_get_main_queue(), ^{
            (void)strongSelf;
            if (!result.error.empty()) {
                if (completion) completion(0, [NSString stringWithUTF8String:result.error.c_str()]);
                return;
            }
            NSInteger applied = 0;
            for (BasicImportMappingNode* node : targets) {
                if (!node->_mapping.empty()) continue;
                auto it = result.mappings.find(node->GetModelName());
                if (it != result.mappings.end() && validSources.count(it->second) > 0) {
                    auto t = sourceTypes.find(it->second);
                    node->Map(it->second,
                              t != sourceTypes.end() ? t->second : std::string("Model"));
                    ++applied;
                }
            }
            if (completion) completion(applied, nil);
        });
    });
}

- (int)loadMapHintsFromPath:(NSString*)path {
    if (!path || path.length == 0) return 0;
    iPadRenderContext* rc = RawRenderContext(_document);
    if (rc == nullptr) return 0;
    std::vector<ImportMappingNode*> roots;
    roots.reserve(_destinationRoots.size());
    for (const auto& r : _destinationRoots) roots.push_back(r.get());
    std::unordered_set<const ImportMappingNode*> noSelection;
    auto hints = LoadMapHintsFile(std::string(path.UTF8String));
    for (const auto& h : hints) {
        AutoMapper::Run(roots, _availableSources, *rc,
                        AutoMapper::MatchRegex, AutoMapper::MatchRegex, AutoMapper::MatchNorm,
                        h.toRegex, h.fromModel, h.applyTo,
                        /*selectOnly=*/false, noSelection);
    }
    return (int)hints.size();
}

- (int)updateModelAliasesFromMapping {
    iPadRenderContext* rc = RawRenderContext(_document);
    if (rc == nullptr) return 0;
    int added = 0;
    // Top-level model aliases only (the clear, high-value case): alias
    // the mapped source MODEL name onto the destination model. The
    // submodel/strand alias guard the desktop applies is deferred.
    for (const auto& root : _destinationRoots) {
        if (!root->HasMapping() || root->_mapping.empty()) continue;
        Model* m = rc->GetModel(root->_model);
        if (m == nullptr) continue;
        m->AddAlias(root->_mapping);
        rc->MarkLayoutModelDirty(root->_model);
        ++added;
    }
    return added;
}

- (void)runAutoMapSelectedTargets:(NSArray<NSNumber*>*)selectedNodeIDs
                          sources:(NSArray<NSString*>*)selectedSourceDisplayNames {
    iPadRenderContext* rc = RawRenderContext(_document);
    if (rc == nullptr) return;

    std::vector<ImportMappingNode*> roots;
    roots.reserve(_destinationRoots.size());
    for (const auto& r : _destinationRoots) roots.push_back(r.get());

    std::unordered_set<const ImportMappingNode*> selectedTargets;
    for (NSNumber* n in selectedNodeIDs) {
        BasicImportMappingNode* node = [self findNodeByID:(intptr_t)n.longLongValue];
        if (node) selectedTargets.insert(node);
    }

    // Mark selected sources in a fresh copy of the available list — so
    // we don't mutate the canonical state mid-call.
    std::unordered_set<std::string> selectedSrcNames;
    for (NSString* s in selectedSourceDisplayNames) {
        selectedSrcNames.insert(s.UTF8String);
    }
    std::vector<AvailableSource> available = _availableSources;
    for (auto& src : available) {
        src.selected = selectedSrcNames.count(src.displayName) != 0;
    }

    AutoMapper::Run(roots, available, *rc,
                    AutoMapper::MatchNorm, AutoMapper::MatchNorm, AutoMapper::MatchNorm,
                    "", "", "B", /*selectOnly=*/true, selectedTargets);
    AutoMapper::Run(roots, available, *rc,
                    AutoMapper::MatchAggressive, AutoMapper::MatchAggressive, AutoMapper::MatchAggressive,
                    "", "", "B", /*selectOnly=*/true, selectedTargets);
    AutoMapper::RunSubModelFallback(roots, available, *rc, /*selectOnly=*/true, selectedTargets);
}

#pragma mark - Save hints

- (BOOL)saveMapHintsToPath:(NSString*)path {
    if (path == nil) return NO;
    std::vector<MapHintEntry> entries;
    for (const auto& root : _destinationRoots) {
        if (!root->HasMapping()) continue;
        if (!root->_mapping.empty()) {
            entries.push_back({ "^" + root->_model + "$", root->_mapping, "B" });
        }
        // Nested entries would be emitted here once submodel/node tree-
        // building lands on iPad.
    }
    return WriteMapHintsFile(path.UTF8String, entries) ? YES : NO;
}

#pragma mark - Source metadata (IE-7)

- (NSString*)sourceVersion {
    if (_sourceFile == nullptr) return nil;
    const std::string& v = _sourceFile->GetVersion();
    return v.empty() ? nil : [NSString stringWithUTF8String:v.c_str()];
}

- (NSInteger)sourceFrequency {
    return _sourceFile != nullptr ? (NSInteger)_sourceFile->GetFrequency() : 0;
}

- (NSInteger)targetFrequency {
    iPadRenderContext* rc = RawRenderContext(_document);
    return rc != nullptr ? (NSInteger)rc->GetSequenceElements().GetFrequency() : 0;
}

- (NSArray<NSString*>*)sourceMissingMedia {
    NSMutableArray<NSString*>* result = [NSMutableArray array];
    if (_sourcePackage == nullptr) return result;
    for (const std::string& m : _sourcePackage->GetMissingMedia()) {
        [result addObject:[NSString stringWithUTF8String:m.c_str()]];
    }
    return result;
}

#pragma mark - Apply

- (BOOL)applyImportWithEraseExisting:(BOOL)eraseExisting
                                lock:(BOOL)lock
                  convertRenderStyle:(BOOL)convertRenderStyle
                                error:(NSError**)outError {
    iPadRenderContext* rc = RawRenderContext(_document);
    if (_lpeMode) {
        // LOR Pixel Editor (.lpe) effect synthesis — the iPad equivalent of
        // desktop xLightsFrame::ImportLPE. Walks the same destination tree but
        // replays through the wx-free core LORPixelEditor reader, translating LPE
        // pixel effects (per left/right side + layer) into xLights effects.
        if (rc == nullptr || _lpe == nullptr) {
            if (outError) {
                *outError = [NSError errorWithDomain:@"XLImportSession" code:10
                              userInfo:@{ NSLocalizedDescriptionKey:
                                          @"Source sequence not loaded." }];
            }
            return NO;
        }
        SequenceElements& targetSE = rc->GetSequenceElements();
        bool const erase = eraseExisting ? true : false;

        for (const auto& root : _destinationRoots) {
            if (!root->HasMapping()) continue;
            Element* targetEl = targetSE.GetElement(root->_model);
            if (targetEl == nullptr) {
                spdlog::warn("XLImportSession::apply(lpe): target element '{}' missing", root->_model);
                continue;
            }
            ModelElement* targetModel = dynamic_cast<ModelElement*>(targetEl);

            if (!root->_mapping.empty()) {
                _lpe->MapPropEffects(targetEl, root->_mapping, erase);
                for (const auto& s : root->_stackedMappings) {
                    targetEl->AddEffectLayer(); // empty separator before stacked mapping
                    int startLayer = (int)targetEl->GetEffectLayerCount();
                    _lpe->MapPropEffects(targetEl, s, false, startLayer);
                }
            }

            if (targetModel == nullptr) continue;
            for (unsigned int j = 0; j < root->GetChildCount(); ++j) {
                BasicImportMappingNode* child = root->GetNthChild(j);
                if (child == nullptr) continue;
                SubModelElement* ste = targetModel->GetSubModel((int)j);
                if (ste == nullptr) continue;
                if (!child->_mapping.empty()) {
                    _lpe->MapPropEffects(ste, child->_mapping, erase);
                    for (const auto& s : child->_stackedMappings) {
                        ste->AddEffectLayer();
                        int startLayer = (int)ste->GetEffectLayerCount();
                        _lpe->MapPropEffects(ste, s, false, startLayer);
                    }
                }
                StrandElement* stre = dynamic_cast<StrandElement*>(ste);
                if (stre == nullptr) continue;
                for (unsigned int n = 0; n < child->GetChildCount(); ++n) {
                    BasicImportMappingNode* ns = child->GetNthChild(n);
                    if (ns == nullptr || ns->_mapping.empty()) continue;
                    EffectLayer* nl = stre->GetNodeLayer((int)n, true);
                    if (nl != nullptr) {
                        _lpe->MapPropNodeEffects(nl, ns->_mapping, 0, erase);
                    }
                }
            }
        }

        rc->MarkRgbEffectsChanged();
        return YES;
    }
    if (_hlsMode) {
        // HLS (.hlsIdata) effect synthesis — the iPad equivalent of desktop
        // xLightsFrame::ImportHLS. Decodes each mapped channel's per-frame colour
        // stream into On / Color Wash effects via the wx-free core HLSFile reader,
        // fanning CCR strand prefixes across node layers.
        if (rc == nullptr || _hls == nullptr) {
            if (outError) {
                *outError = [NSError errorWithDomain:@"XLImportSession" code:10
                              userInfo:@{ NSLocalizedDescriptionKey:
                                          @"Source sequence not loaded." }];
            }
            return NO;
        }
        SequenceElements& targetSE = rc->GetSequenceElements();
        bool const erase = eraseExisting ? true : false;

        for (const auto& root : _destinationRoots) {
            if (!root->HasMapping()) continue;
            Element* targetEl = targetSE.GetElement(root->_model);
            if (targetEl == nullptr) {
                spdlog::warn("XLImportSession::apply(hls): target element '{}' missing", root->_model);
                continue;
            }
            ModelElement* targetModel = dynamic_cast<ModelElement*>(targetEl);

            if (!root->_mapping.empty()) {
                if (_hlsCCRNames.count(root->_mapping) != 0 && targetModel != nullptr) {
                    int node = 0;
                    for (int str = 0; str < targetModel->GetSubModelAndStrandCount(); ++str) {
                        StrandElement* se = targetModel->GetStrand(str, true);
                        if (se == nullptr) continue;
                        for (int n = 0; n < se->GetNodeLayerCount(); ++n) {
                            EffectLayer* nl = se->GetNodeLayer(n, true);
                            std::string nm = _hls->FindStrandName(root->_mapping, node + 1);
                            _hls->MapChannelEffects(nl, nm, _hls->GetChannelColor(nm), erase);
                            ++node;
                        }
                    }
                } else {
                    _hls->MapChannelEffects(targetEl->GetEffectLayer(0), root->_mapping, _hls->GetChannelColor(root->_mapping), erase);
                }
            }

            if (targetModel == nullptr) continue;
            for (unsigned int j = 0; j < root->GetChildCount(); ++j) {
                BasicImportMappingNode* child = root->GetNthChild(j);
                if (child == nullptr || child->_mapping.empty()) continue;
                SubModelElement* ste = targetModel->GetSubModel((int)j);
                if (ste == nullptr) continue;
                if (_hlsCCRNames.count(child->_mapping) != 0) {
                    StrandElement* stre = dynamic_cast<StrandElement*>(ste);
                    if (stre == nullptr) continue;
                    int node = 0;
                    for (int n = 0; n < stre->GetNodeLayerCount(); ++n) {
                        EffectLayer* nl = stre->GetNodeLayer(n, true);
                        std::string nm = _hls->FindStrandName(child->_mapping, node + 1);
                        _hls->MapChannelEffects(nl, nm, _hls->GetChannelColor(nm), erase);
                        ++node;
                    }
                } else {
                    _hls->MapChannelEffects(ste->GetEffectLayer(0), child->_mapping, _hls->GetChannelColor(child->_mapping), erase);
                }
            }
        }

        rc->MarkRgbEffectsChanged();
        return YES;
    }
    if (_lmsMode) {
        // LOR Music / Animation (.lms / .las) effect synthesis — the iPad
        // equivalent of desktop xLightsFrame::ImportLMS. Walks the same
        // destination tree but replays through the wx-free core LORMusic reader,
        // synthesizing On / Color Wash / Twinkle effects (and CCR per-pixel
        // fan-out) rather than copying SequenceElements effects.
        if (rc == nullptr || _lms == nullptr) {
            if (outError) {
                *outError = [NSError errorWithDomain:@"XLImportSession" code:10
                              userInfo:@{ NSLocalizedDescriptionKey:
                                          @"Source sequence not loaded." }];
            }
            return NO;
        }
        SequenceElements& targetSE = rc->GetSequenceElements();
        bool const erase = eraseExisting ? true : false;

        // Selected timing tracks → synthesized timing elements (mirrors the
        // ImportLMS timing loop). Marks come from _lmsTimings.
        for (const auto& track : _timingTracks) {
            if (!track.selected) continue;
            auto itTimings = _lmsTimings.find(track.name);
            if (itTimings == _lmsTimings.end()) continue;

            TimingElement* target = static_cast<TimingElement*>(
                targetSE.AddElement(track.name, "timing", true, true, false, false, false));
            char cnt = '1';
            while (target == nullptr && cnt <= '9') {
                target = static_cast<TimingElement*>(
                    targetSE.AddElement(track.name + "-" + cnt, "timing", true, true, false, false, false));
                ++cnt;
            }
            if (target == nullptr) {
                spdlog::warn("XLImportSession::apply(lms): could not add timing element '{}'", track.name);
                continue;
            }
            if (target->GetEffectLayerCount() == 0) {
                target->AddEffectLayer();
            }
            EffectLayer* targetLayer = target->GetEffectLayer(0);
            for (const auto& t : itTimings->second) {
                targetLayer->AddEffect(0, "", "", "", t.first, t.second, false, false);
            }
        }

        for (const auto& root : _destinationRoots) {
            if (!root->HasMapping()) continue;
            Element* targetEl = targetSE.GetElement(root->_model);
            if (targetEl == nullptr) {
                spdlog::warn("XLImportSession::apply(lms): target element '{}' missing", root->_model);
                continue;
            }
            ModelElement* targetModel = dynamic_cast<ModelElement*>(targetEl);
            Model* mdl = rc->GetModel(targetEl->GetModelName());
            int const cpn = mdl != nullptr ? mdl->GetChanCountPerNode() : 1;

            if (!root->_mapping.empty()) {
                if (_lmsCCRNames.count(root->_mapping) != 0 && targetModel != nullptr && mdl != nullptr) {
                    // CCR prop: fan the prefix across every node layer of every
                    // strand (mirrors desktop MapCCR / MapCCRModel).
                    int node = 0;
                    for (int str = 0; str < mdl->GetNumStrands(); ++str) {
                        StrandElement* se = targetModel->GetStrand(str, true);
                        if (se == nullptr) continue;
                        for (int n = 0; n < se->GetNodeLayerCount(); ++n) {
                            EffectLayer* nl = se->GetNodeLayer(n, true);
                            std::string nm = _lms->ResolveCCRNodeName(root->_mapping, node);
                            _lms->MapChannelEffects(nl, nm, _lms->GetChannelColor(nm), cpn, erase);
                            ++node;
                        }
                    }
                } else {
                    _lms->MapChannelEffects(targetEl->GetEffectLayer(0), root->_mapping, _lms->GetChannelColor(root->_mapping), cpn, erase);
                }
            }

            if (targetModel == nullptr) continue;
            for (unsigned int j = 0; j < root->GetChildCount(); ++j) {
                BasicImportMappingNode* child = root->GetNthChild(j);
                if (child == nullptr || child->_mapping.empty()) continue;
                SubModelElement* ste = targetModel->GetSubModel((int)j);
                if (ste == nullptr) continue;
                if (_lmsCCRNames.count(child->_mapping) != 0) {
                    StrandElement* stre = dynamic_cast<StrandElement*>(ste);
                    if (stre == nullptr) continue;
                    int node = 0;
                    for (int n = 0; n < stre->GetNodeLayerCount(); ++n) {
                        EffectLayer* nl = stre->GetNodeLayer(n, true);
                        std::string nm = _lms->ResolveCCRNodeName(child->_mapping, node);
                        _lms->MapChannelEffects(nl, nm, _lms->GetChannelColor(nm), cpn, erase);
                        ++node;
                    }
                } else {
                    _lms->MapChannelEffects(ste->GetEffectLayer(0), child->_mapping, _lms->GetChannelColor(child->_mapping), cpn, erase);
                }
            }
        }

        rc->MarkRgbEffectsChanged();
        return YES;
    }
    if (_loreditMode) {
        // LOR S5 (.loredit) effect synthesis — the iPad equivalent of desktop
        // xLightsFrame::ImportS5. Walks the same destination tree the .xsq path
        // uses, but replays through the (now wx-free core) MapS5* family rather
        // than MapXLightsEffects, because the source is a LOREdit reader, not a
        // SequenceElements tree.
        if (rc == nullptr || _loredit == nullptr) {
            if (outError) {
                *outError = [NSError errorWithDomain:@"XLImportSession" code:10
                              userInfo:@{ NSLocalizedDescriptionKey:
                                          @"Source sequence not loaded." }];
            }
            return NO;
        }
        SequenceElements& targetSE = rc->GetSequenceElements();
        const EffectManager& effectManager = rc->GetEffectManager();
        int const frequency = targetSE.GetFrequency();
        int const offset = 0; // iPad has no time-offset control yet; desktop's TimeAdjustSpinCtrl defaults to 0
        bool const erase = eraseExisting ? true : false;

        // Selected timing tracks → synthesized timing elements (mirrors the
        // ImportS5 timing loop). In .loredit mode the entries carry no source
        // TimingElement — the marks come from _loreditTimings.
        for (const auto& track : _timingTracks) {
            if (!track.selected) continue;
            auto itTimings = _loreditTimings.find(track.name);
            if (itTimings == _loreditTimings.end()) continue;

            TimingElement* target = static_cast<TimingElement*>(
                targetSE.AddElement(track.name, "timing", true, true, false, false, false));
            char cnt = '1';
            while (target == nullptr && cnt <= '9') {
                target = static_cast<TimingElement*>(
                    targetSE.AddElement(track.name + "-" + cnt, "timing", true, true, false, false, false));
                ++cnt;
            }
            if (target == nullptr) {
                spdlog::warn("XLImportSession::apply(loredit): could not add timing element '{}'", track.name);
                continue;
            }
            if (target->GetEffectLayerCount() == 0) {
                target->AddEffectLayer();
            }
            EffectLayer* targetLayer = target->GetEffectLayer(0);
            for (const auto& t : itTimings->second) {
                targetLayer->AddEffect(0, "", "", "", t.first, t.second, false, false);
            }
        }

        // Mapped models → MapS5*. ri mirrors desktop ImportS5's model-loop
        // index `i` (passed to the per-node MapS5ChannelEffects overload).
        for (size_t ri = 0; ri < _destinationRoots.size(); ++ri) {
            const auto& root = _destinationRoots[ri];
            if (!root->HasMapping()) continue;
            Element* targetEl = targetSE.GetElement(root->_model);
            if (targetEl == nullptr) {
                spdlog::warn("XLImportSession::apply(loredit): target element '{}' missing", root->_model);
                continue;
            }
            ModelElement* targetModel = dynamic_cast<ModelElement*>(targetEl);
            Model* mdl = rc->GetModel(targetEl->GetModelName());

            if (!root->_mapping.empty()) {
                if (!LOREdit::IsNodeStrandMapping(root->_mapping)) {
                    MapS5Effects(effectManager, targetEl, *_loredit, root->_mapping, frequency, offset, erase);
                } else {
                    MapS5ChannelEffects(effectManager, targetEl->GetEffectLayer(0), *_loredit, root->_mapping, frequency, offset, erase);
                }
            }

            if (targetModel == nullptr) continue;
            for (unsigned int j = 0; j < root->GetChildCount(); ++j) {
                BasicImportMappingNode* child = root->GetNthChild(j);
                if (child == nullptr) continue;
                SubModelElement* ste = targetModel->GetSubModel((int)j);

                if (!child->_mapping.empty() && ste != nullptr) {
                    if (!LOREdit::IsNodeStrandMapping(child->_mapping)) {
                        MapS5Effects(effectManager, ste, *_loredit, child->_mapping, frequency, offset, erase);
                    } else {
                        MapS5ChannelEffects(effectManager, ste->GetEffectLayer(0), *_loredit, child->_mapping, frequency, offset, erase);
                    }
                }

                StrandElement* stre = dynamic_cast<StrandElement*>(ste);
                for (unsigned int n = 0; n < child->GetChildCount(); ++n) {
                    BasicImportMappingNode* nodeNode = child->GetNthChild(n);
                    if (nodeNode == nullptr || nodeNode->_mapping.empty()) continue;
                    if (stre == nullptr) continue;
                    NodeLayer* nl = stre->GetNodeLayer((int)n, true);
                    if (nl == nullptr) continue;
                    if (LOREdit::IsNodeStrandMapping(child->_mapping)) {
                        MapS5ChannelEffects(effectManager, nl, *_loredit, child->_mapping, frequency, offset, erase);
                    } else {
                        auto st = _loredit->GetSequencingType(nodeNode->_mapping);
                        if (st == loreditType::CHANNELS) {
                            MapS5ChannelEffects(effectManager, (int)ri, nl, mdl, *_loredit, nodeNode->_mapping, frequency, offset, erase);
                        } else if (st == loreditType::TRACKS) {
                            MapS5(effectManager, 0, nl, *_loredit, nodeNode->_mapping, mdl, frequency, offset, erase);
                        }
                    }
                }
            }
        }

        rc->MarkRgbEffectsChanged();
        return YES;
    }
    if (_vixen3Mode) {
        // Vixen 3 (.tim) effect synthesis — the iPad equivalent of desktop
        // xLightsFrame::ImportVixen3. Walks the same destination tree as the
        // .xsq / .loredit paths but replays through the (now wx-free core)
        // MapVixen3* family.
        if (rc == nullptr || _vixen3 == nullptr) {
            if (outError) {
                *outError = [NSError errorWithDomain:@"XLImportSession" code:10
                              userInfo:@{ NSLocalizedDescriptionKey:
                                          @"Source sequence not loaded." }];
            }
            return NO;
        }
        SequenceElements& targetSE = rc->GetSequenceElements();
        const EffectManager& effectManager = rc->GetEffectManager();
        int const freq = targetSE.GetFrequency();
        int const frameMS = (freq > 0) ? (1000 / freq) : 50;
        long const offset = 0; // iPad has no time-offset control; desktop's TimeAdjustSpinCtrl defaults to 0
        bool const erase = eraseExisting ? true : false;

        // Selected timing tracks (mirror desktop ImportVixen3 timing loop). The
        // marks come straight from the live _vixen3 reader.
        for (const auto& track : _timingTracks) {
            if (!track.selected) continue;

            TimingElement* target = static_cast<TimingElement*>(
                targetSE.AddElement(track.name, "timing", true, true, false, false, false));
            char cnt = '1';
            while (target == nullptr && cnt <= '9') {
                target = static_cast<TimingElement*>(
                    targetSE.AddElement(track.name + "-" + cnt, "timing", true, true, false, false, false));
                ++cnt;
            }
            if (target == nullptr) {
                spdlog::warn("XLImportSession::apply(vixen3): could not add timing element '{}'", track.name);
                continue;
            }
            if (target->GetEffectLayerCount() == 0) {
                target->AddEffectLayer();
            }

            if (_vixen3->GetTimingType(track.name) == "Phrase") {
                AddVixenMarksToLayer(_vixen3->GetTimings(track.name), target->GetEffectLayer(0), frameMS);
                EffectLayer* wordLayer = target->AddEffectLayer();
                AddVixenMarksToLayer(_vixen3->GetRelatedTiming(track.name, "Word"), wordLayer, frameMS);
                EffectLayer* phonemeLayer = target->AddEffectLayer();
                AddVixenMarksToLayer(_vixen3->GetRelatedTiming(track.name, "Phoneme"), phonemeLayer, frameMS);
            } else {
                AddVixenMarksToLayer(_vixen3->GetTimings(track.name), target->GetEffectLayer(0), frameMS);
            }
        }

        // Mapped models → MapVixen3*.
        for (const auto& root : _destinationRoots) {
            if (!root->HasMapping()) continue;
            Element* targetEl = targetSE.GetElement(root->_model);
            if (targetEl == nullptr) {
                spdlog::warn("XLImportSession::apply(vixen3): target element '{}' missing", root->_model);
                continue;
            }
            ModelElement* targetModel = dynamic_cast<ModelElement*>(targetEl);

            if (!root->_mapping.empty()) {
                MapVixen3Effects(effectManager, targetEl, *_vixen3, root->_mapping, offset, frameMS, erase);
            }

            if (targetModel == nullptr) continue;
            for (unsigned int j = 0; j < root->GetChildCount(); ++j) {
                BasicImportMappingNode* child = root->GetNthChild(j);
                if (child == nullptr) continue;
                SubModelElement* ste = targetModel->GetSubModel((int)j);

                if (!child->_mapping.empty() && ste != nullptr) {
                    MapVixen3Effects(effectManager, ste, *_vixen3, child->_mapping, offset, frameMS, erase);
                }

                StrandElement* stre = dynamic_cast<StrandElement*>(ste);
                for (unsigned int n = 0; n < child->GetChildCount(); ++n) {
                    BasicImportMappingNode* nodeNode = child->GetNthChild(n);
                    if (nodeNode == nullptr || nodeNode->_mapping.empty()) continue;
                    if (stre == nullptr) continue;
                    NodeLayer* nl = stre->GetNodeLayer((int)n, true);
                    if (nl == nullptr) continue;
                    // Desktop ImportVixen3 passes the parent MODEL element here
                    // (not the node layer) — preserve that behavior (G7).
                    MapVixen3(targetEl, *_vixen3, nodeNode->_mapping, offset, frameMS, erase);
                }
            }
        }

        rc->MarkRgbEffectsChanged();
        return YES;
    }
    if (rc == nullptr || _sourceElements == nullptr) {
        if (outError) {
            *outError = [NSError errorWithDomain:@"XLImportSession" code:10
                          userInfo:@{ NSLocalizedDescriptionKey:
                                      @"Source sequence not loaded." }];
        }
        return NO;
    }

    // Build the source-name → target-name map for Duplicate-effect
    // remapping. Same shape the desktop builds in ImportXLights.
    std::map<std::string, std::string> mapping;
    std::map<std::string, std::string> mappingModelType;
    for (const auto& root : _destinationRoots) {
        if (root->_mapping.empty()) continue;
        mapping[root->_mapping] = root->_model;
        mappingModelType[root->_mapping] = root->_mappingModelType;
    }
    bool anyTimingSelected = false;
    for (const auto& t : _timingTracks) {
        if (t.selected) { anyTimingSelected = true; break; }
    }
    if (mapping.empty() && !anyTimingSelected) {
        if (outError) {
            *outError = [NSError errorWithDomain:@"XLImportSession" code:11
                          userInfo:@{ NSLocalizedDescriptionKey:
                                      @"No mappings or timing tracks selected." }];
        }
        return NO;
    }

    // The SequencePackage built during loadSourceSequenceAtPath:
    // already wraps the source file and (for `.xsqz`) holds the
    // extracted temp dir. Reuse it so EffectMapper's media helpers
    // see the right shape.
    if (_sourcePackage == nullptr) {
        if (outError) {
            *outError = [NSError errorWithDomain:@"XLImportSession" code:12
                          userInfo:@{ NSLocalizedDescriptionKey:
                                      @"Internal error: source package not initialized." }];
        }
        return NO;
    }
    SequencePackage& xsqPkg = *_sourcePackage;

    SequenceElements& targetSE = rc->GetSequenceElements();
    // Reuse the elementMap / layerMap built during rebuildAvailableSources
    // so EffectMapper can resolve "Model/Strand[/Node]" source names
    // back to their Element* / NodeLayer*.
    std::map<std::string, Element*> elementMap = _sourceElementMap;
    std::map<std::string, EffectLayer*> layerMap = _sourceLayerMap;

    std::vector<EffectLayer*> mapped;
    bool erase = eraseExisting ? true : false;
    bool lockFlag = lock ? true : false;
    bool convertRender = convertRenderStyle ? true : false;

    // Timing-track copy. Mirrors desktop: if a target timing element with
    // the same name exists with no effects, reuse it; otherwise add a new
    // one (suffixing -1, -2, … on collision until AddElement returns
    // non-null). Layers copy through MapXLightsEffects so the embedded-
    // image / settings-fixup paths run consistently.
    for (const auto& track : _timingTracks) {
        if (!track.selected) continue;
        TimingElement* tel = track.sourceElement;
        if (tel == nullptr) continue;

        TimingElement* target = nullptr;
        TimingElement* existing = targetSE.GetTimingElement(tel->GetName());
        if (existing != nullptr &&
            existing->GetEffectLayerCount() > 0 &&
            existing->GetEffectLayer(0)->GetEffectCount() == 0) {
            target = existing;
        }
        if (target == nullptr) {
            target = static_cast<TimingElement*>(
                targetSE.AddElement(tel->GetName(), "timing", true,
                                    tel->GetCollapsed(), tel->GetActive(),
                                    false, false));
            char cnt = '1';
            while (target == nullptr && cnt <= '9') {
                target = static_cast<TimingElement*>(
                    targetSE.AddElement(tel->GetName() + "-" + cnt, "timing", true,
                                        tel->GetCollapsed(), tel->GetActive(),
                                        false, false));
                ++cnt;
            }
        }
        if (target == nullptr) {
            spdlog::warn("XLImportSession::apply: could not add timing element '{}' to target",
                         tel->GetName());
            continue;
        }
        for (int l = 0; l < (int)tel->GetEffectLayerCount(); ++l) {
            EffectLayer* src = tel->GetEffectLayer(l);
            while (l >= (int)target->GetEffectLayerCount()) {
                target->AddEffectLayer();
            }
            EffectLayer* dst = target->GetEffectLayer(l);
            std::vector<EffectLayer*> mapped2;
            MapXLightsEffects(dst, src, mapped2, erase, xsqPkg, lockFlag,
                              mapping, /*convertRender=*/convertRender, mappingModelType);
        }
    }

    // Append a stacked (merged) source onto `target` as a separator layer
    // followed by the source's layers — the iPad analogue of the desktop's
    // `_isStackDuplicate` path (ImportEffects.cpp). Stacked merges never erase
    // the target (the primary mapping already did any erase).
    auto appendStacked = [&](Element* target, const std::string& srcName) {
        EffectLayer* srcSingle = layerMap.count(srcName) ? layerMap[srcName] : nullptr;
        Element* srcEl = elementMap.count(srcName) ? elementMap[srcName] : nullptr;
        if (srcEl == nullptr) srcEl = _sourceElements->GetElement(srcName);
        if (srcSingle != nullptr) {
            target->AddEffectLayer(); // empty separator before stacked mapping
            EffectLayer* newLayer = target->AddEffectLayer();
            std::vector<EffectLayer*> m2;
            MapXLightsEffects(newLayer, srcSingle, m2, false, xsqPkg, lockFlag,
                              mapping, convertRender, mappingModelType);
        } else if (srcEl != nullptr) {
            target->AddEffectLayer(); // empty separator before stacked mapping
            for (size_t x = 0; x < srcEl->GetEffectLayerCount(); ++x) {
                EffectLayer* newLayer = target->AddEffectLayer();
                newLayer->SetLayerName(srcEl->GetEffectLayer(x)->GetLayerName());
                std::vector<EffectLayer*> m2;
                MapXLightsEffects(newLayer, srcEl->GetEffectLayer(x), m2, false,
                                  xsqPkg, lockFlag, mapping, convertRender, mappingModelType);
            }
        }
    };

    for (const auto& root : _destinationRoots) {
        if (!root->HasMapping()) continue;
        Element* targetEl = targetSE.GetElement(root->_model);
        if (targetEl == nullptr) {
            spdlog::warn("XLImportSession::apply: target element '{}' missing from active sequence",
                         root->_model);
            continue;
        }
        ModelElement* targetModel = dynamic_cast<ModelElement*>(targetEl);

        // Top-level model mapping
        if (!root->_mapping.empty()) {
            MapXLightsEffects(targetEl, root->_mapping, *_sourceElements,
                              elementMap, layerMap, mapped, erase,
                              xsqPkg, lockFlag, mapping,
                              /*convertRender=*/convertRender, mappingModelType);
            for (const auto& s : root->_stackedMappings) {
                appendStacked(targetEl, s);
            }
        }

        // Child rows — submodels then strands. Index in the bridge
        // tree matches the desktop's [submodels..., strands...] order
        // and ModelElement::GetSubModel(int) returns them by the same
        // index, so a per-child counter routes correctly.
        if (targetModel == nullptr) continue;
        for (unsigned int j = 0; j < root->GetChildCount(); ++j) {
            BasicImportMappingNode* child = root->GetNthChild(j);
            if (child == nullptr) continue;

            SubModelElement* ste = targetModel->GetSubModel((int)j);
            if (!child->_mapping.empty() && ste != nullptr) {
                MapXLightsEffects(ste, child->_mapping, *_sourceElements,
                                  elementMap, layerMap, mapped, erase,
                                  xsqPkg, lockFlag, mapping,
                                  /*convertRender=*/convertRender, mappingModelType);
                for (const auto& s : child->_stackedMappings) {
                    appendStacked(ste, s);
                }
            }

            // Per-node grandchildren — only meaningful when the child
            // is a Strand (not a SubModel). dynamic_cast filters out
            // submodel children whose tree we never populate with
            // node grandchildren anyway.
            StrandElement* stre = dynamic_cast<StrandElement*>(ste);
            for (unsigned int n = 0; n < child->GetChildCount(); ++n) {
                BasicImportMappingNode* nodeNode = child->GetNthChild(n);
                if (nodeNode == nullptr) continue;
                if (nodeNode->_mapping.empty()) continue;
                if (stre == nullptr) continue;
                NodeLayer* nl = stre->GetNodeLayer((int)n, true);
                if (nl == nullptr) continue;
                MapXLightsStrandEffects(nl, nodeNode->_mapping, layerMap,
                                        *_sourceElements, mapped, erase,
                                        xsqPkg, lockFlag, mapping,
                                        mappingModelType);
            }
        }
    }

    rc->MarkRgbEffectsChanged();
    return YES;
}

@end
