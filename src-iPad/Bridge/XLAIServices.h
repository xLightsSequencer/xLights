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

// Forward declarations of C++ types used internally. The accessor that
// returns a ServiceManager* is only callable from .mm files, so Swift
// can't see it — Swift uses the higher-level NS-typed accessors below.
#ifdef __cplusplus
class ServiceManager;
#endif

NS_ASSUME_NONNULL_BEGIN

// Mirrors src-core/ai/ServiceProperty.h ServiceProperty::Kind. Kept as
// a separate enum so Swift sees a clean Obj-C type instead of a raw int.
typedef NS_ENUM(NSInteger, XLAIPropertyKind) {
    XLAIPropertyKindCategory = 0,
    XLAIPropertyKindBool     = 1,
    XLAIPropertyKindString   = 2,
    XLAIPropertyKindSecret   = 3,
    XLAIPropertyKindInt      = 4,
    XLAIPropertyKindChoice   = 5,
};

// Snapshot of a single editable property on an AI service. Built from
// the C++ ServiceProperty by -[XLAIServices allServices]; the fields
// are read-only because changes go back through XLAIServices, not by
// mutating the snapshot.
@interface XLAIServiceProperty : NSObject

@property (nonatomic, readonly) XLAIPropertyKind kind;
// Dot-qualified id: "ServiceName.PropName" (e.g. "Claude.Key").
// Empty for Category-kind separator entries.
@property (nonatomic, readonly, copy) NSString* propertyId;
@property (nonatomic, readonly, copy) NSString* label;
@property (nonatomic, readonly, copy) NSString* category;
@property (nonatomic, readonly, copy) NSString* helpText;
@property (nonatomic, readonly, copy) NSArray<NSString*>* choices;

// Only one of these is meaningful, picked by `kind`. Defaults are
// safe (NO/0/empty string) for the wrong kind so Swift doesn't have
// to optional-chain.
@property (nonatomic, readonly) BOOL boolValue;
@property (nonatomic, readonly) NSInteger intValue;
@property (nonatomic, readonly, copy) NSString* stringValue;

@end

// Single colour returned from `aiBase::GenerateColorPalette()`.
// Mirrors `aiBase::AIColor`. Hex values are "#RRGGBB" strings.
@interface XLAIPaletteColor : NSObject
@property (nonatomic, readonly, copy) NSString* hexValue;
@property (nonatomic, readonly, copy) NSString* name;
@property (nonatomic, readonly, copy) NSString* descriptionText;
@end

// Snapshot of one registered AI service. Properties are listed in the
// order returned by aiBase::GetProperties() — typically a Category
// header followed by its enable/key/model entries.
@interface XLAIServiceInfo : NSObject

// Service name from aiBase::GetLLMName() (e.g. "Claude").
@property (nonatomic, readonly, copy) NSString* name;
// True if the service has all the config it needs to be called
// (claude requires an API key + at least one capability enabled, etc).
@property (nonatomic, readonly) BOOL available;
// True if any capability is currently enabled.
@property (nonatomic, readonly) BOOL enabled;
// Capabilities the service supports, as XLAICapability* strings (see below).
@property (nonatomic, readonly, copy) NSArray<NSString*>* capabilities;
@property (nonatomic, readonly, copy) NSArray<XLAIServiceProperty*>* properties;

@end

// Capability strings for -[XLAIServices hasEnabledServiceForCapability:]
// and XLAIServiceInfo.capabilities. Values match aiType::TypeSettingsSuffix
// so they're stable across builds.
extern NSString* const XLAICapabilityPrompt;
extern NSString* const XLAICapabilityColorPalettes;
extern NSString* const XLAICapabilityImages;
extern NSString* const XLAICapabilityMapping;
extern NSString* const XLAICapabilitySpeech2Text;

// Singleton ObjC++ bridge that owns the iPad-side AI ServiceManager
// and its IServiceSettingsStore. Initialized lazily on first +shared
// access (typically from XLiPadInit). Lives for the duration of the
// app; never released.
//
// Built-in services (chatGPT, claude, ollama, gemini, GenericClient)
// are registered by ServiceManager at construction. AI plugin loading
// (DLL/dylib) is intentionally disabled on iOS — App Store policy
// forbids loading executable code from outside the app bundle.
@interface XLAIServices : NSObject

+ (instancetype)shared NS_SWIFT_NAME(shared());

// Names of all registered services (matches aiBase::GetLLMName()).
- (NSArray<NSString*>*)serviceNames NS_SWIFT_NAME(serviceNames());

// True if at least one registered service has the given capability
// enabled. Capability strings: XLAICapability* constants. Unknown
// strings return NO.
- (BOOL)hasEnabledServiceForCapability:(NSString*)capability
    NS_SWIFT_NAME(hasEnabledService(forCapability:));

// Snapshots of every registered service + its current properties.
// Cheap to call per redraw — the underlying GetProperties() calls
// just allocate small string vectors.
- (NSArray<XLAIServiceInfo*>*)allServices NS_SWIFT_NAME(allServices());

// Update a single property and persist immediately (matches the
// desktop's "Apply Changes Immediately" preference). Property id is
// the dot-qualified id from XLAIServiceProperty.propertyId.
//
// `setStringProperty` is also used for Secret-kind properties.
- (void)setStringProperty:(NSString*)propertyId value:(NSString*)value
    NS_SWIFT_NAME(setStringProperty(_:value:));
- (void)setBoolProperty:(NSString*)propertyId value:(BOOL)value
    NS_SWIFT_NAME(setBoolProperty(_:value:));
- (void)setIntProperty:(NSString*)propertyId value:(NSInteger)value
    NS_SWIFT_NAME(setIntProperty(_:value:));

// Test the named service by calling aiBase::TestLLM() (which sends a
// "Hello" prompt). Network call runs on a utility queue; completion
// fires on the main queue. `ok=YES` means the service responded with
// a 200 + a non-empty text reply. On failure `message` carries the
// error text returned by the service or HTTP layer.
- (void)testServiceAsync:(NSString*)serviceName
              completion:(void (^)(BOOL ok, NSString* message))completion
    NS_SWIFT_NAME(testService(_:completion:));

// Generate a colour palette from `prompt` using the named service
// (which must support COLORPALETTES — see
// `hasEnabledServiceForCapability:`). Network call runs on a utility
// queue; completion fires on the main queue. On success `colors` is
// non-empty (8 entries when the model behaves; clients should accept
// any size 1..8) and `error` is nil. On failure `colors` is nil and
// `error` carries the message.
- (void)generateColorPalette:(NSString*)prompt
                  forService:(NSString*)serviceName
                  completion:(void (^)(NSArray<XLAIPaletteColor*>* _Nullable colors,
                                       NSString* _Nullable error))completion
    NS_SWIFT_NAME(generateColorPalette(prompt:forService:completion:));

// Run aiBase::GenerateLyricTrack on the named SPEECH2TEXT-capable
// service, against the audio file at `audioPath`. The recognition
// itself can run for many seconds (chunked SFSpeech), so the call
// dispatches to a utility queue and the completion fires on main.
//
// On success: parallel arrays of (word, startMS, endMS) and `error`
// is nil. On failure: arrays are nil and `error` carries the
// message. Pick the service name from `allServices()`'s entries
// whose `capabilities` contains `XLAICapabilitySpeech2Text`.
- (void)generateLyricTrack:(NSString*)audioPath
                forService:(NSString*)serviceName
                completion:(void (^)(NSArray<NSString*>* _Nullable words,
                                     NSArray<NSNumber*>* _Nullable startMS,
                                     NSArray<NSNumber*>* _Nullable endMS,
                                     NSString* _Nullable error))completion
    NS_SWIFT_NAME(generateLyricTrack(audioPath:forService:completion:));

#ifdef __cplusplus
// Internal C++ accessor — callable only from .mm/.cpp files. Returns
// the underlying ServiceManager so effect/import code can call
// findService(...). May return nullptr if init failed.
- (ServiceManager*)serviceManager;
#endif

@end

NS_ASSUME_NONNULL_END
