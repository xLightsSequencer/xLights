#import <QuartzCore/QuartzCore.h>
#include <MetalKit/MetalKit.h>

#include "xLightsTimer.h"


@interface ScreenCallback : NSObject {
}

@property(nonatomic) xLightsTimerDataImpl* data;
@property(nonatomic) int linkIdx;
@end

class API_AVAILABLE(macos(14.0)) xLightsTimerDataImpl {
public:
    xLightsTimerDataImpl(xLightsTimer *t);
    ~xLightsTimerDataImpl();
    
    void setupScreens();
    void releaseScreens();
    
    void stop();
    void start(int ms);
    
    void trigger(int idx, CADisplayLink *l);
    void checkTrigger(int idx);
    
    int curInterval = 0;
    xLightsTimer *timer;
    
    std::vector<CADisplayLink *> links;
    std::vector<double> minimums;
    std::vector<double> maxes;
    std::vector<double> nextPresent;
    std::vector<ScreenCallback *> callbacks;
    std::vector<bool> used;
    int triggerLink = 0;
    int maxTriggerLink = 0;
    
    double nextTriggerTimestamp = 0;
};


@implementation ScreenCallback
- (void)displayLinkCallback:(CADisplayLink *)link  API_AVAILABLE(macos(14.0)) {
    @autoreleasepool {
        _data->trigger(_linkIdx, link);
    }
}
@end


xLightsTimerDataImpl::xLightsTimerDataImpl(xLightsTimer *t) : timer(t) {
}
xLightsTimerDataImpl::~xLightsTimerDataImpl() {
    releaseScreens();
    for (auto c : callbacks) {
        [c release];
    }
}
void xLightsTimerDataImpl::releaseScreens() {
    if (@available(macOS 14.0, *)) {
        for (auto link : links) {
            [link setPaused:true];
            [link invalidate];
            [link release];
        }
        links.clear();
        maxes.clear();
        nextPresent.clear();
        used.clear();
    }
}
void xLightsTimerDataImpl::setupScreens() {
    if (@available(macOS 14.0, *)) {
        releaseScreens();
        
        NSArray<NSScreen *> *screens = [NSScreen screens];
        int maxFPS = 0;
        for (NSScreen *scr in screens) {
            while (links.size() >= callbacks.size()) {
                ScreenCallback *callback = [[[ScreenCallback alloc] init] retain];
                [callback setData:this];
                [callback setLinkIdx:callbacks.size()];
                callbacks.push_back(callback);
            }

            CADisplayLink *link = [scr displayLinkWithTarget:callbacks[links.size()] selector:@selector(displayLinkCallback:)];
            [link addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSRunLoopCommonModes];
            [link setPaused:true];
            [link retain];
            links.push_back(link);
            double mrefresh = [scr minimumRefreshInterval];
            maxes.push_back(1.0l / mrefresh);
            mrefresh = [scr maximumRefreshInterval];
            minimums.push_back(1.0l / mrefresh);
            int m = (int)maxes.back();
            if (m > maxFPS) {
                maxFPS = m;
            }
            nextPresent.push_back(0);
            used.push_back(false);
        }
        for (int x = 0; x < maxes.size(); x++) {
            int m = (int)maxes[x];
            if (m == maxFPS) {
                maxTriggerLink = x;
            }
        }
    }
}


void xLightsTimerDataImpl::stop() {
    for (auto link : links) {
        [link setPaused:true];
    }
    releaseScreens();
    curInterval = 0;
}
void xLightsTimerDataImpl::start(int ms) {
    setupScreens();
    curInterval = ms;
    triggerLink = maxTriggerLink;
    nextTriggerTimestamp = -1;
    for (int x = 0; x < links.size(); x++) {
        CADisplayLink *link = links[x];
        /*
        if (maxes[x] != minimums[x]) {
            //ProMotion display, we'll set a preferred rate
            CAFrameRateRange rng;
            rng.preferred = 1000.0f / ((float)ms);
            if (rng.preferred > maxes[x]) {
                rng.preferred = maxes[x];
                OkAsTrigger = false;
            }
            if (rng.preferred < minimums[x]) {
                rng.preferred = minimums[x];
            }
            rng.minimum = rng.preferred;
            rng.maximum = maxes[x];
            [link setPreferredFrameRateRange:rng];
            triggerLink = x;
        }
        */
        [link setPaused:false];
    }
}
void xLightsTimerDataImpl::trigger(int idx, CADisplayLink *link) {
    nextPresent[idx] = link.targetTimestamp;
    /*
    if (link.targetTimestamp >= nextTriggerTimestamp) {
        if (nextTriggerTimestamp < 0) {
            nextTriggerTimestamp = link.targetTimestamp + ((double)curInterval) / 500.0l;
        } else {
            nextTriggerTimestamp += ((double)curInterval) / 1000.0l;
        }
        timer->Notify();
    }
    */
    //timer->Notify();
    if (idx == triggerLink) {
        timer->Notify();
    }
} 
void xLightsTimerDataImpl::checkTrigger(int idx) {
    if (!used[idx]) {
        used[idx] = true;
        int bestTrigger = idx;
        double max = maxes[idx];
        for (int x = 0; x < nextPresent.size(); x++) {
            if (used[x] && maxes[x] > max) {
                max = maxes[x];
                bestTrigger = x;
            }
        }
        triggerLink = bestTrigger;
    }
}



xLightsTimer::xLightsTimer() {
    data = nullptr;
    if (@available(macOS 14.0, *)) {
        data = new xLightsTimerDataImpl(this);
    }
}
xLightsTimer::~xLightsTimer() {
    if (data) {
        delete data;
    }
}
void xLightsTimer::Stop() {
    if (data) {
        data->stop();
    }
    wxTimer::Stop();
}

bool xLightsTimer::Start(int time, bool oneShot, const std::string& name) {
    if (!oneShot && data) {
        data->start(time);
        return true;
    }
    return wxTimer::Start(time, oneShot);
}
int xLightsTimer::GetInterval() const {
    if (data) {
        return data->curInterval;
    }
    return wxTimer::GetInterval();
}

void xLightsTimer::Notify() {
    wxTimer::Notify();
}
void xLightsTimer::DoSendTimer() {
    
}
void xLightsTimer::SetName(const std::string& name) {
    _name = name;
}
double xLightsTimer::presentTimeForScreen(int i) const {
    if (data && i < data->nextPresent.size()) {
        if (!data->used[i]) {
            data->checkTrigger(i);
        }
        return data->nextPresent[i];
    }
    return -1;
}

