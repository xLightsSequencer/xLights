/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#import "XLSequenceDocument.h"
#import "XLGridMetalBridge.h"
#import <CoreGraphics/CoreGraphics.h>
#include "iPadRenderContext.h"

#include "render/Element.h"
#include "render/EffectLayer.h"
#include "render/Effect.h"
#include "render/SequenceElements.h"
#include "render/SequenceMedia.h"
#include "effects/RenderableEffect.h"
#include "effects/EffectManager.h"
#include "effects/ShaderEffect.h"
#include "graphics/xlGraphicsAccumulators.h"
#include "media/AudioManager.h"
#include "media/VideoReader.h"
#include "render/ValueCurve.h"
#include "models/Model.h"
#include "models/ModelManager.h"
#include "models/ModelGroup.h"
#include "utils/FileUtils.h"
#include "utils/ExternalHooks.h"
#include "utils/xlImage.h"

#include <nlohmann/json.hpp>

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>

#import <os/proc.h>

@implementation XLSequenceDocument {
    std::unique_ptr<iPadRenderContext> _context;
}

- (instancetype)init {
    self = [super init];
    if (self) {
        _context = std::make_unique<iPadRenderContext>();
    }
    return self;
}

- (BOOL)loadShowFolder:(NSString*)path {
    return _context->LoadShowFolder(std::string([path UTF8String]));
}

- (BOOL)loadShowFolder:(NSString*)path mediaFolders:(NSArray<NSString*>*)mediaFolders {
    std::list<std::string> folders;
    for (NSString* f in mediaFolders) {
        folders.push_back(std::string([f UTF8String]));
    }
    return _context->LoadShowFolder(std::string([path UTF8String]), folders);
}

+ (BOOL)obtainAccessToPath:(NSString*)path enforceWritable:(BOOL)enforceWritable {
    if (path.length == 0) return NO;
    return ObtainAccessToURL(std::string([path UTF8String]), enforceWritable) ? YES : NO;
}

// MARK: - Media relocation

- (NSString*)showFolderPath {
    if (!_context) return @"";
    const std::string& s = _context->GetShowDirectory();
    return [NSString stringWithUTF8String:s.c_str()];
}

- (NSArray<NSString*>*)mediaFolderPaths {
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    if (!_context) return out;
    for (const auto& mf : _context->GetMediaFolders()) {
        [out addObject:[NSString stringWithUTF8String:mf.c_str()]];
    }
    return out;
}

- (NSString*)moveFileToShowFolder:(NSString*)sourcePath
                        subdirectory:(NSString*)subdirectory {
    if (!_context || sourcePath.length == 0) return nil;
    std::string result = _context->MoveToShowFolder(
        std::string([sourcePath UTF8String]),
        std::string([(subdirectory ?: @"") UTF8String]),
        /*reuse*/ false);
    if (result.empty()) return nil;
    return [NSString stringWithUTF8String:result.c_str()];
}

- (NSString*)copyFileToMediaFolder:(NSString*)sourcePath
                       mediaFolderPath:(NSString*)mediaFolderPath
                        subdirectory:(NSString*)subdirectory {
    if (!_context || sourcePath.length == 0 || mediaFolderPath.length == 0) {
        return nil;
    }
    std::string result = _context->CopyToMediaFolder(
        std::string([sourcePath UTF8String]),
        std::string([mediaFolderPath UTF8String]),
        std::string([(subdirectory ?: @"") UTF8String]));
    if (result.empty()) return nil;
    return [NSString stringWithUTF8String:result.c_str()];
}

- (BOOL)pathIsInShowOrMediaFolder:(NSString*)path {
    if (!_context || path.length == 0) return NO;
    return _context->IsInShowOrMediaFolder(
        std::string([path UTF8String])) ? YES : NO;
}

- (NSString*)makeRelativePath:(NSString*)path {
    if (!_context || path.length == 0) return path ?: @"";
    std::string s = _context->MakeRelativePath(
        std::string([path UTF8String]));
    return [NSString stringWithUTF8String:s.c_str()];
}

- (BOOL)openSequence:(NSString*)path {
    return _context->OpenSequence(std::string([path UTF8String]));
}

- (void)closeSequence {
    _context->CloseSequence();
}

- (BOOL)isSequenceLoaded {
    return _context->IsSequenceLoaded();
}

- (int)sequenceDurationMS {
    auto* sf = _context->GetSequenceFile();
    return sf ? sf->GetSequenceDurationMS() : 0;
}

- (int)frameIntervalMS {
    auto* sf = _context->GetSequenceFile();
    return sf ? sf->GetFrameMS() : 50;
}

- (NSString*)sequenceName {
    auto* sf = _context->GetSequenceFile();
    if (!sf) return @"";
    std::string path = sf->GetFullPath();
    auto pos = path.rfind('/');
    std::string name = (pos != std::string::npos) ? path.substr(pos + 1) : path;
    return [NSString stringWithUTF8String:name.c_str()];
}

- (int)visibleRowCount {
    return _context->GetSequenceElements().GetRowInformationSize();
}

- (NSString*)rowDisplayNameAtIndex:(int)index {
    auto* row = _context->GetSequenceElements().GetRowInformation(index);
    if (!row) return @"";
    return [NSString stringWithUTF8String:row->displayName.c_str()];
}

- (int)rowLayerIndexAtIndex:(int)index {
    auto* row = _context->GetSequenceElements().GetRowInformation(index);
    return row ? row->layerIndex : 0;
}

- (BOOL)rowIsCollapsedAtIndex:(int)index {
    auto* row = _context->GetSequenceElements().GetRowInformation(index);
    return row ? row->Collapsed : NO;
}

- (NSString*)rowModelNameAtIndex:(int)index {
    auto* row = _context->GetSequenceElements().GetRowInformation(index);
    if (!row || !row->element) return @"";
    // Only return a name for model-backed elements. Timing tracks don't map
    // to a displayable model.
    if (row->element->GetType() != ElementType::ELEMENT_TYPE_MODEL &&
        row->element->GetType() != ElementType::ELEMENT_TYPE_SUBMODEL) {
        return @"";
    }
    return [NSString stringWithUTF8String:row->element->GetModelName().c_str()];
}

// MARK: - Timing Rows

- (NSArray<NSNumber*>*)timingRowIndices {
    NSMutableArray<NSNumber*>* out = [NSMutableArray array];
    auto& se = _context->GetSequenceElements();
    int n = se.GetRowInformationSize();
    for (int i = 0; i < n; i++) {
        auto* row = se.GetRowInformation(i);
        if (row && row->element &&
            row->element->GetType() == ElementType::ELEMENT_TYPE_TIMING) {
            [out addObject:@(i)];
        }
    }
    return out;
}

- (BOOL)timingRowIsActiveAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element ||
        row->element->GetType() != ElementType::ELEMENT_TYPE_TIMING) return NO;
    auto* te = dynamic_cast<TimingElement*>(row->element);
    return te && te->GetActive() ? YES : NO;
}

- (void)setTimingRowActive:(BOOL)active atIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element ||
        row->element->GetType() != ElementType::ELEMENT_TYPE_TIMING) return;
    auto* te = dynamic_cast<TimingElement*>(row->element);
    if (!te) return;
    // Desktop enforces single-active (RowHeading.cpp:365-371) —
    // activating one timing track clears every other. Deactivating
    // the currently-active one just flips it off.
    auto& se = _context->GetSequenceElements();
    if (active) {
        se.DeactivateAllTimingElements();
        te->SetActive(true);
    } else {
        te->SetActive(false);
    }
}

- (int)timingRowColorIndexAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row) return 0;
    return row->colorIndex;
}

- (NSString*)timingRowElementNameAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return @"";
    return [NSString stringWithUTF8String:row->element->GetName().c_str()];
}

- (NSString*)rowLayerNameAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row) return @"";
    return [NSString stringWithUTF8String:row->layerName.c_str()];
}

// MARK: - Model Row Metadata

- (BOOL)rowIsModelGroupAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return NO;
    if (row->element->GetType() != ElementType::ELEMENT_TYPE_MODEL) return NO;
    Model* m = _context->GetModelManager()[row->element->GetModelName()];
    return (m != nullptr && m->GetDisplayAs() == DisplayAsType::ModelGroup) ? YES : NO;
}

- (int)rowLayerCountAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return 0;
    return (int)row->element->GetEffectLayerCount();
}

- (BOOL)rowIsElementCollapsedAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return NO;
    return row->element->GetCollapsed() ? YES : NO;
}

- (void)toggleElementCollapsedAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return;
    row->element->SetCollapsed(!row->element->GetCollapsed());
    _context->GetSequenceElements().PopulateRowInformation();
}

- (BOOL)rowIsSubmodelAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    return (row && row->submodel) ? YES : NO;
}
- (int)rowNestDepthAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    return row ? row->nestDepth : 0;
}
- (int)rowStrandIndexAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    return row ? row->strandIndex : -1;
}
- (int)rowNodeIndexAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    return row ? row->nodeIndex : -1;
}

- (BOOL)rowHasSubmodelsAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return NO;
    // Only top-level model rows have a strand/submodel container; sub-
    // layer rows (layerIndex > 0) forward to the same ModelElement so
    // the answer is identical, but the disclosure affordance only
    // makes sense on the element's primary row.
    auto* me = dynamic_cast<ModelElement*>(row->element);
    if (!me) return NO;
    if (me->GetSubModelAndStrandCount() > 0) return YES;
    // ModelGroups disclose their member models via ShowStrands too.
    Model* m = _context->GetModelManager()[me->GetModelName()];
    if (m && m->GetDisplayAs() == DisplayAsType::ModelGroup) return YES;
    return NO;
}
- (BOOL)rowShowsSubmodelsAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return NO;
    auto* me = dynamic_cast<ModelElement*>(row->element);
    return (me && me->ShowStrands()) ? YES : NO;
}
- (void)toggleRowShowSubmodelsAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return;
    auto* me = dynamic_cast<ModelElement*>(row->element);
    if (!me) return;
    me->ShowStrands(!me->ShowStrands());
    _context->GetSequenceElements().PopulateRowInformation();
}

- (BOOL)rowHasNodesAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return NO;
    auto* se = dynamic_cast<StrandElement*>(row->element);
    return (se && se->GetNodeLayerCount() > 0) ? YES : NO;
}
- (BOOL)rowShowsNodesAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return NO;
    auto* se = dynamic_cast<StrandElement*>(row->element);
    return (se && se->ShowNodes()) ? YES : NO;
}
- (void)toggleRowShowNodesAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return;
    auto* se = dynamic_cast<StrandElement*>(row->element);
    if (!se) return;
    se->ShowNodes(!se->ShowNodes());
    _context->GetSequenceElements().PopulateRowInformation();
}

- (BOOL)insertEffectLayerAboveAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return NO;
    row->element->InsertEffectLayer(row->layerIndex);
    _context->GetSequenceElements().PopulateRowInformation();
    return YES;
}
- (BOOL)insertEffectLayerBelowAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return NO;
    int insertAt = row->layerIndex + 1;
    if (insertAt >= (int)row->element->GetEffectLayerCount()) {
        row->element->AddEffectLayer();
    } else {
        row->element->InsertEffectLayer(insertAt);
    }
    _context->GetSequenceElements().PopulateRowInformation();
    return YES;
}
- (BOOL)removeEffectLayerAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return NO;
    if (row->element->GetEffectLayerCount() <= 1) return NO;
    // Abort render before yanking a layer that may own effects the
    // renderer is walking; matches desktop's `AbortRender()` guard
    // before `RemoveEffectLayer` in `RowHeading.cpp`.
    _context->AbortRender(5000);
    row->element->RemoveEffectLayer(row->layerIndex);
    _context->GetSequenceElements().PopulateRowInformation();
    return YES;
}

- (BOOL)renameTimingTrackAtIndex:(int)rowIndex newName:(NSString*)newName {
    if (!newName || newName.length == 0) return NO;
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return NO;
    if (row->element->GetType() != ElementType::ELEMENT_TYPE_TIMING) return NO;
    std::string newStr([newName UTF8String]);
    auto& se = _context->GetSequenceElements();
    if (se.ElementExists(newStr)) return NO;
    std::string oldStr = row->element->GetName();
    if (oldStr == newStr) return YES;
    se.RenameTimingTrack(oldStr, newStr);
    row->element->SetName(newStr);
    se.PopulateRowInformation();
    return YES;
}

- (BOOL)deleteTimingTrackAtIndex:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return NO;
    if (row->element->GetType() != ElementType::ELEMENT_TYPE_TIMING) return NO;
    std::string name = row->element->GetName();
    // Some effect settings reference the timing track by name; abort
    // in-flight render before the element disappears.
    _context->AbortRender(5000);
    _context->GetSequenceElements().DeleteElement(name);
    return YES;
}

// MARK: - Views

- (NSArray<NSString*>*)availableViews {
    auto& se = _context->GetSequenceElements();
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    int n = se.GetViewCount();
    for (int i = 0; i < n; i++) {
        [out addObject:[NSString stringWithUTF8String:se.GetViewName(i).c_str()]];
    }
    return out;
}

- (int)currentViewIndex {
    return _context->GetSequenceElements().GetCurrentView();
}

- (void)setCurrentViewIndex:(int)viewIndex {
    auto& se = _context->GetSequenceElements();
    if (viewIndex < 0 || viewIndex >= se.GetViewCount()) return;

    // Mirror desktop ViewsModelsPanel::SelectView: for non-Master views we
    // must resolve the view's model list, ensure those models are in the
    // sequence, and populate the view slot in mAllViews before switching.
    // Without this, switching to any non-Master view shows an empty grid.
    std::string viewName = se.GetViewName(viewIndex);
    if (viewIndex > 0) {
        std::string modelsString = se.GetViewModels(viewName);
        se.AddMissingModelsToSequence(modelsString);
        se.PopulateView(modelsString, viewIndex);
    }
    se.SetCurrentView(viewIndex);
    se.SetTimingVisibility(viewName);
    se.PopulateRowInformation();
}

- (EffectLayer*)effectLayerForRow:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return nullptr;
    return row->element->GetEffectLayer(row->layerIndex);
}

- (int)effectCountForRow:(int)rowIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    return layer ? (int)layer->GetEffectCount() : 0;
}

- (NSArray<NSString*>*)effectNamesForRow:(int)rowIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer) return @[];

    NSMutableArray* names = [NSMutableArray array];
    for (int i = 0; i < layer->GetEffectCount(); i++) {
        Effect* e = layer->GetEffect(i);
        [names addObject:[NSString stringWithUTF8String:e->GetEffectName().c_str()]];
    }
    return names;
}

- (NSArray<NSNumber*>*)effectStartTimesForRow:(int)rowIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer) return @[];

    NSMutableArray* times = [NSMutableArray array];
    for (int i = 0; i < layer->GetEffectCount(); i++) {
        Effect* e = layer->GetEffect(i);
        [times addObject:@(e->GetStartTimeMS())];
    }
    return times;
}

- (NSArray<NSNumber*>*)effectEndTimesForRow:(int)rowIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer) return @[];

    NSMutableArray* times = [NSMutableArray array];
    for (int i = 0; i < layer->GetEffectCount(); i++) {
        Effect* e = layer->GetEffect(i);
        [times addObject:@(e->GetEndTimeMS())];
    }
    return times;
}

// MARK: - Model Preview

- (void)setModelColorsAtMS:(int)frameMS {
    _context->SetModelColors(frameMS);
}

- (void*)renderContext {
    return _context.get();
}

- (NSArray<NSString*>*)layoutGroups {
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    [out addObject:@"Default"];
    if (!_context) return out;
    for (const auto& g : _context->GetNamedLayoutGroups()) {
        [out addObject:[NSString stringWithUTF8String:g.name.c_str()]];
    }
    return out;
}

- (NSString*)activeLayoutGroup {
    if (!_context) return @"Default";
    return [NSString stringWithUTF8String:_context->GetActiveLayoutGroup().c_str()];
}

- (void)setActiveLayoutGroup:(NSString*)name {
    if (!_context) return;
    std::string s = name ? std::string([name UTF8String]) : std::string("Default");
    if (_context->GetActiveLayoutGroup() == s) return;
    _context->SetActiveLayoutGroup(s);
    // Each preview pane caches its background texture against the
    // previously-active path; broadcast so they invalidate before the
    // next draw.
    [[NSNotificationCenter defaultCenter]
        postNotificationName:@"XLLayoutGroupChanged" object:self];
}

- (BOOL)layoutMode3D {
    if (!_context) return YES;
    return _context->GetLayoutMode3D() ? YES : NO;
}

// MARK: - Effect Editing

- (BOOL)addEffectToRow:(int)rowIndex
                  name:(NSString*)effectName
               startMS:(int)startMS
                 endMS:(int)endMS {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer) return NO;

    std::string name = [effectName UTF8String];
    Effect* e = layer->AddEffect(0, name, "", "", startMS, endMS, 0, false);
    return e != nullptr;
}

- (BOOL)deleteEffectInRow:(int)rowIndex atIndex:(int)effectIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return NO;

    layer->DeleteEffectByIndex(effectIndex);
    return YES;
}

- (BOOL)moveEffectInRow:(int)rowIndex
                atIndex:(int)effectIndex
              toStartMS:(int)newStartMS
                toEndMS:(int)newEndMS {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return NO;

    Effect* e = layer->GetEffect(effectIndex);
    if (!e) return NO;

    if (newStartMS < 0) newStartMS = 0;
    if (newEndMS <= newStartMS) return NO;

    // Reject overlap with any other effect on the same layer. Effects
    // are stored in start-time order so neighbour check is enough, but
    // we walk the full layer to also defend against stale indexes.
    for (int i = 0; i < layer->GetEffectCount(); i++) {
        if (i == effectIndex) continue;
        Effect* other = layer->GetEffect(i);
        if (!other) continue;
        int os = other->GetStartTimeMS();
        int oe = other->GetEndTimeMS();
        // Overlap: ranges [newStart,newEnd] and [os,oe] intersect iff
        // newStart < oe AND newEnd > os.
        if (newStartMS < oe && newEndMS > os) {
            return NO;
        }
    }

    e->SetStartTimeMS(newStartMS);
    e->SetEndTimeMS(newEndMS);
    return YES;
}

- (NSArray<NSString*>*)availableEffectNames {
    auto& em = _context->GetEffectManager();
    NSMutableArray* names = [NSMutableArray array];
    for (size_t i = 0; i < em.size(); i++) {
        const std::string& name = em.GetEffectName((int)i);
        if (!name.empty()) {
            [names addObject:[NSString stringWithUTF8String:name.c_str()]];
        }
    }
    return names;
}

// MARK: - dynamicOptions sources

- (NSArray<NSString*>*)_timingTrackNamesWithLayerCount:(int)wantLayerCount
                                       acceptLessThan:(BOOL)acceptLessThan {
    // Mirrors JsonEffectPanel::RepopulateTimingTrackChoices. If
    // acceptLessThan is YES the filter is `layers <= wantLayerCount`
    // (the desktop "not lyric" path); otherwise exact match.
    auto& se = _context->GetSequenceElements();
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    for (int i = 0; i < se.GetElementCount(); i++) {
        Element* el = se.GetElement(i);
        if (!el || el->GetType() != ElementType::ELEMENT_TYPE_TIMING) continue;
        int layers = (int)el->GetEffectLayerCount();
        bool match = acceptLessThan ? (layers <= wantLayerCount)
                                    : (layers == wantLayerCount);
        if (!match) continue;
        [out addObject:[NSString stringWithUTF8String:el->GetName().c_str()]];
    }
    return out;
}

- (NSArray<NSString*>*)timingTrackNames {
    return [self _timingTrackNamesWithLayerCount:1 acceptLessThan:YES];
}

- (NSArray<NSString*>*)lyricTimingTrackNames {
    return [self _timingTrackNamesWithLayerCount:3 acceptLessThan:NO];
}

- (NSArray<NSString*>*)perPreviewCameraNames {
    NSMutableArray<NSString*>* out = [NSMutableArray arrayWithObject:@"2D"];
    if (!_context) return out;
    auto& vm = _context->GetViewpointMgr();
    for (int i = 0; i < vm.GetNum3DCameras(); ++i) {
        if (auto* cam = vm.GetCamera3D(i)) {
            NSString* name = [NSString stringWithUTF8String:cam->GetName().c_str()];
            if (name.length) [out addObject:name];
        }
    }
    return out;
}

- (NSDictionary<NSString*, NSNumber*>*)colorCurveModeSupportForRow:(int)rowIndex
                                                            atIndex:(int)effectIndex {
    if (!_context) return @{};
    EffectLayer* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= (int)layer->GetEffectCount()) return @{};
    Effect* effect = layer->GetEffect(effectIndex);
    if (!effect) return @{};
    RenderableEffect* fx = _context->GetEffectManager().GetEffect(effect->GetEffectName());
    if (!fx) return @{};
    const SettingsMap& settings = effect->GetSettings();
    bool linear = fx->SupportsLinearColorCurves(settings);
    bool radial = fx->SupportsRadialColorCurves(settings);
    return @{
        @"linear": @(linear),
        @"radial": @(radial),
    };
}

#pragma mark - Palette save / load / import / export

namespace {

// Strip characters that can't safely live in a filename. Desktop's
// equivalent is `RemoveNonAlphanumeric`; we match that shape so the
// filenames we produce round-trip visually with what desktop writes.
NSString* sanitisePaletteName(NSString* raw) {
    if (raw.length == 0) return @"";
    NSMutableCharacterSet* allowed = [NSMutableCharacterSet alphanumericCharacterSet];
    NSMutableString* out = [NSMutableString string];
    for (NSUInteger i = 0; i < raw.length; i++) {
        unichar c = [raw characterAtIndex:i];
        if ([allowed characterIsMember:c]) {
            [out appendFormat:@"%C", c];
        }
    }
    return out;
}

NSString* autogenPaletteName(NSString* paletteDir) {
    NSFileManager* fm = [NSFileManager defaultManager];
    int i = 1;
    while (i < 1000) {
        NSString* candidate = [NSString stringWithFormat:@"PAL%03d.xpalette", i];
        NSString* full = [paletteDir stringByAppendingPathComponent:candidate];
        if (![fm fileExistsAtPath:full]) {
            return candidate;
        }
        i++;
    }
    return @"PAL999.xpalette";
}

// Strip the trailing filename that desktop appends to each loaded
// palette entry (`<palette-string>,<filename>.xpalette`) so our
// list shows the clean palette itself. We track the filename
// separately.
NSString* trimPaletteStringSuffix(NSString* raw) {
    NSRange lastComma = [raw rangeOfString:@"," options:NSBackwardsSearch];
    if (lastComma.location == NSNotFound) return raw;
    NSString* tail = [raw substringFromIndex:lastComma.location + 1];
    if ([tail.lowercaseString hasSuffix:@".xpalette"]) {
        return [raw substringToIndex:lastComma.location + 1]; // keep trailing comma
    }
    return raw;
}

} // namespace

- (NSArray<NSDictionary<NSString*, NSString*>*>*)savedPalettes {
    NSMutableArray<NSDictionary<NSString*, NSString*>*>* out =
        [NSMutableArray array];
    NSMutableSet<NSString*>* seen = [NSMutableSet set];  // dedupe by palette string

    auto scanDir = ^(NSString* dir, BOOL recurse) {
        NSFileManager* fm = [NSFileManager defaultManager];
        BOOL isDir = NO;
        if (![fm fileExistsAtPath:dir isDirectory:&isDir] || !isDir) return;
        NSArray<NSString*>* entries = [fm contentsOfDirectoryAtPath:dir error:nil];
        for (NSString* name in entries) {
            NSString* full = [dir stringByAppendingPathComponent:name];
            BOOL sub = NO;
            [fm fileExistsAtPath:full isDirectory:&sub];
            if (sub) {
                if (recurse) {
                    NSArray* subEntries = [fm contentsOfDirectoryAtPath:full error:nil];
                    for (NSString* subName in subEntries) {
                        if (![subName.lowercaseString hasSuffix:@".xpalette"]) continue;
                        NSString* p = [full stringByAppendingPathComponent:subName];
                        NSString* content = [NSString stringWithContentsOfFile:p
                                                                      encoding:NSUTF8StringEncoding
                                                                         error:nil];
                        if (content.length == 0) continue;
                        NSString* firstLine = [[content componentsSeparatedByString:@"\n"] firstObject];
                        NSString* palette = trimPaletteStringSuffix(firstLine);
                        if (palette.length > 0 && ![seen containsObject:palette]) {
                            [seen addObject:palette];
                            [out addObject:@{@"filename": subName, @"palette": palette}];
                        }
                    }
                }
                continue;
            }
            if (![name.lowercaseString hasSuffix:@".xpalette"]) continue;
            NSString* content = [NSString stringWithContentsOfFile:full
                                                          encoding:NSUTF8StringEncoding
                                                             error:nil];
            if (content.length == 0) continue;
            NSString* firstLine = [[content componentsSeparatedByString:@"\n"] firstObject];
            NSString* palette = trimPaletteStringSuffix(firstLine);
            if (palette.length > 0 && ![seen containsObject:palette]) {
                [seen addObject:palette];
                [out addObject:@{@"filename": name, @"palette": palette}];
            }
        }
    };

    // Show-folder palettes first (user-writable), then bundled.
    NSString* show = [self showFolderPath];
    if (show.length > 0) {
        scanDir([show stringByAppendingPathComponent:@"Palettes"], YES);
    }
    NSString* bundled = [[NSBundle mainBundle] pathForResource:@"palettes" ofType:nil];
    if (bundled.length > 0) {
        scanDir(bundled, YES);
    }
    return out;
}

- (NSString*)savePaletteString:(NSString*)paletteString
                        asName:(NSString*)name {
    if (paletteString.length == 0) return nil;
    NSString* show = [self showFolderPath];
    if (show.length == 0) return nil;

    NSString* paletteDir = [show stringByAppendingPathComponent:@"Palettes"];
    NSFileManager* fm = [NSFileManager defaultManager];
    BOOL isDir = NO;
    if (![fm fileExistsAtPath:paletteDir isDirectory:&isDir] || !isDir) {
        if (![fm createDirectoryAtPath:paletteDir
           withIntermediateDirectories:YES
                            attributes:nil
                                 error:nil]) {
            return nil;
        }
    }

    NSString* filename;
    NSString* sanitised = sanitisePaletteName(name);
    if (sanitised.length == 0) {
        filename = autogenPaletteName(paletteDir);
    } else {
        filename = [sanitised stringByAppendingString:@".xpalette"];
    }
    NSString* full = [paletteDir stringByAppendingPathComponent:filename];

    // Desktop appends the filename as the "identity" trailer after
    // the palette string — preserve that so the files iPad writes
    // are indistinguishable from desktop's.
    NSString* fileContents = [NSString stringWithFormat:@"%@%@",
                              paletteString, filename];
    NSError* err = nil;
    if (![fileContents writeToFile:full
                        atomically:YES
                          encoding:NSUTF8StringEncoding
                             error:&err]) {
        return nil;
    }
    return filename;
}

- (BOOL)deleteSavedPalette:(NSString*)filename {
    if (filename.length == 0) return NO;
    NSString* show = [self showFolderPath];
    if (show.length == 0) return NO;
    NSString* full = [[show stringByAppendingPathComponent:@"Palettes"]
                      stringByAppendingPathComponent:filename];
    NSFileManager* fm = [NSFileManager defaultManager];
    if (![fm fileExistsAtPath:full]) return NO;
    return [fm removeItemAtPath:full error:nil];
}

- (NSString*)currentPaletteStringForRow:(int)rowIndex
                                atIndex:(int)effectIndex {
    if (!_context) return @"";
    EffectLayer* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= (int)layer->GetEffectCount()) return @"";
    Effect* effect = layer->GetEffect(effectIndex);
    if (!effect) return @"";
    const SettingsMap& settings = effect->GetPaletteMap();
    // Fallback defaults match `ColorPaletteView` so a new / partly-
    // populated palette still serialises as 8 slots.
    static const char* defaults[8] = {
        "#FF0000", "#00FF00", "#0000FF", "#FFFF00",
        "#FFFFFF", "#000000", "#FFA500", "#800080",
    };
    NSMutableString* out = [NSMutableString string];
    for (int i = 0; i < 8; i++) {
        std::string key = "C_BUTTON_Palette" + std::to_string(i + 1);
        std::string v = settings.Get(key, defaults[i]);
        [out appendFormat:@"%s,", v.c_str()];
    }
    return out;
}

- (BOOL)applyPaletteString:(NSString*)paletteString
                     toRow:(int)rowIndex
                   atIndex:(int)effectIndex {
    if (!_context || paletteString.length == 0) return NO;
    EffectLayer* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= (int)layer->GetEffectCount()) return NO;
    Effect* effect = layer->GetEffect(effectIndex);
    if (!effect) return NO;

    // Palette slot values can themselves contain commas when they
    // hold a ColorCurve blob (`Active=TRUE|Id=…|Values=x=0.000^c=#FF00@FF;…|`).
    // Split on top-level commas only — walk the string tracking
    // whether we're inside an `Active=TRUE|…|` block. Trailing
    // filename trailer (".xpalette") is ignored.
    std::string src([paletteString UTF8String]);
    std::vector<std::string> slots;
    size_t start = 0;
    bool inCurve = false;
    for (size_t i = 0; i < src.size(); ++i) {
        char c = src[i];
        if (!inCurve && c == ',') {
            slots.push_back(src.substr(start, i - start));
            start = i + 1;
        } else if (!inCurve && src.compare(i, 12, "Active=TRUE|") == 0) {
            inCurve = true;
            i += 11;
        } else if (inCurve && c == '|') {
            // A curve ends with "|..." — we treat the trailing `|`
            // that closes the `Values=` section as the curve
            // terminator. The ColorCurve serialiser writes the
            // final `|` after the values block, so when we see
            // `|,` (end-of-curve followed by slot separator) close
            // the curve.
            if (i + 1 < src.size() && src[i + 1] == ',') {
                inCurve = false;
            }
        }
    }
    if (start < src.size()) {
        slots.push_back(src.substr(start));
    }

    int applied = 0;
    for (size_t i = 0; i < slots.size() && applied < 8; ++i) {
        std::string v = slots[i];
        if (v.empty()) continue;
        // Drop the trailer "PAL001.xpalette" if it slipped in.
        if (v.find(".xpalette") != std::string::npos) continue;

        std::string key = "C_BUTTON_Palette" + std::to_string(applied + 1);
        effect->GetPaletteMap()[key] = v;
        applied++;
    }
    effect->PaletteMapUpdated();
    return applied > 0;
}

#pragma mark - Value-curve preset save / load

namespace {

NSString* sanitiseVCName(NSString* raw) {
    if (raw.length == 0) return @"";
    NSMutableString* out = [NSMutableString string];
    for (NSUInteger i = 0; i < raw.length; i++) {
        unichar c = [raw characterAtIndex:i];
        if ((c >= '0' && c <= '9') ||
            (c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z')) {
            [out appendFormat:@"%C", c];
        }
    }
    return out;
}

NSString* autogenVCName(NSString* dir) {
    NSFileManager* fm = [NSFileManager defaultManager];
    for (int i = 1; i < 1000; i++) {
        NSString* candidate = [NSString stringWithFormat:@"VC%03d.xvc", i];
        if (![fm fileExistsAtPath:[dir stringByAppendingPathComponent:candidate]]) {
            return candidate;
        }
    }
    return @"VC999.xvc";
}

} // namespace

- (NSArray<NSDictionary<NSString*, NSString*>*>*)savedValueCurves {
    NSMutableArray<NSDictionary<NSString*, NSString*>*>* out = [NSMutableArray array];
    NSMutableSet<NSString*>* seen = [NSMutableSet set];

    auto scanDir = ^(NSString* dir, BOOL recurse) {
        NSFileManager* fm = [NSFileManager defaultManager];
        BOOL isDir = NO;
        if (![fm fileExistsAtPath:dir isDirectory:&isDir] || !isDir) return;
        NSArray* entries = [fm contentsOfDirectoryAtPath:dir error:nil];
        for (NSString* name in entries) {
            NSString* full = [dir stringByAppendingPathComponent:name];
            BOOL sub = NO;
            [fm fileExistsAtPath:full isDirectory:&sub];
            if (sub) {
                if (recurse) {
                    NSArray* subEntries = [fm contentsOfDirectoryAtPath:full error:nil];
                    for (NSString* subName in subEntries) {
                        if (![subName.lowercaseString hasSuffix:@".xvc"]) continue;
                        ValueCurve vc("");
                        vc.LoadXVC([[full stringByAppendingPathComponent:subName] UTF8String]);
                        std::string s = vc.Serialise();
                        NSString* serialised = [NSString stringWithUTF8String:s.c_str()];
                        if (serialised.length > 0 && ![seen containsObject:serialised]) {
                            [seen addObject:serialised];
                            [out addObject:@{@"filename": subName, @"serialised": serialised}];
                        }
                    }
                }
                continue;
            }
            if (![name.lowercaseString hasSuffix:@".xvc"]) continue;
            ValueCurve vc("");
            vc.LoadXVC([full UTF8String]);
            std::string s = vc.Serialise();
            NSString* serialised = [NSString stringWithUTF8String:s.c_str()];
            if (serialised.length > 0 && ![seen containsObject:serialised]) {
                [seen addObject:serialised];
                [out addObject:@{@"filename": name, @"serialised": serialised}];
            }
        }
    };

    NSString* show = [self showFolderPath];
    if (show.length > 0) {
        scanDir([show stringByAppendingPathComponent:@"valuecurves"], YES);
    }
    NSString* bundled = [[NSBundle mainBundle] pathForResource:@"valuecurves" ofType:nil];
    if (bundled.length > 0) {
        scanDir(bundled, YES);
    }
    return out;
}

- (NSString*)saveValueCurveSerialised:(NSString*)serialised
                                asName:(NSString*)name {
    if (serialised.length == 0) return nil;
    NSString* show = [self showFolderPath];
    if (show.length == 0) return nil;

    NSString* dir = [show stringByAppendingPathComponent:@"valuecurves"];
    NSFileManager* fm = [NSFileManager defaultManager];
    BOOL isDir = NO;
    if (![fm fileExistsAtPath:dir isDirectory:&isDir] || !isDir) {
        if (![fm createDirectoryAtPath:dir
           withIntermediateDirectories:YES
                            attributes:nil
                                 error:nil]) {
            return nil;
        }
    }

    NSString* sanitised = sanitiseVCName(name);
    NSString* filename = sanitised.length == 0
        ? autogenVCName(dir)
        : [sanitised stringByAppendingString:@".xvc"];
    NSString* full = [dir stringByAppendingPathComponent:filename];

    // Reuse core's `SaveXVC` — it applies the ID / limits
    // normalisation desktop expects, so files round-trip.
    ValueCurve vc([serialised UTF8String]);
    vc.SaveXVC([full UTF8String]);
    if (![fm fileExistsAtPath:full]) return nil;
    return filename;
}

- (BOOL)deleteSavedValueCurve:(NSString*)filename {
    if (filename.length == 0) return NO;
    NSString* show = [self showFolderPath];
    if (show.length == 0) return NO;
    NSString* full = [[show stringByAppendingPathComponent:@"valuecurves"]
                      stringByAppendingPathComponent:filename];
    NSFileManager* fm = [NSFileManager defaultManager];
    if (![fm fileExistsAtPath:full]) return NO;
    return [fm removeItemAtPath:full error:nil];
}

/// Resolve the target Model for a row's effect, unwrapping ModelGroups
/// the same way desktop does (JsonEffectPanel.cpp:1815-1818). Returns
/// nullptr on any lookup failure.
- (Model*)_targetModelForRow:(int)rowIndex {
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    if (!row || !row->element) return nullptr;
    const std::string& modelName = row->element->GetModelName();
    Model* m = _context->GetModelManager()[modelName];
    if (!m) return nullptr;
    if (m->GetDisplayAs() == DisplayAsType::ModelGroup) {
        auto* mg = dynamic_cast<ModelGroup*>(m);
        if (mg) m = mg->GetFirstModel();
    }
    return m;
}

- (NSArray<NSString*>*)_keysOfFaceStateData:(const FaceStateData&)map {
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    for (const auto& pair : map) {
        [out addObject:[NSString stringWithUTF8String:pair.first.c_str()]];
    }
    return out;
}

- (NSArray<NSString*>*)statesForRow:(int)rowIndex atIndex:(int)effectIndex {
    (void)effectIndex;  // state list is model-scoped, not effect-scoped
    Model* m = [self _targetModelForRow:rowIndex];
    if (!m) return @[];
    return [self _keysOfFaceStateData:m->GetStateInfo()];
}

- (NSArray<NSString*>*)facesForRow:(int)rowIndex atIndex:(int)effectIndex {
    (void)effectIndex;
    Model* m = [self _targetModelForRow:rowIndex];
    if (!m) return @[];
    return [self _keysOfFaceStateData:m->GetFaceInfo()];
}

- (NSArray<NSString*>*)modelNodeNamesForRow:(int)rowIndex atIndex:(int)effectIndex {
    (void)effectIndex;
    Model* m = [self _targetModelForRow:rowIndex];
    if (!m) return @[];
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    uint32_t n = m->GetNumChannels();
    for (uint32_t i = 0; i < n; i++) {
        std::string name = m->GetNodeName((size_t)i, /*def*/ false);
        if (name.empty()) continue;
        if (!name.empty() && name[0] == '-') continue;  // desktop skips "-..." names
        [out addObject:[NSString stringWithUTF8String:name.c_str()]];
    }
    return out;
}

- (NSArray<NSString*>*)effectSettingOptionsForRow:(int)rowIndex
                                          atIndex:(int)effectIndex
                                         settingId:(NSString*)settingId {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return @[];
    Effect* e = layer->GetEffect(effectIndex);
    if (!e) return @[];
    auto& em = _context->GetEffectManager();
    RenderableEffect* re = em.GetEffect(e->GetEffectName());
    if (!re) return @[];
    auto options = re->GetSettingOptions(std::string([settingId UTF8String]));
    NSMutableArray<NSString*>* out = [NSMutableArray arrayWithCapacity:options.size()];
    for (const auto& s : options) {
        [out addObject:[NSString stringWithUTF8String:s.c_str()]];
    }
    return out;
}

- (NSDictionary<NSString*, NSString*>*)effectSettingsForRow:(int)rowIndex atIndex:(int)effectIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return @{};

    Effect* e = layer->GetEffect(effectIndex);
    if (!e) return @{};

    NSMutableDictionary* dict = [NSMutableDictionary dictionary];
    for (const auto& [key, value] : e->GetSettings()) {
        dict[[NSString stringWithUTF8String:key.c_str()]] =
            [NSString stringWithUTF8String:value.c_str()];
    }
    return dict;
}

- (NSDictionary<NSString*, NSString*>*)effectPaletteForRow:(int)rowIndex atIndex:(int)effectIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return @{};

    Effect* e = layer->GetEffect(effectIndex);
    if (!e) return @{};

    NSMutableDictionary* dict = [NSMutableDictionary dictionary];
    for (const auto& [key, value] : e->GetPaletteMap()) {
        dict[[NSString stringWithUTF8String:key.c_str()]] =
            [NSString stringWithUTF8String:value.c_str()];
    }
    return dict;
}

// MARK: - Effect Metadata

- (NSString*)metadataJsonForEffectNamed:(NSString*)effectName {
    auto& em = _context->GetEffectManager();
    RenderableEffect* re = em.GetEffect(std::string([effectName UTF8String]));
    if (!re || !re->HasMetadata()) return @"";

    // nlohmann::json::dump() serializes to a compact JSON string.
    std::string dumped = re->GetMetadata().dump();
    return [NSString stringWithUTF8String:dumped.c_str()];
}

- (NSString*)sharedMetadataJsonNamed:(NSString*)name {
    // Shared metadata lives in <resources>/effectmetadata/shared/<Name>.json
    std::string resDir = FileUtils::GetResourcesDir();
    if (resDir.empty()) return @"";

    std::string path = resDir + "/effectmetadata/shared/" + std::string([name UTF8String]) + ".json";
    std::ifstream f(path);
    if (!f.is_open()) return @"";

    std::stringstream ss;
    ss << f.rdbuf();
    return [NSString stringWithUTF8String:ss.str().c_str()];
}

- (NSString*)shaderDynamicPropertiesJsonForPath:(NSString*)shaderPath {
    if (!shaderPath || shaderPath.length == 0) return @"";

    // Mirrors the desktop path: parse the .fs into a ShaderConfig, ask
    // for the JSON-shaped dynamic-property array, return it as a string
    // the Swift-side panel builder can decode exactly like static
    // effect metadata. Caller owns the returned string; we free the
    // config before we return.
    ShaderConfig* cfg = ShaderEffect::ParseShader(
        std::string([shaderPath UTF8String]),
        &_context->GetSequenceElements());
    if (!cfg) return @"";

    std::string dumped = cfg->GetDynamicPropertiesJson().dump();
    delete cfg;
    return [NSString stringWithUTF8String:dumped.c_str()];
}

// MARK: - Effect Setting Read/Write

// C_-prefixed keys live in palette map; everything else in settings map.
static bool isPaletteKey(const std::string& key) {
    return key.size() > 2 && key[0] == 'C' && key[1] == '_';
}

- (NSString*)effectSettingValueForKey:(NSString*)key
                                inRow:(int)rowIndex
                              atIndex:(int)effectIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return @"";

    Effect* e = layer->GetEffect(effectIndex);
    if (!e) return @"";

    std::string k = [key UTF8String];
    const SettingsMap& map = isPaletteKey(k) ? e->GetPaletteMap() : e->GetSettings();
    if (!map.Contains(k)) return @"";
    return [NSString stringWithUTF8String:map[k].c_str()];
}

- (BOOL)setEffectSettingValue:(NSString*)value
                       forKey:(NSString*)key
                        inRow:(int)rowIndex
                      atIndex:(int)effectIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return NO;

    Effect* e = layer->GetEffect(effectIndex);
    if (!e) return NO;

    std::string k = [key UTF8String];
    std::string v = [value UTF8String];

    if (isPaletteKey(k)) {
        SettingsMap& map = e->GetPaletteMap();
        if (map.Contains(k) && map[k] == v) return NO;
        map[k] = v;
        // Rebuild the derived `mColors` / `mCC` vectors — the render
        // engine reads from those (via `CopyPalette`), not from
        // `mPaletteMap`. Skipping this step leaves the rendered
        // effect stale: the user's enable/disable toggle flips the
        // map entry but the cached colour vector doesn't change, so
        // nothing visibly updates.
        e->PaletteMapUpdated();
        return YES;
    } else {
        // SetSetting returns true if the value actually changed.
        return e->SetSetting(k, v);
    }
}

- (BOOL)removeEffectSettingForKey:(NSString*)key
                            inRow:(int)rowIndex
                          atIndex:(int)effectIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return NO;

    Effect* e = layer->GetEffect(effectIndex);
    if (!e) return NO;

    std::string k = [key UTF8String];
    bool palette = isPaletteKey(k);
    SettingsMap& map = palette ? e->GetPaletteMap() : e->GetSettings();
    if (!map.Contains(k)) return NO;
    map.erase(k);
    if (palette) {
        e->PaletteMapUpdated();   // refresh mColors / mCC + IncrementChangeCount
    } else {
        e->IncrementChangeCount();
    }
    return YES;
}

// MARK: - Fade In/Out

static const char* kFadeInKey  = "T_TEXTCTRL_Fadein";
static const char* kFadeOutKey = "T_TEXTCTRL_Fadeout";

- (float)effectFadeInSecondsForRow:(int)rowIndex atIndex:(int)effectIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return 0.0f;
    Effect* e = layer->GetEffect(effectIndex);
    if (!e) return 0.0f;
    const SettingsMap& map = e->GetSettings();
    if (!map.Contains(kFadeInKey)) return 0.0f;
    return (float)std::strtod(map[kFadeInKey].c_str(), nullptr);
}

- (float)effectFadeOutSecondsForRow:(int)rowIndex atIndex:(int)effectIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return 0.0f;
    Effect* e = layer->GetEffect(effectIndex);
    if (!e) return 0.0f;
    const SettingsMap& map = e->GetSettings();
    if (!map.Contains(kFadeOutKey)) return 0.0f;
    return (float)std::strtod(map[kFadeOutKey].c_str(), nullptr);
}

- (BOOL)setEffectFadeInSeconds:(float)seconds
                          forRow:(int)rowIndex
                         atIndex:(int)effectIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return NO;
    Effect* e = layer->GetEffect(effectIndex);
    if (!e) return NO;
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.2f", seconds);
    return e->SetSetting(kFadeInKey, buf);
}

- (BOOL)setEffectFadeOutSeconds:(float)seconds
                           forRow:(int)rowIndex
                          atIndex:(int)effectIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return NO;
    Effect* e = layer->GetEffect(effectIndex);
    if (!e) return NO;
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.2f", seconds);
    return e->SetSetting(kFadeOutKey, buf);
}

// MARK: - Edge Resize

- (BOOL)resizeEffectEdgeInRow:(int)rowIndex
                       atIndex:(int)effectIndex
                          edge:(int)edge
                        toMS:(int)newMS {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return NO;
    Effect* e = layer->GetEffect(effectIndex);
    if (!e) return NO;

    if (edge == 0) { // left / start
        int maxStart = e->GetEndTimeMS() - 1;
        if (effectIndex > 0) {
            int prevEnd = layer->GetEffect(effectIndex - 1)->GetEndTimeMS();
            if (newMS < prevEnd) newMS = prevEnd;
        }
        if (newMS < 0) newMS = 0;
        if (newMS > maxStart) return NO;
        e->SetStartTimeMS(newMS);
    } else { // right / end
        int minEnd = e->GetStartTimeMS() + 1;
        if (effectIndex + 1 < layer->GetEffectCount()) {
            int nextStart = layer->GetEffect(effectIndex + 1)->GetStartTimeMS();
            if (newMS > nextStart) newMS = nextStart;
        }
        if (newMS < minEnd) return NO;
        e->SetEndTimeMS(newMS);
    }
    return YES;
}

- (void)renderEffectForRow:(int)rowIndex atIndex:(int)effectIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return;

    Effect* e = layer->GetEffect(effectIndex);
    if (!e) return;

    Element* element = layer->GetParentElement();
    if (!element) return;

    _context->RenderEffectForModel(element->GetModelName(),
                                    e->GetStartTimeMS(),
                                    e->GetEndTimeMS(),
                                    false);
}

- (void)renderRangeForRow:(int)rowIndex
                  startMS:(int)startMS
                    endMS:(int)endMS
                    clear:(BOOL)clear {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer) return;
    Element* element = layer->GetParentElement();
    if (!element) return;
    _context->RenderEffectForModel(element->GetModelName(),
                                    startMS, endMS, clear ? true : false);
}

// MARK: - Lock / Disable

- (BOOL)effectIsLockedInRow:(int)rowIndex atIndex:(int)effectIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return NO;
    Effect* e = layer->GetEffect(effectIndex);
    return (e && e->IsLocked()) ? YES : NO;
}

- (void)setEffectLocked:(BOOL)locked inRow:(int)rowIndex atIndex:(int)effectIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return;
    Effect* e = layer->GetEffect(effectIndex);
    if (e) e->SetLocked(locked ? true : false);
}

- (BOOL)effectIsRenderDisabledInRow:(int)rowIndex atIndex:(int)effectIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return NO;
    Effect* e = layer->GetEffect(effectIndex);
    return (e && e->IsEffectRenderDisabled()) ? YES : NO;
}

- (void)setEffectRenderDisabled:(BOOL)disabled inRow:(int)rowIndex atIndex:(int)effectIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return;
    Effect* e = layer->GetEffect(effectIndex);
    if (e) e->SetEffectRenderDisabled(disabled ? true : false);
}

// MARK: - Copy / Paste helpers

- (NSString*)effectSettingsStringForRow:(int)rowIndex atIndex:(int)effectIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return @"";
    Effect* e = layer->GetEffect(effectIndex);
    if (!e) return @"";
    return [NSString stringWithUTF8String:e->GetSettingsAsString().c_str()];
}

- (NSString*)effectPaletteStringForRow:(int)rowIndex atIndex:(int)effectIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return @"";
    Effect* e = layer->GetEffect(effectIndex);
    if (!e) return @"";
    return [NSString stringWithUTF8String:e->GetPaletteAsString().c_str()];
}

- (NSString*)effectNameForRow:(int)rowIndex atIndex:(int)effectIndex {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer || effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return @"";
    Effect* e = layer->GetEffect(effectIndex);
    if (!e) return @"";
    return [NSString stringWithUTF8String:e->GetEffectName().c_str()];
}

- (int)addEffectToRow:(int)rowIndex
                 name:(NSString*)effectName
             settings:(NSString*)settings
              palette:(NSString*)palette
              startMS:(int)startMS
                endMS:(int)endMS {
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer) return -1;

    std::string name = [effectName UTF8String];
    std::string st   = settings ? std::string([settings UTF8String]) : std::string();
    std::string pal  = palette  ? std::string([palette  UTF8String]) : std::string();

    Effect* e = layer->AddEffect(0, name, st, pal, startMS, endMS, 0, false);
    if (!e) return -1;
    // AddEffect re-sorts; find the new effect's index by identity.
    for (int i = 0; i < layer->GetEffectCount(); i++) {
        if (layer->GetEffect(i) == e) return i;
    }
    return -1;
}

// MARK: - Controller Output

- (BOOL)startOutput {
    return _context->GetOutputManager().StartOutput();
}

- (void)stopOutput {
    _context->GetOutputManager().StopOutput();
}

- (BOOL)isOutputting {
    return _context->GetOutputManager().IsOutputting();
}

- (void)outputFrame:(int)frameMS {
    auto& om = _context->GetOutputManager();
    auto& sd = _context->GetSequenceData();
    if (!sd.IsValidData() || !om.IsOutputting()) return;

    int frame = frameMS / sd.FrameTime();
    if (frame < 0 || (unsigned int)frame >= sd.NumFrames()) return;

    om.StartFrame(frameMS);
    auto& fd = sd[frame];
    om.SetManyChannels(0, (unsigned char*)&fd[0], sd.NumChannels());
    om.EndFrame();
}

// MARK: - Rendering

- (void)renderAll {
    _context->RenderAll();
}

- (BOOL)isRenderDone {
    return _context->IsRenderDone() ? YES : NO;
}

- (BOOL)abortRenderAndWait:(NSTimeInterval)timeoutSeconds {
    if (!_context) return YES;
    // Signal every in-flight render job to bail. Workers test the
    // abort flag at their next frame boundary, so this unblocks them
    // within milliseconds for typical sequences.
    _context->AbortRender();
    // Spin-wait on IsRenderDone(). The poll interval is short because
    // we're on the main thread here and want the UI to close promptly,
    // but aborted jobs finish quickly so the expected case is one or
    // two iterations. The timeout is a safety net — we'd rather force
    // a late teardown than hang the app indefinitely on a stuck job.
    NSDate* deadline = [NSDate dateWithTimeIntervalSinceNow:
                        timeoutSeconds > 0 ? timeoutSeconds : 5.0];
    while (!_context->IsRenderDone()) {
        if ([[NSDate date] compare:deadline] == NSOrderedDescending) {
            return NO;
        }
        [NSThread sleepForTimeInterval:0.01];
    }
    return YES;
}

- (void)handleMemoryWarning {
    _context->HandleMemoryWarning();
}

- (void)handleMemoryCritical {
    _context->HandleMemoryCritical();
}

+ (int64_t)availableMemoryMB {
    return (int64_t)(os_proc_available_memory() / (1024 * 1024));
}

- (int)pixelCountAtMS:(int)frameMS {
    auto pixels = _context->GetAllModelPixels(frameMS);
    return (int)pixels.size();
}

- (NSData*)pixelDataAtMS:(int)frameMS {
    auto pixels = _context->GetAllModelPixels(frameMS);
    if (pixels.empty()) return nil;
    return [NSData dataWithBytes:pixels.data() length:pixels.size() * sizeof(iPadRenderContext::PixelData)];
}

// MARK: - Audio Playback

- (AudioManager*)audioManager {
    return _context->GetCurrentMediaManager();
}

- (BOOL)hasAudio {
    auto* am = [self audioManager];
    return am != nullptr && am->IsOk();
}

- (void)audioPlay {
    auto* am = [self audioManager];
    if (am) am->Play();
}

- (void)audioPause {
    auto* am = [self audioManager];
    if (am) am->Pause();
}

- (void)audioStop {
    auto* am = [self audioManager];
    if (am) am->Stop();
}

- (void)audioSeekToMS:(long)positionMS {
    auto* am = [self audioManager];
    if (am) am->Seek(positionMS);
}

- (long)audioTellMS {
    auto* am = [self audioManager];
    return am ? am->Tell() : 0;
}

- (int)audioPlayingState {
    auto* am = [self audioManager];
    if (!am) return 2; // STOPPED
    return (int)am->GetPlayingState();
}

- (void)setAudioVolume:(int)volume {
    auto* am = [self audioManager];
    if (am) am->SetVolume(volume);
}

- (int)audioVolume {
    auto* am = [self audioManager];
    return am ? am->GetVolume() : 100;
}

- (NSData*)waveformDataFromMS:(long)startMS
                         toMS:(long)endMS
                   numSamples:(int)numSamples {
    auto* am = [self audioManager];
    if (!am || !am->IsOk() || numSamples <= 0) return nil;

    long rate = am->GetRate();
    long trackSize = am->GetTrackSize();
    long startSample = (startMS * rate) / 1000;
    long endSample = (endMS * rate) / 1000;
    if (startSample < 0) startSample = 0;
    if (endSample > trackSize) endSample = trackSize;
    if (startSample >= endSample) return nil;

    long totalSamples = endSample - startSample;
    long samplesPerBucket = totalSamples / numSamples;
    if (samplesPerBucket < 1) samplesPerBucket = 1;

    // Output: numSamples * 2 floats (min, max pairs)
    std::vector<float> peaks(numSamples * 2);
    float* leftData = am->GetRawLeftDataPtr(startSample);
    if (!leftData) return nil;

    for (int i = 0; i < numSamples; i++) {
        long bucketStart = i * samplesPerBucket;
        long bucketEnd = std::min(bucketStart + samplesPerBucket, totalSamples);
        float mn = 0, mx = 0;
        for (long s = bucketStart; s < bucketEnd; s++) {
            float v = leftData[s];
            if (v < mn) mn = v;
            if (v > mx) mx = v;
        }
        peaks[i * 2] = mn;
        peaks[i * 2 + 1] = mx;
    }

    return [NSData dataWithBytes:peaks.data() length:peaks.size() * sizeof(float)];
}

// MARK: - Effect icons (XPM -> BGRA)

namespace {

// Decode "#rrggbb" / "#rrggbbaa" / "None" / "none". Returns true on
// success; colour is returned as 0xAARRGGBB. Named color names (red,
// black, etc.) aren't supported — xLights xpms only use hex and None.
static bool xpmParseColor(const std::string& spec, uint32_t* out) {
    if (spec == "None" || spec == "none") {
        *out = 0x00000000; // fully transparent
        return true;
    }
    if (spec.size() < 7 || spec[0] != '#') return false;
    auto fromHex = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
        if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
        return -1;
    };
    if (spec.size() == 7) {
        int r = (fromHex(spec[1]) << 4) | fromHex(spec[2]);
        int g = (fromHex(spec[3]) << 4) | fromHex(spec[4]);
        int b = (fromHex(spec[5]) << 4) | fromHex(spec[6]);
        if (r < 0 || g < 0 || b < 0) return false;
        *out = 0xFF000000u | (uint32_t(r) << 16) | (uint32_t(g) << 8) | uint32_t(b);
        return true;
    }
    if (spec.size() == 9) {
        int r = (fromHex(spec[1]) << 4) | fromHex(spec[2]);
        int g = (fromHex(spec[3]) << 4) | fromHex(spec[4]);
        int b = (fromHex(spec[5]) << 4) | fromHex(spec[6]);
        int a = (fromHex(spec[7]) << 4) | fromHex(spec[8]);
        if (r < 0 || g < 0 || b < 0 || a < 0) return false;
        *out = (uint32_t(a) << 24) | (uint32_t(r) << 16) | (uint32_t(g) << 8) | uint32_t(b);
        return true;
    }
    return false;
}

// Decode an XPM `const char* const*` into BGRA pixel bytes. Returns an
// NSData with width*height*4 bytes, matching CGBitmapContext's expected
// layout for `.bgra8 | .premultipliedFirst` (kCGImageAlphaPremultiplied
// First + byteOrder32Little).
static NSData* xpmToBGRA(const char* const* xpm, int* outW, int* outH) {
    if (!xpm || !xpm[0]) return nil;

    // Header: "width height numColors charsPerPixel [x_hot y_hot]"
    int width = 0, height = 0, numColors = 0, cpp = 0;
    if (sscanf(xpm[0], "%d %d %d %d", &width, &height, &numColors, &cpp) != 4) {
        return nil;
    }
    if (width <= 0 || height <= 0 || numColors <= 0 || cpp <= 0 || cpp > 4) {
        return nil;
    }

    // Build color table keyed on the cpp-character pixel key.
    std::unordered_map<std::string, uint32_t> colorMap;
    colorMap.reserve((size_t)numColors);
    for (int i = 0; i < numColors; i++) {
        const char* line = xpm[1 + i];
        if (!line) return nil;
        std::string l(line);
        if ((int)l.size() < cpp) return nil;
        std::string key = l.substr(0, cpp);
        // After key, XPM has any whitespace, "c", whitespace, color.
        // Find the "c" type token then the value.
        size_t pos = cpp;
        while (pos < l.size() && isspace((unsigned char)l[pos])) pos++;
        // Skip the type char 'c'/'g'/'s'/'m'/'t' and its trailing space.
        // Only 'c' (color) is used by xLights XPMs.
        if (pos >= l.size()) return nil;
        pos++;
        while (pos < l.size() && isspace((unsigned char)l[pos])) pos++;
        std::string color = l.substr(pos);
        uint32_t argb = 0;
        if (!xpmParseColor(color, &argb)) {
            argb = 0x00000000; // unknown -> transparent
        }
        colorMap[key] = argb;
    }

    // Pixels.
    std::vector<uint8_t> bgra((size_t)width * (size_t)height * 4, 0);
    for (int y = 0; y < height; y++) {
        const char* row = xpm[1 + numColors + y];
        if (!row) return nil;
        std::string rowStr(row);
        if ((int)rowStr.size() < width * cpp) return nil;
        for (int x = 0; x < width; x++) {
            std::string key = rowStr.substr((size_t)x * cpp, cpp);
            auto it = colorMap.find(key);
            uint32_t argb = (it != colorMap.end()) ? it->second : 0;
            uint8_t a = (argb >> 24) & 0xFF;
            uint8_t r = (argb >> 16) & 0xFF;
            uint8_t g = (argb >> 8) & 0xFF;
            uint8_t b = argb & 0xFF;
            // Premultiply (Core Graphics with .premultipliedFirst).
            if (a != 255) {
                r = (uint8_t)((uint32_t)r * a / 255);
                g = (uint8_t)((uint32_t)g * a / 255);
                b = (uint8_t)((uint32_t)b * a / 255);
            }
            size_t o = ((size_t)y * (size_t)width + (size_t)x) * 4;
            // byteOrder32Little + .premultipliedFirst: memory layout is
            // B, G, R, A (little-endian read of 0xAARRGGBB).
            bgra[o + 0] = b;
            bgra[o + 1] = g;
            bgra[o + 2] = r;
            bgra[o + 3] = a;
        }
    }

    if (outW) *outW = width;
    if (outH) *outH = height;
    return [NSData dataWithBytes:bgra.data() length:bgra.size()];
}

static int xpmSizeIndexForDesired(int desired) {
    // iconData[0..4] = {16,24,32,48,64}. Round UP so the iPad retina
    // scale still has crisp pixels to sample.
    if (desired <= 16) return 0;
    if (desired <= 24) return 1;
    if (desired <= 32) return 2;
    if (desired <= 48) return 3;
    return 4;
}

} // namespace

- (int)appendEffectBackgroundForRow:(int)rowIndex
                            atIndex:(int)effectIndex
                                 x1:(float)x1
                                 y1:(float)y1
                                 x2:(float)x2
                                 y2:(float)y2
                             bridge:(id)bridge
                          drawRamps:(BOOL)drawRamps {
    if (!bridge || !_context) return 1;
    auto* layer = [self effectLayerForRow:rowIndex];
    if (!layer) return 1;
    if (effectIndex < 0 || effectIndex >= layer->GetEffectCount()) return 1;
    Effect* e = layer->GetEffect(effectIndex);
    if (!e) return 1;
    if (e->GetPaletteSize() == 0) return 1;

    XLGridMetalBridge* b = (XLGridMetalBridge*)bridge;
    auto* accPtr = (xlVertexColorAccumulator*)[b effectBackgroundAccumulator];
    if (!accPtr) return 1;

    RenderableEffect* ef = _context->GetEffectManager()[e->GetEffectIndex()];
    if (!ef) return 1;

    // Mirror EffectsGrid::DrawEffectBackground (EffectsGrid.cpp:6572):
    // channel-block strands get the strand's node mask; single-color /
    // node-single-color models get node 0's mask; everything else
    // passes nullptr (no tint).
    auto* row = _context->GetSequenceElements().GetRowInformation(rowIndex);
    xlColor colorMask = xlColor::NilColor();
    if (row && row->element) {
        Model* m = _context->GetModelManager()[row->element->GetModelName()];
        if (m) {
            if (m->GetDisplayAs() == DisplayAsType::ChannelBlock) {
                auto* se = dynamic_cast<StrandElement*>(row->element);
                colorMask = se ? m->GetNodeMaskColor(se->GetStrand()) : xlWHITE;
            } else {
                const std::string& st = m->GetStringType();
                if (st.rfind("Single Color", 0) == 0 || st == "Node Single Color") {
                    colorMask = m->GetNodeMaskColor(0);
                }
            }
        }
    }
    xlColor* maskPtr = colorMask.IsNilColor() ? nullptr : &colorMask;

    return ef->DrawEffectBackground(e, (int)x1, (int)y1, (int)x2, (int)y2,
                                     *accPtr, maskPtr, drawRamps ? true : false);
}

// MARK: - Media picker helpers

namespace {

NSString* mediaTypeToString(MediaType t) {
    switch (t) {
        case MediaType::Image:      return @"image";
        case MediaType::SVG:        return @"svg";
        case MediaType::Shader:     return @"shader";
        case MediaType::TextFile:   return @"text";
        case MediaType::BinaryFile: return @"binary";
        case MediaType::Video:      return @"video";
    }
    return @"";
}

std::optional<MediaType> stringToMediaType(NSString* s) {
    if ([s isEqualToString:@"image"])  return MediaType::Image;
    if ([s isEqualToString:@"svg"])    return MediaType::SVG;
    if ([s isEqualToString:@"shader"]) return MediaType::Shader;
    if ([s isEqualToString:@"text"])   return MediaType::TextFile;
    if ([s isEqualToString:@"binary"]) return MediaType::BinaryFile;
    if ([s isEqualToString:@"video"])  return MediaType::Video;
    return std::nullopt;
}

// Locate a cache entry by path scoped to the caller-provided media
// type. Only consults the cache that matches `type`, so we never
// create a stray wrong-type entry — the per-type `Get…` accessors are
// create-on-access, which used to mint an `ImageCacheEntry` for every
// video path the media picker enumerated and then log
// "Error loading image file: …mp4" when the image decoder failed on
// MPEG-4 bytes. `HasMedia` / `HasImage` are non-creating presence
// checks; once we know the path is present, the type-specific
// accessor safely returns the existing entry.
std::shared_ptr<MediaCacheEntry> lookupMediaEntry(SequenceMedia& media,
                                                   const std::string& path,
                                                   MediaType type) {
    switch (type) {
        case MediaType::Image:
            if (!media.HasImage(path)) return nullptr;
            if (auto e = media.GetImage(path))
                return std::static_pointer_cast<MediaCacheEntry>(e);
            return nullptr;
        case MediaType::SVG:
            if (!media.HasMedia(path)) return nullptr;
            if (auto e = media.GetSVG(path))
                return std::static_pointer_cast<MediaCacheEntry>(e);
            return nullptr;
        case MediaType::Shader:
            if (!media.HasMedia(path)) return nullptr;
            if (auto e = media.GetShader(path))
                return std::static_pointer_cast<MediaCacheEntry>(e);
            return nullptr;
        case MediaType::TextFile:
            if (!media.HasMedia(path)) return nullptr;
            if (auto e = media.GetTextFile(path))
                return std::static_pointer_cast<MediaCacheEntry>(e);
            return nullptr;
        case MediaType::BinaryFile:
            if (!media.HasMedia(path)) return nullptr;
            if (auto e = media.GetBinaryFile(path))
                return std::static_pointer_cast<MediaCacheEntry>(e);
            return nullptr;
        case MediaType::Video:
            if (!media.HasMedia(path)) return nullptr;
            if (auto e = media.GetVideo(path))
                return std::static_pointer_cast<MediaCacheEntry>(e);
            return nullptr;
    }
    return nullptr;
}

} // namespace

- (NSArray<NSDictionary<NSString*, NSString*>*>*)mediaPathsInSequence {
    if (!_context) return @[];
    auto& elements = _context->GetSequenceElements();
    auto& media = elements.GetSequenceMedia();
    auto paths = media.GetAllMediaPaths();
    NSMutableArray<NSDictionary<NSString*, NSString*>*>* out =
        [NSMutableArray arrayWithCapacity:paths.size()];
    for (const auto& p : paths) {
        NSString* pathStr = [NSString stringWithUTF8String:p.first.c_str()];
        NSString* typeStr = mediaTypeToString(p.second);
        if (pathStr && typeStr.length) {
            [out addObject:@{@"path": pathStr, @"type": typeStr}];
        }
    }
    return out;
}

- (int)ensureThumbnailPreviewForPath:(NSString*)path
                            mediaType:(NSString*)mediaType
                            maxWidth:(int)maxWidth
                           maxHeight:(int)maxHeight {
    if (!_context || !path) return 0;
    auto typeOpt = stringToMediaType(mediaType);
    if (!typeOpt) return 0;
    auto& media = _context->GetSequenceElements().GetSequenceMedia();
    std::string spath([path UTF8String]);
    auto entry = lookupMediaEntry(media, spath, *typeOpt);
    if (!entry) return 0;
    if (!entry->isLoaded()) {
        entry->Load();
        if (!entry->IsOk()) return 0;
    }
    if (!entry->HasPreview()) {
        if (entry->GetType() == MediaType::Shader) {
            // `MediaCacheEntry::GeneratePreview` is a no-op for shader
            // entries — shader frames come from actually running the
            // shader through the render engine at default params
            // against a 64×64 matrix model. Route through the
            // dedicated path (iPadRenderContext::GenerateShaderPreview
            // mirrors desktop's ShaderPreviewGenerator).
            auto* shaderEntry = static_cast<ShaderMediaCacheEntry*>(entry.get());
            _context->GenerateShaderPreview(shaderEntry);
        } else {
            entry->GeneratePreview(maxWidth, maxHeight);
        }
    }
    return (int)entry->GetPreviewFrameCount();
}

- (NSData*)thumbnailPNGForPath:(NSString*)path
                     mediaType:(NSString*)mediaType
                    frameIndex:(int)frameIndex {
    if (!_context || !path || frameIndex < 0) return nil;
    auto typeOpt = stringToMediaType(mediaType);
    if (!typeOpt) return nil;
    auto& media = _context->GetSequenceElements().GetSequenceMedia();
    std::string spath([path UTF8String]);
    auto entry = lookupMediaEntry(media, spath, *typeOpt);
    if (!entry || !entry->HasPreview()) return nil;
    if ((size_t)frameIndex >= entry->GetPreviewFrameCount()) return nil;
    auto frame = entry->GetPreviewFrame((size_t)frameIndex);
    if (!frame || !frame->IsOk()) return nil;
    std::vector<uint8_t> png;
    if (!frame->SaveAsPNG(png)) return nil;
    return [NSData dataWithBytes:png.data() length:png.size()];
}

- (long)videoDurationMSForPath:(NSString*)path {
    if (!_context || !path) return 0;
    std::string spath([path UTF8String]);
    auto& media = _context->GetSequenceElements().GetSequenceMedia();

    // Prefer the existing VideoMediaCacheEntry — its duration is
    // cached on the entry (populated free-of-charge by
    // `GeneratePreview`'s VideoReader, or lazily by the first
    // `GetDurationMS` call). Non-creating lookup via `HasMedia`
    // first so we don't mint an accidental entry for a non-video
    // path.
    if (media.HasMedia(spath)) {
        if (auto entry = media.GetVideo(spath)) {
            if (!entry->isLoaded()) entry->Load();
            return (long)entry->GetDurationMS();
        }
    }

    // The path has never been referenced by an effect (so the cache
    // never loaded it). Fall back to a fresh FixFile + VideoReader
    // probe. Calling ObtainAccessToURL first activates the ancestor
    // bookmark's security scope so AVFoundation / FFmpeg can read
    // the bytes even when the file lives in iCloud Drive.
    std::string resolved = FileUtils::FixFile("", spath);
    if (resolved.empty()) resolved = spath;
    ObtainAccessToURL(resolved, false);
    long ms = VideoReader::GetVideoLength(resolved);
    return (ms > 0) ? ms : 0;
}

- (long)thumbnailFrameTimeMSForPath:(NSString*)path
                          mediaType:(NSString*)mediaType
                         frameIndex:(int)frameIndex {
    if (!_context || !path || frameIndex < 0) return 0;
    auto typeOpt = stringToMediaType(mediaType);
    if (!typeOpt) return 0;
    auto& media = _context->GetSequenceElements().GetSequenceMedia();
    std::string spath([path UTF8String]);
    auto entry = lookupMediaEntry(media, spath, *typeOpt);
    if (!entry || !entry->HasPreview()) return 0;
    if ((size_t)frameIndex >= entry->GetPreviewFrameCount()) return 0;
    return entry->GetPreviewFrameTime((size_t)frameIndex);
}

- (NSData*)iconBGRAForEffectNamed:(NSString*)effectName
                      desiredSize:(int)desiredSize
                        outputSize:(int*)outputSize {
    if (!_context || !effectName) return nil;
    std::string name([effectName UTF8String]);
    RenderableEffect* fx = _context->GetEffectManager().GetEffect(name);
    if (!fx) return nil;

    // Grab the XPM at the bucket index (same indexing desktop
    // uses in `EffectIconCache`). Effects like `On` bind a single
    // 16-px XPM to every index — we resample below when the
    // native size doesn't match the requested bucket, mirroring
    // desktop's `wxImage::Scale` at cache-build time.
    int idx = xpmSizeIndexForDesired(desiredSize);
    const char* const* xpm = fx->GetIconData(idx);
    int srcW = 0, srcH = 0;
    NSData* srcData = xpmToBGRA(xpm, &srcW, &srcH);
    if (!srcData) return nil;

    // Native size matches the request — no resample needed.
    if (srcW == desiredSize && srcH == desiredSize) {
        if (outputSize) *outputSize = srcW;
        return srcData;
    }

    // Rescale to desiredSize via CoreGraphics. High-quality
    // interpolation gives much better results than the GPU's
    // bilinear filter at display time, especially when scaling
    // 16 → 48+ for effects that only ship a single XPM size.
    int dstW = desiredSize;
    int dstH = desiredSize;
    CGColorSpaceRef cs = CGColorSpaceCreateDeviceRGB();

    // Wrap source bytes in a CGImage.
    CGDataProviderRef provider = CGDataProviderCreateWithCFData(
        (__bridge CFDataRef)srcData);
    CGImageRef srcImage = CGImageCreate(
        srcW, srcH, 8, 32, srcW * 4, cs,
        kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little,
        provider, nullptr, true, kCGRenderingIntentDefault);
    CGDataProviderRelease(provider);

    NSMutableData* outData = [NSMutableData dataWithLength:(NSUInteger)dstW * dstH * 4];
    CGContextRef ctx = CGBitmapContextCreate(
        outData.mutableBytes, dstW, dstH, 8, dstW * 4, cs,
        kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little);
    CGColorSpaceRelease(cs);
    if (!ctx || !srcImage) {
        if (ctx) CGContextRelease(ctx);
        if (srcImage) CGImageRelease(srcImage);
        return nil;
    }
    CGContextSetInterpolationQuality(ctx, kCGInterpolationHigh);
    CGContextDrawImage(ctx, CGRectMake(0, 0, dstW, dstH), srcImage);
    CGContextRelease(ctx);
    CGImageRelease(srcImage);

    if (outputSize) *outputSize = dstW;
    return outData;
}

@end
