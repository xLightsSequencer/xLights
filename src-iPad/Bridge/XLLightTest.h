/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#pragma once

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

// Which family of test patterns to run. Mirrors xltest::TestMode, and the
// tabs of the desktop test dialog.
typedef NS_ENUM(NSInteger, XLLightTestMode) {
    XLLightTestModeStandard = 0,
    XLLightTestModeRGB = 1,
    XLLightTestModeRGBCycle = 2,
    XLLightTestModeController = 3,
};

// Mirrors xltest::TestFunction.
typedef NS_ENUM(NSInteger, XLLightTestFunction) {
    XLLightTestFunctionOff = 0,
    XLLightTestFunctionChase = 1,
    XLLightTestFunctionDim = 2,
    XLLightTestFunctionTwinkle = 3,
    XLLightTestFunctionShimmer = 4,
    XLLightTestFunctionRGBW = 5,
    XLLightTestFunctionPortCycle = 6,
    XLLightTestFunctionColorBlocks = 7,
};

// Drives the shared core light-test engine (src-core/outputs/TestPatternEngine)
// against the show's configured controllers. The desktop equivalent is
// PixelTestDialog; both sides run the same pattern maths.
//
// Live output on iPad goes out over sACN / ArtNet / DDP, which the app is
// entitled for (com.apple.developer.networking.multicast). USB / serial DMX
// controllers have no iOS transport and are reported as untestable rather
// than silently doing nothing.
NS_SWIFT_NAME(LightTest)
@interface XLLightTest : NSObject

#pragma mark - Pattern parameters

@property (nonatomic) XLLightTestMode mode;
@property (nonatomic) XLLightTestFunction function;
/// 0-100.
@property (nonatomic) NSInteger speed;
/// Channels (Standard) or pixels (RGB) per chase group. Ignored when
/// `chaseWholeSelection` is set.
@property (nonatomic) NSInteger chaseGrouping;
@property (nonatomic) BOOL chaseWholeSelection;
/// 1-in-N channels lit at any moment during a twinkle.
@property (nonatomic) NSInteger twinkleRatio;

@property (nonatomic) NSInteger backgroundIntensity;
@property (nonatomic) NSInteger highlightIntensity;
@property (nonatomic) NSInteger backgroundRed;
@property (nonatomic) NSInteger backgroundGreen;
@property (nonatomic) NSInteger backgroundBlue;
@property (nonatomic) NSInteger highlightRed;
@property (nonatomic) NSInteger highlightGreen;
@property (nonatomic) NSInteger highlightBlue;

/// Light every 50th pixel half-white so long strings can be counted.
@property (nonatomic) BOOL tag50th;
/// Stop sending to outputs that hold no selected channel.
@property (nonatomic) BOOL suppressUnusedOutputs;

#pragma mark - State

@property (nonatomic, readonly, copy) NSString* status;
@property (nonatomic, readonly) NSInteger selectedChannelCount;
@property (nonatomic, readonly) BOOL running;

#pragma mark - Selection sources

/// Every model in the show with its absolute channel span. Keys:
///   "name"          — NSString
///   "displayAs"     — NSString
///   "startChannel"  — NSNumber (uint32, 1-based)
///   "endChannel"    — NSNumber (uint32, 1-based)
///   "nodeCount"     — NSNumber (int)
///   "channelsPerNode" — NSNumber (int)
///   "controllerName" — NSString (empty when unassigned)
///   "testable"      — NSNumber (BOOL) NO when the model's controller has no
///                     iOS-reachable transport (serial / USB)
///   "untestableReason" — NSString (empty when testable)
///   "submodels"     — NSArray<NSDictionary> of {"name","startChannel","endChannel"}
- (NSArray<NSDictionary<NSString*, id>*>*)modelItems;

/// Per-node channel spans for one model, for node-level selection. Keys:
///   "node"          — NSNumber (int, 1-based)
///   "startChannel"  — NSNumber (uint32, 1-based absolute)
///   "channels"      — NSNumber (int, channels per node)
///   "colours"       — NSString (per-channel colour letters, e.g. "RGB")
- (NSArray<NSDictionary<NSString*, id>*>*)nodeItemsForModel:(NSString*)modelName
    NS_SWIFT_NAME(nodeItems(forModel:));

/// Controllers and their output ports, for the port-cycle / pixel-count
/// tests. Keys:
///   "name" / "ip" / "vendor" / "model" — NSString
///   "testable"        — NSNumber (BOOL)
///   "untestableReason" — NSString
///   "ports"           — NSArray<NSDictionary>:
///       "port"         — NSNumber (int, 1-based)
///       "name"         — NSString
///       "startChannel" / "endChannel" — NSNumber (uint32, 1-based)
///       "pixels"       — NSNumber (int)
///       "models"       — NSArray<NSString> model names on the port
- (NSArray<NSDictionary<NSString*, id>*>*)controllerItems;

#pragma mark - Channel selection

- (void)selectFrom:(uint32_t)start to:(uint32_t)end;
- (void)deselectFrom:(uint32_t)start to:(uint32_t)end;
/// 0 = none selected, 1 = every channel selected, -1 = partial.
- (NSInteger)selectionStateFrom:(uint32_t)start to:(uint32_t)end;
- (void)clearSelection;

#pragma mark - Run control

/// Enables output and begins a test. Returns NO when output could not be
/// started (no controllers, or none reachable).
- (BOOL)start;
- (void)stop;
/// Render and send one frame. Call at the rate given by `frameIntervalMS`.
- (void)tick;
@property (nonatomic, readonly) NSInteger frameIntervalMS;

#pragma mark - Presets

/// Test presets are stored in the show's networks file by shared core code,
/// so presets saved on desktop appear here and vice versa.
- (NSArray<NSString*>*)presetNames;
- (BOOL)loadPreset:(NSString*)name;
/// Overwrites an existing preset of the same name.
- (BOOL)savePreset:(NSString*)name;

- (instancetype)init NS_UNAVAILABLE;

@end

NS_ASSUME_NONNULL_END
