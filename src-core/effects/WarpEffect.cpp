/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <format>
#include <sstream>

#include "../../include/warp-64.xpm"
#include "../../include/warp-48.xpm"
#include "../../include/warp-32.xpm"
#include "../../include/warp-24.xpm"
#include "../../include/warp-16.xpm"

#include "WarpEffect.h"
#include "../render/Effect.h"
#include "../render/RenderBuffer.h"
#include "../models/Model.h"
#include "UtilClasses.h"
#include "../render/DissolveTransitionPattern.h"
#include "UtilFunctions.h"

#include "ispc/WarpFunctions.ispc.h"
#include "Parallel.h"

namespace
{
    template <class T> T CLAMP( const T& lo, const T&val, const T& hi )
    {
        return std::min( hi, std::max( lo, val ) );
    }
    struct LinearInterpolater
    {
       double operator()( double t ) const { return t; }
    };

    template <class T> double interpolate( double x, double loIn, double loOut, double hiIn, double hiOut, const T& interpolater )
    {
       return ( loIn != hiIn )
          ? ( loOut + (hiOut - loOut) * interpolater( (x-loIn)/(hiIn-loIn) ) )
          : ( (loOut + hiOut) / 2 );
    }
    struct ColorBuffer
    {
       ColorBuffer( const xlColorVector& i_cv, int i_w, int i_h ) : cv( i_cv ), w( i_w ), h( i_h ) {}

       xlColor GetPixel( int x, int y ) const
       {
          return ( x >= 0 && x < w && y >= 0 && y <h ) ? cv[y*w + x] : xlBLACK;
       }

       const xlColorVector & cv;
       const int w;
       const int h;
    };
    struct Vec2D
    {
      Vec2D( double i_x = 0., double i_y = 0. ) : x( i_x ), y( i_y ) {}

      Vec2D    operator+( const Vec2D& p ) const { return Vec2D( x + p.x, y + p.y ); }
      Vec2D    operator-( const Vec2D& p ) const { return Vec2D( x - p.x, y - p.y ); }
      double   operator^( const Vec2D& p ) const { return x * p.y - y * p.x; }
      Vec2D    operator*( const double& k ) const { return Vec2D( x*k, y*k ); }
      Vec2D    operator*( const Vec2D& p ) const { return Vec2D( x * p.x, y * p.y ); }
      Vec2D    operator/( const double& k ) const { return *this * ( 1 / k ); }
      Vec2D    operator+=( const Vec2D& p ) { return *this = *this + p; }
      Vec2D    operator-=( const Vec2D& p ) { return *this = *this - p; }
      Vec2D    operator*=( const double& k ) { return *this = *this * k; }
      Vec2D    operator/=( const double& k ) { return *this = *this / k; }
      Vec2D    operator-() const { return Vec2D( -x, -y ); }
      Vec2D    Min( const Vec2D& p ) const { return Vec2D( std::min( x, p.x ), std::min( y, p.y ) ); }
      Vec2D    Max( const Vec2D& p ) const { return Vec2D( std::max( x, p.x ), std::max( y, p.y ) ); }
      double   Len2() const { return x * x + y * y; }
      double   Len() const { return ::sqrt( Len2() ); }
      double   Dist2( const Vec2D& p ) const { return ( *this - p ).Len2(); }
      double   Dist( const Vec2D& p ) const { return ( *this - p ).Len(); }
      Vec2D    Norm() const { return Len() > 0 ? *this / Len() : Vec2D( 0, 0 ); }
      bool     IsNormal() const { return fabs( Len2() - 1 ) < 1e-6; }
      Vec2D    Rotate( const double& fAngle ) const
      {
         float cs = RenderBuffer::cos( fAngle );
         float sn = RenderBuffer::sin( fAngle );
         return Vec2D( x*cs + y * sn, -x * sn + y * cs );
      }
      static Vec2D lerp( const Vec2D& a, const Vec2D& b, double progress )
      {
         double x = a.x + progress * ( b.x - a.x );
         double y = a.y + progress * ( b.y - a.y );
         return Vec2D( x, y );
      }
      double x, y;
    };

   struct WarpEffectParams
   {
      WarpEffectParams( float i_progress, const Vec2D& i_xy, float i_speed, float i_frequency )
         : progress(i_progress), xy(i_xy), speed(i_speed), frequency(i_frequency) {}
      float progress;
      Vec2D xy;
      float speed;
      float frequency;
   };

   constexpr int ISPC_SIMD_PAD = 16;

   struct WarpISPCBuffers {
       ispc::WarpData wdata;
       xlColorVector cvOrig;
       xlColorVector cvDst;
       int count;
       xlColor* dstPixels;

       WarpISPCBuffers(RenderBuffer& rb, const WarpEffectParams& params) {
           dstPixels = rb.GetPixels();
           count = rb.GetPixelCount();
           cvOrig.resize(count + ISPC_SIMD_PAD, xlBLACK);
           memcpy((void*)cvOrig.data(), dstPixels, count * sizeof(xlColor));
           cvDst.resize(count + ISPC_SIMD_PAD, xlBLACK);

           wdata.width     = rb.BufferWi;
           wdata.height    = rb.BufferHt;
           wdata.warpMode  = 0;
           wdata.progress  = params.progress;
           wdata.cx        = (float)params.xy.x;
           wdata.cy        = (float)params.xy.y;
           wdata.speed     = params.speed;
           wdata.frequency = params.frequency;
           wdata.dissolveW = (int)DissolvePatternWidth;
           wdata.dissolveH = (int)DissolvePatternHeight;
       }

       const uint32_t* src32() const { return (const uint32_t*)cvOrig.data(); }
       uint32_t* dst32() { return (uint32_t*)cvDst.data(); }
       const ispc::uint8_t4* src4() const { return (const ispc::uint8_t4*)cvOrig.data(); }
       ispc::uint8_t4* dst4() { return (ispc::uint8_t4*)cvDst.data(); }

       void copyBack() {
           memcpy((void*)dstPixels, cvDst.data(), count * sizeof(xlColor));
       }
   };

   void RenderSampleOn(RenderBuffer& rb, double x, double y)
   {
       int xx = x * (rb.BufferWi - 1);
       int yy = y * (rb.BufferHt - 1);
       xlColor c = rb.GetPixel(xx, yy);

       for (int yyy = 0; yyy < rb.BufferHt; ++yyy)
       {
           for (int xxx = 0; xxx < rb.BufferWi; ++xxx)
           {
               rb.SetPixel(xxx, yyy, c);
           }
       }
   }
}

WarpEffect::WarpEffect(int i) : RenderableEffect(i, "Warp", warp_16_xpm, warp_24_xpm, warp_32_xpm, warp_48_xpm, warp_64_xpm)
{
}

WarpEffect::~WarpEffect()
{
}

std::list<std::string> WarpEffect::CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache)
{
    std::list<std::string> res = RenderableEffect::CheckEffectSettings(settings, media, model, eff, renderCache);

    if (settings.Get("T_CHECKBOX_Canvas", "0") == "0")
    {
        res.push_back(std::format("    WARN: Canvas mode not enabled on a warp effect. Without canvas mode warp won't do anything. Effect: Warp, Model: {}, Start {}", model->GetFullName(), FORMATTIME(eff->GetStartTimeMS())));
    }

    return res;
}

void WarpEffect::Render(Effect *eff, const SettingsMap &SettingsMap, RenderBuffer &buffer)
{
    float progress = buffer.GetEffectTimeIntervalPosition(1.f);

    std::string warpTypeString = SettingsMap.Get( "CHOICE_Warp_Type", "water drops" );
    WarpEffect::WarpType warpType = mapWarpType(warpTypeString);
    std::string warpTreatment = SettingsMap.Get( "CHOICE_Warp_Treatment_APPLYLAST", "constant");
    std::string warpStrCycleCount = SettingsMap.Get( "TEXTCTRL_Warp_Cycle_Count", "1" );
    std::string speedStr = SettingsMap.Get( "TEXTCTRL_Warp_Speed", "20" );
    std::string freqStr = SettingsMap.Get( "TEXTCTRL_Warp_Frequency", "20" );
    int xPercentage = GetValueCurveInt( "Warp_X", 0, SettingsMap, progress, 0, 100, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    int yPercentage = GetValueCurveInt( "Warp_Y", 0, SettingsMap, progress, 0, 100, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    double x = 0.01 * xPercentage;
    double y = 0.01 * yPercentage;
    float speed = std::strtof( speedStr.c_str(), nullptr );
    float frequency = std::strtof( freqStr.c_str(), nullptr );

    WarpEffectParams params( progress, Vec2D( x, y ), speed, frequency );
    if (warpType == WarpEffect::WarpType::SAMPLE_ON) {
        RenderSampleOn(buffer, x, y);
        return;
    }

    int count = buffer.GetPixelCount();
    if (count == 0) return;

    // Adjust params for specific warp types before creating buffers
    if (warpType == WarpEffect::WarpType::WAVY) {
        LinearInterpolater interpolater;
        params.speed = interpolate( params.speed, 0.0,0.5, 40.0,5.0, interpolater );
    } else if (warpType == WarpEffect::WarpType::SINGLE_WATER_DROP) {
        float cycleCount = std::strtof( warpStrCycleCount.c_str(), nullptr );
        float intervalLen = 1.f / cycleCount;
        float scaledProgress = progress / intervalLen;
        float intervalProgress, intervalIndex;
        intervalProgress = std::modf( scaledProgress, &intervalIndex );
        LinearInterpolater interpolater;
        params.progress = interpolate( intervalProgress, 0.0,0.20, 1.0,0.45, interpolater );
    } else if (warpType == WarpEffect::WarpType::CIRCULAR_SWIRL) {
        params.speed = interpolate( params.speed, 0.0, 1.0, 40.0, 9.0, LinearInterpolater() );
    }

    // For transition-style warps, compute progress based on treatment
    bool inward = true;
    if (warpType == WarpEffect::WarpType::RIPPLE ||
        warpType == WarpEffect::WarpType::DISSOLVE ||
        warpType == WarpEffect::WarpType::BANDED_SWIRL ||
        warpType == WarpEffect::WarpType::CIRCLE_REVEAL ||
        warpType == WarpEffect::WarpType::CIRCULAR_SWIRL ||
        warpType == WarpEffect::WarpType::DROP) {
        if (warpTreatment == "constant") {
            float cycleCount = std::strtof(warpStrCycleCount.c_str(), nullptr);
            float intervalLen = 1.f / (2 * cycleCount);
            float scaledProgress = progress / intervalLen;
            float intervalProgress, intervalIndex;
            intervalProgress = std::modf(scaledProgress, &intervalIndex);
            if (int(intervalIndex) % 2)
                intervalProgress = 1.f - intervalProgress;
            params.progress = intervalProgress;
            if (warpType == WarpEffect::WarpType::CIRCULAR_SWIRL ||
                warpType == WarpEffect::WarpType::DROP) {
                params.progress = 1.f - params.progress;
            }
        } else {
            inward = (warpTreatment == "in");
            if (warpType == WarpEffect::WarpType::CIRCULAR_SWIRL) {
                if (warpTreatment == "in")
                    params.progress = 1.f - params.progress;
            } else if (warpType == WarpEffect::WarpType::DROP) {
                if (warpTreatment == "in")
                    params.progress = 1.f - params.progress;
            }
        }
    }

    WarpISPCBuffers buf(buffer, params);

    constexpr int bfBlockSize = 4096;
    int blocks = count / bfBlockSize + 1;
    const ispc::WarpData* wd = &buf.wdata;
    auto s32 = buf.src32(); auto d32 = buf.dst32();
    auto s4 = buf.src4(); auto d4 = buf.dst4();
    auto dissolve = DissolveTransitonPattern;

    parallel_for(0, blocks, [wd, warpType, s32, d32, s4, d4, dissolve, inward, count](int blk) {
        int start = blk * bfBlockSize;
        int end = std::min(start + bfBlockSize, count);
        switch (warpType) {
            case WarpEffect::WarpType::WATER_DROPS:       ispc::WarpWaterDrops(wd, start, end, s32, d32); break;
            case WarpEffect::WarpType::SINGLE_WATER_DROP: ispc::WarpSingleWaterDrop(wd, start, end, s32, d32); break;
            case WarpEffect::WarpType::MIRROR:            ispc::WarpMirror(wd, start, end, s32, d32); break;
            case WarpEffect::WarpType::COPY:              ispc::WarpCopy(wd, start, end, s32, d32); break;
            case WarpEffect::WarpType::FLIP:              ispc::WarpFlip(wd, start, end, s32, d32); break;
            case WarpEffect::WarpType::DROP:              ispc::WarpDrop(wd, start, end, s32, d32, dissolve); break;
            case WarpEffect::WarpType::WAVY:              ispc::WarpWavy(wd, start, end, s32, d32, dissolve); break;
            case WarpEffect::WarpType::RIPPLE:            ispc::WarpRipple(wd, start, end, s4, d4, inward); break;
            case WarpEffect::WarpType::DISSOLVE:          ispc::WarpDissolve(wd, start, end, s4, d4, dissolve, inward); break;
            case WarpEffect::WarpType::CIRCLE_REVEAL:     ispc::WarpCircleReveal(wd, start, end, s4, d4, inward); break;
            case WarpEffect::WarpType::BANDED_SWIRL:      ispc::WarpBandedSwirl(wd, start, end, s4, d4, inward); break;
            case WarpEffect::WarpType::CIRCULAR_SWIRL:    ispc::WarpCircularSwirl(wd, start, end, s4, d4); break;
            default: break;
        }
    });

    buf.copyBack();
}


WarpEffect::WarpType WarpEffect::mapWarpType(const std::string &s) {
    if (s == "water drops") {
        return WarpEffect::WATER_DROPS;
    }
    if (s == "dissolve") {
        return WarpEffect::DISSOLVE;
    }
    if (s == "circle reveal") {
        return WarpEffect::CIRCLE_REVEAL;
    }
    if (s == "banded swirl") {
        return WarpEffect::BANDED_SWIRL;
    }
    if (s == "ripple") {
        return WarpEffect::RIPPLE;
    }
    if (s == "single water drop") {
        return WarpEffect::SINGLE_WATER_DROP;
    }
    if (s == "circular swirl") {
        return WarpEffect::CIRCULAR_SWIRL;
    }
    if (s == "drop") {
        return WarpEffect::DROP;
    }
    if (s == "wavy") {
        return WarpEffect::WAVY;
    }
    if (s == "sample on") {
        return WarpEffect::SAMPLE_ON;
    }
    if (s == "mirror") {
        return WarpEffect::MIRROR;
    }
    if (s == "flip") {
        return WarpEffect::FLIP;
    }
    return WarpEffect::COPY;
}
