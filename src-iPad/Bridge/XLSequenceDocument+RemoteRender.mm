/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#import "XLSequenceDocument+RemoteRender.h"

#include "iPadRenderContext.h"

#include <vector>

@implementation XLSequenceDocument (RemoteRender)

- (iPadRenderContext*)remoteRenderContext {
    return static_cast<iPadRenderContext*>([self renderContext]);
}

- (void)markRemoteChangeForRows:(NSArray<NSNumber*>*)rows fromMS:(int)startMS toMS:(int)endMS {
    iPadRenderContext* ctx = [self remoteRenderContext];
    if (ctx == nullptr) {
        return;
    }
    std::vector<SeqObjectId> ids;
    ids.reserve(rows.count);
    for (NSNumber* n in rows) {
        ids.push_back((SeqObjectId)n.unsignedLongLongValue);
    }
    ctx->MarkRemoteChange(ids, startMS, endMS);
}

- (void)setVisibleTimeRangeFromMS:(int)startMS toMS:(int)endMS {
    if (iPadRenderContext* ctx = [self remoteRenderContext]) {
        ctx->SetVisibleTimeRange(startMS, endMS);
    }
}

- (void)remoteRendersPlaybackStartedFromMS:(int)startMS toMS:(int)endMS {
    if (iPadRenderContext* ctx = [self remoteRenderContext]) {
        ctx->RemoteRendersPlaybackStarted(startMS, endMS);
    }
}

- (void)remoteRendersPlaybackStopped {
    if (iPadRenderContext* ctx = [self remoteRenderContext]) {
        ctx->RemoteRendersPlaybackStopped();
    }
}

- (void)flushRemoteRenders {
    if (iPadRenderContext* ctx = [self remoteRenderContext]) {
        ctx->FlushRemoteRenders();
    }
}

@end
