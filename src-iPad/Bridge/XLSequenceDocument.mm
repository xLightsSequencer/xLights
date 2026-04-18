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
#include "utils/FileUtils.h"
#include "utils/ExternalHooks.h"

#include <nlohmann/json.hpp>

#include <fstream>
#include <memory>
#include <sstream>
#include <string>

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

@end
