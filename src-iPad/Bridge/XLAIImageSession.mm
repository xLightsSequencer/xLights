/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#import "XLAIImageSession.h"
#import "XLAIServices.h"

#include "ai/ServiceManager.h"
#include "ai/aiBase.h"

#include <spdlog/spdlog.h>

#include <memory>
#include <string>

@implementation XLAIImageSession {
    // Owned by the session — created by aiBase::createAIImageGenerator()
    // (raw pointer, caller-owned per the aiBase contract). Wrapped in
    // unique_ptr so ARC's auto-generated dealloc handles cleanup via
    // the C++ destructor — avoids the static analyzer's
    // "missing [super dealloc]" false positive on a manual -dealloc.
    std::unique_ptr<aiBase::AIImageGenerator> _generator;
}

+ (nullable instancetype)sessionForService:(NSString*)serviceName {
    if (serviceName.length == 0) return nil;

    XLAIServices* svc = [XLAIServices shared];
    ServiceManager* mgr = [svc serviceManager];
    if (!mgr) return nil;

    aiBase* service = mgr->getService(std::string([serviceName UTF8String]));
    if (!service) return nil;
    if (!service->IsEnabledForType(aiType::IMAGES)) return nil;

    aiBase::AIImageGenerator* gen = service->createAIImageGenerator();
    if (!gen) return nil;

    return [[XLAIImageSession alloc] initWithGenerator:gen serviceName:serviceName];
}

- (instancetype)initWithGenerator:(aiBase::AIImageGenerator*)generator
                       serviceName:(NSString*)serviceName {
    self = [super init];
    if (self) {
        _generator.reset(generator);
        _serviceName = [serviceName copy];
    }
    return self;
}

- (void)generate:(NSString*)prompt
      completion:(void (^)(NSData* _Nullable, NSString* _Nullable))completion {
    if (completion == nil) return;
    if (!_generator || prompt.length == 0) {
        dispatch_async(dispatch_get_main_queue(), ^{
            completion(nil, prompt.length == 0
                ? @"Prompt cannot be empty"
                : @"Image generator is not available");
        });
        return;
    }

    std::string p([prompt UTF8String] ?: "");

    // Hop to a utility queue: the generator's own networking might
    // dispatch, but the contract is "callback may fire from any
    // thread", so we want to be on a background queue when we
    // schedule the call. The completion is marshalled back to main
    // before invoking the Swift block.
    dispatch_async(dispatch_get_global_queue(QOS_CLASS_USER_INITIATED, 0), ^{
        // _generator is captured by raw pointer — the session must
        // outlive the callback. SwiftUI keeps the session in @State
        // while the sheet is open, which covers the full lifecycle.
        _generator->generateImage(p, [completion](aiBase::AIImageResult res) {
            NSData* png = nil;
            NSString* err = nil;
            if (!res.error.empty()) {
                err = [[NSString alloc] initWithBytes:res.error.data()
                                                length:res.error.size()
                                              encoding:NSUTF8StringEncoding];
                if (err == nil) err = @"AI image generation failed";
            } else if (!res.pngBytes.empty()) {
                png = [NSData dataWithBytes:res.pngBytes.data()
                                     length:res.pngBytes.size()];
            } else {
                err = @"AI image generation returned no data";
            }

            dispatch_async(dispatch_get_main_queue(), ^{
                completion(png, err);
            });
        });
    });
}

@end
