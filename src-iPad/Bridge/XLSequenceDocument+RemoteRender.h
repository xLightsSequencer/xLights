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

#import "XLSequenceDocument.h"

NS_ASSUME_NONNULL_BEGIN

// Re-rendering edits from other participants in a shared session, visible
// range first (xLightsShowContext::MarkRemoteChange and friends). Until
// setVisibleTimeRange is called nothing waits: every change renders at once.
// A host that reports the visible range must also report playback starting,
// so nothing waiting in the played range is left stale. Main thread.
@interface XLSequenceDocument (RemoteRender)

// SequenceChange rows (SeqObjectId values) changed over [startMS, endMS]
// (-1 / -1: the whole sequence).
- (void)markRemoteChangeForRows:(NSArray<NSNumber*>*)rows
                         fromMS:(int)startMS
                           toMS:(int)endMS
    NS_SWIFT_NAME(markRemoteChange(rows:fromMS:toMS:));

// The time range the sequencer grid shows.
- (void)setVisibleTimeRangeFromMS:(int)startMS
                             toMS:(int)endMS
    NS_SWIFT_NAME(setVisibleTimeRange(fromMS:toMS:));

// Before the first frame plays (-1 / -1: everything), and when it stops.
- (void)remoteRendersPlaybackStartedFromMS:(int)startMS
                                      toMS:(int)endMS
    NS_SWIFT_NAME(remoteRendersPlaybackStarted(fromMS:toMS:));
- (void)remoteRendersPlaybackStopped;

// Start rendering everything still waiting (before an fseq export).
- (void)flushRemoteRenders;

@end

NS_ASSUME_NONNULL_END
