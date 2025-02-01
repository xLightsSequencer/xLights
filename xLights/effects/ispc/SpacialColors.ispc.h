#pragma once

struct SpacialData {
    uint32 colorIdx;
    float x;
    float y;
    float r;
    
    uint32 result;
};


extern "C" uniform int32 getSpacialColorForGang(void * uniform buffer, uniform float xcenter, uniform float ycenter, uniform float maxradius, uniform int activeLanes, uniform int programCount, SpacialData d[]);


inline uint8<4> getSpacialColor(void * uniform buffer, uint32 colorIdx, uniform float xcenter, uniform float ycenter, float x, float y, float r, uniform float maxradius) {
    SpacialData d = { colorIdx, x, y, r, 0};
    getSpacialColorForGang(buffer, xcenter, ycenter, maxradius, lanemask(), programCount, &d);
    uint8<4> result;
    result.r = (d.result & 0xFF);
    result.g = ((d.result >> 8) & 0xFF);
    result.b = ((d.result >> 16) & 0xFF);
    result.a = ((d.result >> 24) & 0xFF);
    return result;
}

