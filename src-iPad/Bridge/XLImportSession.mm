#import "XLImportSession.h"
#import "XLSequenceDocument.h"

#include "iPadRenderContext.h"

#include "import_export/AutoMapper.h"
#include "import_export/BasicImportMappingNode.h"
#include "import_export/EffectMapper.h"
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

#include <spdlog/spdlog.h>

#include <filesystem>
#include <list>
#include <map>
#include <memory>
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

// The iPad-lib target compiles without ARC, so every NSString /
// NSArray ivar set via direct `row->_field = autoreleasedValue` would
// dangle as soon as the surrounding autorelease pool drained. We
// retain on assignment and release in dealloc to keep these objects
// alive across SwiftUI's redraw cycles.

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
- (void)dealloc {
    [_displayName release];
    [_canonicalName release];
    [_modelType release];
    [super dealloc];
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
- (void)dealloc {
    [_name release];
    [super dealloc];
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
    BOOL _isGroup;
    BOOL _isSubmodel;
    NSInteger _effectCount;
    NSArray<XLImportMappingRow*>* _children;
}
- (void)dealloc {
    [_model release];
    [_strand release];
    [_node release];
    [_mapping release];
    [_mappingModelType release];
    [_children release];
    [super dealloc];
}
- (intptr_t)nodeID { return _nodeID; }
- (NSString*)model { return _model; }
- (NSString*)strand { return _strand; }
- (NSString*)node { return _node; }
- (NSString*)mapping { return _mapping; }
- (NSString*)mappingModelType { return _mappingModelType; }
- (BOOL)isGroup { return _isGroup; }
- (BOOL)isSubmodel { return _isSubmodel; }
- (NSInteger)effectCount { return _effectCount; }
- (NSArray<XLImportMappingRow*>*)children { return _children; }
@end

@implementation XLImportSession {
    // Plain pointer — the import sheet that owns this session is
    // strictly shorter-lived than the host XLSequenceDocument, so
    // there's no retain-cycle risk and no need for __weak (which the
    // iPad-lib target doesn't compile with ARC).
    XLSequenceDocument* _document;

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
}

- (instancetype)initWithDocument:(XLSequenceDocument*)document {
    self = [super init];
    if (self) {
        _document = document;
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
        [d release]; [c release]; [t release];
        [arr addObject:row];
        [row release];
    }
    return [arr autorelease];
}

- (XLImportMappingRow*)snapshotRow:(BasicImportMappingNode*)node {
    XLImportMappingRow* row = [[[XLImportMappingRow alloc] init] autorelease];
    row->_nodeID = (intptr_t)node;
    row->_model = [[NSString alloc] initWithUTF8String:node->_model.c_str()];
    row->_strand = [[NSString alloc] initWithUTF8String:node->_strand.c_str()];
    row->_node = [[NSString alloc] initWithUTF8String:node->_node.c_str()];
    row->_mapping = [[NSString alloc] initWithUTF8String:node->_mapping.c_str()];
    row->_mappingModelType = [[NSString alloc] initWithUTF8String:node->_mappingModelType.c_str()];
    row->_isGroup = node->_group ? YES : NO;
    row->_isSubmodel = node->_isSubmodel ? YES : NO;
    row->_effectCount = node->_effectCount;
    NSMutableArray* kids = [[NSMutableArray alloc] initWithCapacity:node->GetChildCount()];
    for (unsigned int i = 0; i < node->GetChildCount(); ++i) {
        BasicImportMappingNode* child = node->GetNthChild(i);
        if (child) [kids addObject:[self snapshotRow:child]];
    }
    row->_children = kids;  // owned (alloc/initWithCapacity), released in dealloc
    return row;
}

- (NSArray<XLImportMappingRow*>*)destinationRows {
    NSMutableArray* arr = [[NSMutableArray alloc] initWithCapacity:_destinationRoots.size()];
    for (const auto& root : _destinationRoots) {
        // snapshotRow returns autoreleased — array retains it on add.
        [arr addObject:[self snapshotRow:root.get()]];
    }
    return [arr autorelease];
}

- (NSArray<XLImportTimingTrack*>*)timingTracks {
    NSMutableArray* arr = [[NSMutableArray alloc] initWithCapacity:_timingTracks.size()];
    for (const auto& t : _timingTracks) {
        XLImportTimingTrack* row = [[XLImportTimingTrack alloc] init];
        row->_name = [[NSString alloc] initWithUTF8String:t.name.c_str()];
        row->_alreadyExists = t.alreadyExists ? YES : NO;
        row->_selected = t.selected ? YES : NO;
        [arr addObject:row];
        [row release];
    }
    return [arr autorelease];
}

- (void)setTimingTrackImport:(NSString*)name enabled:(BOOL)enabled {
    if (name == nil) return;
    std::string n = name.UTF8String;
    for (auto& t : _timingTracks) {
        if (t.name == n) { t.selected = enabled ? true : false; return; }
    }
}

#pragma mark - Mutations

- (BasicImportMappingNode*)findNodeByID:(intptr_t)nodeID {
    for (const auto& root : _destinationRoots) {
        if ((intptr_t)root.get() == nodeID) return root.get();
        for (unsigned int i = 0; i < root->GetChildCount(); ++i) {
            BasicImportMappingNode* c = root->GetNthChild(i);
            if ((intptr_t)c == nodeID) return c;
        }
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

#pragma mark - Apply

- (BOOL)applyImportWithEraseExisting:(BOOL)eraseExisting
                                lock:(BOOL)lock
                                error:(NSError**)outError {
    iPadRenderContext* rc = RawRenderContext(_document);
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
                              mapping, /*convertRender=*/false, mappingModelType);
        }
    }

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
                              /*convertRender=*/false, mappingModelType);
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
                                  /*convertRender=*/false, mappingModelType);
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
