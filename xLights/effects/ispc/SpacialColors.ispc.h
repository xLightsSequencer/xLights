#pragma once

struct SpacialData {
    uint32 colorIdx;
    float x;
    float y;
    float r;
    
    uint32 result;
};


extern "C" uniform uint32 getSpacialColorForGang(void * uniform buffer, uniform float xcenter, uniform float ycenter, uniform float maxradius, uniform int activeLanes, uniform int programCount, SpacialData d[]);


inline uint8<4> getSpacialColor(void * uniform buffer, uint32 colorIdx, uniform float xcenter, uniform float ycenter, float x, float y, float r, uniform float maxradius) {
    SpacialData d = { colorIdx, x, y, r, 0};
    getSpacialColorForGang(buffer, xcenter, ycenter, maxradius, lanemask(), programCount, &d);
    return d.result;
}

