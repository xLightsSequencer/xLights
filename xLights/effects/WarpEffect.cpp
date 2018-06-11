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

   float genWave( float len, float speed, float time, float PI )
   {
      float wave = RenderBuffer::sin( speed * PI * len + time );
      wave = ( wave + 1.0 ) * 0.5;
      wave -= 0.3;
      wave *= wave * wave;
      return wave;
   }

   xlColor waterDrops( const ColorBuffer& cb, double s, double t, float progress )
   {
      const float PI = 3.14159265359;
#define time (-progress * 35.0)
      Vec2D so( 0.5f, 0.5f );
      Vec2D pos2( Vec2D( s, t ) - so );
      Vec2D pos2n( pos2.Norm() );

      double len = pos2.Len();
      float wave = genWave( len, 20.f, time, PI );
#undef time
      Vec2D uv2( -pos2n * wave / ( 1.0 + 5.0 * len ) );

      Vec2D st( s + uv2.x, t + uv2.y );
      return tex2D( cb, st.x, st.y );
   }

   xlColor rippleIn( const ColorBuffer& cb, double s, double t, float progress )
   {
      const Vec2D center( 0.5, 0.5 );
      const double frequency = 20;
      const double speed = 10;
      const double amplitude = 0.15;

      Vec2D toUV( s - center.x, t - center.y );
      double distanceFromCenter = toUV.Len();
      Vec2D normToUV = toUV / distanceFromCenter;

      double wave = RenderBuffer::cos( frequency * distanceFromCenter - speed * progress );
      double offset = progress * wave * amplitude;

      Vec2D newUV = center + normToUV * ( distanceFromCenter + offset );

      xlColor c1 = tex2D( cb, s, t );
      xlColor c2 = tex2D( cb, newUV.x, newUV.y );

      return lerp( c2, c1, progress );
   }
   xlColor rippleOut( const ColorBuffer& cb, double s, double t, float progress )
   {
      const Vec2D center( 0.5, 0.5 );
      const double frequency = 20;
      const double speed = 10;
      const double amplitude = 0.15;

      Vec2D toUV( s - center.x, t - center.y );
      double distanceFromCenter = toUV.Len();
      Vec2D normToUV = toUV / distanceFromCenter;

      double wave = RenderBuffer::cos( frequency * distanceFromCenter - speed * progress );
      double offset = progress * wave * amplitude;

      Vec2D newUV = center + normToUV * ( distanceFromCenter + offset );

      xlColor c1 = tex2D( cb, s, t );
      xlColor c2 = tex2D( cb, newUV.x, newUV.y );

      return lerp( c1, c2, progress );
   }

   xlColor dissolveIn( const ColorBuffer& cb, double s, double t, float progress )
   {
      xlColor dissolveColor = dissolveTex( s, t );
      unsigned char byteProgress = (unsigned char)( 255 * progress );
      return (dissolveColor.red <= byteProgress) ? tex2D( cb, s, t ) : xlBLACK;
   }
   xlColor dissolveOut( const ColorBuffer& cb, double s, double t, float progress )
   {
      xlColor dissolveColor = dissolveTex( s, t );
      unsigned char byteProgress = (unsigned char)( 255 * progress );
      return (dissolveColor.red > byteProgress) ? tex2D( cb, s, t ) : xlBLACK;
   }

   xlColor circleRevealIn( const ColorBuffer& cb, double s, double t, float progress )
   {
      const float FuzzyAmount = 0.04f;
      const float CircleSize = 0.60f;
      const Vec2D CenterPt( 0.5, 0.5 );

      float radius = -FuzzyAmount + progress * (CircleSize + 2.0 * FuzzyAmount);
      float fromCenter = ( Vec2D(s,t) - CenterPt).Len();
      float distFromCircle = fromCenter - radius;

      xlColor c = tex2D( cb, s, t );
      float p = CLAMP((distFromCircle + FuzzyAmount) / (2.0 * FuzzyAmount), 0., 1. );
      return lerp( c, xlBLACK, p );
   }
   xlColor circleRevealOut( const ColorBuffer& cb, double s, double t, float progress )
   {
      const float FuzzyAmount = 0.04f;
      const float CircleSize = 0.60f;
      const Vec2D CenterPt( 0.5, 0.5 );

      float radius = -FuzzyAmount + (1-progress) * (CircleSize + 2.0 * FuzzyAmount);
      float fromCenter = ( Vec2D(s,t) - CenterPt).Len();
      float distFromCircle = fromCenter - radius;

      xlColor c = tex2D( cb, s, t );
      float p = CLAMP((distFromCircle + FuzzyAmount) / (2.0 * FuzzyAmount), 0., 1. );
      return lerp( c, xlBLACK, p );
   }

   xlColor bandedSwirlIn( const ColorBuffer& cb, double s, double t, float progress )
   {
      const Vec2D Center( 0.5, 0.5 );
      const double Frequency = 20.;
      const double TwistAmount = 1.6;

      Vec2D toUV( Vec2D( s, t ) - Center );
      double distanceFromCenter = toUV.Len();
      Vec2D normToUV( toUV / distanceFromCenter );
      float angle = ::atan2( normToUV.y, normToUV.x );

      angle += RenderBuffer::sin( distanceFromCenter * Frequency ) * TwistAmount * (1 - progress);
      Vec2D newUV( RenderBuffer::cos( angle ), RenderBuffer::sin( angle ) );
      newUV = newUV * distanceFromCenter + Center;

      xlColor c1 = tex2D( cb, s, t );
      xlColor c2 = tex2D( cb, newUV.x, newUV.y );

      return lerp( c1, c2, progress );
   }
   xlColor bandedSwirlOut( const ColorBuffer& cb, double s, double t, float progress )
   {
      const Vec2D Center( 0.5, 0.5 );
      const double Frequency = 20.;
      const double TwistAmount = 1.6;

      Vec2D toUV( Vec2D( s, t ) - Center );
      double distanceFromCenter = toUV.Len();
      Vec2D normToUV( toUV / distanceFromCenter );
      float angle = ::atan2( normToUV.y, normToUV.x );

      angle += RenderBuffer::sin( distanceFromCenter * Frequency ) * TwistAmount * progress;
      Vec2D newUV( RenderBuffer::cos( angle ), RenderBuffer::sin( angle ) );
      newUV = newUV * distanceFromCenter + Center;

      xlColor c1 = tex2D( cb, s, t );
      xlColor c2 = tex2D( cb, newUV.x, newUV.y );

      return lerp( c2, c1, progress );
   }

   typedef xlColor( *PixelTransform ) ( const ColorBuffer& cb, double s, double t, float progress );

   void RenderPixelTransform( PixelTransform transform, RenderBuffer& rb, float progress )
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

    p->Slider_Warp_X->SetValue( 50 );
    p->TextCtrl_Warp_X->SetValue( "50" );

    p->Slider_Warp_Y->SetValue( 50 );
    p->TextCtrl_Warp_Y->SetValue( "50" );

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

     int xvalue = p->Slider_Warp_X->GetValue();
     if ( 50 != xvalue )
        ret << "E_TEXTCTRL_Warp_X=" << p->TextCtrl_Warp_X->GetValue().ToStdString() << ",";

     int yvalue = p->Slider_Warp_Y->GetValue();
     if ( 50 != yvalue )
        ret << "E_TEXTCTRL_Warp_Y=" << p->TextCtrl_Warp_Y->GetValue().ToStdString() << ",";

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
    if ( settingsMap.Get( "E_TEXTCTRL_Warp_X", "" )== "50" )
      settingsMap.erase( "E_TEXTCTRL_Warp_X" );
    if ( settingsMap.Get( "E_TEXTCTRL_Warp_Y", "" )== "50" )
      settingsMap.erase( "E_TEXTCTRL_Warp_Y" );
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
   std::string warpStrX = SettingsMap.Get( "TEXTCTRL_Warp_X", "50" );
   std::string warpStrY = SettingsMap.Get( "TEXTCTRL_Warp_Y", "50" );
   std::string speedStr = SettingsMap.Get( "TEXTCTRL_Warp_Speed", "20" );
   std::string freqStr = SettingsMap.Get( "TEXTCTRL_Warp_Frequency", "20" );
   float x = 0.01f * std::stoi( warpStrX );
   float y = 0.01f * std::stoi( warpStrY );
   float speed = std::stof( speedStr );
   float freq = std::stof( freqStr );
   float progress = buffer.GetEffectTimeIntervalPosition(1.f);

   if ( warpType == "water drops" )
      RenderPixelTransform( waterDrops, buffer, progress );
   else if ( warpType == "ripple" )
      RenderPixelTransform( warpTreatment == "in" ? rippleIn : rippleOut, buffer, progress );
   else if ( warpType == "dissolve" )
      RenderPixelTransform( warpTreatment == "in" ? dissolveIn : dissolveOut, buffer, progress );
   else if ( warpType == "banded swirl" )
      RenderPixelTransform( warpTreatment == "in" ? bandedSwirlIn : bandedSwirlOut, buffer, progress );
   else if ( warpType == "circle reveal" )
      RenderPixelTransform( warpTreatment == "in" ? circleRevealIn : circleRevealOut, buffer, progress );
}
