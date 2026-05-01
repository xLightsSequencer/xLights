/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#import "XLAIServices.h"
#import "XLiPadServiceSettingsStore.h"

#include "ai/ServiceManager.h"
#include "ai/ServiceProperty.h"
#include "ai/aiBase.h"
#include "ai/aiType.h"

#include <spdlog/spdlog.h>

#include <memory>
#include <optional>
#include <string>
#include <variant>

NSString* const XLAICapabilityPrompt        = @"Prompt";
NSString* const XLAICapabilityColorPalettes = @"ColorPalette";
NSString* const XLAICapabilityImages        = @"Images";
NSString* const XLAICapabilityMapping       = @"Mapping";
NSString* const XLAICapabilitySpeech2Text   = @"Speech2Text";

namespace {

NSString* nsFromStd(const std::string& s) {
    return [[NSString alloc] initWithBytes:s.data()
                                     length:s.size()
                                   encoding:NSUTF8StringEncoding] ?: @"";
}

std::string stdFromNS(NSString* _Nullable s) {
    if (s == nil) return {};
    const char* utf8 = [s UTF8String];
    return utf8 ? std::string(utf8) : std::string();
}

XLAIPropertyKind kindFromCore(ServiceProperty::Kind k) {
    switch (k) {
        case ServiceProperty::Kind::Category: return XLAIPropertyKindCategory;
        case ServiceProperty::Kind::Bool:     return XLAIPropertyKindBool;
        case ServiceProperty::Kind::String:   return XLAIPropertyKindString;
        case ServiceProperty::Kind::Secret:   return XLAIPropertyKindSecret;
        case ServiceProperty::Kind::Int:      return XLAIPropertyKindInt;
        case ServiceProperty::Kind::Choice:   return XLAIPropertyKindChoice;
    }
    return XLAIPropertyKindString;
}

NSString* capabilityName(aiType::TYPE t) {
    switch (t) {
        case aiType::PROMPT:        return XLAICapabilityPrompt;
        case aiType::COLORPALETTES: return XLAICapabilityColorPalettes;
        case aiType::IMAGES:        return XLAICapabilityImages;
        case aiType::MAPPING:       return XLAICapabilityMapping;
        case aiType::SPEECH2TEXT:   return XLAICapabilitySpeech2Text;
    }
    return @"";
}

std::optional<aiType::TYPE> capabilityFromName(NSString* name) {
    if ([name isEqualToString:XLAICapabilityPrompt])        return aiType::PROMPT;
    if ([name isEqualToString:XLAICapabilityColorPalettes]) return aiType::COLORPALETTES;
    if ([name isEqualToString:XLAICapabilityImages])        return aiType::IMAGES;
    if ([name isEqualToString:XLAICapabilityMapping])       return aiType::MAPPING;
    if ([name isEqualToString:XLAICapabilitySpeech2Text])   return aiType::SPEECH2TEXT;
    return std::nullopt;
}

// Split "Service.Property" — returns (serviceName, fullPropertyId, true)
// or (empty, empty, false) if the id doesn't have a service prefix.
std::pair<std::string, bool> serviceNameFromPropertyId(NSString* propertyId) {
    NSRange dot = [propertyId rangeOfString:@"."];
    if (dot.location == NSNotFound) return { {}, false };
    NSString* svc = [propertyId substringToIndex:dot.location];
    return { stdFromNS(svc), true };
}

} // namespace

#pragma mark - XLAIPaletteColor

@implementation XLAIPaletteColor {
    NSString* _hexValue;
    NSString* _name;
    NSString* _descriptionText;
}

- (instancetype)initFromCore:(const aiBase::AIColor&)c {
    self = [super init];
    if (self) {
        _hexValue        = [nsFromStd(c.hexValue) copy];
        _name            = [nsFromStd(c.name) copy];
        _descriptionText = [nsFromStd(c.description) copy];
    }
    return self;
}

- (NSString*)hexValue        { return _hexValue        ?: @""; }
- (NSString*)name            { return _name            ?: @""; }
- (NSString*)descriptionText { return _descriptionText ?: @""; }

@end

#pragma mark - XLAIServiceProperty

@implementation XLAIServiceProperty {
    NSString* _propertyId;
    NSString* _label;
    NSString* _category;
    NSString* _helpText;
    NSArray<NSString*>* _choices;
    NSString* _stringValue;
}

@synthesize kind = _kind;
@synthesize boolValue = _boolValue;
@synthesize intValue = _intValue;

- (instancetype)initFromCore:(const ServiceProperty&)p {
    self = [super init];
    if (self) {
        _kind       = kindFromCore(p.kind);
        _propertyId = [nsFromStd(p.id) copy];
        _label      = [nsFromStd(p.label) copy];
        _category   = [nsFromStd(p.category) copy];
        _helpText   = [nsFromStd(p.help) copy];

        NSMutableArray<NSString*>* ch = [NSMutableArray arrayWithCapacity:p.choices.size()];
        for (auto const& c : p.choices) [ch addObject:nsFromStd(c)];
        _choices = [ch copy];

        // Pull the right value out of the variant.
        _boolValue   = NO;
        _intValue    = 0;
        _stringValue = @"";
        if (std::holds_alternative<bool>(p.value)) {
            _boolValue = std::get<bool>(p.value) ? YES : NO;
        } else if (std::holds_alternative<int>(p.value)) {
            _intValue = (NSInteger)std::get<int>(p.value);
        } else if (std::holds_alternative<std::string>(p.value)) {
            _stringValue = [nsFromStd(std::get<std::string>(p.value)) copy];
        }
    }
    return self;
}

- (NSString*)propertyId  { return _propertyId  ?: @""; }
- (NSString*)label       { return _label       ?: @""; }
- (NSString*)category    { return _category    ?: @""; }
- (NSString*)helpText    { return _helpText    ?: @""; }
- (NSArray<NSString*>*)choices { return _choices ?: @[]; }
- (NSString*)stringValue { return _stringValue ?: @""; }

@end

#pragma mark - XLAIServiceInfo

@implementation XLAIServiceInfo {
    NSString* _name;
    NSArray<NSString*>* _capabilities;
    NSArray<XLAIServiceProperty*>* _properties;
}

@synthesize available = _available;
@synthesize enabled = _enabled;

- (instancetype)initFromService:(aiBase*)service {
    self = [super init];
    if (self) {
        _name = [nsFromStd(service->GetLLMName()) copy];
        _available = service->IsAvailable() ? YES : NO;
        _enabled   = service->IsEnabled()   ? YES : NO;

        NSMutableArray<NSString*>* caps = [NSMutableArray array];
        for (auto t : service->GetTypes()) {
            NSString* n = capabilityName(t);
            if (n.length > 0) [caps addObject:n];
        }
        _capabilities = [caps copy];

        auto coreProps = service->GetProperties();
        NSMutableArray<XLAIServiceProperty*>* props = [NSMutableArray arrayWithCapacity:coreProps.size()];
        for (auto const& p : coreProps) {
            [props addObject:[[XLAIServiceProperty alloc] initFromCore:p]];
        }
        _properties = [props copy];
    }
    return self;
}

- (NSString*)name { return _name ?: @""; }
- (NSArray<NSString*>*)capabilities  { return _capabilities ?: @[]; }
- (NSArray<XLAIServiceProperty*>*)properties { return _properties ?: @[]; }

// We deliberately do NOT override -isEqual: / -hash. NSObject's
// pointer equality is what we want: every -allServices call returns
// freshly-allocated snapshots, so SwiftUI sees the array as
// "changed" on every reload and re-renders accordingly. A previous
// attempt here defined value-equality on name + available + enabled
// — but that suppressed re-renders when the user toggled an
// individual capability bool, since name / available / enabled
// didn't flip (toggling Color Palette off still leaves Images +
// Speech2Text enabled, so `enabled` stays true). Toggles appeared
// to "not work" until a final bool flipped the aggregate, at which
// point every row updated at once. Pointer equality avoids that
// whole class of bug; the .onChange(of: services) handler in the
// sheet is idempotent so firing every reload is harmless.

@end

#pragma mark - XLAIServices

@implementation XLAIServices {
    std::unique_ptr<XLiPadServiceSettingsStore> _store;
    std::unique_ptr<ServiceManager> _manager;
}

+ (instancetype)shared {
    static XLAIServices* instance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        instance = [[XLAIServices alloc] init];
    });
    return instance;
}

- (instancetype)init {
    self = [super init];
    if (self) {
        _store = std::make_unique<XLiPadServiceSettingsStore>();
        // pluginDir = "" — App Store policy forbids dynamic loading,
        // so we ship only the built-in services compiled into core.
        // ServiceManager itself registers AppleIntelligence (Apple
        // Silicon only), so iPad and desktop see the same set without
        // duplicating the platform check here.
        _manager = std::make_unique<ServiceManager>(_store.get(), std::string());

        if (auto services = _manager->getServices(); !services.empty()) {
            std::string names;
            for (auto* s : services) {
                if (!names.empty()) names += ", ";
                names += s->GetLLMName();
            }
            spdlog::info("XLAIServices: registered {} AI service(s): {}",
                         services.size(), names);
        } else {
            spdlog::warn("XLAIServices: no AI services registered");
        }
    }
    return self;
}

- (ServiceManager*)serviceManager {
    return _manager.get();
}

- (NSArray<NSString*>*)serviceNames {
    if (!_manager) return @[];
    auto services = _manager->getServices();
    NSMutableArray<NSString*>* names = [NSMutableArray arrayWithCapacity:services.size()];
    for (auto* s : services) [names addObject:nsFromStd(s->GetLLMName())];
    return names;
}

- (BOOL)hasEnabledServiceForCapability:(NSString*)capability {
    if (!_manager || capability == nil) return NO;
    auto t = capabilityFromName(capability);
    if (!t) return NO;
    return _manager->findService(*t) != nullptr ? YES : NO;
}

- (NSArray<XLAIServiceInfo*>*)allServices {
    if (!_manager) return @[];
    auto services = _manager->getServices();
    NSMutableArray<XLAIServiceInfo*>* infos = [NSMutableArray arrayWithCapacity:services.size()];
    for (auto* s : services) {
        [infos addObject:[[XLAIServiceInfo alloc] initFromService:s]];
    }
    return infos;
}

// Common dispatch helper — find the service for a property id, apply
// the lambda, and persist. Mirrors the wxPropertyGrid change handler.
- (void)applyPropertyId:(NSString*)propertyId
                handler:(void (^)(aiBase* service, const std::string& id))handler {
    if (!_manager || propertyId.length == 0) return;
    auto [svcName, ok] = serviceNameFromPropertyId(propertyId);
    if (!ok) return;
    aiBase* service = _manager->getService(svcName);
    if (!service) return;
    handler(service, stdFromNS(propertyId));
    service->SaveSettings();
}

- (void)setStringProperty:(NSString*)propertyId value:(NSString*)value {
    NSString* v = value ?: @"";
    [self applyPropertyId:propertyId handler:^(aiBase* service, const std::string& id) {
        service->SetProperty(id, stdFromNS(v));
    }];
}

- (void)setBoolProperty:(NSString*)propertyId value:(BOOL)value {
    [self applyPropertyId:propertyId handler:^(aiBase* service, const std::string& id) {
        service->SetProperty(id, value ? true : false);
    }];
}

- (void)setIntProperty:(NSString*)propertyId value:(NSInteger)value {
    [self applyPropertyId:propertyId handler:^(aiBase* service, const std::string& id) {
        service->SetProperty(id, (int)value);
    }];
}

- (void)testServiceAsync:(NSString*)serviceName
              completion:(void (^)(BOOL ok, NSString* message))completion {
    if (completion == nil) return;
    if (!_manager || serviceName.length == 0) {
        dispatch_async(dispatch_get_main_queue(), ^{
            completion(NO, @"AI services not initialized");
        });
        return;
    }

    std::string name = stdFromNS(serviceName);
    aiBase* service = _manager->getService(name);
    if (!service) {
        dispatch_async(dispatch_get_main_queue(), ^{
            completion(NO, [NSString stringWithFormat:@"Service '%@' not found", serviceName]);
        });
        return;
    }
    if (!service->IsAvailable()) {
        dispatch_async(dispatch_get_main_queue(), ^{
            completion(NO, [NSString stringWithFormat:
                @"%@ is not configured. Add an API key and enable at least one capability.",
                serviceName]);
        });
        return;
    }

    // The CallLLM path runs a synchronous libcurl request. Hop to a
    // utility queue so the SwiftUI view stays responsive, then marshal
    // the result back to main for the alert.
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0), ^{
        auto [msg, ok] = service->TestLLM();
        NSString* nsMsg = nsFromStd(msg);
        BOOL nsOk = ok ? YES : NO;
        dispatch_async(dispatch_get_main_queue(), ^{
            completion(nsOk, nsMsg.length > 0 ? nsMsg : @"No response");
        });
    });
}

- (void)generateLyricTrack:(NSString*)audioPath
                forService:(NSString*)serviceName
                completion:(void (^)(NSArray<NSString*>* _Nullable,
                                     NSArray<NSNumber*>* _Nullable,
                                     NSArray<NSNumber*>* _Nullable,
                                     NSString* _Nullable))completion {
    if (completion == nil) return;
    if (!_manager || serviceName.length == 0 || audioPath.length == 0) {
        dispatch_async(dispatch_get_main_queue(), ^{
            completion(nil, nil, nil, audioPath.length == 0
                ? @"Audio path is empty"
                : @"AI services not initialized");
        });
        return;
    }

    aiBase* service = _manager->getService(stdFromNS(serviceName));
    if (!service) {
        dispatch_async(dispatch_get_main_queue(), ^{
            completion(nil, nil, nil,
                [NSString stringWithFormat:@"Service '%@' not found", serviceName]);
        });
        return;
    }
    if (!service->IsEnabledForType(aiType::SPEECH2TEXT)) {
        dispatch_async(dispatch_get_main_queue(), ^{
            completion(nil, nil, nil,
                [NSString stringWithFormat:
                    @"%@ is not configured for speech-to-text.", serviceName]);
        });
        return;
    }

    std::string p = stdFromNS(audioPath);

    dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0), ^{
        aiBase::AILyricTrack track = service->GenerateLyricTrack(p);

        if (!track.error.empty()) {
            NSString* err = nsFromStd(track.error);
            dispatch_async(dispatch_get_main_queue(), ^{
                completion(nil, nil, nil, err);
            });
            return;
        }

        NSMutableArray<NSString*>* words = [NSMutableArray arrayWithCapacity:track.lyrics.size()];
        NSMutableArray<NSNumber*>* starts = [NSMutableArray arrayWithCapacity:track.lyrics.size()];
        NSMutableArray<NSNumber*>* ends   = [NSMutableArray arrayWithCapacity:track.lyrics.size()];
        for (auto const& l : track.lyrics) {
            [words addObject:nsFromStd(l.word)];
            [starts addObject:@(l.startMS)];
            [ends addObject:@(l.endMS)];
        }
        dispatch_async(dispatch_get_main_queue(), ^{
            completion(words, starts, ends, nil);
        });
    });
}

- (void)generateColorPalette:(NSString*)prompt
                  forService:(NSString*)serviceName
                  completion:(void (^)(NSArray<XLAIPaletteColor*>* _Nullable,
                                       NSString* _Nullable))completion {
    if (completion == nil) return;
    if (!_manager || serviceName.length == 0 || prompt.length == 0) {
        dispatch_async(dispatch_get_main_queue(), ^{
            completion(nil, prompt.length == 0
                ? @"Prompt cannot be empty"
                : @"AI services not initialized");
        });
        return;
    }

    aiBase* service = _manager->getService(stdFromNS(serviceName));
    if (!service) {
        dispatch_async(dispatch_get_main_queue(), ^{
            completion(nil, [NSString stringWithFormat:@"Service '%@' not found", serviceName]);
        });
        return;
    }
    if (!service->IsEnabledForType(aiType::COLORPALETTES)) {
        dispatch_async(dispatch_get_main_queue(), ^{
            completion(nil, [NSString stringWithFormat:
                @"%@ is not configured for color palette generation.", serviceName]);
        });
        return;
    }

    std::string p = stdFromNS(prompt);

    dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0), ^{
        aiBase::AIColorPalette palette = service->GenerateColorPalette(p);

        if (!palette.error.empty()) {
            NSString* err = nsFromStd(palette.error);
            dispatch_async(dispatch_get_main_queue(), ^{
                completion(nil, err);
            });
            return;
        }
        if (palette.colors.empty()) {
            dispatch_async(dispatch_get_main_queue(), ^{
                completion(nil, @"AI returned an empty palette");
            });
            return;
        }

        NSMutableArray<XLAIPaletteColor*>* colors = [NSMutableArray arrayWithCapacity:palette.colors.size()];
        for (auto const& c : palette.colors) {
            [colors addObject:[[XLAIPaletteColor alloc] initFromCore:c]];
        }
        dispatch_async(dispatch_get_main_queue(), ^{
            completion(colors, nil);
        });
    });
}

@end
