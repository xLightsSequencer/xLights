#include "WarpEffect.h"
#include "OnPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../xLightsMain.h" //xLightsFrame
#include "../OpenGLShaders.h"

#include <math.h>

static const std::string TEXTCTRL_Eff_On_Start("TEXTCTRL_Eff_On_Start");
static const std::string TEXTCTRL_Eff_On_End("TEXTCTRL_Eff_On_End");
static const std::string CHECKBOX_On_Shimmer("CHECKBOX_On_Shimmer");
static const std::string TEXTCTRL_On_Cycles("TEXTCTRL_On_Cycles");

#include "../../include/On.xpm"

namespace
{
   template <class T> T CLAMP( const T& lo, const T&val, const T& hi )
   {
      return std::min( hi, std::max( lo, val ) );
   }

   struct Vec2D
   {
      Vec2D( double i_x = 0., double i_y = 0. ) : x( i_x ), y( i_y ) {}

      double   operator*( const Vec2D& p ) const { return x * p.x + y * p.y; }
      Vec2D    operator+( const Vec2D& p ) const { return Vec2D( x + p.x, y + p.y ); }
      Vec2D    operator-( const Vec2D& p ) const { return Vec2D( x - p.x, y - p.y ); }
      double   operator^( const Vec2D& p ) const { return x * p.y - y * p.x; }
      Vec2D    operator*( const double& k ) const { return Vec2D( x*k, y*k ); }
      Vec2D    operator/( const double& k ) const { return *this * ( 1 / k ); }
      Vec2D    operator+=( const Vec2D& p ) { return *this = *this + p; }
      Vec2D    operator-=( const Vec2D& p ) { return *this = *this - p; }
      Vec2D    operator*=( const double& k ) { return *this = *this * k; }
      Vec2D    operator/=( const double& k ) { return *this = *this / k; }
      Vec2D    operator-() const { return Vec2D( -x, -y ); }
      Vec2D    Min( const Vec2D& p ) const { return Vec2D( std::min( x, p.x ), std::min( y, p.y ) ); }
      Vec2D    Max( const Vec2D& p ) const { return Vec2D( std::max( x, p.x ), std::max( y, p.y ) ); }
      double   Len2() const { return *this * *this; }
      double   Len() const { return ::sqrt( Len2() ); }
      double   Dist2( const Vec2D& p ) const { return ( *this - p ).Len2(); }
      double   Dist( const Vec2D& p ) const { return ( *this - p ).Len(); }
      Vec2D    Norm() const { return Len() > 0 ? *this / Len() : Vec2D( 0, 0 ); }
      BOOL     IsNormal() const { return fabs( Len2() - 1 ) < 1e-6; }
      Vec2D    Rotate( const double& fAngle ) const
      {
         double cs = ::cos( fAngle );
         double sn = ::sin( fAngle );
         return Vec2D( x*cs + y * sn, -x * sn + y * cs );
      }
      Vec2D    Rotate( const Vec2D& p, const double& fAngle ) const { return ( *this - p ).Rotate( fAngle ) + p; }
      Vec2D    Rotate90() const { return Vec2D( y, -x ); }

      double x, y;
   };

   xlColor lerp( const xlColor& a, const xlColor& b, double progress )
   {
      double progressInv = 1 - progress;
      double red   = progress * b.red   + progressInv * a.red;
      double green = progress * b.green + progressInv * a.green;
      double blue  = progress * b.blue  + progressInv * a.blue;

      return xlColor( uint8_t( red ), uint8_t( green ), uint8_t( blue ) );
   }

   struct ColorBuffer
   {
      ColorBuffer( const xlColorVector& i_cv, int i_w, int i_h ) : cv( i_cv ), w( i_w ), h( i_h ) {}

      xlColor GetPixel( int x, int y ) const
      {
         return ( x >= 0 && x < w && y >= 0 && y <h ) ? cv[y*w + x] : xlBLACK;
      }

      const xlColorVector &cv;
      const int w;
      const int h;
   };

   xlColor tex2D( const ColorBuffer& cb, double s, double t )
   {
      s = CLAMP( 0., s, 1. );
      t = CLAMP( 0., t, 1. );

      int x = int( s * ( cb.w - 1 ) );
      int y = int( t * ( cb.h - 1 ) );

      return cb.GetPixel( x, y );
   }

   xlColor rippleIn( const ColorBuffer& cb, double s, double t, double progress )
   {
      const Vec2D center( 0.5, 0.5 );
      const double frequency = 20;
      const double speed = 10;
      const double amplitude = 0.05;

      Vec2D toUV( s - center.x, t - center.y );
      double distanceFromCenter = toUV.Len();
      Vec2D normToUV = toUV / distanceFromCenter;

      double wave = /*::cos*/RenderBuffer::cos( frequency * distanceFromCenter - speed * progress );
      double offset2 = ( 1 - progress ) * wave * amplitude;

      Vec2D newUV2 = center + normToUV * ( distanceFromCenter + offset2 );

      xlColor c1 = xlBLACK;
      xlColor c2 = tex2D( cb, newUV2.x, newUV2.y );

      return lerp( c1, c2, progress );
   }

   xlColor radialBlurOut( const ColorBuffer& cb, double s, double t, double progress )
   {
      const Vec2D center( 0.5, 0.5 );
      Vec2D uv( s, t );
      Vec2D toUV( uv - center );

      double prog = progress * 0.10;
      const int Count = 5;
      double red = 0, green = 0, blue = 0, alpha = 0;
      for ( int i = 0; i < Count; ++i )
      {
          Vec2D foo = uv - toUV * prog * i;
          xlColor rgba = tex2D( cb, foo.x, foo.y );

          red += rgba.red;
          green += rgba.green;
          blue += rgba.blue;
          alpha += rgba.alpha;
      }

      const double Divisor = 255 * Count;
      red /= Divisor;
      green /= Divisor;
      blue /= Divisor;
      alpha /= Divisor;

      xlColor c1 = xlColor  ( uint8_t( red*255 ), uint8_t( green*255 ), uint8_t( blue*255 ), uint8_t( alpha*255 ) );
      xlColor c2 = xlBLACK;
      return lerp( c1, c2, progress );
   }

   typedef xlColor( *PixelTransform ) ( const ColorBuffer& cb, double s, double t, double progress );

   void RenderPixelTransform( PixelTransform transform, double progress, RenderBuffer& rb )
   {
      xlColorVector cvOrig( rb.pixels );
      ColorBuffer cb( cvOrig, rb.BufferWi, rb.BufferHt );

      for ( int y = 0; y < rb.BufferHt; ++y )
      {
         double t = double( y ) / ( rb.BufferHt - 1 );
         for ( int x = 0; x < rb.BufferWi; ++x )
         {
            double s = double( x ) / ( rb.BufferWi - 1 );
            rb.SetPixel( x, y, transform( cb, s, t, progress ) );
         }
      }
   }
}

WarpEffect::WarpEffect(int i) : RenderableEffect(i, "Warp", On, On, On, On, On)
{
    //ctor
}

WarpEffect::~WarpEffect()
{
    //dtor
}

wxPanel *WarpEffect::CreatePanel(wxWindow *parent) {
    return new OnPanel(parent);
}

void WarpEffect::SetDefaultParameters(Model *cls) {
    OnPanel *p = (OnPanel*)panel;
    p->CheckBoxShimmer->SetValue(false);
    p->TextCtrlStart->SetValue("100");
    p->TextCtrlEnd->SetValue("100");
    p->TextCtrlCycles->SetValue("1.0");
    p->BitmapButton_On_Transparency->SetActive(false);
    SetSliderValue(p->Slider_On_Transparency, 0);
}

std::string WarpEffect::GetEffectString() {
    OnPanel *p = (OnPanel*)panel;
    std::stringstream ret;
    if (100 != p->SliderStart->GetValue()) {
        ret << "E_TEXTCTRL_Eff_On_Start=";
        ret << p->TextCtrlStart->GetValue().ToStdString();
        ret << ",";
    }
    if (100 != p->SliderEnd->GetValue()) {
        ret << "E_TEXTCTRL_Eff_On_End=";
        ret << p->TextCtrlEnd->GetValue().ToStdString();
        ret << ",";
    }
    if (10 != p->SliderCycles->GetValue()) {
        ret << "E_TEXTCTRL_On_Cycles=";
        ret << p->TextCtrlCycles->GetValue().ToStdString();
        ret << ",";
    }
    if (p->CheckBoxShimmer->GetValue()) {
        ret << "E_CHECKBOX_On_Shimmer=1,";
    }
    if (p->BitmapButton_On_Transparency->GetValue()->IsActive()) {
        ret << "E_VALUECURVE_On_Transparency=";
        ret << p->BitmapButton_On_Transparency->GetValue()->Serialise();
        ret << ",";
    } else if (p->Slider_On_Transparency->GetValue() > 0) {
        ret << "E_TEXTCTRL_On_Transparency=";
        ret << p->TextCtrlOnTransparency->GetValue();
        ret << ",";
    }
    return ret.str();
}

namespace
{
   void GetOnEffectColors( const Effect *e, xlColor &start, xlColor &end ) {
      int starti = e->GetSettings().GetInt( "E_TEXTCTRL_Eff_On_Start", 100 );
      int endi = e->GetSettings().GetInt( "E_TEXTCTRL_Eff_On_End", 100 );
      xlColor newcolor;
      newcolor = e->GetPalette()[0];
      if ( starti == 100 && endi == 100 )
      {
         start = end = newcolor;
      }
      else
      {
         HSVValue hsv = newcolor.asHSV();
         hsv.value = ( hsv.value * starti ) / 100;
         start = hsv;
         hsv = newcolor.asHSV();
         hsv.value = ( hsv.value * endi ) / 100;
         end = hsv;
      }
   }
}

int WarpEffect::DrawEffectBackground(const Effect *e, int x1, int y1, int x2, int y2,
                                     DrawGLUtils::xlAccumulator &bg, xlColor* colorMask, bool ramp)
{
    if (ramp)
    {
        bool shimmer = e->GetSettings().GetInt("E_CHECKBOX_On_Shimmer", 0) > 0;
        int starti = e->GetSettings().GetInt("E_TEXTCTRL_Eff_On_Start", 100);
        int endi = e->GetSettings().GetInt("E_TEXTCTRL_Eff_On_End", 100);
        xlColor color = e->GetPalette()[0];
        color.ApplyMask(colorMask);
        int height = y2 - y1;

        float starty = y2 - starti * height / 100.0f;
        float endy = y2 - endi * height / 100.0f;
        float m = float(endy - starty) / float(x2 - x1);

        bg.AddVertex(x1, starty, color);
        bg.AddVertex(x1, y2, color);
        bg.AddVertex(x2, y2, color);
        bg.AddVertex(x2, endy, color);
        bg.AddVertex(x2, y2, color);
        bg.AddVertex(x1, starty, color);

        if (shimmer)
        {
            const int gap = 3;
            for (int x = x1 + (gap*3); x < x2; x += (gap*3)) {
                float newY = m * (x - x1) + starty;
                float newY2 = m * (x + (gap) - x1) + starty;
                bg.AddVertex(x, y2, xlBLACK);
                bg.AddVertex(x, newY, xlBLACK);
                bg.AddVertex(x + (gap), newY2, xlBLACK);

                bg.AddVertex(x, y2, xlBLACK);
                bg.AddVertex(x + (gap), y2, xlBLACK);
                bg.AddVertex(x + (gap), newY2, xlBLACK);
            }
        }

        return 2;
    }
    else
    {
        if (e->HasBackgroundDisplayList()) {
            DrawGLUtils::DrawDisplayList(x1, y1, x2 - x1, y2 - y1, e->GetBackgroundDisplayList(), bg);
            return e->GetBackgroundDisplayList().iconSize;
        }
        xlColor start;
        xlColor end;
        GetOnEffectColors(e, start, end);

        start.ApplyMask(colorMask);
        end.ApplyMask(colorMask);

        bg.AddVertex(x1, y1, start);
        bg.AddVertex(x1, y2, start);
        bg.AddVertex(x2, y2, end);

        bg.AddVertex(x2, y2, end);
        bg.AddVertex(x2, y1, end);
        bg.AddVertex(x1, y1, start);
        return 2;
    }
}

void WarpEffect::RemoveDefaults(const std::string &version, Effect *effect)
{
    SettingsMap &settingsMap = effect->GetSettings();
    if (settingsMap.Get("E_TEXTCTRL_Eff_On_Start", "") == "100") {
        settingsMap.erase("E_TEXTCTRL_Eff_On_Start");
    }
    if (settingsMap.Get("E_TEXTCTRL_Eff_On_End", "") == "100") {
        settingsMap.erase("E_TEXTCTRL_Eff_On_End");
    }
    if (settingsMap.Get("E_CHECKBOX_On_Shimmer", "") == "0") {
        settingsMap.erase("E_CHECKBOX_On_Shimmer");
    }
    if (settingsMap.Get("E_TEXTCTRL_On_Cycles", "") == "1.0") {
        settingsMap.erase("E_TEXTCTRL_On_Cycles");
    }
    RenderableEffect::RemoveDefaults(version, effect);
}


void WarpEffect::Render(Effect *eff, SettingsMap &SettingsMap, RenderBuffer &buffer)
{
    bool canUseShaders = OpenGLShaders::HasShaderSupport();

    int start = SettingsMap.GetInt(TEXTCTRL_Eff_On_Start, 100);
    int end = SettingsMap.GetInt(TEXTCTRL_Eff_On_End, 100);
    bool shimmer = SettingsMap.GetInt(CHECKBOX_On_Shimmer, 0) > 0;
    float cycles = SettingsMap.GetDouble(TEXTCTRL_On_Cycles, 1.0);

    int cidx = 0;
    if (shimmer) {
        int tot = buffer.curPeriod - buffer.curEffStartPer;
        if (tot % 2) {
            if (buffer.palette.Size() <= 1) {
                return;
            }
            cidx = 1;
        }
    }

    bool spatialcolour = buffer.palette.IsSpatial(cidx);
    double adjust = buffer.GetEffectTimeIntervalPosition(cycles);

    xlColor color;
    if (start == 100 && end == 100) {
        buffer.palette.GetColor(cidx, color);
    } else {
        HSVValue hsv;
        buffer.palette.GetHSV(cidx,hsv);
        double d = adjust;
        d = start + (end - start) * d;
        d = d / 100.0;
        hsv.value = hsv.value * d;
        color = hsv;
    }

    int transparency = GetValueCurveInt("On_Transparency", 0, SettingsMap, adjust, /*ON_TRANSPARENCY_MIN*/0, /*ON_TRANSPARENCY_MAX*/100);
    if (transparency) {
        transparency *= 255;
        transparency /= 100;
        color.alpha = 255 - transparency;
    }


    int ms = eff->GetStartTimeMS();
    if ( ms )
        RenderPixelTransform( radialBlurOut, adjust, buffer );
    else
        RenderPixelTransform( rippleIn, adjust, buffer );

    if (shimmer || cycles != 1.0) {
        std::lock_guard<std::recursive_mutex> lock(eff->GetBackgroundDisplayList().lock);
        eff->GetBackgroundDisplayList().resize((buffer.curEffEndPer - buffer.curEffStartPer + 1) * 6);
        buffer.CopyPixelsToDisplayListX(eff, 0, 0, 0);
    } else if (buffer.needToInit) {
        std::lock_guard<std::recursive_mutex> lock(eff->GetBackgroundDisplayList().lock);
        eff->GetBackgroundDisplayList().resize(6);
        if (start == 100 && end == 100) {
            buffer.palette.GetColor(0, color);
            buffer.SetDisplayListHRect(eff, 0, 0.0, 0.0, 1.0, 1.0, color, color);
        } else {
            HSVValue hsv;
            buffer.palette.GetHSV(cidx,hsv);
            hsv.value = hsv.value * start / 100.0;
            color = hsv;

            buffer.palette.GetHSV(cidx,hsv);
            hsv.value = hsv.value * end / 100.0;
            buffer.SetDisplayListHRect(eff, 0, 0.0, 0.0, 1.0, 1.0, color, xlColor(hsv));
        }
        buffer.needToInit = false;
    }
}
