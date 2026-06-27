#pragma once

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

// One effect matched by `XLSequenceDocument findEffectsMatching…`.
// Mirrors the columns of desktop's FindDataPanel result list
// (Element / Strand-Submodel / Layer / Effect / Start) plus the
// location data the iPad needs to drive `jumpToEffect`.
//
// Marked Sendable for Swift 6: every property is `readonly` and only
// set in the designated init, so the value is fully immutable once it
// reaches Swift — safe to hop off the query worker queue to MainActor.
NS_SWIFT_SENDABLE
@interface XLFindEffectResult : NSObject
@property (nonatomic, readonly, copy) NSString* modelName;     // parent model
@property (nonatomic, readonly, copy) NSString* elementName;   // model / strand / submodel display name
@property (nonatomic, readonly, copy) NSString* effectName;
@property (nonatomic, readonly) NSInteger layerIndex;
@property (nonatomic, readonly) NSInteger startTimeMS;
// The single key=value setting (or palette entry) that matched the
// query — shown as a result subtitle so the user sees *why* it hit.
@property (nonatomic, readonly, copy) NSString* matchedSetting;

- (instancetype)initWithModelName:(NSString*)modelName
                      elementName:(NSString*)elementName
                       effectName:(NSString*)effectName
                       layerIndex:(NSInteger)layerIndex
                      startTimeMS:(NSInteger)startTimeMS
                   matchedSetting:(NSString*)matchedSetting NS_DESIGNATED_INITIALIZER;
- (instancetype)init NS_UNAVAILABLE;
@end

NS_ASSUME_NONNULL_END
