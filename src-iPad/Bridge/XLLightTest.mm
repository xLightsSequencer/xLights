/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#import "XLLightTest+Internal.h"

#include "controllers/ControllerUploadData.h"
#include "models/Model.h"
#include "models/ModelGroup.h"
#include "models/ModelManager.h"
#include "models/SubModel.h"
#include "outputs/ChannelTracker.h"
#include "outputs/Controller.h"
#include "outputs/Output.h"
#include "outputs/OutputManager.h"
#include "outputs/TestPatternEngine.h"
#include "outputs/TestPreset.h"

#include <chrono>
#include <map>
#include <string>
#include <vector>

#include <spdlog/spdlog.h>

namespace {

constexpr int kFrameIntervalMS = 50;

NSString* Str(const std::string& s)
{
    return [NSString stringWithUTF8String:s.c_str()];
}

// Per-channel colour letters for a model, indexed by (channel - firstChannel)
// modulo the model's channels-per-node. Mirrors ModelTestItem's mapping in the
// desktop dialog so the RGBW cycle lights the same wires on both platforms.
std::string ChannelColoursFor(Model* model)
{
    const int perNode = model->GetChanCountPerNode();
    if (perNode == 1) {
        return std::string(1, Model::EncodeColour(model->GetNodeMaskColor(0))) + "   ";
    }
    if (perNode == 4) {
        const std::string rgb = model->GetRGBOrder();
        const std::string st = model->GetStringType();
        if (st.find("W" + rgb) != std::string::npos) {
            return "W" + rgb + "  ";
        }
        return rgb + "W  ";
    }
    return model->GetRGBOrder() + "   ";
}

} // namespace

@implementation XLLightTest {
    OutputManager* _om;
    ModelManager* _mm;

    ChannelTracker _tracker;
    xltest::TestPatternEngine _engine;

    BOOL _running;
    BOOL _channelsDirty;
    xltest::TestMode _lastMode;
    xltest::TestFunction _lastFunction;
    std::chrono::steady_clock::time_point _startTime;

    // Absolute-channel -> colour-letter lookup, rebuilt with the selection.
    // Only the RGBW cycle consults it.
    std::map<uint32_t, char> _colourCache;
    BOOL _colourCacheValid;
}

- (instancetype)initWithOutputManager:(OutputManager*)outputManager
                         modelManager:(ModelManager*)modelManager
{
    self = [super init];
    if (self) {
        _om = outputManager;
        _mm = modelManager;
        _running = NO;
        _channelsDirty = YES;
        _colourCacheValid = NO;
        _lastMode = xltest::TestMode::Standard;
        _lastFunction = xltest::TestFunction::OFF;

        _mode = XLLightTestModeStandard;
        _function = XLLightTestFunctionOff;
        _speed = 50;
        _chaseGrouping = 1;
        _chaseWholeSelection = NO;
        _twinkleRatio = 10;
        _backgroundIntensity = 0;
        _highlightIntensity = 255;
        _backgroundRed = _backgroundGreen = _backgroundBlue = 0;
        _highlightRed = _highlightGreen = _highlightBlue = 255;
        _tag50th = NO;
        _suppressUnusedOutputs = NO;
    }
    return self;
}

#pragma mark - State

- (NSString*)status
{
    return Str(_engine.GetStatus());
}

- (NSInteger)selectedChannelCount
{
    NSInteger n = 0;
    long start = 0;
    long end = 0;
    _tracker.GetFirstRange(start, end);
    while (start > 0) {
        n += (end - start + 1);
        _tracker.GetNextRange(start, end);
    }
    return n;
}

- (BOOL)running
{
    return _running;
}

- (NSInteger)frameIntervalMS
{
    return kFrameIntervalMS;
}

#pragma mark - Testability

// A controller can only be driven from iOS over a network transport. Serial /
// USB controllers are configurable here but never testable, so the UI can say
// so instead of appearing to run and lighting nothing.
- (BOOL)controllerTestable:(Controller*)c reason:(std::string&)reason
{
    if (c == nullptr) {
        reason = "No controller assigned";
        return NO;
    }
    if (!c->IsActive()) {
        reason = "Controller is inactive";
        return NO;
    }
    if (c->GetType() == CONTROLLER_SERIAL) {
        reason = "Serial/USB controllers cannot be driven from iPadOS";
        return NO;
    }
    if (c->GetType() == CONTROLLER_NULL) {
        reason = "Null controller has no outputs";
        return NO;
    }
    return YES;
}

#pragma mark - Selection sources

- (NSArray<NSDictionary<NSString*, id>*>*)modelItems
{
    NSMutableArray<NSDictionary<NSString*, id>*>* out = [NSMutableArray array];
    if (_mm == nullptr || _om == nullptr) return out;

    for (auto it = _mm->begin(); it != _mm->end(); ++it) {
        Model* m = it->second;
        if (m == nullptr) continue;
        if (m->GetDisplayAs() == DisplayAsType::SubModel) continue;
        if (m->GetDisplayAs() == DisplayAsType::ModelGroup) continue;

        const uint32_t first = m->GetFirstChannel() + 1;
        const uint32_t last = m->GetLastChannel() + 1;

        std::string reason;
        int32_t offset = 0;
        Controller* c = _om->GetController(first, offset);
        const BOOL testable = [self controllerTestable:c reason:reason];

        NSMutableArray<NSDictionary*>* subs = [NSMutableArray array];
        for (Model* sm : m->GetSubModels()) {
            if (sm == nullptr) continue;
            [subs addObject:@{
                @"name": Str(sm->GetName()),
                @"startChannel": @(sm->GetFirstChannel() + 1),
                @"endChannel": @(sm->GetLastChannel() + 1),
                @"nodeCount": @(sm->GetNodeCount()),
            }];
        }

        [out addObject:@{
            @"name": Str(m->GetName()),
            @"displayAs": Str(m->GetDisplayAsString()),
            @"startChannel": @(first),
            @"endChannel": @(last),
            @"nodeCount": @(m->GetNodeCount()),
            @"channelsPerNode": @(m->GetChanCountPerNode()),
            @"controllerName": Str(m->GetControllerName()),
            @"testable": @(testable),
            @"untestableReason": Str(reason),
            @"submodels": subs,
        }];
    }
    return out;
}

- (NSArray<NSDictionary<NSString*, id>*>*)nodeItemsForModel:(NSString*)modelName
{
    NSMutableArray<NSDictionary<NSString*, id>*>* out = [NSMutableArray array];
    if (_mm == nullptr || modelName == nil) return out;

    Model* m = (*_mm)[modelName.UTF8String];
    if (m == nullptr) return out;

    const int perNode = m->GetChanCountPerNode();
    const std::string colours = ChannelColoursFor(m);
    const uint32_t nodes = m->GetNodeCount();

    for (uint32_t i = 0; i < nodes; i++) {
        [out addObject:@{
            @"node": @(i + 1),
            @"startChannel": @(m->NodeStartChannel(i) + 1),
            @"channels": @(perNode),
            @"colours": Str(colours.substr(0, perNode > 0 ? perNode : 0)),
        }];
    }
    return out;
}

// Model Groups as a test target (desktop's Model Groups tree,
// PixelTestDialog.cpp:1386). A group's members need not be contiguous
// in channel space, so the group carries no range of its own — each
// member model does, and selection happens per member. Groups whose
// members resolve to nothing testable are still listed, with the
// reason, rather than silently dropped.
- (NSArray<NSDictionary<NSString*, id>*>*)groupItems
{
    NSMutableArray<NSDictionary<NSString*, id>*>* out = [NSMutableArray array];
    if (_mm == nullptr || _om == nullptr) return out;

    for (auto it = _mm->begin(); it != _mm->end(); ++it) {
        Model* g = it->second;
        if (g == nullptr) continue;
        if (g->GetDisplayAs() != DisplayAsType::ModelGroup) continue;
        auto* mg = dynamic_cast<ModelGroup*>(g);
        if (mg == nullptr) continue;

        NSMutableArray<NSDictionary*>* members = [NSMutableArray array];
        for (Model* m : mg->GetFlatModels()) {
            if (m == nullptr) continue;
            if (m->GetDisplayAs() == DisplayAsType::ModelGroup) continue;
            const uint32_t first = m->GetFirstChannel() + 1;
            const uint32_t last = m->GetLastChannel() + 1;
            std::string reason;
            int32_t offset = 0;
            Controller* c = _om->GetController(first, offset);
            const BOOL testable = [self controllerTestable:c reason:reason];
            [members addObject:@{
                @"name": Str(m->GetName()),
                @"startChannel": @(first),
                @"endChannel": @(last),
                @"nodeCount": @(m->GetNodeCount()),
                @"channelsPerNode": @(m->GetChanCountPerNode()),
                @"testable": @(testable),
                @"untestableReason": Str(reason),
            }];
        }
        if (members.count == 0) continue;

        [out addObject:@{
            @"name": Str(mg->GetName()),
            @"models": members,
        }];
    }
    return out;
}

// Raw universe / channel targeting (desktop's Outputs tree,
// PixelTestDialog.cpp:1377 / PopulateOutputTree). Controller → its
// outputs, each with the absolute channel span the selection model
// works in.
- (NSArray<NSDictionary<NSString*, id>*>*)outputItems
{
    NSMutableArray<NSDictionary<NSString*, id>*>* out = [NSMutableArray array];
    if (_om == nullptr) return out;

    for (Controller* c : _om->GetControllers()) {
        if (c == nullptr) continue;
        std::string reason;
        const BOOL testable = [self controllerTestable:c reason:reason];

        NSMutableArray<NSDictionary*>* outputs = [NSMutableArray array];
        for (const auto& o : c->GetOutputs()) {
            if (o == nullptr) continue;
            const uint32_t start = o->GetStartChannel();
            [outputs addObject:@{
                @"description": Str(o->GetLongDescription()),
                @"universe": @(o->GetUniverse()),
                @"startChannel": @(start),
                @"endChannel": @(start + o->GetChannels() - 1),
                @"channels": @(o->GetChannels()),
            }];
        }
        if (outputs.count == 0) continue;

        [out addObject:@{
            @"name": Str(c->GetName()),
            @"testable": @(testable),
            @"untestableReason": Str(reason),
            @"outputs": outputs,
        }];
    }
    return out;
}

- (NSArray<NSDictionary<NSString*, id>*>*)controllerItems
{
    NSMutableArray<NSDictionary<NSString*, id>*>* out = [NSMutableArray array];
    if (_om == nullptr || _mm == nullptr) return out;

    for (Controller* c : _om->GetControllers()) {
        if (c == nullptr) continue;

        std::string reason;
        const BOOL testable = [self controllerTestable:c reason:reason];

        NSMutableArray<NSDictionary*>* ports = [NSMutableArray array];
        if (testable) {
            UDController ud(c, _om, _mm, false);
            for (int p = 1; p <= ud.GetMaxPixelPort(); p++) {
                UDControllerPort* port = ud.GetControllerPixelPort(p);
                if (port == nullptr) continue;
                if (port->GetModels().empty()) continue;

                NSMutableArray<NSString*>* models = [NSMutableArray array];
                for (auto* pm : port->GetModels()) {
                    if (pm != nullptr && pm->GetModel() != nullptr) {
                        [models addObject:Str(pm->GetModel()->GetName())];
                    }
                }

                [ports addObject:@{
                    @"port": @(p),
                    @"name": Str(port->GetPortName()),
                    @"startChannel": @(port->GetStartChannel()),
                    @"endChannel": @(port->GetEndChannel()),
                    @"pixels": @(port->Pixels()),
                    @"models": models,
                }];
            }
        }

        [out addObject:@{
            @"name": Str(c->GetName()),
            @"ip": Str(c->GetResolvedIP()),
            @"vendor": Str(c->GetVendor()),
            @"model": Str(c->GetModel()),
            @"testable": @(testable),
            @"untestableReason": Str(reason),
            @"ports": ports,
        }];
    }
    return out;
}

#pragma mark - Channel selection

- (void)selectFrom:(uint32_t)start to:(uint32_t)end
{
    if (start == 0 || end < start) return;
    _tracker.AddRange(start, end);
    _channelsDirty = YES;
}

- (void)deselectFrom:(uint32_t)start to:(uint32_t)end
{
    if (start == 0 || end < start) return;
    _tracker.RemoveRange(start, end);
    _channelsDirty = YES;
}

- (NSInteger)selectionStateFrom:(uint32_t)start to:(uint32_t)end
{
    if (start == 0 || end < start) return 0;

    BOOL any = NO;
    BOOL all = YES;
    for (uint32_t ch = start; ch <= end; ch++) {
        if (_tracker.IsChannelOn(ch)) {
            any = YES;
        } else {
            all = NO;
        }
        if (any && !all) return -1;
    }
    if (!any) return 0;
    return all ? 1 : -1;
}

- (void)clearSelection
{
    _tracker.Clear();
    _channelsDirty = YES;
}

#pragma mark - Run control

- (BOOL)start
{
    if (_om == nullptr) return NO;
    if (_running) return YES;

    if (!_om->StartOutput()) return NO;

    _running = YES;
    _channelsDirty = YES;
    _engine.Reset();
    _startTime = std::chrono::steady_clock::now();
    return YES;
}

- (void)stop
{
    if (_om == nullptr) return;
    if (_running) {
        // Push one all-off frame before dropping output so nothing is left lit.
        _om->StartFrame(0);
        _om->AllOff();
        _om->EndFrame();
        _om->SuspendAll(false);
        _om->StopOutput();
    }
    _running = NO;
    _engine.Reset();
}

- (std::vector<uint32_t>)checkedChannels
{
    std::vector<uint32_t> out;
    long ch = _tracker.GetFirst();
    while (ch > 0) {
        out.push_back(static_cast<uint32_t>(ch));
        ch = _tracker.GetNext();
    }
    return out;
}

- (void)rebuildColourCache
{
    _colourCache.clear();
    if (_mm != nullptr) {
        for (auto it = _mm->begin(); it != _mm->end(); ++it) {
            Model* m = it->second;
            if (m == nullptr) continue;
            if (m->GetDisplayAs() == DisplayAsType::SubModel) continue;
            if (m->GetDisplayAs() == DisplayAsType::ModelGroup) continue;

            const int perNode = m->GetChanCountPerNode();
            if (perNode <= 0) continue;
            const std::string colours = ChannelColoursFor(m);
            const uint32_t first = m->GetFirstChannel() + 1;
            const uint32_t last = m->GetLastChannel() + 1;
            for (uint32_t ch = first; ch <= last; ch++) {
                _colourCache[ch] = colours[(ch - first) % perNode];
            }
        }
    }
    _colourCacheValid = YES;
}

- (std::vector<uint32_t>)checkedChannelsOfColour:(char)col
{
    if (!_colourCacheValid) [self rebuildColourCache];

    std::vector<uint32_t> out;
    long ch = _tracker.GetFirst();
    while (ch > 0) {
        auto found = _colourCache.find(static_cast<uint32_t>(ch));
        if (found != _colourCache.end() && found->second == col) {
            out.push_back(static_cast<uint32_t>(ch));
        }
        ch = _tracker.GetNext();
    }
    return out;
}

- (xltest::TestParameters)buildParameters
{
    xltest::TestParameters p;
    p.mode = static_cast<xltest::TestMode>(_mode);
    p.function = static_cast<xltest::TestFunction>(_function);
    p.speed = static_cast<int>(_speed);
    p.chaseGrouping = static_cast<int>(_chaseGrouping);
    p.chaseWholeSelection = _chaseWholeSelection ? true : false;
    p.twinkleRatio = static_cast<int>(_twinkleRatio);
    p.backgroundIntensity = static_cast<int>(_backgroundIntensity);
    p.highlightIntensity = static_cast<int>(_highlightIntensity);
    p.backgroundColor[0] = static_cast<int>(_backgroundRed);
    p.backgroundColor[1] = static_cast<int>(_backgroundGreen);
    p.backgroundColor[2] = static_cast<int>(_backgroundBlue);
    p.highlightColor[0] = static_cast<int>(_highlightRed);
    p.highlightColor[1] = static_cast<int>(_highlightGreen);
    p.highlightColor[2] = static_cast<int>(_highlightBlue);
    p.tag50th = _tag50th ? true : false;
    p.frameTimeMS = kFrameIntervalMS;

    if (p.mode == xltest::TestMode::Controller && _om != nullptr && _mm != nullptr) {
        for (Controller* c : _om->GetControllers()) {
            std::string reason;
            if (c == nullptr || ![self controllerTestable:c reason:reason]) continue;

            UDController ud(c, _om, _mm, false);
            for (int i = 1; i <= ud.GetMaxPixelPort(); i++) {
                UDControllerPort* port = ud.GetControllerPixelPort(i);
                if (port == nullptr || port->GetModels().empty()) continue;

                // Only cycle ports the user actually selected.
                if ([self selectionStateFrom:port->GetStartChannel() to:port->GetEndChannel()] == 0) continue;

                xltest::TestPort tp;
                tp.port = static_cast<uint16_t>(i);
                tp.firstChannel = port->GetStartChannel();
                tp.lastChannel = port->GetEndChannel();
                p.ports.push_back(tp);
            }
        }
    }

    return p;
}

- (void)tick
{
    if (!_running || _om == nullptr) return;

    const auto now = std::chrono::steady_clock::now();
    const long curtime = static_cast<long>(
        std::chrono::duration_cast<std::chrono::milliseconds>(now - _startTime).count());

    const xltest::TestParameters p = [self buildParameters];

    if (p.mode != _lastMode || p.function != _lastFunction) {
        _lastMode = p.mode;
        _lastFunction = p.function;
        _channelsDirty = YES;
    }

    if (_channelsDirty) {
        xltest::TestPatternEngine::ApplySuspend(_om, _tracker, _suppressUnusedOutputs ? true : false);
        if (p.function == xltest::TestFunction::RGBW) {
            _engine.SetChannels([self checkedChannels],
                                [self checkedChannelsOfColour:'R'],
                                [self checkedChannelsOfColour:'G'],
                                [self checkedChannelsOfColour:'B'],
                                [self checkedChannelsOfColour:'W']);
        } else {
            _engine.SetChannels([self checkedChannels]);
        }
        _channelsDirty = NO;
    }

    _om->StartFrame(curtime);
    _engine.Frame(_om, p, curtime);
    _om->EndFrame();
}

#pragma mark - Presets

- (NSArray<NSString*>*)presetNames
{
    NSMutableArray<NSString*>* out = [NSMutableArray array];
    if (_om == nullptr) return out;

    auto presets = _om->GetTestPresets();
    presets.sort();
    for (const auto& p : presets) {
        [out addObject:Str(p)];
    }
    return out;
}

- (BOOL)loadPreset:(NSString*)name
{
    if (_om == nullptr || name == nil) return NO;

    TestPreset* preset = _om->GetTestPreset(name.UTF8String);
    if (preset == nullptr) return NO;

    const long total = _om->GetTotalChannels();
    _tracker.Clear();
    for (const auto ch : preset->GetChannels()) {
        if (ch > 0 && ch < total) {
            _tracker.AddRange(ch, ch);
        }
    }
    _channelsDirty = YES;
    return YES;
}

- (BOOL)savePreset:(NSString*)name
{
    if (_om == nullptr || name == nil || name.length == 0 || name.length > 240) return NO;

    TestPreset* preset = _om->CreateTestPreset(name.UTF8String);
    if (preset == nullptr) return NO;

    long start = 0;
    long end = 0;
    _tracker.GetFirstRange(start, end);
    while (start > 0) {
        preset->AddChannelRange(start, end);
        _tracker.GetNextRange(start, end);
    }

    _om->Save();
    return YES;
}

@end
