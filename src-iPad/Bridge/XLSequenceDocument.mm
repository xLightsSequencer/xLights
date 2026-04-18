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
#include "iPadRenderContext.h"

#include "render/Element.h"
#include "render/EffectLayer.h"
#include "render/Effect.h"
#include "effects/RenderableEffect.h"
#include "effects/EffectManager.h"
#include "media/AudioManager.h"
#include "models/Model.h"
#include "models/ModelManager.h"
#include "utils/FileUtils.h"
#include "utils/ExternalHooks.h"

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
    if (te) te->SetActive(active ? true : false);
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
        e->IncrementChangeCount();
        return YES;
    } else {
        // SetSetting returns true if the value actually changed.
        return e->SetSetting(k, v);
    }
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

- (NSData*)iconBGRAForEffectNamed:(NSString*)effectName
                      desiredSize:(int)desiredSize
                        outputSize:(int*)outputSize {
    if (!_context || !effectName) return nil;
    std::string name([effectName UTF8String]);
    RenderableEffect* fx = _context->GetEffectManager().GetEffect(name);
    if (!fx) return nil;
    int idx = xpmSizeIndexForDesired(desiredSize);
    const char* const* xpm = fx->GetIconData(idx);
    int w = 0, h = 0;
    NSData* data = xpmToBGRA(xpm, &w, &h);
    if (!data) return nil;
    if (outputSize) *outputSize = w; // square icons — w == h for xLights set
    return data;
}

@end
