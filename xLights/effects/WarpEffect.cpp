#include <wx/checkbox.h>

#include <sstream>

#include "../../include/warp-64.xpm"
#include "../../include/warp-48.xpm"
#include "../../include/warp-32.xpm"
#include "../../include/warp-24.xpm"
#include "../../include/warp-16.xpm"

#include "WarpEffect.h"
#include "WarpPanel.h"
#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../xLightsMain.h"
#include "../DissolveTransitionPattern.h"
#include "../xLightsApp.h"
#include "../TimingPanel.h"

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
      bool     IsNormal() const { return fabs( Len2() - 1 ) < 1e-6; }
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
      double red   = a.red   + progress * ( b.red   - a.red   );
      double green = a.green + progress * ( b.green - a.green );
      double blue  = a.blue  + progress * ( b.blue  - a.blue  );

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

   xlColor dissolveTex( double s, double t )
   {
      const unsigned char *data = DissolveTransitonPattern;
      s = CLAMP( 0., s, 1. );
      t = CLAMP( 0., t, 1. );

      int x = int( s * (DissolvePatternWidth - 1 ) );
      int y = int( t * (DissolvePatternHeight -1 ) );

      const unsigned char *val = data + y * DissolvePatternWidth + x;
      return xlColor( *val, *val, *val );
   }

   struct WarpEffectParams
   {
      WarpEffectParams( float i_progress, const Vec2D& i_xy, float i_speed, float i_frequency )
         : progress(i_progress), xy(i_xy), speed(i_speed), frequency(i_frequency) {}
      float progress;
      Vec2D xy;
      float speed;
      float frequency;
   };

   float genWave( float len, float speed, float time, float PI )
   {
      float wave = RenderBuffer::sin( speed * PI * len + time );
      wave = ( wave + 1.0 ) * 0.5;
      wave -= 0.3;
      wave *= wave * wave;
      return wave;
   }

   xlColor waterDrops( const ColorBuffer& cb, double s, double t, const WarpEffectParams& params )
   {
      const float PI = 3.14159265359;
#define time (-params.progress * 35.0)
      Vec2D pos2( Vec2D( s, t ) - params.xy );
      Vec2D pos2n( pos2.Norm() );

      double len = pos2.Len();
      float wave = genWave( len, params.speed, time, PI );
#undef time
      Vec2D uv2( -pos2n * wave / ( 1.0 + 5.0 * len ) );

      Vec2D st( s + uv2.x, t + uv2.y );
      return tex2D( cb, st.x, st.y );
   }

   xlColor rippleIn( const ColorBuffer& cb, double s, double t, const WarpEffectParams& params )
   {
      const double amplitude = 0.15;

      Vec2D toUV( s - params.xy.x, t - params.xy.y );
      double distanceFromCenter = toUV.Len();
      Vec2D normToUV = toUV / distanceFromCenter;

      double wave = RenderBuffer::cos( params.frequency * distanceFromCenter - params.speed * params.progress );
      double offset = params.progress * wave * amplitude;

      Vec2D newUV = params.xy + normToUV * ( distanceFromCenter + offset );

      xlColor c1 = tex2D( cb, s, t );
      xlColor c2 = tex2D( cb, newUV.x, newUV.y );

      return lerp( c2, c1, params.progress );
   }
   xlColor rippleOut( const ColorBuffer& cb, double s, double t, const WarpEffectParams& params )
   {
      const double amplitude = 0.15;

      Vec2D toUV( s - params.xy.x, t - params.xy.y );
      double distanceFromCenter = toUV.Len();
      Vec2D normToUV = toUV / distanceFromCenter;

      double wave = RenderBuffer::cos( params.frequency * distanceFromCenter - params.speed * params.progress );
      double offset = params.progress * wave * amplitude;

      Vec2D newUV = params.xy + normToUV * ( distanceFromCenter + offset );

      xlColor c1 = tex2D( cb, s, t );
      xlColor c2 = tex2D( cb, newUV.x, newUV.y );

      return lerp( c1, c2, params.progress );
   }

   xlColor dissolveIn( const ColorBuffer& cb, double s, double t, const WarpEffectParams& params )
   {
      xlColor dissolveColor = dissolveTex( s, t );
      unsigned char byteProgress = (unsigned char)( 255 * params.progress );
      return (dissolveColor.red <= byteProgress) ? tex2D( cb, s, t ) : xlBLACK;
   }
   xlColor dissolveOut( const ColorBuffer& cb, double s, double t, const WarpEffectParams& params )
   {
      xlColor dissolveColor = dissolveTex( s, t );
      unsigned char byteProgress = (unsigned char)( 255 * params.progress );
      return (dissolveColor.red > byteProgress) ? tex2D( cb, s, t ) : xlBLACK;
   }

   xlColor circleRevealIn( const ColorBuffer& cb, double s, double t, const WarpEffectParams& params )
   {
      const float FuzzyAmount = 0.04f;
      const float CircleSize = 0.60f;

      float radius = -FuzzyAmount + params.progress * (CircleSize + 2.0 * FuzzyAmount);
      float fromCenter = ( Vec2D(s,t) - params.xy).Len();
      float distFromCircle = fromCenter - radius;

      xlColor c = tex2D( cb, s, t );
      float p = CLAMP((distFromCircle + FuzzyAmount) / (2.0 * FuzzyAmount), 0., 1. );
      return lerp( c, xlBLACK, p );
   }
   xlColor circleRevealOut( const ColorBuffer& cb, double s, double t, const WarpEffectParams& params )
   {
      const float FuzzyAmount = 0.04f;
      const float CircleSize = 0.60f;

      float radius = -FuzzyAmount + (1-params.progress) * (CircleSize + 2.0 * FuzzyAmount);
      float fromCenter = ( Vec2D(s,t) - params.xy).Len();
      float distFromCircle = fromCenter - radius;

      xlColor c = tex2D( cb, s, t );
      float p = CLAMP((distFromCircle + FuzzyAmount) / (2.0 * FuzzyAmount), 0., 1. );
      return lerp( c, xlBLACK, p );
   }

   xlColor bandedSwirlIn( const ColorBuffer& cb, double s, double t, const WarpEffectParams& params )
   {
      const double TwistAmount = 1.6;

      Vec2D toUV( Vec2D( s, t ) - params.xy );
      double distanceFromCenter = toUV.Len();
      Vec2D normToUV( toUV / distanceFromCenter );
      float angle = ::atan2( normToUV.y, normToUV.x );

      angle += RenderBuffer::sin( distanceFromCenter * params.frequency ) * TwistAmount * (1 - params.progress);
      Vec2D newUV( RenderBuffer::cos( angle ), RenderBuffer::sin( angle ) );
      newUV = newUV * distanceFromCenter + params.xy;

      xlColor c1 = tex2D( cb, s, t );
      xlColor c2 = tex2D( cb, newUV.x, newUV.y );

      return lerp( c1, c2, params.progress );
   }
   xlColor bandedSwirlOut( const ColorBuffer& cb, double s, double t, const WarpEffectParams& params )
   {
      const double TwistAmount = 1.6;

      Vec2D toUV( Vec2D( s, t ) - params.xy );
      double distanceFromCenter = toUV.Len();
      Vec2D normToUV( toUV / distanceFromCenter );
      float angle = ::atan2( normToUV.y, normToUV.x );

      angle += RenderBuffer::sin( distanceFromCenter * params.frequency ) * TwistAmount * params.progress;
      Vec2D newUV( RenderBuffer::cos( angle ), RenderBuffer::sin( angle ) );
      newUV = newUV * distanceFromCenter + params.xy;

      xlColor c1 = tex2D( cb, s, t );
      xlColor c2 = tex2D( cb, newUV.x, newUV.y );

      return lerp( c2, c1, params.progress );
   }

   typedef xlColor( *PixelTransform ) ( const ColorBuffer& cb, double s, double t, const WarpEffectParams& params );

   void RenderPixelTransform( PixelTransform transform, RenderBuffer& rb, const WarpEffectParams& params )
   {
      xlColorVector cvOrig( rb.pixels );
      ColorBuffer cb( cvOrig, rb.BufferWi, rb.BufferHt );

      for ( int y = 0; y < rb.BufferHt; ++y )
      {
         double t = double( y ) / ( rb.BufferHt - 1 );
         for ( int x = 0; x < rb.BufferWi; ++x )
         {
            double s = double( x ) / ( rb.BufferWi - 1 );
            rb.SetPixel( x, y, transform( cb, s, t, params ) );
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

wxPanel *WarpEffect::CreatePanel(wxWindow *parent)
{
    return new WarpPanel(parent);
}

void WarpEffect::SetDefaultParameters()
{
    WarpPanel *p = (WarpPanel *)panel;

    p->Choice_Warp_Type->SetSelection( 0 );
    p->Choice_Warp_Treatment->SetSelection( 0 );

    SetSliderValue( p->Slider_Warp_X, 50 );
    p->BitmapButton_Warp_X->SetActive( false );

    SetSliderValue( p->Slider_Warp_Y, 50 );
    p->BitmapButton_Warp_Y->SetActive( false );

    p->Slider_Warp_Cycle_Count->SetValue( 1 );
    p->TextCtrl_Warp_Cycle_Count->SetValue( "1" );

    p->Slider_Warp_Speed->SetValue( 20 );
    p->TextCtrl_Warp_Speed->SetValue( "20" );

    p->Slider_Warp_Frequency->SetValue( 20 );
    p->TextCtrl_Warp_Frequency->SetValue( "20" );

    // Turn on canvas mode as this really only makes sense in canvas mode
    xLightsFrame* frame = xLightsApp::GetFrame();
    TimingPanel* layerBlendingPanel = frame->GetLayerBlendingPanel();
    layerBlendingPanel->CheckBox_Canvas->SetValue(true);
}

std::string WarpEffect::GetEffectString()
{
     WarpPanel *p = (WarpPanel *)panel;
     std::stringstream ret;

     wxString warpType( p->Choice_Warp_Type->GetStringSelection() );
     if ( "water drops" != warpType )
        ret << "E_CHOICE_Warp_Type=" << warpType.ToStdString() << ",";

     wxString warpTreatment( p->Choice_Warp_Treatment->GetStringSelection() );
     if ( "constant" != warpTreatment )
        ret << "E_CHOICE_Warp_Treatment=" << warpTreatment.ToStdString() << ",";

     if ( p->BitmapButton_Warp_X->GetValue()->IsActive() )
     {
        ret << "E_VALUECURVE_Warp_X=";
        ret << p->BitmapButton_Warp_X->GetValue()->Serialise();
        ret << ",";
     }
     else
     {
        int xvalue = p->Slider_Warp_X->GetValue();
        if ( 50 != xvalue )
          ret << "E_TEXTCTRL_Warp_X=" << p->TextCtrl_Warp_X->GetValue().ToStdString() << ",";
     }

     if ( p->BitmapButton_Warp_Y->GetValue()->IsActive() )
     {
        ret << "E_VALUECURVE_Warp_Y=";
        ret << p->BitmapButton_Warp_Y->GetValue()->Serialise();
        ret << ",";
     }
     else
     {
        int yvalue = p->Slider_Warp_Y->GetValue();
        if ( 50 != yvalue )
           ret << "E_TEXTCTRL_Warp_Y=" << p->TextCtrl_Warp_Y->GetValue().ToStdString() << ",";
     }

     int cycleCount = p->Slider_Warp_Cycle_Count->GetValue();
     if ( 1 != cycleCount )
      ret << "E_TEXTCTRL_Warp_Cycle_Count=" << p->TextCtrl_Warp_Cycle_Count->GetValue().ToStdString() << ",";

     int speed = p->Slider_Warp_Speed->GetValue();
     if ( 20 != speed )
        ret << "E_TEXTCTRL_Warp_Speed=" << p->TextCtrl_Warp_Speed->GetValue().ToStdString() << ",";

     int freq = p->Slider_Warp_Frequency->GetValue();
     if ( 20 != freq )
       ret << "E_TEXTCTRL_Warp_Frequency=" << p->TextCtrl_Warp_Frequency->GetValue().ToStdString() << ",";

     return ret.str();
}

void WarpEffect::RemoveDefaults(const std::string &version, Effect *effect)
{
    SettingsMap &settingsMap = effect->GetSettings();

    if ( settingsMap.Get( "E_CHOICE_Warp_Type", "" )== "water drops" )
      settingsMap.erase( "E_CHOICE_Warp_Type" );
    if ( settingsMap.Get( "E_CHOICE_Warp_Treatment", "" )== "constant" )
      settingsMap.erase( "E_CHOICE_Warp_Treatment" );
    if ( settingsMap.Get( "E_TEXTCTRL_Warp_Cycle_Count", "" ) == "1" )
      settingsMap.erase( "E_TEXTCTRL_Warp_Cycle_Count" );
    if ( settingsMap.Get( "E_TEXTCTRL_Warp_Speed", "" )== "20" )
      settingsMap.erase( "E_TEXTCTRL_Warp_Speed" );
    if ( settingsMap.Get( "E_TEXTCTRL_Warp_Frequency", "" )== "20" )
      settingsMap.erase( "E_TEXTCTRL_Warp_Frequency" );

    RenderableEffect::RemoveDefaults(version, effect);
}

void WarpEffect::Render(Effect *eff, SettingsMap &SettingsMap, RenderBuffer &buffer)
{
   std::string warpType = SettingsMap.Get( "CHOICE_Warp_Type", "water drops" );
   std::string warpTreatment = SettingsMap.Get( "CHOICE_Warp_Treatment", "constant");
   std::string warpStrCycleCount = SettingsMap.Get( "TEXTCTRL_Warp_Cycle_Count", "1" );
   std::string speedStr = SettingsMap.Get( "TEXTCTRL_Warp_Speed", "20" );
   std::string freqStr = SettingsMap.Get( "TEXTCTRL_Warp_Frequency", "20" );
   int xPercentage = GetValueCurveInt( "Warp_X", 0, SettingsMap, 1.f, 0, 100 );
   int yPercentage = GetValueCurveInt( "Warp_Y", 0, SettingsMap, 1.f, 0, 100 );
   double x = 0.01 * xPercentage;
   double y = 0.01 * yPercentage;
   float speed = std::stof( speedStr );
   float frequency = std::stof( freqStr );
   float progress = buffer.GetEffectTimeIntervalPosition(1.f);

   WarpEffectParams params( progress, Vec2D( x, y ), speed, frequency );
   if ( warpType == "water drops" )
      RenderPixelTransform( waterDrops, buffer, params );
   else
   {
      PixelTransform xform = nullptr;
      // the other warps were originally intended as transitions in or out... for constant
      // treatment, we'll just cycle between progress of [0,1] and [1,0]
      if ( warpTreatment == "constant" )
      {
         float cycleCount = std::stof( warpStrCycleCount );
         float intervalLen = 1.f / (2 * cycleCount );
         float scaledProgress = progress / intervalLen;
         float intervalProgress, intervalIndex;
         intervalProgress = std::modf( scaledProgress, &intervalIndex );
         if ( int(intervalIndex) % 2 )
            intervalProgress = 1.f - intervalProgress;
         //LinearInterpolater interpolater;
         //float interpolatedProgress = interpolate( intervalProgress, 0.0,0.2, 1.0,0.8, interpolater );
         params.progress = intervalProgress;
         if ( warpType == "ripple" )
            xform = rippleIn;
         else if ( warpType == "dissolve" )
            xform = dissolveIn;
         else if ( warpType == "banded swirl" )
            xform = bandedSwirlIn;
         else if ( warpType == "circle reveal" )
            xform = circleRevealIn;
      }
      else
      {
         if ( warpType == "ripple" )
            xform = ( warpTreatment == "in" )? rippleIn : rippleOut;
         else if ( warpType == "dissolve" )
            xform = ( warpTreatment == "in" ) ? dissolveIn : dissolveOut;
         else if ( warpType == "banded swirl" )
            xform = ( warpTreatment == "in" ) ? bandedSwirlIn : bandedSwirlOut;
         else if ( warpType == "circle reveal" )
            xform = ( warpTreatment == "in" ) ? circleRevealIn : circleRevealOut;
      }
      if ( xform != nullptr )
         RenderPixelTransform( xform, buffer, params );
   }
}
