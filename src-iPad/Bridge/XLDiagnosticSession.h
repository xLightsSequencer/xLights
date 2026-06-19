//
//  XLDiagnosticSession.h
//  xLights-iPadLib
//
//  Tracks "is a session currently running" via a sentinel file at
//  Library/Logs/Sessions/current-session.txt. If the file exists at
//  launch time, the previous session was killed without reaching the
//  orderly background-handoff path (force-quit, OOM kill, watchdog
//  terminate, native crash). The diagnostic uploader sees this and
//  bundles the previous session's logs for upload.
//
//  The sentinel is NOT a substitute for MetricKit's MXCrashDiagnostic
//  — those come with stack traces, ~24h after the event. The sentinel
//  catches the much-faster "next-launch" signal and the OOM-kill
//  cases MetricKit doesn't always cover.
//

#pragma once

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface XLDiagnosticSession : NSObject

// Returns the JSON dictionary of any pre-existing sentinel found at
// launch (= previous session crashed), or nil if none. Reading does
// not delete it — call -clearStaleSentinel after staging the upload.
+ (nullable NSDictionary*)readStaleSentinel
    NS_SWIFT_NAME(readStaleSentinel());

// Delete the sentinel file. Call once the previous session's logs
// have been staged for upload (or determined not to be uploadable).
+ (void)clearStaleSentinel
    NS_SWIFT_NAME(clearStaleSentinel());

// Write a sentinel describing the current session. Idempotent —
// overwrites any existing sentinel for the same session.
+ (void)beginCurrentSession
    NS_SWIFT_NAME(beginCurrentSession());

// Remove the current-session sentinel. Call when the app reaches
// scenePhase .background (orderly suspend handoff).
+ (void)endCurrentSession
    NS_SWIFT_NAME(endCurrentSession());

@end

NS_ASSUME_NONNULL_END
