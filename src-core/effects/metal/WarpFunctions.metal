#include <metal_stdlib>
using namespace metal;

#include "MetalEffectDataTypes.h"


constant int DissolvePatternWidth = 512;
constant int DissolvePatternHeight = 512;
constant float PI = 3.14159;


float dissolveTex(float s, float t, const device uchar* dissolvePattern) {
    s = clamp(s, 0.0, 1.0);
    t = clamp(t, 0.0, 1.0);

    int x = int( s * (DissolvePatternWidth - 1 ) );
    int y = int( t * (DissolvePatternHeight -1 ) );

    float val = dissolvePattern[y * DissolvePatternWidth + x];
    val = val / 255.0f;
    return val;
}
float2 noiseVec(float2 p, const device uchar* dissolvePattern) {
    float noiseColor = dissolveTex(p.x, p.y, dissolvePattern);
    return float2( noiseColor, noiseColor);
}
float lerp(float a, float b, float progress) {
    return a + progress * ( b - a );
}
uchar lerp(uchar a, uchar b, float progress) {
    return round(float(a) + progress * (float(b) - float(a)));
}
uchar4 lerp(uchar4 a, uchar4 b, float progress) {
    return {lerp(a.r, b.r, progress), lerp(a.g, b.g, progress), lerp(a.b, b.b, progress), 255};
}
float noise(float2 p, const device uchar* dissolvePattern) {
    float2 i, f;
    float xint, yint;
    f.x = modf(p.x, xint);
    f.y = modf(p.y, yint);
    i = {xint, yint};

    float2 u( f * f * (3. - (2. * f) ) );

    float2 aa( noiseVec( i + float2(0.,0.), dissolvePattern ) );
    float2 bb( f - float2(0.,0.) );
    float2 cc( noiseVec( i + float2(1.,0.), dissolvePattern ) );
    float2 dd( f - float2(1.,0.) );
    float2 ee( noiseVec( i + float2(0.,1.), dissolvePattern ) );
    float2 ff( f - float2(0.,1.) );
    float2 gg( noiseVec( i + float2(1.,1.), dissolvePattern ) );
    float2 hh( f - float2(1.,1.) );

    float ab = dot( aa, bb );
    float cd = dot( cc, dd );
    float ef = dot( ee, ff );
    float gh = dot( gg, hh );

    float foo = lerp( ab, cd, u.x );
    float bar = lerp( ef, gh, u.x );

    return lerp( foo, bar, u.y );
}










kernel void WarpEffectMirror(constant WarpData &data,
                             device uchar4* result,
                             uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    
    if (index.x <= data.xPos && index.y <= data.yPos) return;
    
    int x = index.x;
    int y = index.y;
    if (x > (int)data.xPos) {
        x = data.xPos - (index.x - data.xPos);
    }
    if (y > (int)data.yPos) {
        y = data.yPos - (index.y - data.yPos);
    }
    int didx = index.y * data.width + index.x;
    if (x < 0 || y < 0) {
        result[didx] = {0, 0, 0, 255};
    } else {
        int sidx = y * data.width + x;
        result[didx] = result[sidx];
    }
}
kernel void WarpEffectCopy(constant WarpData &data,
                           device uchar4* result,
                           const device uchar4* src,
                           uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    
    if (index.x <= data.xPos && index.y <= data.yPos) return;
    
    uint x = index.x;
    uint y = index.y;
    if (x > data.xPos) {
        x = index.x - data.xPos;
    }
    if (y > data.yPos) {
        y = index.y - data.yPos;
    }
    uint didx = index.y * data.width + index.x;
    if (x < 0 || y < 0) {
        result[didx] = {0, 0, 0, 255};
    } else {
        uint sidx = y * data.width + x;
        result[didx] = src[sidx];
    }
}
kernel void WarpEffectSampleOn(constant WarpData &data,
                               device uchar4* result,
                               uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;

    uint didx = index.y * data.width + index.x;
    uint sidx = data.yPos * data.width + data.xPos;
    if (sidx != didx) {
        result[didx] = result[sidx];
    }
}



int tex2DClamp(float2 uv, int width, int height) {
    uv.x = clamp(uv.x, 0.0, 1.0);
    uv.y = clamp(uv.y, 0.0, 1.0);
    int y = round(uv.y * (height - 1));
    int x = round(uv.x * (width - 1));
    uint sidx = y * width + x;
    return sidx;
}
int tex2DBlack(float2 uv, int width, int height) {
    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {
        return -1;
    }
    int y = round(uv.y * (height - 1));
    int x = round(uv.x * (width - 1));
    uint sidx = y * width + x;
    return sidx;
}


kernel void WarpEffectWavy(constant WarpData &data,
                           device uchar4* result,
                           const device uchar4* src,
                           const device uchar* dissolvePattern,
                           uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    uint didx = index.y * data.width + index.x;

    float2 uv = float2(float(index.x) / float(data.width), float(index.y) / float(data.height));
    float time = data.speed * data.progress;

    uv.x += 0.4 * noise(time + 0.3 * uv, dissolvePattern);
    uv.y += 0.5 * noise(time + 0.5 * uv, dissolvePattern);
    
    int sidx = tex2DBlack(uv, data.width, data.height);
    if (sidx < 0) {
        result[didx] = {0, 0, 0, 255};
    } else {
        result[didx] = src[sidx];
    }
}


float getDropletHeight(const float2 uv, const float2 dropletPosition, float time) {
    const float dropletExpandSpeed = 1.5;
    const float dropletHeightFactor = 0.3f;
    const float dropletRipple = /*80.0*/60.0; // possible progress

    float decayRate = 0.5; // smaller = faster drops
    float dropletStrength = 1.0; // larger = bigger impact (0.5 min)
    float dropletStrengthBias = 0.6f;
    float dropFraction = time / decayRate;
    float dummy;
    dropFraction = modf(dropFraction, dummy);

    float ringRadius = dropletExpandSpeed * dropFraction * dropletStrength - dropletStrengthBias;
    float distanceToDroplet = length(float2(uv - dropletPosition));

   float dropletHeight = distanceToDroplet > ringRadius ? 0.0 : distanceToDroplet;
   dropletHeight = cos(PI + (dropletHeight - ringRadius) * dropletRipple * dropletStrength) * 0.5 + 0.5;
   dropletHeight *= 1.0 - dropFraction;
   dropletHeight *= distanceToDroplet > ringRadius ? 0.0 : distanceToDroplet / ringRadius;

   return (1.0 - (cos(dropletHeight * PI) + 1.0) * 0.5) * dropletHeightFactor;
}

kernel void WarpEffectSingleWaterDrop(constant WarpData &data,
                                      device uchar4* result,
                                      const device uchar4* src,
                                      uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    uint didx = index.y * data.width + index.x;

    float2 uv = float2(float(index.x) / float(data.width), float(index.y) / float(data.height));
    float2 paramsXY = float2(float(data.xPos) / float(data.width), float(data.yPos) / float(data.height));
    float2 pos2(uv - paramsXY);
    float2 pos2n = normalize(pos2);

    float dh = getDropletHeight( uv - float2( 0.5,0.5 ), paramsXY - float2( 0.5, 0.5 ), data.progress );
    float2 uv2 = -pos2n * dh / ( 1.0 + 3.0 * length(pos2));

    int sidx = tex2DClamp({uv.x + uv2.x, uv.y + uv2.y}, data.width, data.height);
    result[didx] = src[sidx];
}

float genWave( float len, float speed, float time ) {
   float wave = sin(speed * PI * len + time);
   wave = ( wave + 1.0 ) * 0.5;
   wave -= 0.3f;
   wave *= wave * wave;
   return wave;
}
kernel void WarpEffectWaterDrops(constant WarpData &data,
                                 device uchar4* result,
                                 const device uchar4* src,
                                 uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    uint didx = index.y * data.width + index.x;

    float2 uv = float2(float(index.x) / float(data.width), float(index.y) / float(data.height));
    float2 paramsXY = float2(float(data.xPos) / float(data.width), float(data.yPos) / float(data.height));
    float2 pos2(uv - paramsXY);
    float2 pos2n = normalize(pos2);
    float len = length(pos2);
    float wave = genWave(len, data.speed, -data.progress * 35.0);

    float2 uv2(-pos2n * wave / ( 1.0 + 5.0 * len ));
    float2 st(uv.x + uv2.x, uv.y + uv2.y);
    int sidx = tex2DClamp(st, data.width, data.height);
    result[didx] = src[sidx];
}



kernel void WarpEffectDissolve(constant WarpData &data,
                               device uchar4* result,
                               const device uchar* dissolvePattern,
                               uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    uint didx = index.y * data.width + index.x;

    float dissolveColor = dissolveTex(float(index.x) / float(data.width), float(index.y) / float(data.height), dissolvePattern);
    if (data.direction) {
        if (dissolveColor <= data.progress) {
            result[didx] = {0, 0, 0, 255};
        }
    } else {
        if (dissolveColor > data.progress) {
            result[didx] = {0, 0, 0, 255};
        }
    }
}

kernel void WarpEffectRipple(constant WarpData &data,
                             device uchar4* result,
                             const device uchar4* src,
                             uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    uint didx = index.y * data.width + index.x;

    float2 uv = float2(float(index.x) / float(data.width), float(index.y) / float(data.height));
    float2 paramsXY = float2(float(data.xPos) / float(data.width), float(data.yPos) / float(data.height));
    
    const float amplitude = 0.15;

    float2 toUV( uv.x - paramsXY.x, uv.y - paramsXY.y );
    float distanceFromCenter = length(toUV);
    float2 normToUV = toUV / distanceFromCenter;

    float wave = cos(data.frequency * distanceFromCenter - data.speed * data.progress);
    float offset = data.progress * wave * amplitude;

    float2 newUV = paramsXY + normToUV * ( distanceFromCenter + offset );
    int sidx = tex2DClamp(newUV, data.width, data.height);
    if (data.direction) {
        result[didx] = lerp(src[didx], src[sidx], data.progress);
    } else {
        result[didx] = lerp(src[sidx], src[didx], data.progress);
    }
}

kernel void WarpEffectDrop(constant WarpData &data,
                           device uchar4* result,
                           const device uchar4* src,
                           const device uchar* dissolvePattern,
                           uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    uint didx = index.y * data.width + index.x;
    
    
    float2 uv = float2(float(index.x) / float(data.width), float(index.y) / float(data.height));
    const float notSoRandomY = 0.16;
    float noise = dissolveTex(uv.x, notSoRandomY, dissolvePattern);
    int sidx = tex2DClamp({uv.x, uv.y + noise * data.progress}, data.width, data.height);
    result[didx] = src[sidx];
}


kernel void WarpEffectCircleSwirl(constant WarpData &data,
                                  device uchar4* result,
                                  const device uchar4* src,
                                  uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    uint didx = index.y * data.width + index.x;
    
    float2 uv = float2(float(index.x) / float(data.width), float(index.y) / float(data.height));
    float2 paramsXY = float2(float(data.xPos) / float(data.width), float(data.yPos) / float(data.height));
    
    float2 dir( uv - paramsXY );
    float len = length(dir);

    float radius = (1. - data.progress) * 0.70710678;
    if (len < radius) {
        float angle = -data.speed * len * data.progress * PI;
        float cs = cos(angle);
        float sn = sin(angle);
        float2 rotated( dir.x * cs + dir.y * sn, -dir.x * sn + dir.y * cs );
        float2 scaled( rotated * (1. - data.progress) + paramsXY );
        float nprogress = 1.0 - data.progress;
        float x = paramsXY.x + nprogress * ( scaled.x - paramsXY.x );
        float y = paramsXY.y + nprogress * ( scaled.y - paramsXY.y );
        float2 newUV( x, y);
        int sidx = tex2DClamp(newUV, data.width, data.height);
        result[didx] = src[sidx];
    } else {
        result[didx] = {0, 0, 0, 255};
    }
}
kernel void WarpEffectCircleReveal(constant WarpData &data,
                                   device uchar4* result,
                                   uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    uint didx = index.y * data.width + index.x;
    
    float2 uv = float2(float(index.x) / float(data.width), float(index.y) / float(data.height));
    float2 paramsXY = float2(float(data.xPos) / float(data.width), float(data.yPos) / float(data.height));

    const float FuzzyAmount = 0.04f;
    const float CircleSize = 0.60f;

    float progress = data.progress;
    if (data.direction) {
        progress = 1.0 - data.progress;
    }
    float radius = -FuzzyAmount + progress * (CircleSize + 2.0 * FuzzyAmount);
    float fromCenter = length(uv - paramsXY);
    float distFromCircle = fromCenter - radius;

    float p = clamp((distFromCircle + FuzzyAmount) / (2.0 * FuzzyAmount), 0.0, 1.0 );
    result[didx] = lerp(result[didx], {0, 0, 0, 255}, p);
}
kernel void WarpEffectBandedSwirl(constant WarpData &data,
                                  device uchar4* result,
                                  const device uchar4* src,
                                  uint2 index [[thread_position_in_grid]]) {
    if (index.x >= data.width) return;
    if (index.y >= data.height) return;
    uint didx = index.y * data.width + index.x;
     
    float2 uv = float2(float(index.x) / float(data.width), float(index.y) / float(data.height));
    float2 paramsXY = float2(float(data.xPos) / float(data.width), float(data.yPos) / float(data.height));

    const float TwistAmount = 1.6;
    float progress = data.progress;
    if (!data.direction) {
        progress = 1.0 - data.progress;
    }

    float2 toUV( uv - paramsXY );
    float distanceFromCenter = length(toUV);
    float2 normToUV(toUV / distanceFromCenter);
    float angle = atan2(normToUV.y, normToUV.x);

    angle += sin(distanceFromCenter * data.frequency) * TwistAmount * progress;
    float2 newUV(cos( angle ), sin( angle ));
    newUV = newUV * distanceFromCenter + paramsXY;
    int c2 = tex2DClamp(newUV, data.width, data.height);
    if (data.direction) {
        result[didx] = lerp(src[c2], src[didx], data.progress );
    } else {
        result[didx] = lerp(src[didx], src[c2], data.progress );
    }
 }
