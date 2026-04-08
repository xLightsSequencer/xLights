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

   // ISPC warp mode integers (must match WarpFunctions.ispc defines)
   enum WarpISPCMode {
       WARP_ISPC_WATER_DROPS       = 0,
       WARP_ISPC_RIPPLE_IN         = 1,
       WARP_ISPC_RIPPLE_OUT        = 2,
       WARP_ISPC_DISSOLVE_IN       = 3,
       WARP_ISPC_DISSOLVE_OUT      = 4,
       WARP_ISPC_CIRCLE_REVEAL_IN  = 5,
       WARP_ISPC_CIRCLE_REVEAL_OUT = 6,
       WARP_ISPC_BANDED_SWIRL_IN   = 7,
       WARP_ISPC_BANDED_SWIRL_OUT  = 8,
       WARP_ISPC_SINGLE_WATER_DROP = 9,
       WARP_ISPC_CIRCULAR_SWIRL    = 10,
       WARP_ISPC_DROP              = 11,
       WARP_ISPC_WAVY              = 12,
       WARP_ISPC_MIRROR            = 13,
       WARP_ISPC_COPY              = 14,
       WARP_ISPC_FLIP              = 15,
   };

   void RenderPixelTransformISPC(int ispcMode, RenderBuffer& rb, const WarpEffectParams& params)
   {
       xlColor* pixels = rb.GetPixels();
       int count = rb.GetPixelCount();
       xlColorVector cvOrig(pixels, pixels + count);

       ispc::WarpData wdata;
       wdata.width     = rb.BufferWi;
       wdata.height    = rb.BufferHt;
       wdata.warpMode  = ispcMode;
       wdata.progress  = params.progress;
       wdata.cx        = (float)params.xy.x;
       wdata.cy        = (float)params.xy.y;
       wdata.speed     = params.speed;
       wdata.frequency = params.frequency;
       wdata.dissolveW = (int)DissolvePatternWidth;
       wdata.dissolveH = (int)DissolvePatternHeight;

       constexpr int bfBlockSize = 4096;
       int blocks = count / bfBlockSize + 1;
       parallel_for(0, blocks, [&wdata, &cvOrig, &rb, count](int blk) {
           int start = blk * bfBlockSize;
           int end = std::min(start + bfBlockSize, count);
           ispc::WarpEffectISPC(
               &wdata, start, end,
               (const ispc::uint8_t4*)cvOrig.data(),
               (ispc::uint8_t4*)rb.GetPixels(),
               DissolveTransitonPattern);
       });
   }

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
    if ( warpType == WarpEffect::WarpType::WATER_DROPS) {
        RenderPixelTransformISPC( WARP_ISPC_WATER_DROPS, buffer, params );
    } else if (warpType == WarpEffect::WarpType::SAMPLE_ON) {
        RenderSampleOn(buffer, x, y);
    } else if (warpType == WarpEffect::WarpType::WAVY) {
        LinearInterpolater interpolater;
        params.speed = interpolate( params.speed, 0.0,0.5, 40.0,5.0, interpolater );
        RenderPixelTransformISPC( WARP_ISPC_WAVY, buffer, params );
    } else if (warpType == WarpEffect::WarpType::MIRROR) {
        RenderPixelTransformISPC( WARP_ISPC_MIRROR, buffer, params );
    } else if (warpType == WarpEffect::WarpType::COPY) {
        RenderPixelTransformISPC( WARP_ISPC_COPY, buffer, params );
    } else if (warpType == WarpEffect::WarpType::FLIP) {
        RenderPixelTransformISPC( WARP_ISPC_FLIP, buffer, params );
    } else if (warpType == WarpEffect::WarpType::SINGLE_WATER_DROP) {
        float cycleCount = std::strtof( warpStrCycleCount.c_str(), nullptr );
        float intervalLen = 1.f / cycleCount;
        float scaledProgress = progress / intervalLen;
        float intervalProgress, intervalIndex;
        intervalProgress = std::modf( scaledProgress, &intervalIndex );

        LinearInterpolater interpolater;
        float interpolatedProgress = interpolate( intervalProgress, 0.0,0.20, 1.0,0.45, interpolater );

        params.progress = interpolatedProgress;
        RenderPixelTransformISPC( WARP_ISPC_SINGLE_WATER_DROP, buffer, params );
    } else {
        int ispcMode = -1;
        // the other warps were originally intended as transitions in or out... for constant
        // treatment, we'll just cycle between progress of [0,1] and [1,0]. "constant" wasn't
        // a very good description, maybe back-and-forth or something would be more accurate
        if (warpTreatment == "constant") {
            float cycleCount = std::strtof(warpStrCycleCount.c_str(), nullptr);
            float intervalLen = 1.f / (2 * cycleCount );
            float scaledProgress = progress / intervalLen;
            float intervalProgress, intervalIndex;
            intervalProgress = std::modf( scaledProgress, &intervalIndex );
            if (int(intervalIndex) % 2) {
                intervalProgress = 1.f - intervalProgress;
            }
            params.progress = intervalProgress;
            if (warpType == WarpEffect::WarpType::RIPPLE) {
                ispcMode = WARP_ISPC_RIPPLE_IN;
            } else if (warpType == WarpEffect::WarpType::DISSOLVE) {
                ispcMode = WARP_ISPC_DISSOLVE_IN;
            } else if (warpType == WarpEffect::WarpType::BANDED_SWIRL) {
                ispcMode = WARP_ISPC_BANDED_SWIRL_IN;
            } else if (warpType == WarpEffect::WarpType::CIRCLE_REVEAL) {
                ispcMode = WARP_ISPC_CIRCLE_REVEAL_IN;
            } else if (warpType == WarpEffect::WarpType::CIRCULAR_SWIRL) {
                params.progress = 1. - params.progress;
                ispcMode = WARP_ISPC_CIRCULAR_SWIRL;
            } else if (warpType == WarpEffect::WarpType::DROP) {
                params.progress = 1. - params.progress;
                ispcMode = WARP_ISPC_DROP;
            }
        } else {
            if (warpType == WarpEffect::WarpType::RIPPLE) {
                ispcMode = (warpTreatment == "in") ? WARP_ISPC_RIPPLE_IN : WARP_ISPC_RIPPLE_OUT;
            } else if (warpType == WarpEffect::WarpType::DISSOLVE) {
                ispcMode = (warpTreatment == "in") ? WARP_ISPC_DISSOLVE_IN : WARP_ISPC_DISSOLVE_OUT;
            } else if (warpType == WarpEffect::WarpType::BANDED_SWIRL) {
                ispcMode = (warpTreatment == "in") ? WARP_ISPC_BANDED_SWIRL_IN : WARP_ISPC_BANDED_SWIRL_OUT;
            } else if (warpType == WarpEffect::WarpType::CIRCLE_REVEAL) {
                ispcMode = (warpTreatment == "in") ? WARP_ISPC_CIRCLE_REVEAL_IN : WARP_ISPC_CIRCLE_REVEAL_OUT;
            } else if (warpType == WarpEffect::WarpType::CIRCULAR_SWIRL) {
                ispcMode = WARP_ISPC_CIRCULAR_SWIRL;
            } else if (warpType == WarpEffect::WarpType::DROP) {
                if (warpTreatment == "in") {
                    params.progress = 1. - params.progress;
                }
                ispcMode = WARP_ISPC_DROP;
            }
        }

        if (warpType == WarpEffect::WarpType::CIRCULAR_SWIRL) {
            params.speed = interpolate( params.speed, 0.0, 1.0, 40.0, 9.0, LinearInterpolater() );
            if (warpTreatment == "in") {
                params.progress = 1. - params.progress;
            }
        }

        if (ispcMode >= 0) {
            RenderPixelTransformISPC( ispcMode, buffer, params );
        }
    }
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
