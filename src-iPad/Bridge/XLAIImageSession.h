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

// One-shot image-generation session. Owns the underlying C++
// `aiBase::AIImageGenerator` (created by `aiBase::createAIImageGenerator()`)
// and forwards prompts to it. Held in SwiftUI `@State` so the C++
// generator survives across re-rolls; deallocated when the sheet
// closes.
//
// The C++ generator may exist before `IsAvailable()` returns true,
// but creating it on a service that hasn't been configured will yield
// a session that can't generate (the API key check happens in
// `generateImage`). `+sessionForService:` returns nil if the named
// service doesn't exist, doesn't support IMAGES, or doesn't return
// a generator.
@interface XLAIImageSession : NSObject

+ (nullable instancetype)sessionForService:(NSString*)serviceName
    NS_SWIFT_NAME(session(forService:));

// Service name this session was created for (matches the value passed
// to +sessionForService:). Useful for the SwiftUI title bar.
@property (nonatomic, readonly, copy) NSString* serviceName;

// Generate a PNG from the given prompt. Completion always fires on
// the main queue. On success, `pngData` is non-nil and `error` is
// nil. On failure, `pngData` is nil and `error` carries an
// explanation. May be called multiple times on the same session for
// re-rolls — each call replaces the previous result.
- (void)generate:(NSString*)prompt
      completion:(void (^)(NSData* _Nullable pngData,
                           NSString* _Nullable error))completion
    NS_SWIFT_NAME(generate(_:completion:));

@end

NS_ASSUME_NONNULL_END
