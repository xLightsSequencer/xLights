#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

// Crash-time capture for the iPad app.
//
// The desktop builds its report from inside wx's fatal-exception hook. iPad had
// no equivalent: the process was gone before anything ran, so a crash report
// carried no backtrace and we waited on Apple's MetricKit payload, which shows
// up a day later with a truncated stack. This writes a record at the moment of
// the fault; the next launch rotates it and the normal diagnostic upload ships
// it.
@interface XLCrashCapture : NSObject

// Installs the fatal-signal handlers and registers the core unhandled-exception
// callback. Call once, early in startup, after the log directory exists.
+ (void)installWithLogsDirectory:(NSString*)logsDir NS_SWIFT_NAME(install(logsDirectory:));

// Moves a record left by a previous run aside so this session's handler has a
// clean slot to write into, and so the packager has a stable name to ship.
// Returns YES when a record was found - i.e. the previous run crashed.
+ (BOOL)rotatePendingRecord NS_SWIFT_NAME(rotatePendingRecord());

// True when a rotated record is waiting to be uploaded. The stale-session
// sentinel catches the usual case, but a crash while backgrounded has already
// cleared it, so the uploader checks this too rather than sitting on the
// record until the next unclean exit.
+ (BOOL)hasPendingRecord NS_SWIFT_NAME(hasPendingRecord());

// Filename the packager picks up (lives directly in Library/Logs).
@property (class, readonly) NSString* pendingRecordFileName;

@end

NS_ASSUME_NONNULL_END
