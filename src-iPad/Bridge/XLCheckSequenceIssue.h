#pragma once

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

// Severity tiers mirror the desktop CheckSequenceReport::ReportIssue::Type.
typedef NS_ENUM(NSInteger, XLCheckSequenceSeverity) {
    XLCheckSequenceSeverityInfo = 0,
    XLCheckSequenceSeverityWarning = 1,
    XLCheckSequenceSeverityCritical = 2,
};

// Single issue surfaced by `XLSequenceDocument runSequenceCheck`.
// Mostly mirrors `CheckSequenceReport::ReportIssue` from desktop;
// adds optional model / effect / start-time so the iPad UI can offer
// a "jump to" affordance later.
//
// Marked Sendable for Swift 6: every property is `readonly` and only
// initialised in the designated init, so the value is fully immutable
// once handed to Swift — safe to cross actor boundaries (SwiftUI hops
// the result off the SequenceChecker worker queue back to MainActor).
NS_SWIFT_SENDABLE
@interface XLCheckSequenceIssue : NSObject
@property (nonatomic, readonly) XLCheckSequenceSeverity severity;
// Top-level grouping: "controllers" / "models" / "sequence" / etc.
// Matches `CheckSequenceReport::ReportSection::id`.
@property (nonatomic, readonly, copy) NSString* sectionID;
// Display title for the section (e.g. "Controller Checks") —
// resolved from REPORT_SECTIONS so SwiftUI doesn't have to keep a
// parallel mapping.
@property (nonatomic, readonly, copy) NSString* sectionTitle;
@property (nonatomic, readonly, copy) NSString* category;     // "transitions" / "videocache" / …
@property (nonatomic, readonly, copy) NSString* message;
@property (nonatomic, readonly, copy, nullable) NSString* modelName;
@property (nonatomic, readonly, copy, nullable) NSString* effectName;
@property (nonatomic, readonly) NSInteger startTimeMS;        // -1 if not effect-specific
@property (nonatomic, readonly) NSInteger layerIndex;         // -1 if not effect-specific

- (instancetype)initWithSeverity:(XLCheckSequenceSeverity)severity
                         sectionID:(NSString*)sectionID
                      sectionTitle:(NSString*)sectionTitle
                          category:(NSString*)category
                           message:(NSString*)message
                         modelName:(nullable NSString*)modelName
                        effectName:(nullable NSString*)effectName
                       startTimeMS:(NSInteger)startTimeMS
                        layerIndex:(NSInteger)layerIndex NS_DESIGNATED_INITIALIZER;
- (instancetype)init NS_UNAVAILABLE;
@end

NS_ASSUME_NONNULL_END
