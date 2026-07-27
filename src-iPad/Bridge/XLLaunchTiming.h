//
//  XLLaunchTiming.h
//  xLights-iPadLib
//
//  Coarse launch-phase timing. MetricKit tells us a launch was slow but
//  not which part was slow, and MXAppLaunchDiagnostic's call-stack tree
//  lands in Apple's prewarm machinery rather than our code — so the only
//  way to attribute the time is to stamp the phases ourselves.
//
//  Marks are logged as "LAUNCH-PHASE ..." (so they can be grepped out of
//  an uploaded xLights.log) and mirrored into Library/Logs as a sidecar
//  the diagnostics zip picks up.
//

#pragma once

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

NS_SWIFT_NAME(LaunchTiming)
@interface XLLaunchTiming : NSObject

// Stamp t0. Idempotent — the first call wins, so it is safe to call from
// whichever of the app-init entry points runs first.
+ (void)begin;

// Record a phase boundary. Logs elapsed-since-begin and delta-since-the
// -previous-mark, both in ms.
+ (void)mark:(NSString*)name;

// Same, but for work that finished off the launch critical path (the show
// folder load). Kept distinct so a reader can tell "time to first frame"
// from "time until the app was actually usable".
+ (void)markPostLaunch:(NSString*)name;

// Write the accumulated marks to Library/Logs/xlLaunchTiming.txt. Called
// once the launch sequence has settled; safe to call repeatedly (rewrites).
+ (void)flushSidecar;

@end

NS_ASSUME_NONNULL_END
