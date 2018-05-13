#include "WarpEffect.h"
#include "OnPanel.h"
#include "WarpPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../xLightsMain.h" //xLightsFrame
#include "../OpenGLShaders.h"

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

      double wave = RenderBuffer::cos( frequency * distanceFromCenter - speed * progress );
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

}

WarpEffect::~WarpEffect()
{

}

wxPanel *WarpEffect::CreatePanel(wxWindow *parent)
{
    return new WarpPanel(parent);
}

void WarpEffect::SetDefaultParameters(Model *cls)
{
    WarpPanel *p = (WarpPanel *)panel;
    p->Choice_Warp_Effect->SetValue( "radial blur" );
    p->Choice_Warp_Type->SetValue( "in" );
}

std::string WarpEffect::GetEffectString()
{
     WarpPanel *p = (WarpPanel *)panel;
     std::stringstream ret;

     wxString warpEffect( p->Choice_Warp_Effect->GetStringSelection() );
     ret << "E_CHOICE_Warp_Effect=" << warpEffect.ToStdString() << ",";

     wxString warpType( p->Choice_Warp_Type->GetStringSelection() );
     ret << "E_CHOICE_Warp_Type=" << warpType.ToStdString() << ",";

     return ret.str();
}

void WarpEffect::RemoveDefaults(const std::string &version, Effect *effect)
{
    SettingsMap &settingsMap = effect->GetSettings();

    if ( settingsMap.Get( "CHOICE_Warp_Effect", "" )== "radial blur" )
      settingsMap.erase( "CHOICE_Warp_Effect" );
    if ( settingsMap.Get( "CHOICE_Warp_Type", "" )== "in" )
      settingsMap.erase( "CHOICE_Warp_Type" );

    RenderableEffect::RemoveDefaults(version, effect);
}

void WarpEffect::Render(Effect *eff, SettingsMap &SettingsMap, RenderBuffer &buffer)
{
    bool canUseShaders = OpenGLShaders::HasShaderSupport();
    std::string warpEffect = SettingsMap.Get( "CHOICE_Warp_Effect", "radial blur" );
    std::string warpType = SettingsMap.Get( "CHOICE_Warp_Type", "in");

    // todo - act on "ripple" vs. "radial blur" and "in" vs. "out"

    int ms = eff->GetStartTimeMS();
    float cycles = /*SettingsMap.GetDouble(TEXTCTRL_On_Cycles, 1.0)*/1.f;
    double adjust = buffer.GetEffectTimeIntervalPosition(cycles);
    if ( ms )
        RenderPixelTransform( radialBlurOut, adjust, buffer );
    else
        RenderPixelTransform( rippleIn, adjust, buffer );
}
