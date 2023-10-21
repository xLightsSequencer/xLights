/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "PixelBuffer.h"
#include <wx/tokenzr.h>
#include "DimmingCurve.h"
#include "models/ModelManager.h"
#include "models/SingleLineModel.h"
#include "models/ModelGroup.h"
#include "UtilClasses.h"
#include "AudioManager.h"
#include "BufferPanel.h"
#include "xLightsMain.h"
#include <log4cpp/Category.hh>

#include <cmath>
#include <random>
#include "Parallel.h"
#include "UtilFunctions.h"
#include "DissolveTransitionPattern.h"
#include "GPURenderUtils.h"

// This is needed for visual studio
#ifdef _MSC_VER
#define M_PI_2 1.57079632679489661923
#endif

namespace
{
   template <class T> T CLAMP( const T& lo, const T&val, const T& hi )
   {
      return std::min( hi, std::max( lo, val ) );
   }

   double SmoothStep( double edge0, double edge1, double x )
   {
      double t = CLAMP( (x - edge0) / (edge1 - edge0), 0.0, 1.0 );
      return t * t * (3.0 - 2.0 * t);
   }

   struct ColorBuffer
   {
      ColorBuffer( const xlColorVector& i_cv, int i_w, int i_h ) : cv( i_cv ), w( i_w ), h( i_h ) {}
      ColorBuffer( xlColor *i_cv, int count, int i_w, int i_h ) : cv( i_cv, &i_cv[count] ), w( i_w ), h( i_h ) {}

      xlColor GetPixel( int x, int y ) const
      {
         return ( x >= 0 && x < w && y >= 0 && y <h ) ? cv[y*w + x] : xlBLACK;
      }
      xlColorVector cv;
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

   xlColor tex2D( const RenderBuffer& rb, double s, double t )
   {
      s = CLAMP( 0., s, 1. );
      t = CLAMP( 0., t, 1. );

      int x = int( s * ( rb.BufferWi - 1 ) );
      int y = int( t * ( rb.BufferHt - 1 ) );

      return rb.GetPixel( x, y );
   }

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
      Vec2D    RotateAbout( double angle, const Vec2D& pt ) const
      {
         Vec2D p( *this - pt );
         p = p.Rotate( angle );
         return p + pt;
      }

      static Vec2D lerp( const Vec2D& a, const Vec2D& b, double progress )
      {
         double x = a.x + progress * ( b.x - a.x );
         double y = a.y + progress * ( b.y - a.y );
         return Vec2D( x, y );
      }
      double x, y;
   };

   xlColor tex2D(const ColorBuffer& cb, const Vec2D& st) {
       return tex2D(cb, st.x, st.y);
   }

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused"
   xlColor lerp(const xlColor& a, const xlColor& b, double progress) {
       double red = a.red + progress * (b.red - a.red);
       double green = a.green + progress * (b.green - a.green);
       double blue = a.blue + progress * (b.blue - a.blue);

       return xlColor(uint8_t(red), uint8_t(green), uint8_t(blue));
   }

    Vec2D operator +( double a, const Vec2D& b )
    {
        return Vec2D( a + b.x, a + b.y );
    }
    Vec2D operator -( double a, const Vec2D& b )
    {
        return Vec2D( a - b.x, a - b.y );
    }
    Vec2D operator *( double a, const Vec2D& b )
    {
        return Vec2D( a * b.x, a * b.y );
    }

    double dot( const Vec2D& a, const Vec2D& b )
    {
        return a.x * b.x + a.y * b.y;
    }

    double lerp( double a, double b, double progress )
    {
        return a + progress * ( b - a );
    }

    xlColor operator+=( xlColor& lhs, const xlColor& rhs )
    {
        lhs.red += rhs.red;
        lhs.green += rhs.green;
        lhs.blue += rhs.blue;
        return lhs;
    }
#pragma clang diagnostic pop


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

   struct Vec3D
   {
      Vec3D( double i_x = 0., double i_y = 0., double i_z = 0. ) : x( i_x ), y( i_y ), z( i_z ) {}
      Vec3D operator -() const { return Vec3D( -x, -y, -z ); }
      Vec3D operator+( const Vec3D& p ) const { return Vec3D( x + p.x, y + p.y, z + p.z ); }
      Vec3D operator-( const Vec3D& p ) const { return Vec3D( x - p.x, y - p.y, z - p.z ); }

      double x, y, z;
   };
   Vec3D operator *( double a, const Vec3D& b )
   {
      return Vec3D( a * b.x, a * b.y, a * b.z );
   }

   double dot( const Vec3D& a, const Vec3D& b )
   {
      return a.x * b.x + a.y * b.y + a.z * b.z;
   }

   // code for fold transition
   xlColor foldIn( const ColorBuffer& cb0, const RenderBuffer* rb1, double s, double t, float progress, bool isReverse )
   {
      const double CAMERA_DIST = 2.;
      Vec3D ray( s*2-1, t*2-1, CAMERA_DIST );
      float phi = progress * PI;   // rotation angle

      // rotated basis vectors
      Vec3D rx( RenderBuffer::cos( phi ), 0., isReverse ? RenderBuffer::sin( phi ) : -RenderBuffer::sin( phi ) );
      Vec3D ry( 0., 1., 0. );
      Vec3D rz( -rx.z, 0., rx.x );

      // corner and direction vectors of "from" polygon
      Vec3D p0( -rx + Vec3D( 0., -1., CAMERA_DIST ) );
      Vec3D u0( rx );
      Vec3D v0( ry );
      Vec3D n0( rz );

      // ray-plane intersection
      Vec3D a0( ( dot( p0, n0) / dot( ray, n0 ) ) * ray );

      Vec2D uv0( dot( a0 - p0, u0 ) / 2., dot( a0 - p0, v0 ) / 2. );
      if ( uv0.x >= 0. && uv0.x < 1. && uv0.y >= 0. && uv0.y < 1. )
      {
         if ( rb1 == nullptr )
            return tex2D( cb0, 1-uv0.x, uv0.y );

         if ( progress < 0.5 )
         {
            xlColor c = tex2D( *rb1, uv0.x, uv0.y );
            return ( c != xlCLEAR ) ? c : tex2D( cb0, 1-uv0.x, uv0.y );
         }
         return tex2D( cb0, 1-uv0.x, uv0.y );
      }
      return xlBLACK;
   }
   xlColor foldOut( const ColorBuffer& cb0, const RenderBuffer* rb1, double s, double t, float progress, bool isReverse )
   {
      const double CAMERA_DIST = 2.;
      Vec3D ray( s*2-1, t*2-1, CAMERA_DIST );
      float phi = progress * PI;   // rotation angle

      // rotated basis vectors
      Vec3D rx( RenderBuffer::cos( phi ), 0., isReverse ? RenderBuffer::sin( phi ) : -RenderBuffer::sin( phi ) );
      Vec3D ry( 0., 1., 0. );
      Vec3D rz( -rx.z, 0., rx.x );

      // corner and direction vectors of "from" polygon
      Vec3D p0( -rx + Vec3D( 0., -1., CAMERA_DIST ) );
      Vec3D u0( rx );
      Vec3D v0( ry );
      Vec3D n0( rz );

      // ray-plane intersection
      Vec3D a0( ( dot( p0, n0) / dot( ray, n0 ) ) * ray );

      Vec2D uv0( dot( a0 - p0, u0 ) / 2., dot( a0 - p0, v0 ) / 2. );
      if ( uv0.x >= 0. && uv0.x < 1. && uv0.y >= 0. && uv0.y < 1. )
      {
         if ( rb1 == nullptr )
            return tex2D( cb0, uv0.x, uv0.y );

         if ( progress < 0.5 )
            return tex2D( *rb1, uv0.x, uv0.y );
         xlColor c = tex2D( cb0, 1-uv0.x, uv0.y );
         return ( c != xlCLEAR ) ? c : tex2D( cb0, uv0.x, uv0.y );
      }
      return xlBLACK;
   }
   void foldIn( RenderBuffer& rb0, const ColorBuffer& cb0, const RenderBuffer* rb1, double progress, bool isReverse )
   {
      if ( progress < 0. || progress > 1. )
         return;
      parallel_for(0, rb0.BufferHt, [&rb0, &cb0, &rb1, progress, isReverse](int y) {
         double t = double( y ) / ( rb0.BufferHt - 1 );
         for ( int x = 0; x < rb0.BufferWi; ++x ) {
            double s = double( x ) / ( rb0.BufferWi - 1 );
            rb0.SetPixel( x, y, foldIn( cb0, rb1, s, t, progress, isReverse ) );
         }
      }, 25);
   }
   void foldOut( RenderBuffer& rb0, const ColorBuffer& cb0, const RenderBuffer* rb1, double progress, bool isReverse )
   {
      if ( progress < 0. || progress > 1. )
         return;
      parallel_for(0, rb0.BufferHt, [&rb0, &cb0, &rb1, progress, isReverse](int y) {
         double t = double( y ) / ( rb0.BufferHt - 1 );
         for ( int x = 0; x < rb0.BufferWi; ++x ) {
            double s = double( x ) / ( rb0.BufferWi - 1 );
            rb0.SetPixel( x, y, foldOut( cb0, rb1, s, t, progress, isReverse ) );
         }
      }, 25);
   }

   // code for dissolve transition
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
   xlColor dissolveIn( const ColorBuffer& cb, double s, double t, float progress )
   {
      xlColor dissolveColor = dissolveTex( s, t );
      unsigned char byteProgress = (unsigned char)( 255 * progress );
      return (dissolveColor.red <= byteProgress) ? tex2D( cb, s, t ) : xlBLACK;
   }
   void dissolveIn( RenderBuffer& rb0, const ColorBuffer& cb0, double progress )
   {
      if ( progress < 0. || progress > 1. )
         return;
      parallel_for(0, rb0.BufferHt, [&rb0, &cb0, progress](int y) {
         double t = double( y ) / ( rb0.BufferHt - 1 );
         for ( int x = 0; x < rb0.BufferWi; ++x ) {
            double s = double( x ) / ( rb0.BufferWi - 1 );
            rb0.SetPixel( x, y, dissolveIn( cb0, s, t, progress ) );
         }
      }, 25);
   }
   xlColor dissolveOut( const ColorBuffer& cb, double s, double t, float progress )
   {
      xlColor dissolveColor = dissolveTex( s, t );
      unsigned char byteProgress = (unsigned char)( 255 * progress );
      return (dissolveColor.red > byteProgress) ? tex2D( cb, s, t ) : xlBLACK;
   }
   void dissolveOut( RenderBuffer& rb0, const ColorBuffer& cb0, double progress )
   {
      if ( progress < 0. || progress > 1. )
         return;
      parallel_for(0, rb0.BufferHt, [&rb0, &cb0, progress](int y) {
         double t = double( y ) / ( rb0.BufferHt - 1 );
         for ( int x = 0; x < rb0.BufferWi; ++x ) {
            double s = double( x ) / ( rb0.BufferWi - 1 );
            rb0.SetPixel( x, y, dissolveOut( cb0, s, t, progress ) );
         }
      }, 25);
   }

   // code for circular-swirl transition
   xlColor circularSwirl( const ColorBuffer& cb, const Vec2D& xy, float speed, double s, double t, float progress )
   {
      Vec2D uv( s, t );
      Vec2D dir( uv - xy );
      double len = dir.Len();

      double radius = (1. - progress) * 0.70710678;
      if ( len < radius )
      {
         Vec2D rotated( dir.Rotate( -speed * len * progress * PI ) );
         Vec2D scaled( rotated * (1. - progress) + xy );

         Vec2D newUV( Vec2D::lerp( xy, scaled, 1. - progress ) );

         return tex2D( cb, newUV.x, newUV.y );
      }

      return xlBLACK;
   }
   void circularSwirl( RenderBuffer& rb0, const ColorBuffer& cb0, const Vec2D& xy, float speed, double progress )
   {
      if ( progress < 0. || progress > 1. )
         return;
      parallel_for(0, rb0.BufferHt, [&rb0, &cb0, xy, speed, progress](int y) {
         double t = double( y ) / ( rb0.BufferHt - 1 );
         for ( int x = 0; x < rb0.BufferWi; ++x ) {
            double s = double( x ) / ( rb0.BufferWi - 1 );
            rb0.SetPixel( x, y, circularSwirl( cb0, xy, speed, s, t, progress ) );
         }
      }, 25);
   }

   // code for bowTie transition
   float check( const Vec2D& p1, const Vec2D& p2, const Vec2D& p3 )
   {
      return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
   }
   bool pointInTriangle( const Vec2D& pt, const Vec2D& p1, const Vec2D& p2, const Vec2D& p3 )
   {
      bool b1 = check( pt, p1, p2 ) < 0.0;
      bool b2 = check( pt, p2, p3 ) < 0.0;
      bool b3 = check( pt, p3, p1 ) < 0.0;
      return b1 == b2 && b2 == b3;
   }
   const double bowTieHeight = 0.5;
   xlColor bowTie_firstHalf( const ColorBuffer& cb, const RenderBuffer* rb1, const Vec2D& uv, double progress, double adjust )
   {
      if ( uv.y < 0.5 )
      {
         Vec2D botLeft( 0., progress-bowTieHeight );
         Vec2D botRight( 1., progress-bowTieHeight );
         Vec2D tip( adjust, progress );
         if ( pointInTriangle( uv, botLeft, botRight, tip ) )
            return tex2D( cb, uv );
      }
      else
      {
         Vec2D topLeft( 0., 1.-progress+bowTieHeight );
         Vec2D topRight( 1., 1.-progress+bowTieHeight );
         Vec2D tip( adjust, 1.-progress );
         if ( pointInTriangle( uv, topLeft, topRight, tip ) )
            return tex2D( cb, uv );
      }
      return (rb1 == nullptr) ? xlBLACK : tex2D( *rb1, uv.x, uv.y );
   }
   xlColor bowTie_secondHalf( const ColorBuffer& cb, const RenderBuffer* rb1, const Vec2D& uv, double progress, double adjust )
   {
      if ( uv.x > adjust )
      {
         Vec2D top( progress + bowTieHeight, 1. );
         Vec2D bot( progress + bowTieHeight, 0. );
         Vec2D tip( lerp( adjust, 1.0, 2.0 * (progress - 0.5) ), 0.5 );
         if ( pointInTriangle( uv, top, bot, tip) )
            return ( rb1 == nullptr ) ? xlBLACK : tex2D( *rb1, uv.x, uv.y );
      }
      else
      {
         Vec2D top( 1.0-progress - bowTieHeight, 1. );
         Vec2D bot( 1.0-progress - bowTieHeight, 0. );
         Vec2D tip( lerp( adjust, 0.0, 2.0 * (progress - 0.5) ), 0.5 );
         if ( pointInTriangle( uv, top, bot, tip) )
            return ( rb1 == nullptr ) ? xlBLACK : tex2D( *rb1, uv.x, uv.y );
      }
      return tex2D( cb, uv );
   }
   xlColor bowTie( const ColorBuffer& cb, const RenderBuffer* rb1, double s, double t, double progress, double adjust, bool isReversed )
   {
      Vec2D xy( s, t );
      if ( isReversed )
         return ( progress < 0.5 ) ? bowTie_secondHalf( cb, rb1, xy, 1.-progress, adjust ) : bowTie_firstHalf( cb, rb1, xy, 1.-progress, adjust );
      else
         return ( progress < 0.5 ) ? bowTie_firstHalf( cb, rb1, xy, progress, adjust ) : bowTie_secondHalf( cb, rb1, xy, progress, adjust );
   }
   void bowTie( RenderBuffer& rb0, const ColorBuffer& cb0, const RenderBuffer* rb1, double progress, int adjust, bool isReversed )
   {
      if ( progress < 0. || progress > 1. )
         return;
      double bowTieAdjust = 0.01 * adjust;
      parallel_for(0, rb0.BufferHt, [&rb0, &cb0, &rb1, progress, bowTieAdjust, isReversed](int y) {
         double t = double( y ) / ( rb0.BufferHt - 1 );
         for ( int x = 0; x < rb0.BufferWi; ++x ) {
            double s = double( x ) / ( rb0.BufferWi - 1 );
            rb0.SetPixel( x, y, bowTie( cb0, rb1, s, t, progress, bowTieAdjust, isReversed ) );
         }
      }, 25);
   }

   // code for zoom transition
   Vec2D zoom( const Vec2D& uv, float amount )
   {
      return ((uv - 0.5) * (1.0-amount)) + 0.5;
   }
   xlColor zoomTransition( const ColorBuffer& cb, double s, double t, float progress )
   {
      return tex2D( cb, zoom( Vec2D( s, t ), 1.f-progress ) );
   }
   void zoomTransition( RenderBuffer& rb0, const ColorBuffer& cb0, double progress )
   {
      if ( progress < 0. || progress > 1. )
         return;
      parallel_for(0, rb0.BufferHt, [&rb0, &cb0, progress](int y) {
         double t = double( y ) / ( rb0.BufferHt - 1 );
         for ( int x = 0; x < rb0.BufferWi; ++x ) {
            double s = double( x ) / ( rb0.BufferWi - 1 );
            rb0.SetPixel( x, y, zoomTransition( cb0, s, t, progress ) );
         }
      }, 25);
   }

   // code for doorway transition
   const float reflection = 0.4f;
   const float perspective = 0.4f;
   const float depth = 3.f;
   const Vec2D boundMin(0.0, 0.0);
   const Vec2D boundMax(1.0, 1.0);
   bool inBounds ( const Vec2D& p)
   {
      return ( boundMin.x < p.x && boundMin.y < p.y ) && ( p.x < boundMax.x && p.y < boundMax.y );
   }
   Vec2D project( const Vec2D& p )
   {
      return p * Vec2D(1.0, -1.2 ) + Vec2D( 0.0, -0.02 );
   }
   xlColor bgColor( const Vec2D& p, Vec2D& pto, const ColorBuffer& cb )
   {
      xlColor c = xlBLACK;
      pto = project( pto );
      if ( inBounds( pto ) )
      {
         xlColor toColor = tex2D( cb, pto );
         c += lerp( xlBLACK, toColor, reflection * lerp( 1.0, 0.0, pto.y ) );
      }
      return c;
   }
   xlColor doorway( const ColorBuffer& cb0, const RenderBuffer* rb1, double s, double t, float progress )
   {
      Vec2D pfr( -1. );
      Vec2D pto( -1. );
      Vec2D p( s, t );
      double middleSlit = 2.0 * fabs(p.x-0.5) - progress;
      if ( middleSlit > 0.0 )
      {
         pfr = p + (p.x > 0.5 ? -1.0 : 1.0) * Vec2D(0.5*progress, 0.0);
         double d = 1.0/(1.0+perspective*progress*(1.0-middleSlit));
         pfr.y -= d/2.;
         pfr.y *= d;
         pfr.y += d/2.;
      }
      double size = lerp( 1.0, depth, 1.-progress );
      pto = (p + Vec2D(-0.5, -0.5)) * Vec2D(size, size) + Vec2D(0.5, 0.5);

      if ( inBounds( pfr ) ) // sliding left/right
          return ( rb1 == nullptr ) ? xlBLACK : tex2D( *rb1, pfr.x, pfr.y );
      if ( inBounds( pto ) ) // zooming in
          return tex2D( cb0, pto );
      // reflection part
      return bgColor( p, pto, cb0 );
   }
   void doorway( RenderBuffer& rb0, const ColorBuffer& cb0, const RenderBuffer* rb1, double progress )
   {
      if ( progress < 0. || progress > 1. )
         return;
      parallel_for(0, rb0.BufferHt, [&rb0, &cb0, &rb1, progress](int y) {
         double t = double( y ) / ( rb0.BufferHt - 1 );
         for ( int x = 0; x < rb0.BufferWi; ++x ) {
            double s = double( x ) / ( rb0.BufferWi - 1 );
            rb0.SetPixel( x, y, doorway( cb0, rb1, s, t, progress ) );
         }
      }, 25);
   }

   // code for blobs transition
   const float blobsSmoothness = 0.01f;
   const float blobsSeed = 12.9898f;

   float blobsRandom( const Vec2D& co )
   {
      float a = blobsSeed;
      float b = 78.233f;
      float c = 43758.5453f;
      float dt = dot( co, Vec2D(a, b) );
      float sn = dt - 3.14f * floorf(dt / 3.14f);

      float intpart;
      return std::modf( sin( sn ) * c, &intpart );
   }

   float blobsNoise( const Vec2D& st )
   {
       Vec2D i, f;
       f.x = std::modf( st.x, &i.x );
       f.y = std::modf( st.y, &i.y );

      // Four corners in 2D of a tile
      float a = blobsRandom(i);
      float b = blobsRandom(i + Vec2D(1.0, 0.0));
      float c = blobsRandom(i + Vec2D(0.0, 1.0));
      float d = blobsRandom(i + Vec2D(1.0, 1.0));

      // Cubic Hermine Curve.  Same as SmoothStep()
      Vec2D u( f*f*(3.0-2.0*f) );

      // Mix 4 coorners porcentages
      return lerp( a, b, u.x ) +
             (c - a) * u.y * ( 1.0 - u.x ) +
             (d - b) * u.x * u.y;
   }
   xlColor blobs( const ColorBuffer& cb0, const RenderBuffer* rb1, double s, double t, double progress, double scale )
   {
      xlColor fromColor( (rb1 == nullptr) ? xlBLACK : tex2D( *rb1, s, t ) );
      xlColor toColor( tex2D( cb0, s, t ) );
      float n = blobsNoise( scale * Vec2D( s, t ) );

      float p = lerp( -blobsSmoothness, 1.- + blobsSmoothness, progress );
      float lo = p - blobsSmoothness;
      float hi = p + blobsSmoothness;

      float q = SmoothStep( lo, hi, n );

      return lerp( fromColor, toColor, 1.f - q );
   }
   void blobs( RenderBuffer& rb0, const ColorBuffer& cb0, const RenderBuffer* rb1, double progress, int adjust )
   {
      if ( progress < 0. || progress > 1. )
         return;
      double scale = interpolate( double( adjust ), 0., 4., 100., 14., LinearInterpolater() );
      parallel_for(0, rb0.BufferHt, [&rb0, &cb0, &rb1, progress, scale](int y) {
         double t = double( y ) / ( rb0.BufferHt - 1 );
         for ( int x = 0; x < rb0.BufferWi; ++x ) {
            double s = double( x ) / ( rb0.BufferWi - 1 );
            rb0.SetPixel( x, y, blobs( cb0, rb1, s, t, progress, scale ) );
         }
      }, 25);
   }

   // code for pinwheel transition
   xlColor pinwheelTransition( const ColorBuffer& cb0, const RenderBuffer* rb1, double s, double t, double progress, double wheelAdjust )
   {
      double x = s - 0.5;
      double y = t - 0.5;
      if ( t < 0.5 ) // this seems needed due to differences between GLSL and C++ versions of atan2()
      {
         y = -y;
         x = -x;
      }
      double arcTangent = std::atan2( y, x );
      double dummy;
      double toProgress = std::modf( arcTangent / M_PI * wheelAdjust , &dummy );
      return ( toProgress > progress ) ? ( tex2D( cb0, s, t ) ) : ( (rb1 == nullptr) ? xlBLACK : tex2D( *rb1, s, t ) );
   }
   void pinwheelTransition( RenderBuffer& rb0, const ColorBuffer& cb0, const RenderBuffer* rb1, double progress, int wheelAdjust )
   {
      if ( progress < 0. || progress > 1. )
         return;
      double adjust = std::floor( interpolate( wheelAdjust, 0., 3.0, 100., 10.0, LinearInterpolater() ) );

      parallel_for(0, rb0.BufferHt, [&rb0, &cb0, &rb1, progress, adjust](int y) {
         double t = double( y ) / ( rb0.BufferHt - 1 );
         for ( int x = 0; x < rb0.BufferWi; ++x ) {
            double s = double( x ) / ( rb0.BufferWi - 1 );
            rb0.SetPixel( x, y, pinwheelTransition( cb0, rb1, s, t, progress, adjust ) );
         }
      }, 25);
   }

   // code for star transition
   xlColor starTransition( const ColorBuffer& cb, const RenderBuffer* rb1, double s, double t, double progress, int numSegments, bool shouldReverse )
   {
      Vec2D xy( s, t );

      double angle = std::atan2( xy.y - 0.5, xy.x - 0.5 ) - 0.5 * PI;
      double radius = ( cos( numSegments * angle ) + 4.0) / 4.0;
      double difference = Vec2D( xy - Vec2D( 0.5, 0.5 ) ).Len();

      if ( shouldReverse )
      {
       if ( difference > radius * progress )
         return tex2D( cb, xy );
       else
         return ( rb1 == nullptr ) ? xlBLACK : tex2D( *rb1, xy.x, xy.y );      }
      else
      {
      if ( difference > radius * progress )
         return ( rb1 == nullptr ) ? xlBLACK : tex2D( *rb1, xy.x, xy.y );
      else
         return tex2D( cb, xy );
      }
   }
   void starTransition( RenderBuffer& rb0, const ColorBuffer& cb0, const RenderBuffer* rb1, double progress, int adjustValue, bool shouldReverse )
   {
      if ( progress < 0. || progress > 1. )
         return;

      // want to default to a 6-point star at 50%
      int numSegments = ( adjustValue == 0 ) ? 1 : ( 1 + adjustValue / 10 );
      if ( shouldReverse )
         progress = 1. - progress;

      // for this transition, we fudge the progress a bit b/c not much happens at the end
      progress = interpolate( progress, 0.0, 0.0, 1.0, 0.85, LinearInterpolater() );

      parallel_for(0, rb0.BufferHt, [&rb0, &cb0, &rb1, progress, numSegments, shouldReverse](int y) {
         double t = double( y ) / ( rb0.BufferHt - 1 );
         for ( int x = 0; x < rb0.BufferWi; ++x ) {
            double s = double( x ) / ( rb0.BufferWi - 1 );
            rb0.SetPixel( x, y, starTransition( cb0, rb1, s, t, progress, numSegments, shouldReverse ) );
         }
      }, 25);
   }

   // code for swap transition
   namespace SwapTransitionCode
   {
      double reflection = 0.4f;

      const Vec2D boundMin( 0.0, 0.0 );
      const Vec2D boundMax( 1.0, 1.0 );
      bool lessThan( const Vec2D&  lhs, const Vec2D& rhs )
      {
         return lhs.x < rhs.x && lhs.y < rhs.y;
      }
      bool inBounds_for_swap( const Vec2D& p )
      {
         return lessThan( boundMin, p ) && lessThan( p, boundMax );
      }
      Vec2D project_for_swap( const Vec2D& p )
      {
         return p * Vec2D( 1.0, -1.2 ) + Vec2D( 0.0, -0.02 );
      }
      xlColor bgColor( const Vec2D& p, const Vec2D& pfr, const Vec2D& pto, const ColorBuffer& toBuffer, const RenderBuffer* fromBuffer )
      {
         xlColor c = xlBLACK;

         Vec2D projectedPFR( project_for_swap( pfr ) );
         if ( inBounds_for_swap( projectedPFR ) )
         {
            c += lerp( xlBLACK, (fromBuffer == nullptr) ? xlBLACK : tex2D( *fromBuffer, projectedPFR.x, projectedPFR.y ), reflection * lerp(1.0, 0.0, projectedPFR.y) );
         }

         Vec2D projectedPTO( project_for_swap( pto ) );
         if ( inBounds_for_swap( projectedPTO ) )
         {
            c += lerp( xlBLACK, tex2D( toBuffer, projectedPTO ), reflection * lerp( 1.0, 0.0, projectedPTO.y ) );
         }
         return c;
      }
   }
   xlColor swapTransition( const ColorBuffer& cb, const RenderBuffer* rb1, double s, double t, double progress )
   {
      double size = lerp( 1.0, depth, progress );
      double persp = perspective * progress;

      Vec2D pto( -1.0, -1.0 );
      Vec2D pfr( ( Vec2D( s, t ) + Vec2D(-0.0, -0.5 ) ) * Vec2D( size / (1.0 - perspective * progress), size / (1.0 - size * persp * s) ) + Vec2D( 0.0, 0.5 ) );

      size = lerp( 1.0, depth, 1.-progress );
      persp = perspective * (1. - progress );
      pto = ( Vec2D( s, t ) + Vec2D( -1.0, -0.5 ) ) * Vec2D( size / (1.0-perspective*(1.0-progress) ), size / (1.0-size*persp*(0.5-s) ) ) + Vec2D( 1.0, 0.5 );

      if ( progress < 0.5 )
      {
         if ( SwapTransitionCode::inBounds_for_swap( pfr ) )
             return (rb1 == nullptr) ? xlBLACK : tex2D( *rb1, pfr.x, pfr.y );
         if ( SwapTransitionCode::inBounds_for_swap( pto ) )
            return tex2D( cb, pto );
      }

      if ( SwapTransitionCode::inBounds_for_swap( pto ) )
         return tex2D( cb, pto );
      if ( SwapTransitionCode::inBounds_for_swap( pfr ) )
         return (rb1 == nullptr) ? xlBLACK : tex2D( *rb1, pfr.x, pfr.y );
      return SwapTransitionCode::bgColor( Vec2D( s, t ), pfr, pto, cb, rb1 );
   }

   void swapTransition( RenderBuffer& rb0, const ColorBuffer& cb0, const RenderBuffer* rb1, double progress )
   {
      if ( progress < 0. || progress > 1. )
         return;

      parallel_for(0, rb0.BufferHt, [&rb0, &cb0, &rb1, progress](int y) {
         double t = double( y ) / ( rb0.BufferHt - 1 );
         for ( int x = 0; x < rb0.BufferWi; ++x ) {
            double s = double( x ) / ( rb0.BufferWi - 1 );
            rb0.SetPixel( x, y, swapTransition( cb0, rb1, s, t, progress ) );
         }
      }, 25);
   }

   // code for shatter transition
   namespace ShatterTransitionCode
   {
      // procedural white noise
      Vec2D hash2(  const Vec2D& p )
      {
         double a = dot( p, Vec2D( 127.1, 311.7) );
         double b = dot( p, Vec2D( 269.5, 183.3 ) );
         Vec2D c( RenderBuffer::sin( a ), RenderBuffer::sin( b ) );
         Vec2D d( 4958.5453 * c );
         double dummy1, dummy2;
         return Vec2D( std::modf( d.x, &dummy1 ), std::modf( d.y, &dummy2 ) );
      }

      Vec2D voronoi( const Vec2D& x )
      {
         Vec2D n;
         Vec2D f( std::modf( x.x, &n.x), std::modf( x.y, &n.y ) );
         Vec2D mc;
         double md = 8.0;
         for ( int j = -1; j <= 1; ++j )
         {
            for ( int i = -1; i <= 1; ++i )
            {
               Vec2D g( i, j );
               Vec2D o( hash2( n + g ) );
               Vec2D r( g + o - f );
               double d = dot( r, r );
               if ( d < md )
               {
                  md = d;
                  mc = x + r;
               }
            }
         }
         return mc;
      }
   }

   xlColor shatterTransition( const ColorBuffer& cb, const RenderBuffer* rb1, double s, double t, double progress )
   {
      double num = 8.;

      Vec2D texCentered( Vec2D( s, t ) - Vec2D( 0.5, 0.5 ) );
      double ang = std::atan2( texCentered.y, texCentered.x );
      double a = progress * 8;
      double originalLength = ( -1. + std::sqrt( 1. + 4. * a * texCentered.Len() ) ) / (2. * a);
      if ( a == 0. )
         originalLength = texCentered.Len();

      Vec2D originalLocation( Vec2D( 0.5, 0.5 )  + originalLength * Vec2D( RenderBuffer::cos( ang ), RenderBuffer::sin( ang ) ) );
      //Vec2D ol( texCentered / (progress + 1. ) + Vec2D( 0.5, 0.5 ) );
      Vec2D originalShard( ShatterTransitionCode::voronoi( num * originalLocation ) );
      Vec2D originalCenter( originalShard.x / num, originalShard.y / num );
      double ca = std::atan2( originalCenter.y - 0.5, originalCenter.x - 0.5 );
      double originalCenterLength = Vec2D( originalCenter - Vec2D( 0.5, 0.5 ) ).Len();
      double currentCenterLength = originalCenterLength + originalCenterLength * originalCenterLength * a;
      Vec2D currentCenter( Vec2D( 0.5, 0.5 ) + currentCenterLength * Vec2D( RenderBuffer::cos( ca ), RenderBuffer::sin( ca ) ) );
      Vec2D c4( ( Vec2D( s, t ) - currentCenter ) / ( 1. + 0.6 *progress ) + originalCenter );
      Vec2D currentShard( ShatterTransitionCode::voronoi( num * c4 ) );

      double transition = 1.;
      if ( originalShard.Dist( currentShard ) < 0.0001 )
         transition = SmoothStep( 0.70, 1.0, progress );

      xlColor toColor = tex2D( cb, c4 );
      xlColor fromColor = ( rb1 == nullptr ) ? xlBLACK : tex2D( *rb1, s, t );
      return lerp( toColor, fromColor, transition );
   }

   void shatterTransition( RenderBuffer& rb0, const ColorBuffer& cb0, const RenderBuffer* rb1, double progress )
   {
      if ( progress < 0. || progress > 1. )
         return;

      parallel_for(0, rb0.BufferHt, [&rb0, &cb0, &rb1, progress](int y) {
         double t = double( y ) / ( rb0.BufferHt - 1 );
         for ( int x = 0; x < rb0.BufferWi; ++x ) {
            double s = double( x ) / ( rb0.BufferWi - 1 );
            rb0.SetPixel( x, y, shatterTransition( cb0, rb1, s, t, progress ) );
         }
      }, 25);
   }

   // code for circles transition
   xlColor circlesTransition( const ColorBuffer& cb, const RenderBuffer* rb1, double s, double t, double progress, double n )
   {
      const int NumCircles = 3;

      double dummy1, dummy2;
      Vec2D cell( std::modf( s * n, &dummy1 ), std::modf( t * n , &dummy2 ) );

      double m = 0.;

      double alphaPerCircle = 1. / double( NumCircles ) + 0.01;
      for ( int i = 0; i < NumCircles; ++i )
      {
         double delay = i * /*0.3*/0.5 / (NumCircles - 1);
         double p = std::max( 0., progress - delay );
         m += alphaPerCircle * ( 1. - SmoothStep( p * 1.40, p * 1.45, Vec2D( cell - Vec2D( 0.5, 0.5 ) ).Len() ) );
      }

      m = std::min( 1., m );
      return lerp ( tex2D( cb, s, t ), ( rb1 == nullptr ) ? xlBLACK : tex2D( *rb1, s, t ), m );
   }

   void circlesTransition( RenderBuffer& rb0, const ColorBuffer& cb0, const RenderBuffer* rb1, double progress, int adjustValue )
   {
      if ( progress < 0. || progress > 1. )
         return;
      double n = std::floor( interpolate( double( adjustValue ), 0., 2., 100., 8., LinearInterpolater() ) );

      parallel_for(0, rb0.BufferHt, [&rb0, &cb0, &rb1, progress, n](int y) {
         double t = double( y ) / ( rb0.BufferHt - 1 );
         for ( int x = 0; x < rb0.BufferWi; ++x ) {
            double s = double( x ) / ( rb0.BufferWi - 1 );
            rb0.SetPixel( x, y, circlesTransition( cb0, rb1, s, t, progress, n ) );
         }
      }, 25);
   }
}

PixelBufferClass::PixelBufferClass(xLightsFrame *f) : frame(f)
{
    frameTimeInMs = 50;
    model = nullptr;
    numLayers = 0;
    zbModel = nullptr;
    ssModel = nullptr;
}

PixelBufferClass::~PixelBufferClass()
{
    if (ssModel != nullptr) {
        delete ssModel;
    }
    if (zbModel != nullptr) {
        delete zbModel;
    }
    for (int x = 0; x < numLayers; x++) {
        delete layers[x];
    }
    if (gpuRenderData) {
        GPURenderUtils::cleanUp(this);
    }
}


void PixelBufferClass::reset(int nlayers, int timing, bool isNode)
{
    for (int x = 0; x < numLayers; x++)
    {
        delete layers[x];
    }
    layers.clear();
    frameTimeInMs = timing;

    numLayers = nlayers;
    layers.resize(nlayers);

    for (int x = 0; x < numLayers; x++)
    {
        layers[x] = new LayerInfo(frame);
        layers[x]->buffer.SetFrameTimeInMs(frameTimeInMs);
        if (x == (numLayers-1)) {
            // for the model "blend" layer, use the "Single Line" style so none of the nodes will overlap with others
            // in the renderbuff which can occur if the group defaults to per-preview or similar
            model->InitRenderBufferNodes("Single Line", "2D", "None", layers[x]->buffer.Nodes, layers[x]->BufferWi, layers[x]->BufferHt, layers[x]->stagger);
            layers[x]->bufferType = "Single Line";
        } else {
            model->InitRenderBufferNodes("Default", "2D", "None", layers[x]->buffer.Nodes, layers[x]->BufferWi, layers[x]->BufferHt, layers[x]->stagger);
            layers[x]->bufferType = "Default";
        }
        layers[x]->camera = "2D";
        layers[x]->bufferTransform = "None";
        layers[x]->outTransitionType = "Fade";
        layers[x]->inTransitionType = "Fade";
        layers[x]->subBuffer = "";
        layers[x]->isChromaKey = false;
        layers[x]->chromaSensitivity = 1;
        layers[x]->freezeAfterFrame = 99999;
        layers[x]->suppressUntil = 0;
        layers[x]->chromaKeyColour = *wxBLACK;
        layers[x]->sparklesColour = *wxWHITE;
        layers[x]->brightnessValueCurve = "";
        layers[x]->hueAdjustValueCurve = "";
        layers[x]->saturationAdjustValueCurve = "";
        layers[x]->valueAdjustValueCurve = "";
        layers[x]->blurValueCurve = "";
        layers[x]->sparklesValueCurve = "";
        layers[x]->rotationValueCurve = "";
        layers[x]->xrotationValueCurve = "";
        layers[x]->yrotationValueCurve = "";
        layers[x]->zoomValueCurve = "";
        layers[x]->rotationsValueCurve = "";
        layers[x]->pivotpointxValueCurve = "";
        layers[x]->pivotpointyValueCurve = "";
        layers[x]->xpivotValueCurve = "";
        layers[x]->ypivotValueCurve = "";
        layers[x]->BufferOffsetX = 0;
        layers[x]->BufferOffsetY = 0;
        layers[x]->stagger = 0;
        layers[x]->buffer.InitBuffer(layers[x]->BufferHt, layers[x]->BufferWi, layers[x]->bufferTransform, isNode);
        GPURenderUtils::setupRenderBuffer(this, &layers[x]->buffer);
    }
}

void PixelBufferClass::InitPerModelBuffers(const ModelGroup& model, int layer, int timing)
{
    for (const auto& it : model.ActiveModels()) {
        Model* m = it;
        wxASSERT(m != nullptr);
        RenderBuffer* buf = new RenderBuffer(frame);
        buf->SetFrameTimeInMs(timing);
        m->InitRenderBufferNodes("Default", "2D", "None", buf->Nodes, buf->BufferWi, buf->BufferHt, 0);
        buf->InitBuffer(buf->BufferHt, buf->BufferWi, "None");
        GPURenderUtils::setupRenderBuffer(this, buf);
        layers[layer]->shallowModelBuffers.push_back(std::unique_ptr<RenderBuffer>(buf));
    }
}

void PixelBufferClass::InitPerModelBuffersDeep(const ModelGroup& model, int layer, int timing)
{
    for (const auto& it : model.GetFlatModels(false, true)) {
        Model* m = it;
        wxASSERT(m != nullptr);
        RenderBuffer* buf = new RenderBuffer(frame);
        buf->SetFrameTimeInMs(timing);
        m->InitRenderBufferNodes("Default", "2D", "None", buf->Nodes, buf->BufferWi, buf->BufferHt, 0);
        buf->InitBuffer(buf->BufferHt, buf->BufferWi, "None");
        GPURenderUtils::setupRenderBuffer(this, buf);
        layers[layer]->deepModelBuffers.push_back(std::unique_ptr<RenderBuffer>(buf));
    }
}

void PixelBufferClass::InitBuffer(const Model& pbc, int layers, int timing, bool zeroBased)
{
    modelName = pbc.GetFullName();
    if (zeroBased) {
        zbModel = pbc.GetModelManager().CreateModel(pbc.GetModelXml(), 0, 0, zeroBased);
        model = zbModel;
    } else {
        model = &pbc;
    }
    reset(layers + 1, timing);
}

void PixelBufferClass::InitStrandBuffer(const Model &pbc, int strand, int timing, int layers)
{
    if (ssModel == nullptr) {
        ssModel = new SingleLineModel(pbc.GetModelManager());
    }

    ssModel->Reset(pbc.GetStrandLength(strand), pbc, strand);
    model = ssModel;
    reset(layers + 1, timing);
}

void PixelBufferClass::InitNodeBuffer(const Model &pbc, int strand, int node, int timing)
{
    modelName = pbc.GetFullName();
    if (ssModel == nullptr) {
        ssModel = new SingleLineModel(pbc.GetModelManager());
    }
    ssModel->Reset(1, pbc, strand, node);
    model = ssModel;
    reset(2, timing, true);
}

void PixelBufferClass::Clear(int which)
{
    if (which != -1) {
        layers[which]->clear(); //just clear this one
    } else {
        for (auto &a : layers) {
            a->clear();
        }
    }
}

void PixelBufferClass::GetNodeChannelValues(size_t nodenum, unsigned char *buf)
{
    layers[0]->buffer.Nodes[nodenum]->GetForChannels(buf);
}
void PixelBufferClass::SetNodeChannelValues(size_t nodenum, const unsigned char *buf)
{
    layers[0]->buffer.Nodes[nodenum]->SetFromChannels(buf);
}
xlColor PixelBufferClass::GetNodeColor(size_t nodenum) const
{
    xlColor color;
    layers[0]->buffer.Nodes[nodenum]->GetColor(color);
    return color;
}
xlColor PixelBufferClass::GetNodeMaskColor(size_t nodenum) const
{
    xlColor color;
    layers[0]->buffer.Nodes[nodenum]->GetMaskColor(color);
    return color;
}
uint32_t PixelBufferClass::NodeStartChannel(size_t nodenum) const
{
    return layers[0]->buffer.Nodes.size() && nodenum < layers[0]->buffer.Nodes.size() ? layers[0]->buffer.Nodes[nodenum]->ActChan: 0;
}
uint32_t PixelBufferClass::GetNodeCount() const
{
    return layers[0]->buffer.Nodes.size();
}
uint32_t PixelBufferClass::GetChanCountPerNode() const
{
    uint32_t NodeCnt=GetNodeCount();
    if (NodeCnt == 0)
    {
        return 0;
    }
    return layers[0]->buffer.Nodes[0]->GetChanCount();
}

bool MixTypeHandlesAlpha(MixTypes mt)
{
    return mt == MixTypes::Mix_Normal;
}

static std::map<std::string, MixTypes> MixTypesMap = {
    { "Effect 1", MixTypes::Mix_Effect1 },
    { "Effect 2", MixTypes::Mix_Effect2 },
    { "1 is Mask", MixTypes::Mix_Mask1 },
    { "2 is Mask", MixTypes::Mix_Mask2 },
    { "1 is Unmask", MixTypes::Mix_Unmask1 },
    { "2 is Unmask", MixTypes::Mix_Unmask2 },
    { "1 is True Unmask", MixTypes::Mix_TrueUnmask1 },
    { "2 is True Unmask", MixTypes::Mix_TrueUnmask2 },
    { "1 reveals 2", MixTypes::Mix_1_reveals_2 },
    { "2 reveals 1", MixTypes::Mix_2_reveals_1 },
    { "Shadow 1 on 2", MixTypes::Mix_Shadow_1on2 },
    { "Shadow 2 on 1", MixTypes::Mix_Shadow_2on1 },
    { "Layered", MixTypes::Mix_Layered },
    { "Normal", MixTypes::Mix_Normal },
    { "Highlight", MixTypes::Mix_Highlight },
    { "Highlight Vibrant", MixTypes::Mix_Highlight_Vibrant },
    { "Additive", MixTypes::Mix_Additive },
    { "Subtractive", MixTypes::Mix_Subtractive },
    { "Brightness", MixTypes::Mix_AsBrightness },
    { "Average", MixTypes::Mix_Average },
    { "Bottom-Top", MixTypes::Mix_BottomTop },
    { "Left-Right", MixTypes::Mix_LeftRight },
    { "Max", MixTypes::Mix_Max },
    { "Min", MixTypes::Mix_Min }
};

std::vector<std::string> PixelBufferClass::GetMixTypes()
{
    std::vector <std::string> res;
    for (const auto& it : MixTypesMap) {
        res.push_back(it.first);
    }
    return res;
}

// convert MixName to MixType enum
void PixelBufferClass::SetMixType(int layer, const std::string& MixName)
{
    auto it = MixTypesMap.find(MixName);
    if (it == MixTypesMap.end()) {
        layers[layer]->mixType = MixTypes::Mix_Effect1;
    } else {
        layers[layer]->mixType = it->second;
    }
    layers[layer]->buffer.SetAllowAlphaChannel(MixTypeHandlesAlpha(layers[layer]->mixType));
}

double ColourDistance(xlColor e1, xlColor e2)
{
    long rmean = ((long)e1.red + (long)e2.red) / 2;
    long r = (long)e1.red - (long)e2.red;
    long g = (long)e1.green - (long)e2.green;
    long b = (long)e1.blue - (long)e2.blue;
    return sqrt((((512 + rmean) * r * r) >> 8) + 4 * g * g + (((767 - rmean) * b * b) >> 8));
}

void PixelBufferClass::mixColors(const wxCoord &x, const wxCoord &y, xlColor &fg, xlColor &bg, int layerNum)
{
    static const int n = 0;  //increase to change the curve of the crossfade

    LayerInfo *layer = layers[layerNum];
    if (!layer->buffer.allowAlpha && layer->fadeFactor != 1.0) {
        //need to fade the first here as we're not mixing anything
        HSVValue hsv0 = fg.asHSV();
        hsv0.value *= layer->fadeFactor;
        fg = hsv0;
    }

    // Apply ChromaKey if it is enabled
    if (layer->isChromaKey) {
        xlColor c(fg);
        if (c.alpha < 255) {
            c.red = (int)(c.red * c.alpha) / 255;
            c.green = (int)(c.green * c.alpha) / 255;
            c.blue = (int)(c.blue * c.alpha) / 255;
            c.alpha = 255;
        }
        if (ColourDistance(c, layer->chromaKeyColour) < layer->chromaSensitivity * 402 / 255) {
            return;
        }
    }

    float effectMixThreshold = layer->outputEffectMixThreshold;
    switch (layer->mixType)
    {
    case MixTypes::Mix_Normal:
        fg.alpha = fg.alpha * layer->fadeFactor * (1.0 - effectMixThreshold);
        bg.AlphaBlendForgroundOnto(fg);
        break;
    case MixTypes::Mix_Effect1:
    case MixTypes::Mix_Effect2:
    {
        double emt, emtNot;
        if (!layer->effectMixVaries) {
            emt = effectMixThreshold;
            if ((emt > 0.000001) && (emt < 0.99999)) {
                emtNot = 1 - effectMixThreshold;
                //make cross-fade linear
                emt = cos((M_PI/4)*(pow(2*emt-1,2*n+1)+1));
                emtNot = cos((M_PI/4)*(pow(2*emtNot-1,2*n+1)+1));
            } else {
                emtNot = effectMixThreshold;
                emt = 1 - effectMixThreshold;
            }
        } else {
            emt = effectMixThreshold;
            emtNot = 1 - effectMixThreshold;
        }

        if (layer->mixType == MixTypes::Mix_Effect2) {
            fg.Set(fg.Red()*(emtNot),fg.Green()*(emtNot), fg.Blue()*(emtNot));
            bg.Set(bg.Red()*(emt),bg.Green()*(emt), bg.Blue()*(emt));
        } else {
            fg.Set(fg.Red()*(emt),fg.Green()*(emt), fg.Blue()*(emt));
            bg.Set(bg.Red()*(emtNot),bg.Green()*(emtNot), bg.Blue()*(emtNot));
        }
        bg.Set(fg.Red()+bg.Red(), fg.Green()+bg.Green(), fg.Blue()+bg.Blue());
        break;
    }
    case MixTypes::Mix_Mask1:
    {
        // first masks second
        HSVValue hsv0 = fg.asHSV();
        if (hsv0.value > effectMixThreshold) {
            bg.Set(0, 0, 0);
        }
        break;
    }
    case MixTypes::Mix_Mask2:
    {
        // second masks first
        HSVValue hsv1 = bg.asHSV();
        if (hsv1.value <= effectMixThreshold) {
            bg = fg;
        } else {
            bg.Set(0, 0, 0);
        }
        break;
    }
    case MixTypes::Mix_Unmask1:
    {
        // first unmasks second
        HSVValue hsv0 = fg.asHSV();
        if (hsv0.value > effectMixThreshold) {
            HSVValue hsv1 = bg.asHSV();
            hsv1.value = hsv0.value;
            bg = hsv1;
        } else {
            bg.Set(0, 0, 0);
        }
        break;
    }
    case MixTypes::Mix_TrueUnmask1:
    {
        // first unmasks second
        HSVValue hsv0 = fg.asHSV();
        if (hsv0.value <= effectMixThreshold) {
            bg.Set(0, 0, 0);
        }
        break;
    }
    case MixTypes::Mix_Unmask2:
    {
        // second unmasks first
        HSVValue hsv1 = bg.asHSV();
        if (hsv1.value > effectMixThreshold) {
            HSVValue hsv0 = fg.asHSV();
            // if effect 2 is non black
            hsv0.value = hsv1.value;
            bg = hsv0;
        } else {
            bg.Set(0, 0, 0);
        }
        break;
    }
    case MixTypes::Mix_TrueUnmask2:
    {
        // second unmasks first
        HSVValue hsv1 = bg.asHSV();
        if (hsv1.value > effectMixThreshold) {
            // if effect 2 is non black
            bg = fg;
        } else {
            bg.Set(0, 0, 0);
        }
        break;
    }
    case MixTypes::Mix_Shadow_1on2:
    {
        // Effect 1 shadows onto effect 2
        HSVValue hsv0 = fg.asHSV();
        HSVValue hsv1 = bg.asHSV();
        //   if (hsv0.value > effectMixThreshold[layer]) {
        // if effect 1 is non black
        //  to shadow we will shift the hue on the primary layer using the hue and brightness from the
        //  other layer
        if(hsv0.value>0.0) hsv1.hue = hsv1.hue + (hsv0.value*(hsv1.hue-hsv0.hue))/5.0;
        // hsv1.value = hsv0.value;
        //hsv1.saturation = hsv0.saturation;
        bg = hsv1;
        //   }
        break;
    }
    case MixTypes::Mix_Shadow_2on1:
    {
        // Effect 2 shadows onto effect 1
        HSVValue hsv0 = fg.asHSV();
        HSVValue hsv1 = bg.asHSV();
        // if effect 1 is non black
        if(hsv1.value>0.0) {
            hsv0.hue = hsv0.hue + (hsv1.value*(hsv0.hue-hsv1.hue))/2.0;
        }
        bg = hsv0;
        break;
    }
    case MixTypes::Mix_Layered:
    {
        HSVValue hsv1 = bg.asHSV();
        if (hsv1.value <= effectMixThreshold) {
            bg = fg;
        }
        break;
    }
    case MixTypes::Mix_Average:
        // only average when both colors are non-black
        if (bg == xlBLACK) {
            bg = fg;
        } else if (fg != xlBLACK) {
            bg.Set((fg.Red() + bg.Red()) / 2, (fg.Green() + bg.Green()) / 2, (fg.Blue() + bg.Blue()) / 2, (fg.alpha + bg.alpha) / 2);
        }
        break;
    case MixTypes::Mix_BottomTop:
        bg = y < layer->BufferHt/2 ? fg : bg;
        break;
    case MixTypes::Mix_LeftRight:
        bg = x < layer->BufferWi/2 ? fg : bg;
        break;
    case MixTypes::Mix_1_reveals_2:
    {
        HSVValue hsv0 = fg.asHSV();
        bg = hsv0.value > effectMixThreshold ? fg : bg; // if effect 1 is non black
        break;
    }
    case MixTypes::Mix_2_reveals_1:
    {
        HSVValue hsv1 = bg.asHSV();
        bg = hsv1.value > effectMixThreshold ? bg : fg; // if effect 2 is non black
        break;
    }
    case MixTypes::Mix_Highlight:
    {
        bool effect1HasColor = (fg.red > 0 || fg.green > 0 || fg.blue > 0);
        bool effect2HasColor = (bg.red > 0 || bg.green > 0 || bg.blue > 0);
        HSVValue hsv1 = bg.asHSV();

        if (effect1HasColor && (effect2HasColor || hsv1.value > effectMixThreshold)) {
            bg = fg;
        }
    } break;
    case MixTypes::Mix_Highlight_Vibrant:
    {
        HSVValue hsv1 = bg.asHSV();
        if (hsv1.value > effectMixThreshold) {
            
            int r = fg.red + bg.red;
            int g = fg.green + bg.green;
            int b = fg.blue + bg.blue;

            if (r > 255)
                r = 255;
            if (g > 255)
                g = 255;
            if (b > 255)
                b = 255;

            bg.Set(r, g, b);
        }
    } break;
    case MixTypes::Mix_Additive:
        {
            int r = fg.red + bg.red;
            int g = fg.green + bg.green;
            int b = fg.blue + bg.blue;
            if (r > 255) r = 255;
            if (g > 255) g = 255;
            if (b > 255) b = 255;
            bg.Set(r, g, b);
        }
        break;
    case MixTypes::Mix_Subtractive:
        {
            int r = bg.red - fg.red;
            int g = bg.green - fg.green;
            int b = bg.blue - fg.blue;
            if (r < 0)
                r = 0;
            if (g < 0) g = 0;
            if (b < 0) b = 0;
            bg.Set(r, g, b);
        }
        break;

    case MixTypes::Mix_Min:
        {
            float alpha = (float)fg.alpha / 255.0;
            int r = std::min(fg.red, bg.red) * alpha;
            int g = std::min(fg.green, bg.green) * alpha;
            int b = std::min(fg.blue, bg.blue) * alpha;
            bg.Set(r, g, b);
        }
        break;
    case MixTypes::Mix_Max:
        {
            float alpha = (float)fg.alpha / 255.0;
            int r = std::max(fg.red, bg.red) * alpha;
            int g = std::max(fg.green, bg.green) * alpha;
            int b = std::max(fg.blue, bg.blue) * alpha;
            bg.Set(r, g, b);
        }
        break;
    case MixTypes::Mix_AsBrightness:
        {
        float alpha = (float)fg.alpha / 255.0;
        int r = fg.red * bg.red / 255 * alpha;
        int g = fg.green * bg.green / 255 * alpha;
        int b = fg.blue * bg.blue / 255 * alpha;
        bg.Set(r, g, b);
    }
        break;
    }
}

void PixelBufferClass::GetMixedColor(int node, const std::vector<bool> & validLayers, int EffectPeriod, int saveLayer)
{
    auto &sparkle = layers[0]->buffer.Nodes[node]->sparkle;
    int cnt = 0;
    xlColor c(xlBLACK);
    xlColor color;

    for (int layer = numLayers - 1; layer >= 0; layer--) {
        if (validLayers[layer]) {
            auto thelayer = layers[layer];
            if (node >= thelayer->buffer.Nodes.size()) {
                //logger_base.crit("PixelBufferClass::GetMixedColor thelayer->buffer.Nodes does not contain node %d as it is only %d in size ... this was going to crash.", node, thelayer->buffer.Nodes.size());
            } else {
                auto &coord = thelayer->buffer.Nodes[node]->Coords[0];
                int x = coord.bufX;
                int y = coord.bufY;

                if (thelayer->isMasked(x, y)
                    || x < 0
                    || y < 0
                    || x >= thelayer->BufferWi
                    || y >= thelayer->BufferHt
                    ) {
                    color.Set(0, 0, 0, 0);
                } else {
                    thelayer->buffer.GetPixel(x, y, color);
                }

                // adjust for HSV adjustments
                if (thelayer->needsHSVAdjust) {
                    HSVValue hsv = color.asHSV();

                    if (thelayer->outputHueAdjust != 0) {
                        hsv.hue += thelayer->outputHueAdjust;
                        if (hsv.hue < 0) {
                            hsv.hue += 1.0;
                        } else if (hsv.hue > 1) {
                            hsv.hue -= 1.0;
                        }
                    }

                    if (thelayer->outputSaturationAdjust != 0) {
                        hsv.saturation += thelayer->outputSaturationAdjust;
                        if (hsv.saturation < 0) {
                            hsv.saturation = 0.0;
                        } else if (hsv.saturation > 1) {
                            hsv.saturation = 1.0;
                        }
                    }

                    if (thelayer->outputValueAdjust != 0) {
                        hsv.value += thelayer->outputValueAdjust;
                        if (hsv.value < 0) {
                            hsv.value = 0.0;
                        } else if (hsv.value > 1) {
                            hsv.value = 1.0;
                        }
                    }

                    unsigned char alpha = color.Alpha();
                    color = hsv;
                    color.alpha = alpha;
                }

                // add sparkles
                if (color != xlBLACK &&
                    (thelayer->use_music_sparkle_count ||
                        thelayer->sparkle_count > 0 ||
                        thelayer->outputSparkleCount > 0)) {

                    int sc = thelayer->outputSparkleCount;
                    switch (sparkle % (208 - sc))
                    {
                    case 1:
                    case 7:
                        // too dim
                        //color.Set("#444444");
                        break;
                    case 2:
                    case 6:
                        color = thelayer->sparklesColour.ApplyBrightness(0.53f);
                        break;
                    case 3:
                    case 5:
                        color = thelayer->sparklesColour.ApplyBrightness(0.75f);
                        break;
                    case 4:
                        color = thelayer->sparklesColour;
                        break;
                    default:
                        break;
                    }
                    sparkle++;
                }
                int b = thelayer->outputBrightnessAdjust;
                if (thelayer->contrast != 0) {
                    //contrast is not 0, can handle brightness change at same time
                    HSVValue hsv = color.asHSV();
                    hsv.value = hsv.value * ((double)b / 100.0);

                    // Apply Contrast
                    if (hsv.value < 0.5) {
                        // reduce brightness when below 0.5 in the V value or increase if > 0.5
                        hsv.value = hsv.value - (hsv.value* ((double)thelayer->contrast / 100.0));
                    } else {
                        hsv.value = hsv.value + (hsv.value* ((double)thelayer->contrast / 100.0));
                    }

                    if (hsv.value < 0.0) hsv.value = 0.0;
                    if (hsv.value > 1.0) hsv.value = 1.0;
                    unsigned char alpha = color.Alpha();
                    color = hsv;
                    color.alpha = alpha;
                } else if (b != 100) {
                    //just brightness
                    float ba = b;
                    ba /= 100.0f;
                    float f = color.red * ba;
                    color.red = std::min((int)f, 255);
                    f = color.green * ba;
                    color.green = std::min((int)f, 255);
                    f = color.blue * ba;
                    color.blue = std::min((int)f, 255);
                }

                if (cnt > 0) {
                    mixColors(x, y, color, c, layer);
                } else if (thelayer->fadeFactor != 1.0) {
                    //need to fade the first here as we're not mixing anything
                    HSVValue hsv = color.asHSV();
                    hsv.value *= thelayer->fadeFactor;
                    if (color.alpha != 255) {
                        hsv.value *= color.alpha;
                        hsv.value /= 255.0f;
                    }
                    c = hsv;
                } else {
                    c.AlphaBlendForgroundOnto(color);
                }

                cnt++;
            }
        }
    }
    // set color for physical output
    layers[saveLayer]->buffer.Nodes[node]->SetColor(c);
}

void PixelBufferClass::GetMixedColor(int lx, int ly, xlColor& c, const std::vector<bool> & validLayers, int EffectPeriod)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    int cnt = 0;
    c = xlBLACK;
    xlColor color;

    for (int layer = numLayers - 1; layer >= 0; layer--) {
        if (validLayers[layer]) {
            auto thelayer = layers[layer];
            
            int x = lx - thelayer->BufferOffsetX;
            int y = ly - thelayer->BufferOffsetY;

            // TEMPORARY - THIS SHOULD BE REMOVED BUT I WANT TO SEE WHAT IS CAUSING SOME RANDOM CRASHES - KW - 2017.7
            if (thelayer == nullptr) {
                logger_base.crit("PixelBufferClass::GetMixedColor thelayer is nullptr ... this is going to crash.");
            }

            if (x >= thelayer->BufferWi || y >= thelayer->BufferHt || x < 0 || y < 0) {
                // out of bounds
            } else {
                int effStartPer, effEndPer;
                thelayer->buffer.GetEffectPeriods(effStartPer, effEndPer);
                float offset = ((float)(EffectPeriod - effStartPer)) / ((float)(effEndPer - effStartPer));
                offset = std::min(offset, 1.0f);

                if (thelayer->isMasked(x, y)
                    || x < 0
                    || y < 0
                    || x >= thelayer->BufferWi
                    || y >= thelayer->BufferHt
                    ) {
                    color.Set(0, 0, 0, 0);
                } else {
                    thelayer->buffer.GetPixel(x, y, color);
                }

                float ha;
                if (thelayer->HueAdjustValueCurve.IsActive()) {
                    ha = thelayer->HueAdjustValueCurve.GetOutputValueAt(offset, thelayer->buffer.GetStartTimeMS(), thelayer->buffer.GetEndTimeMS()) / 100.0;
                } else {
                    ha = (float)thelayer->hueadjust / 100.0;
                }
                float sa;
                if (thelayer->SaturationAdjustValueCurve.IsActive()) {
                    sa = thelayer->SaturationAdjustValueCurve.GetOutputValueAt(offset, thelayer->buffer.GetStartTimeMS(), thelayer->buffer.GetEndTimeMS()) / 100.0;
                } else {
                    sa = (float)thelayer->saturationadjust / 100.0;
                }

                float va;
                if (thelayer->ValueAdjustValueCurve.IsActive()) {
                    va = thelayer->ValueAdjustValueCurve.GetOutputValueAt(offset, thelayer->buffer.GetStartTimeMS(), thelayer->buffer.GetEndTimeMS()) / 100.0;
                } else {
                    va = (float)thelayer->valueadjust / 100.0;
                }

                // adjust for HSV adjustments
                if (ha != 0 || sa != 0 || va != 0) {
                    HSVValue hsv = color.asHSV();

                    if (ha != 0) {
                        hsv.hue += ha;
                        if (hsv.hue < 0) {
                            hsv.hue += 1.0;
                        } else if (hsv.hue > 1) {
                            hsv.hue -= 1.0;
                        }
                    }

                    if (sa != 0) {
                        hsv.saturation += sa;
                        if (hsv.saturation < 0) {
                            hsv.saturation = 0.0;
                        } else if (hsv.saturation > 1) {
                            hsv.saturation = 1.0;
                        }
                    }

                    if (va != 0) {
                        hsv.value += va;
                        if (hsv.value < 0) {
                            hsv.value = 0.0;
                        } else if (hsv.value > 1) {
                            hsv.value = 1.0;
                        }
                    }

                    unsigned char alpha = color.Alpha();
                    color = hsv;
                    color.alpha = alpha;
                }

                int b;
                if (thelayer->BrightnessValueCurve.IsActive()) {
                    b = (int)thelayer->BrightnessValueCurve.GetOutputValueAt(offset, thelayer->buffer.GetStartTimeMS(), thelayer->buffer.GetEndTimeMS());
                } else {
                    b = thelayer->brightness;
                }
                if (thelayer->contrast != 0) {
                    //contrast is not 0, can handle brightness change at same time
                    HSVValue hsv = color.asHSV();
                    hsv.value = hsv.value * ((double)b / 100.0);

                    // Apply Contrast
                    if (hsv.value < 0.5) {
                        // reduce brightness when below 0.5 in the V value or increase if > 0.5
                        hsv.value = hsv.value - (hsv.value* ((double)thelayer->contrast / 100.0));
                    } else {
                        hsv.value = hsv.value + (hsv.value* ((double)thelayer->contrast / 100.0));
                    }

                    if (hsv.value < 0.0) hsv.value = 0.0;
                    if (hsv.value > 1.0) hsv.value = 1.0;
                    unsigned char alpha = color.Alpha();
                    color = hsv;
                    color.alpha = alpha;
                } else if (b != 100) {
                    //just brightness
                    float ba = b;
                    ba /= 100.0f;
                    float f = color.red * ba;
                    color.red = std::min((int)f, 255);
                    f = color.green * ba;
                    color.green = std::min((int)f, 255);
                    f = color.blue * ba;
                    color.blue = std::min((int)f, 255);
                }

                if (cnt > 0) {
                    mixColors(x, y, color, c, layer);
                } else if (thelayer->fadeFactor != 1.0) {
                    //need to fade the first here as we're not mixing anything
                    HSVValue hsv = color.asHSV();
                    hsv.value *= thelayer->fadeFactor;
                    if (color.alpha != 255) {
                        hsv.value *= color.alpha;
                        hsv.value /= 255.0f;
                    }
                    c = hsv;
                } else {
                    c.AlphaBlendForgroundOnto(color);
                }

                cnt++;
            }
        }
    }
}

//http://blog.ivank.net/fastest-gaussian-blur.html
static void boxesForGauss(int d, int n, std::vector<float> &boxes)  // standard deviation, number of boxes
{
    switch (d) {
        case 2:
        case 3:
            boxes.push_back(1.0);
            break;
        case 4:
        case 5:
        case 6:
            boxes.push_back(3.0);
            break;
        case 7:
        case 8:
        case 9:
            boxes.push_back(5.0);
            break;
        case 10:
        case 11:
        case 12:
            boxes.push_back(7.0);
            break;
        case 13:
        case 14:
        case 15:
            boxes.push_back(9.0);
            break;
        default:
            break;
    }
    float b = boxes.back();
    switch (d) {
        case 2:
        case 4:
        case 5:
        case 7:
        case 8:
        case 10:
        case 11:
        case 13:
        case 14:
            boxes.push_back(b);
            break;
        default:
            boxes.push_back(b + 2.0);
            break;
    }
    switch (d) {
        case 4:
        case 7:
        case 10:
        case 13:
            boxes.push_back(b);
            break;
        default:
            boxes.push_back(b + 2.0);
    }
}

#define RED(a, b) a[(b)*4]
#define GREEN(a, b) a[(b)*4 + 1]
#define BLUE(a, b) a[(b)*4 + 2]
#define ALPHA(a, b) a[(b)*4 + 3]
static inline void SET(std::vector<float>& ar, int idx, float r, float g, float b, float a) {
    idx *= 4;
    ar[idx++] = r;
    ar[idx++] = g;
    ar[idx++] = b;
    ar[idx] = a;
}

static void boxBlurH_4 (const std::vector<float>& scl, std::vector<float>& tcl, int w, int h, float r) {
    float iarr = 1.0f / (r+r+1.0f);
    for(int i=0; i<h; i++) {
        int ti = i*w;
        int li = ti;
        int ri = ti+r;
        int maxri = ti + w - 1;
        int fvIdx = ti;
        int lvIdx = ti+w-1;

        float valr = (r+1.0) * RED(scl,fvIdx);
        float valg = (r+1.0) * GREEN(scl,fvIdx);
        float valb = (r+1.0) * BLUE(scl,fvIdx);
        float vala = (r+1.0) * ALPHA(scl,fvIdx);

        float fvRed = RED(scl, fvIdx);
        float fvGreen = GREEN(scl, fvIdx);
        float fvBlue = BLUE(scl, fvIdx);
        float fvAlpha = ALPHA(scl, fvIdx);
        float lvRed = RED(scl, lvIdx);
        float lvGreen = GREEN(scl, lvIdx);
        float lvBlue = BLUE(scl, lvIdx);
        float lvAlpha = ALPHA(scl, lvIdx);

        for (int j=0; j<r; j++) {
            int idx = j < w ? ti+j : lvIdx;
            valr += RED(scl, idx);
            valg += GREEN(scl, idx);
            valb += BLUE(scl, idx);
            vala += ALPHA(scl, idx);
        }
        for (int j=0  ; j<=r ; j++) {
            int idx = ri <= maxri ? ri++ : lvIdx;
            valr += RED(scl, idx) - fvRed;
            valg += GREEN(scl, idx) - fvGreen;
            valb += BLUE(scl, idx) - fvBlue;
            vala += ALPHA(scl, idx) - fvAlpha;

            if (ti <= maxri) {
                SET(tcl, ti, valr*iarr, valg*iarr, valb*iarr, vala*iarr);
                ti++;
            }
        }
        for (int j=r+1; j<w-r; j++) {
            int c = ri <= maxri ? ri++ : lvIdx;
            int c2 = li <= maxri ? li++ : lvIdx;
            valr += RED(scl, c) - RED(scl, c2);
            valg += GREEN(scl, c) - GREEN(scl, c2);
            valb += BLUE(scl, c) - BLUE(scl, c2);
            vala += ALPHA(scl, c) - ALPHA(scl, c2);
            if (ti <= maxri) {
                SET(tcl, ti, valr*iarr, valg*iarr, valb*iarr, vala*iarr);
                ti++;
            }
        }

        for (int j=w-r; j<w  ; j++) {
            int c2 = li <= maxri ? li++: lvIdx;
            valr += lvRed - RED(scl, c2);
            valg += lvGreen - GREEN(scl, c2);
            valb += lvBlue - BLUE(scl, c2);
            vala += lvAlpha - ALPHA(scl, c2);
            if (ti <= maxri) {
                SET(tcl, ti, valr*iarr, valg*iarr, valb*iarr, vala*iarr);
                ti++;
            }
        }
    }
}

static void boxBlurT_4 (const std::vector<float>& scl, std::vector<float>& tcl, int w, int h, float r) {
    float iarr = 1.0f / (r+r+1.0f);
    for(int i=0; i<w; i++) {
        int ti = i;
        int li = ti;
        int ri = ti+r*w;

        int maxri = ti+w*(h-1);

        int fvIdx = ti;
        int lvIdx = ti+w*(h-1);

        float fvRed = RED(scl, fvIdx);
        float fvGreen = GREEN(scl, fvIdx);
        float fvBlue = BLUE(scl, fvIdx);
        float fvAlpha = ALPHA(scl, fvIdx);
        float lvRed = RED(scl, lvIdx);
        float lvGreen = GREEN(scl, lvIdx);
        float lvBlue = BLUE(scl, lvIdx);
        float lvAlpha = ALPHA(scl, lvIdx);

        float valr = (r+1)*fvRed;
        float valg = (r+1)*fvGreen;
        float valb = (r+1)*fvBlue;
        float vala = (r+1)*fvAlpha;

        for(int j=0; j<r; j++) {
            int idx = j < w ? ti+j*w : lvIdx;
            valr += RED(scl, idx);
            valg += GREEN(scl, idx);
            valb += BLUE(scl, idx);
            vala += ALPHA(scl, idx);
        }
        for(int j=0  ; j<=r ; j++) {
            int idx = ri <= maxri ? ri : lvIdx;
            valr += RED(scl, idx) - fvRed;
            valg += GREEN(scl, idx) - fvGreen;
            valb += BLUE(scl, idx) - fvBlue;
            vala += ALPHA(scl, idx) - fvAlpha;
            if (ti <= maxri) {
                SET(tcl, ti, valr*iarr, valg*iarr, valb*iarr, vala*iarr);
            }
            ri+=w;
            ti+=w;
        }
        for(int j=r+1; j<h-r; j++) {
            int c = ri <= maxri ? ri : lvIdx;
            int c2 = li <= maxri ? li : lvIdx;
            valr += RED(scl, c) - RED(scl, c2);
            valg += GREEN(scl, c) - GREEN(scl, c2);
            valb += BLUE(scl, c) - BLUE(scl, c2);
            vala += ALPHA(scl, c) - ALPHA(scl, c2);
            if (ti <= maxri) {
                SET(tcl, ti, valr*iarr, valg*iarr, valb*iarr, vala*iarr);
            }
            li+=w; ri+=w; ti+=w;
        }
        for(int j=h-r; j<h  ; j++) {
            int c2 = li <= maxri ? li : lvIdx;
            valr += lvRed - RED(scl, c2);
            valg += lvGreen - GREEN(scl, c2);
            valb += lvBlue - BLUE(scl, c2);
            vala += lvAlpha - ALPHA(scl, c2);
            if (ti <= maxri) {
                SET(tcl, ti, valr*iarr, valg*iarr, valb*iarr, vala*iarr);
            }
            li += w;
            ti += w;
        }
    }
}

static void boxBlur_4(std::vector<float>& scl, std::vector<float>& tcl, int w, int h, float r, int size) {
    tcl = scl;
    //memcpy(tcl, scl, sizeof(float)*4*size);
    boxBlurH_4(tcl, scl, w, h, r);
    boxBlurT_4(scl, tcl, w, h, r);
}

static void gaussBlur_4(std::vector<float>& scl, std::vector<float>& tcl, int w, int h, int r, int size) {
    std::vector<float> bxs;
    boxesForGauss(r - 1, 3, bxs);
    boxBlur_4 (scl, tcl, w, h, (bxs[0]-1)/2, size);
    boxBlur_4 (tcl, scl, w, h, (bxs[1]-1)/2, size);
    boxBlur_4 (scl, tcl, w, h, (bxs[2]-1)/2, size);
}

static inline int roundInt(float r) {
    int tmp = static_cast<int> (r);
    tmp += (r-tmp>=.5) - (r-tmp<=-.5);
    return tmp;
}

void PixelBufferClass::Blur(LayerInfo* layer, float offset)
{
    int b;
    if (layer->BlurValueCurve.IsActive()) {
        b = (int)layer->BlurValueCurve.GetOutputValueAt(offset, layer->buffer.GetStartTimeMS(), layer->buffer.GetEndTimeMS());
    } else {
        b = layer->blur;
    }

    if (layer->BufferWi == 1 && layer->BufferHt == 1) {
        return;
    }
    if (b < 2) {
        return;
    }
    if (b > 2 && layer->BufferWi > 6 && layer->BufferHt > 6) {
        if (!GPURenderUtils::Blur(&layer->buffer, b)) {
            GPURenderUtils::waitForRenderCompletion(&layer->buffer);
            int os = std::max((int)layer->buffer.pixelVector.size(), layer->BufferWi * layer->BufferHt);
            int pixCount = layer->buffer.pixelVector.size();
            std::vector<float> input;
            input.resize(os * 4);
            std::vector<float> tmp;
            tmp.resize(os * 4);
            //float * input = new float[pixCount * 4];
            //float * tmp = new float[pixCount * 4];
            for (int x = 0; x < pixCount; x++) {
                const xlColor &c = layer->buffer.pixels[x];
                input[x * 4] = c.red;
                input[x * 4 + 1] = c.green;
                input[x * 4 + 2] = c.blue;
                input[x * 4 + 3] = c.alpha;
            }
            gaussBlur_4(input, tmp, layer->BufferWi, layer->BufferHt, b, pixCount);

            for (int x = 0; x < pixCount; x++) {
                layer->buffer.pixels[x].Set(roundInt(tmp[x*4]),
                                            roundInt(tmp[x*4 + 1]),
                                            roundInt(tmp[x*4 + 2]),
                                            roundInt(tmp[x*4 + 3]));
            }
        }
    } else {
        //small blur
        GPURenderUtils::waitForRenderCompletion(&layer->buffer);

        int d;
        int u;
        if (b % 2 == 0) {
            d = b / 2;
            u = (b - 1) / 2;
        } else {
            d = (b - 1) / 2;
            u = (b - 1) / 2;
        }
        RenderBuffer orig(layer->buffer);
        for (int x = 0; x < layer->BufferWi; x++) {
            for (int y = 0; y < layer->BufferHt; y++) {
                int r = 0;
                int g = 0;
                int b2 = 0;
                int a = 0;
                int sm = 0;
                for (int i = x - d; i <= x + u; i++) {
                    if (i >= 0 && i < layer->BufferWi) {
                        for (int j = y - d; j <= y + u; j++) {
                            if (j >=0 && j < layer->BufferHt) {
                                const xlColor &c = orig.GetPixel(i, j);
                                r += c.red;
                                g += c.green;
                                b2 += c.blue;
                                a += c.alpha;
                                ++sm;
                            }
                        }
                    }
                }
                if (sm == 0) {
                    sm = 1;
                }
                layer->buffer.SetPixel(x, y, xlColor(r/sm, g/sm, b2/sm, a/sm));
            }
        }
    }
}

void PixelBufferClass::SetPalette(int layer, xlColorVector& newcolors, xlColorCurveVector& newcc)
{
    RenderBuffer& buf = layers[layer]->buffer;
    buf.SetPalette(newcolors, newcc);
    if (layers[layer]->modelBuffers) {
        for (auto& it : *(layers[layer]->modelBuffers)) {
            it->SetPalette(newcolors, newcc);
        }
    }
}

static const std::string CHOICE_LayerMethod("CHOICE_LayerMethod");
static const std::string SLIDER_EffectLayerMix("SLIDER_EffectLayerMix");
static const std::string CHECKBOX_LayerMorph("CHECKBOX_LayerMorph");
static const std::string CHECKBOX_Canvas("CHECKBOX_Canvas");
static const std::string TEXTCTRL_Fadein("TEXTCTRL_Fadein");
static const std::string TEXTCTRL_Fadeout("TEXTCTRL_Fadeout");
static const std::string SLIDER_Blur("SLIDER_Blur");
static const std::string SLIDER_Zoom("SLIDER_Zoom");
static const std::string SLIDER_Rotation("SLIDER_Rotation");
static const std::string SLIDER_XRotation("SLIDER_XRotation");
static const std::string SLIDER_YRotation("SLIDER_YRotation");
static const std::string SLIDER_Rotations("SLIDER_Rotations");
static const std::string SLIDER_ZoomQuality("SLIDER_ZoomQuality");
static const std::string CHOICE_RZ_RotationOrder("CHOICE_RZ_RotationOrder");
static const std::string SLIDER_PivotPointX("SLIDER_PivotPointX");
static const std::string SLIDER_PivotPointY("SLIDER_PivotPointY");
static const std::string SLIDER_XPivot("SLIDER_XPivot");
static const std::string SLIDER_YPivot("SLIDER_YPivot");

static const std::string CHECKBOX_OverlayBkg("CHECKBOX_OverlayBkg");
static const std::string CHOICE_BufferStyle("CHOICE_BufferStyle");
static const std::string SPINCTRL_BufferStagger("SPINCTRL_BufferStagger");
static const std::string CHOICE_PerPreviewCamera("CHOICE_PerPreviewCamera");
static const std::string CHOICE_BufferTransform("CHOICE_BufferTransform");
static const std::string CUSTOM_SubBuffer("CUSTOM_SubBuffer");
static const std::string VALUECURVE_Blur("VALUECURVE_Blur");
static const std::string VALUECURVE_Sparkles("VALUECURVE_SparkleFrequency");
static const std::string VALUECURVE_Brightness("VALUECURVE_Brightness");
static const std::string VALUECURVE_HueAdjust("VALUECURVE_Color_HueAdjust");
static const std::string VALUECURVE_SaturationAdjust("VALUECURVE_Color_SaturationAdjust");
static const std::string VALUECURVE_ValueAdjust("VALUECURVE_Color_ValueAdjust");
static const std::string VALUECURVE_Zoom("VALUECURVE_Zoom");
static const std::string VALUECURVE_Rotation("VALUECURVE_Rotation");
static const std::string VALUECURVE_XRotation("VALUECURVE_XRotation");
static const std::string VALUECURVE_YRotation("VALUECURVE_YRotation");
static const std::string VALUECURVE_Rotations("VALUECURVE_Rotations");
static const std::string VALUECURVE_PivotPointX("VALUECURVE_PivotPointX");
static const std::string VALUECURVE_PivotPointY("VALUECURVE_PivotPointY");
static const std::string VALUECURVE_XPivot("VALUECURVE_XPivot");
static const std::string VALUECURVE_YPivot("VALUECURVE_YPivot");
static const std::string STR_DEFAULT("Default");
static const std::string STR_EMPTY("");

static const std::string SPINCTRL_FreezeEffectAtFrame("SPINCTRL_FreezeEffectAtFrame");
static const std::string SPINCTRL_SuppressEffectUntil("SPINCTRL_SuppressEffectUntil");
static const std::string SLIDER_ChromaSensitivity("SLIDER_ChromaSensitivity");
static const std::string CHECKBOX_Chroma("CHECKBOX_Chroma");
static const std::string COLOURPICKERCTRL_ChromaColour("COLOURPICKERCTRL_ChromaColour");
static const std::string COLOURPICKERCTRL_SparklesColour("COLOURPICKERCTRL_SparklesColour");
static const std::string SLIDER_SparkleFrequency("SLIDER_SparkleFrequency");
static const std::string CHECKBOX_MusicSparkles("CHECKBOX_MusicSparkles");
static const std::string SLIDER_Brightness("SLIDER_Brightness");
static const std::string SLIDER_HueAdjust("SLIDER_Color_HueAdjust");
static const std::string SLIDER_SaturationAdjust("SLIDER_Color_SaturationAdjust");
static const std::string SLIDER_ValueAdjust("SLIDER_Color_ValueAdjust");
static const std::string SLIDER_Contrast("SLIDER_Contrast");
static const std::string STR_NORMAL("Normal");
static const std::string STR_NONE("None");
static const std::string STR_FADE("Fade");
static const std::string STR_FOLD("Fold");
static const std::string STR_DISSOLVE("Dissolve");
static const std::string STR_CIRCULAR_SWIRL("Circular Swirl");
static const std::string STR_BOW_TIE("Bow Tie");
static const std::string STR_ZOOM("Zoom");
static const std::string STR_DOORWAY("Doorway");
static const std::string STR_BLOBS("Blobs");
static const std::string STR_PINWHEEL("Pinwheel");
static const std::string STR_STAR("Star");
static const std::string STR_SWAP("Swap");
static const std::string STR_SHATTER("Shatter");
static const std::string STR_CIRCLES("Circles");

static const std::string CHOICE_In_Transition_Type("CHOICE_In_Transition_Type");
static const std::string CHOICE_Out_Transition_Type("CHOICE_Out_Transition_Type");
static const std::string SLIDER_In_Transition_Adjust("SLIDER_In_Transition_Adjust");
static const std::string SLIDER_Out_Transition_Adjust("SLIDER_Out_Transition_Adjust");
static const std::string CHECKBOX_In_Transition_Reverse("CHECKBOX_In_Transition_Reverse");
static const std::string CHECKBOX_Out_Transition_Reverse("CHECKBOX_Out_Transition_Reverse");

void ComputeValueCurve(const std::string& valueCurve, ValueCurve& theValueCurve, int divisor = 1)
{
    if (valueCurve == STR_EMPTY) {
        theValueCurve.SetDefault();
        return;
    }

    theValueCurve.SetDivisor(divisor);
    theValueCurve.Deserialise(valueCurve);
}

void ComputeSubBuffer(const std::string &subBuffer, std::vector<NodeBaseClassPtr> &newNodes,
                      int &bufferWi, int &bufferHi,
                      int &buffOffsetX, int &buffOffsetY,
                      float progress, long startMS, long endMS) {
    if (subBuffer == STR_EMPTY) {
        return;
    }
    wxString sb = subBuffer;
    sb.Replace("Max", "yyz");

    wxArrayString v = wxSplit(sb, 'x');

    bool fx1vc = v.size() > 0 && v[0].Contains("Active=TRUE");
    bool fy1vc = v.size() > 1 && v[1].Contains("Active=TRUE");
    bool fx2vc = v.size() > 2 && v[2].Contains("Active=TRUE");
    bool fy2vc = v.size() > 3 && v[3].Contains("Active=TRUE");
    bool fxvc = v.size() > 4 && v[4].Contains("Active=TRUE"); // X centre
    bool fyvc = v.size() > 5 && v[5].Contains("Active=TRUE"); // y centre

    float x = 0.0;
    if (fxvc) {
        v[4].Replace("yyz", "Max");
        ValueCurve vc(v[4].ToStdString());
        vc.SetLimits(SB_CENTRE_MIN, SB_CENTRE_MAX);
        x = vc.GetOutputValueAt(progress, startMS, endMS);
    } else if (v.size() > 4) {
        x = wxAtof(v[4]);
    }

    float y = 0.0;
    if (fyvc) {
        v[5].Replace("yyz", "Max");
        ValueCurve vc(v[5].ToStdString());
        vc.SetLimits(SB_CENTRE_MIN, SB_CENTRE_MAX);
        y = vc.GetOutputValueAt(progress, startMS, endMS);
    } else if (v.size() > 5) {
        y = wxAtof(v[5]);
    }

    float x1 = 0.0;
    if (fx1vc) {
        v[0].Replace("yyz", "Max");
        ValueCurve vc(v[0].ToStdString());
        vc.SetLimits(SB_LEFT_BOTTOM_MIN, SB_LEFT_BOTTOM_MAX);
        x1 = vc.GetOutputValueAt(progress, startMS, endMS);
    } else if (v.size() > 0) {
        x1 = wxAtof(v[0]);
    }
    x1 += x;

    float y1 = 0.0;
    if (fy1vc) {
        v[1].Replace("yyz", "Max");
        ValueCurve vc(v[1].ToStdString());
        vc.SetLimits(SB_LEFT_BOTTOM_MIN, SB_LEFT_BOTTOM_MAX);
        y1 = vc.GetOutputValueAt(progress, startMS, endMS);
    } else if (v.size() > 1) {
        y1 = wxAtof(v[1]);
    }
    y1 += y;

    float x2 = 100.0;
    if (fx2vc) {
        v[2].Replace("yyz", "Max");
        ValueCurve vc(v[2].ToStdString());
        vc.SetLimits(SB_RIGHT_TOP_MIN, SB_RIGHT_TOP_MAX);
        x2 = vc.GetOutputValueAt(progress, startMS, endMS);
    } else if (v.size() > 2) {
        x2 = wxAtof(v[2]);
    }
    x2 += x;

    float y2 = 100.0;
    if (fy2vc) {
        v[3].Replace("yyz", "Max");
        ValueCurve vc(v[3].ToStdString());
        vc.SetLimits(SB_RIGHT_TOP_MIN, SB_RIGHT_TOP_MAX);
        y2 = vc.GetOutputValueAt(progress, startMS, endMS);
    } else if (v.size() > 3) {
        y2 = wxAtof(v[3]);
    }
    y2 += y;

    if (x1 > x2) std::swap(x1, x2);
    if (y1 > y2) std::swap(y1, y2);

    x1 *= (float)bufferWi;
    x2 *= (float)bufferWi;
    y1 *= (float)bufferHi;
    y2 *= (float)bufferHi;
    x1 /= 100.0;
    x2 /= 100.0;
    y1 /= 100.0;
    y2 /= 100.0;

    int x1Int = std::round(x1);
    int x2Int = std::round(x2);
    int y1Int = std::round(y1);
    int y2Int = std::round(y2);

    bufferWi = x2Int - x1Int;
    bufferHi = y2Int - y1Int;
    if (bufferWi < 1) bufferWi = 1;
    if (bufferHi < 1) bufferHi = 1;
    for (size_t x = 0; x < newNodes.size(); x++) {
        for (auto &it2 : newNodes[x]->Coords) {
            it2.bufX -= x1Int;
            it2.bufY -= y1Int;
        }
    }
    buffOffsetX = x1Int;
    buffOffsetY = y1Int;
}

namespace
{
   ValueCurve valueCurveFromSettingsMap( const SettingsMap &settingsMap, const std::string& name )
   {
      ValueCurve vc;
      std::string vn = "VALUECURVE_" + name;
      if ( settingsMap.Contains( vn ) )
      {
         std::string serializedVC( settingsMap.Get( vn, "" ) );

         vc.SetDivisor( 1 );
         vc.SetLimits( 0, 100 );
         vc.Deserialise( serializedVC );
      }
      return vc;
   }
}

void PixelBufferClass::SetLayerSettings(int layer, const SettingsMap &settingsMap) {
    LayerInfo *inf = layers[layer];
    inf->persistent = settingsMap.GetBool(CHECKBOX_OverlayBkg);
    inf->mask.clear();

    inf->fadeInSteps = (int)(settingsMap.GetDouble(TEXTCTRL_Fadein, 0.0)*1000)/frameTimeInMs;
    inf->fadeOutSteps = (int)(settingsMap.GetDouble(TEXTCTRL_Fadeout, 0.0)*1000)/frameTimeInMs;

    inf->inTransitionType = settingsMap.Get(CHOICE_In_Transition_Type, STR_FADE);
    inf->outTransitionType = settingsMap.Get(CHOICE_Out_Transition_Type, STR_FADE);
    inf->inTransitionAdjust = settingsMap.GetInt(SLIDER_In_Transition_Adjust, 0);
    inf->outTransitionAdjust = settingsMap.GetInt(SLIDER_Out_Transition_Adjust, 0);
    inf->InTransitionAdjustValueCurve = valueCurveFromSettingsMap( settingsMap, "In_Transition_Adjust" );
    inf->OutTransitionAdjustValueCurve = valueCurveFromSettingsMap( settingsMap, "Out_Transition_Adjust" );
    inf->inTransitionReverse = settingsMap.GetBool(CHECKBOX_In_Transition_Reverse);
    inf->outTransitionReverse = settingsMap.GetBool(CHECKBOX_Out_Transition_Reverse);

    inf->blur = settingsMap.GetInt(SLIDER_Blur, 1);
    inf->rotation = settingsMap.GetInt(SLIDER_Rotation, 0);
    inf->xrotation = settingsMap.GetInt(SLIDER_XRotation, 0);
    inf->yrotation = settingsMap.GetInt(SLIDER_YRotation, 0);
    inf->rotations = (float)settingsMap.GetInt(SLIDER_Rotations, 0) / 10.0f;
    inf->zoom = (float)settingsMap.GetInt(SLIDER_Zoom, 10) / 10.0f;
    inf->zoomquality = settingsMap.GetInt(SLIDER_ZoomQuality, 1);
    inf->rotationorder = settingsMap.Get(CHOICE_RZ_RotationOrder, "X-Y-Z");
    inf->pivotpointx = settingsMap.GetInt(SLIDER_PivotPointX, 50);
    inf->pivotpointy = settingsMap.GetInt(SLIDER_PivotPointY, 50);
    inf->xpivot = settingsMap.GetInt(SLIDER_XPivot, 50);
    inf->ypivot = settingsMap.GetInt(SLIDER_YPivot, 50);
    inf->sparkle_count = settingsMap.GetInt(SLIDER_SparkleFrequency, 0);
    inf->use_music_sparkle_count = settingsMap.GetBool(CHECKBOX_MusicSparkles, false);

    inf->isChromaKey = settingsMap.GetBool(CHECKBOX_Chroma, false);
    inf->chromaSensitivity = settingsMap.GetInt(SLIDER_ChromaSensitivity, 1);
    inf->freezeAfterFrame = settingsMap.GetInt(SPINCTRL_FreezeEffectAtFrame, 999999);
    inf->suppressUntil = settingsMap.GetInt(SPINCTRL_SuppressEffectUntil, 0);
    inf->chromaKeyColour.SetFromString(settingsMap.Get(COLOURPICKERCTRL_ChromaColour, STR_EMPTY));
    inf->sparklesColour.SetFromString(settingsMap.Get(COLOURPICKERCTRL_SparklesColour, "#FFFFFF"));
    inf->brightness = settingsMap.GetInt(SLIDER_Brightness, 100);
    inf->hueadjust = settingsMap.GetInt(SLIDER_HueAdjust, 0);
    inf->saturationadjust = settingsMap.GetInt(SLIDER_SaturationAdjust, 0);
    inf->valueadjust = settingsMap.GetInt(SLIDER_ValueAdjust, 0);
    inf->contrast=settingsMap.GetInt(SLIDER_Contrast, 0);

    SetMixType(layer, settingsMap.Get(CHOICE_LayerMethod, STR_NORMAL));

    inf->effectMixThreshold = (float)settingsMap.GetInt(SLIDER_EffectLayerMix, 0)/100.0;
    inf->effectMixVaries = settingsMap.GetBool(CHECKBOX_LayerMorph);
    inf->canvas = settingsMap.GetBool(CHECKBOX_Canvas, false);

    inf->type = settingsMap.Get(CHOICE_BufferStyle, STR_DEFAULT);
    inf->camera = settingsMap.Get(CHOICE_PerPreviewCamera, "2D");
    inf->transform = settingsMap.Get(CHOICE_BufferTransform, STR_NONE);
    inf->stagger = settingsMap.GetInt(SPINCTRL_BufferStagger, 0);

    std::string type = settingsMap.Get(CHOICE_BufferStyle, STR_DEFAULT);
    int stagger = settingsMap.GetInt(SPINCTRL_BufferStagger, 0);
    const std::string &camera = settingsMap.Get(CHOICE_PerPreviewCamera, "2D");
    const std::string &transform = settingsMap.Get(CHOICE_BufferTransform, STR_NONE);
    const std::string &subBuffer = settingsMap.Get(CUSTOM_SubBuffer, STR_EMPTY);
    const std::string &blurValueCurve = settingsMap.Get(VALUECURVE_Blur, STR_EMPTY);
    const std::string &sparklesValueCurve = settingsMap.Get(VALUECURVE_Sparkles, STR_EMPTY);
    const std::string &brightnessValueCurve = settingsMap.Get(VALUECURVE_Brightness, STR_EMPTY);
    const std::string &hueAdjustValueCurve = settingsMap.Get(VALUECURVE_HueAdjust, STR_EMPTY);
    const std::string &saturationAdjustValueCurve = settingsMap.Get(VALUECURVE_SaturationAdjust, STR_EMPTY);
    const std::string &valueAdjustValueCurve = settingsMap.Get(VALUECURVE_ValueAdjust, STR_EMPTY);
    const std::string &rotationValueCurve = settingsMap.Get(VALUECURVE_Rotation, STR_EMPTY);
    const std::string &xrotationValueCurve = settingsMap.Get(VALUECURVE_XRotation, STR_EMPTY);
    const std::string &yrotationValueCurve = settingsMap.Get(VALUECURVE_YRotation, STR_EMPTY);
    const std::string &zoomValueCurve = settingsMap.Get(VALUECURVE_Zoom, STR_EMPTY);
    const std::string &rotationsValueCurve = settingsMap.Get(VALUECURVE_Rotations, STR_EMPTY);
    const std::string &pivotpointxValueCurve = settingsMap.Get(VALUECURVE_PivotPointX, STR_EMPTY);
    const std::string &pivotpointyValueCurve = settingsMap.Get(VALUECURVE_PivotPointY, STR_EMPTY);
    const std::string &xpivotValueCurve = settingsMap.Get(VALUECURVE_XPivot, STR_EMPTY);
    const std::string &ypivotValueCurve = settingsMap.Get(VALUECURVE_YPivot, STR_EMPTY);

    if (inf->bufferType != type ||
        inf->camera != camera ||
        inf->bufferTransform != transform ||
        inf->subBuffer != subBuffer ||
        inf->stagger != stagger ||
        inf->blurValueCurve != blurValueCurve ||
        inf->sparklesValueCurve != sparklesValueCurve ||
        inf->zoomValueCurve != zoomValueCurve ||
        inf->rotationValueCurve != rotationValueCurve ||
        inf->xrotationValueCurve != xrotationValueCurve ||
        inf->yrotationValueCurve != yrotationValueCurve ||
        inf->rotationsValueCurve != rotationsValueCurve ||
        inf->pivotpointxValueCurve != pivotpointxValueCurve ||
        inf->pivotpointyValueCurve != pivotpointyValueCurve ||
        inf->xpivotValueCurve != xpivotValueCurve ||
        inf->ypivotValueCurve != ypivotValueCurve ||
        inf->brightnessValueCurve != brightnessValueCurve ||
        inf->hueAdjustValueCurve != hueAdjustValueCurve ||
        inf->saturationAdjustValueCurve != saturationAdjustValueCurve ||
        inf->valueAdjustValueCurve != valueAdjustValueCurve)
    {
        // This function is useful for testing issues where PixelBufferClass::MergeBuffersForLayer asserts
        //if (model->GetDisplayAs() == "ModelGroup")
        //{
        //    dynamic_cast<const ModelGroup*>(model)->TestNodeInit();
        //}

        int origNodeCount = inf->buffer.Nodes.size();
        inf->buffer.Nodes.clear();

        // If we are a 'Per Model Default' render buffer then we need to ensure we create a full set of pixels
        // so we change the type of the render buffer but just for model initialisation
        // 2019-02-22 This was "Horizontal Per Model" but it causes DMX Model issues ...
        // so I have changed it to "Single Line". In theory both should create all the nodes
        auto tt = type;
        bool go_deep = false;
        if (model->Name() == "PRESET_Matrix_XYZZY") {
            // for presets we just turn it into the non-per model style
            if (StartsWith(type, "Per Model")) {
                type = type.substr(10);
                if (EndsWith(type, "Deep")) {
                    type = type.substr(0, type.length() - 5);
                }
            }
        } else {
            if (StartsWith(type, "Per Model")) {
                tt = "Single Line";
                if (type.compare(type.length() - 4, 4, "Deep") == 0) {
                    go_deep = true;
                }
            }
        }

        inf->BufferOffsetX = 0;
        inf->BufferOffsetY = 0;
        model->InitRenderBufferNodes(tt, camera, transform, inf->buffer.Nodes, inf->BufferWi, inf->BufferHt, inf->stagger, go_deep);
        if (origNodeCount != 0 && origNodeCount != inf->buffer.Nodes.size()) {
            inf->buffer.Nodes.clear();
            model->InitRenderBufferNodes(tt, camera, transform, inf->buffer.Nodes, inf->BufferWi, inf->BufferHt, inf->stagger, go_deep);
        }

        ComputeSubBuffer(subBuffer, inf->buffer.Nodes,
                         inf->BufferWi, inf->BufferHt,
                         inf->BufferOffsetX, inf->BufferOffsetY,
                         0, inf->buffer.GetStartTimeMS(), inf->buffer.GetEndTimeMS());

        ComputeValueCurve(brightnessValueCurve, inf->BrightnessValueCurve);
        ComputeValueCurve(hueAdjustValueCurve, inf->HueAdjustValueCurve);
        ComputeValueCurve(saturationAdjustValueCurve, inf->SaturationAdjustValueCurve);
        ComputeValueCurve(valueAdjustValueCurve, inf->ValueAdjustValueCurve);
        ComputeValueCurve(blurValueCurve, inf->BlurValueCurve);
        ComputeValueCurve(sparklesValueCurve, inf->SparklesValueCurve);
        ComputeValueCurve(rotationValueCurve, inf->RotationValueCurve);
        ComputeValueCurve(xrotationValueCurve, inf->XRotationValueCurve);
        ComputeValueCurve(yrotationValueCurve, inf->YRotationValueCurve);
        ComputeValueCurve(zoomValueCurve, inf->ZoomValueCurve, 10);
        ComputeValueCurve(rotationsValueCurve, inf->RotationsValueCurve, 10);
        ComputeValueCurve(pivotpointxValueCurve, inf->PivotPointXValueCurve);
        ComputeValueCurve(pivotpointyValueCurve, inf->PivotPointYValueCurve);
        ComputeValueCurve(xpivotValueCurve, inf->XPivotValueCurve);
        ComputeValueCurve(ypivotValueCurve, inf->YPivotValueCurve);
        inf->bufferType = type;
        inf->camera = camera;
        inf->bufferTransform = transform;
        inf->subBuffer = subBuffer;
        inf->blurValueCurve = blurValueCurve;
        inf->sparklesValueCurve = sparklesValueCurve;
        inf->brightnessValueCurve = brightnessValueCurve;
        inf->hueAdjustValueCurve = hueAdjustValueCurve;
        inf->saturationAdjustValueCurve = saturationAdjustValueCurve;
        inf->valueAdjustValueCurve = valueAdjustValueCurve;
        inf->zoomValueCurve = zoomValueCurve;
        inf->rotationValueCurve = rotationValueCurve;
        inf->xrotationValueCurve = xrotationValueCurve;
        inf->yrotationValueCurve = yrotationValueCurve;
        inf->rotationsValueCurve = rotationsValueCurve;
        inf->pivotpointxValueCurve = pivotpointxValueCurve;
        inf->pivotpointyValueCurve = pivotpointyValueCurve;
        inf->xpivotValueCurve = xpivotValueCurve;
        inf->ypivotValueCurve = ypivotValueCurve;
        inf->stagger = stagger;

        // we create the buffer oversized to prevent issues
        inf->buffer.InitBuffer(inf->BufferHt, inf->BufferWi, inf->bufferTransform);
        GPURenderUtils::setupRenderBuffer(this, &inf->buffer);

        if (type.compare(0, 9, "Per Model") == 0 && model->GetDisplayAs() == "ModelGroup") {
            if (type.compare(type.length()-4, 4, "Deep") == 0) {
                inf->modelBuffers = &inf->deepModelBuffers;
                const ModelGroup* gp = dynamic_cast<const ModelGroup*>(model);
                if (gp != nullptr) {
                    std::list<Model*> flat_models = gp->GetFlatModels(false, true);
                    std::list<Model*>::iterator it_m = flat_models.begin();
                    for (const auto& it : inf->deepModelBuffers) {
                        std::string ntype = "Default"; // type.substr(10, type.length() - 10);
                        int bw, bh;
                        it->Nodes.clear();
                        (*it_m)->InitRenderBufferNodes(ntype, camera, transform, it->Nodes, bw, bh, 0);
                        if (bw == 0)
                            bw = 1; // zero sized buffers are a problem
                        if (bh == 0)
                            bh = 1;
                        it->InitBuffer(bh, bw, transform);
                        it->SetAllowAlphaChannel(inf->buffer.allowAlpha);
                        GPURenderUtils::setupRenderBuffer(this, it.get());
                        ++it_m;
                    }
                }
            } else {
                inf->modelBuffers = &inf->shallowModelBuffers;
                const ModelGroup* gp = dynamic_cast<const ModelGroup*>(model);
                if (gp != nullptr) {
                    int cnt = 0;
                    for (const auto& it : inf->shallowModelBuffers) {
                        std::string ntype = type.substr(10, type.length() - 10);
                        int bw, bh;
                        it->Nodes.clear();
                        gp->ActiveModels()[cnt]->InitRenderBufferNodes(ntype, camera, transform, it->Nodes, bw, bh, 0);
                        if (bw == 0)
                            bw = 1; // zero sized buffers are a problem
                        if (bh == 0)
                            bh = 1;
                        it->InitBuffer(bh, bw, transform);
                        it->SetAllowAlphaChannel(inf->buffer.allowAlpha);
                        GPURenderUtils::setupRenderBuffer(this, it.get());
                        ++cnt;
                    }
                }
            }
        } else {
            inf->modelBuffers = nullptr;
        }
    }
}

bool PixelBufferClass::IsPersistent(int layer) {
    return layers[layer]->persistent;
}

int PixelBufferClass::GetFreezeFrame(int layer)
{
    return layers[layer]->freezeAfterFrame;
}

int PixelBufferClass::GetSuppressUntil(int layer)
{
    return layers[layer]->suppressUntil;
}

RenderBuffer& PixelBufferClass::BufferForLayer(int layer, int idx)
{
    if (idx >= 0 && layers[layer]->modelBuffers && idx < layers[layer]->modelBuffers->size()) {
        return *(*layers[layer]->modelBuffers)[idx];
    }
    return layers[layer]->buffer;
}

uint32_t PixelBufferClass::BufferCountForLayer(int layer)
{
    if (layers[layer]->modelBuffers) {
        return layers[layer]->modelBuffers->size();
    }
    return 1;
}

void PixelBufferClass::UnMergeBuffersForLayer(int layer) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (layers[layer]->modelBuffers) {
        //get all the data
        xlColor color;
        int nc = 0;
        
        GPURenderUtils::waitForRenderCompletion(&layers[layer]->buffer);

        for (const auto& modelBuffer : *(layers[layer]->modelBuffers)) {
            for (const auto& mbnode : modelBuffer->Nodes) {
                if (nc < layers[layer]->buffer.Nodes.size()) {
                    auto &node = layers[layer]->buffer.Nodes[nc];
                    layers[layer]->buffer.GetPixel(node->Coords[0].bufX, node->Coords[0].bufY, color);
                    for (const auto& coord : mbnode->Coords) {
                        modelBuffer->SetPixel(coord.bufX, coord.bufY, color);
                    }
                    nc++;
                } else {
                    // Where this happens it is usually a sign that there is a bug in one of our models where it creates a different number of nodes depending on the render buffer size
                    // To find the cause uncomment the model group function TestNodeInit and the call in PixelBuffer::SetLayerSettings

                    if (layers[layer]->buffer.curPeriod == layers[layer]->buffer.curEffStartPer) {
                        logger_base.warn("PixelBufferClass::UnMergeBuffersForLayer(%d) Model '%s' Mismatch in number of nodes across layers.", layer, (const char*)modelName.c_str());
                        for (int i = 0; i < GetLayerCount(); i++) {
                            logger_base.warn("    Layer %d node count %d buffer '%s'", i, (int)layers[i]->buffer.Nodes.size(), (const char*)layers[i]->bufferType.c_str());
                        }
                        int mbnodes = 0;
                        for (const auto& mb : *(layers[layer]->modelBuffers)) {
                            mbnodes += mb->Nodes.size();
                        }
                        wxASSERT(false);
                    }
                }
            }
        }
    }
}
void PixelBufferClass::MergeBuffersForLayer(int layer) {
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (layers[layer]->modelBuffers) {
        //get all the data
        xlColor color;
        int nc = 0;
        for (auto& modelBuffer : *(layers[layer]->modelBuffers)) {
            GPURenderUtils::waitForRenderCompletion(modelBuffer.get());
        }
        for (const auto& modelBuffer : *(layers[layer]->modelBuffers)) {
            for (const auto& node : modelBuffer->Nodes) {
                if (nc < layers[layer]->buffer.Nodes.size()) {
                    modelBuffer->GetPixel(node->Coords[0].bufX, node->Coords[0].bufY, color);
                    for (const auto& coord : layers[layer]->buffer.Nodes[nc]->Coords) {
                        layers[layer]->buffer.SetPixel(coord.bufX, coord.bufY, color);
                    }
                    nc++;
                } else {
                    // Where this happens it is usually a sign that there is a bug in one of our models where it creates a different number of nodes depending on the render buffer size
                    // To find the cause uncomment the model group function TestNodeInit and the call in PixelBuffer::SetLayerSettings

                    if (layers[layer]->buffer.curPeriod == layers[layer]->buffer.curEffStartPer) {
                        logger_base.warn("PixelBufferClass::MergeBuffersForLayer(%d) Model '%s' Mismatch in number of nodes across layers.", layer, (const char*)modelName.c_str());
                        for (int i = 0; i < GetLayerCount(); i++) {
                            logger_base.warn("    Layer %d node count %d buffer '%s'", i, (int)layers[i]->buffer.Nodes.size(), (const char*)layers[i]->bufferType.c_str());
                        }
                        int mbnodes = 0;
                        for (const auto& mb : *(layers[layer]->modelBuffers)) {
                            mbnodes += mb->Nodes.size();
                        }
                        wxASSERT(false);
                    }
                }
            }
        }
    }
}

void PixelBufferClass::SetLayer(int layer, int period, bool resetState)
{
    layers[layer]->buffer.SetState(period, resetState, modelName);
    if (layers[layer]->modelBuffers == &layers[layer]->deepModelBuffers) {
        const ModelGroup* grp = dynamic_cast<const ModelGroup*>(model);
        std::list<Model*> flat_models = grp->GetFlatModels(false, true);
        std::list<Model*>::iterator it_m = flat_models.begin();
        for (auto it = layers[layer]->modelBuffers->begin(); it != layers[layer]->modelBuffers->end(); ++it, it_m++) {
            if (frame->AllModels[(*it_m)->Name()] == nullptr) {
                (*it)->SetState(period, resetState, (*it_m)->GetFullName());
            } else {
                (*it)->SetState(period, resetState, (*it_m)->Name());
            }
        }
    } else if (layers[layer]->modelBuffers) {
        int cnt = 0;
        const ModelGroup* grp = dynamic_cast<const ModelGroup*>(model);
        for (auto it = layers[layer]->modelBuffers->begin(); it != layers[layer]->modelBuffers->end(); ++it, cnt++) {
            if (frame->AllModels[grp->ActiveModels()[cnt]->Name()] == nullptr) {
                (*it)->SetState(period, resetState, grp->ActiveModels()[cnt]->GetFullName());
            } else {
                (*it)->SetState(period, resetState, grp->ActiveModels()[cnt]->Name());
            }
        }
    }
}

void PixelBufferClass::SetTimes(int layer, int startTime, int endTime)
{
    layers[layer]->buffer.SetEffectDuration(startTime, endTime);
    if (layers[layer]->modelBuffers) {
        for (const auto& it : *(layers[layer]->modelBuffers)) {
            it->SetEffectDuration(startTime, endTime);
        }
    }
}

static inline bool IsInRange(const std::vector<bool> &restrictRange, size_t start) {
    if (restrictRange.empty()) {
        return true;
    }
    if (start >= restrictRange.size()) {
        return false;
    }
    return restrictRange[start];
}

void PixelBufferClass::GetColors(unsigned char *fdata, const std::vector<bool> &restrictRange) {

    // KW ... I think this needs to be optimised

    if (layers[0] != nullptr) { // I dont like this ... it should never be null
        if (layers[0]->buffer.Nodes.size() < 1000) {
            //smaller model, no sense in setting up the parallel_for
            for (auto &n : layers[0]->buffer.Nodes) {
                size_t start = n->ActChan;
                if (IsInRange(restrictRange, start)) {
                    if (n->model != nullptr) { // nor this
                        DimmingCurve *curve = n->model->modelDimmingCurve;
                        if (curve != nullptr) {
                            if (n->GetChanCount() == 1) {
                                uint8_t buf[3] = {0, 0, 0};
                                n->GetForChannels(buf);
                                xlColor color(buf[0], buf[0], buf[0]);
                                curve->apply(color);

                                n->SetColor(color);
                            } else {
                                xlColor color;
                                n->GetColor(color);
                                curve->apply(color);
                                n->SetColor(color);
                            }
                        }
                    }
                    n->GetForChannels(&fdata[start]);
                }
            }
        } else {
            parallel_for(0,  layers[0]->buffer.Nodes.size(), [&](int i) {
                auto &n = layers[0]->buffer.Nodes[i];
                size_t start = n->ActChan;
                if (IsInRange(restrictRange, start)) {
                    if (n->model != nullptr) { // nor this
                        DimmingCurve *curve = n->model->modelDimmingCurve;
                        if (curve != nullptr) {
                            if (n->GetChanCount() == 1) {
                                uint8_t buf[3] = {0, 0, 0};
                                n->GetForChannels(buf);
                                xlColor color(buf[0], buf[0], buf[0]);
                                curve->apply(color);

                                n->SetColor(color);
                            } else {
                                xlColor color;
                                n->GetColor(color);
                                curve->apply(color);
                                n->SetColor(color);
                            }
                        }
                    }
                    n->GetForChannels(&fdata[start]);
                }
            }, 500);
        }
    }
}

void PixelBufferClass::SetColors(int layer, const unsigned char *fdata)
{
    if (layer >= layers.size()) return;

    if (layers[layer]->buffer.Nodes.size() < 1000) {
        xlColor color;
        for (const auto &n : layers[layer]->buffer.Nodes) {
            size_t start = n->ActChan;

            n->SetFromChannels(&fdata[start]);
            n->GetColor(color);

            DimmingCurve *curve = n->model->modelDimmingCurve;
            if (curve != nullptr) {
                curve->reverse(color);
            }
            for (const auto &a : n->Coords) {
                layers[layer]->buffer.SetPixel(a.bufX, a.bufY, color);
            }
        }
    } else {
        parallel_for(0,  layers[layer]->buffer.Nodes.size(), [&](int i) {
            auto &n = layers[layer]->buffer.Nodes[i];
            xlColor color;
            size_t start = n->ActChan;
            n->SetFromChannels(&fdata[start]);
            n->GetColor(color);

            DimmingCurve *curve = n->model->modelDimmingCurve;
            if (curve != nullptr) {
                curve->reverse(color);
            }
            for (const auto &a : n->Coords) {
                layers[layer]->buffer.SetPixel(a.bufX, a.bufY, color);
            }
        },  500);
    }

}

void PixelBufferClass::RotateX(RenderBuffer &buffer, GPURenderUtils::RotoZoomSettings &settings)
{
    // Now do the rotation around a point on the x axis

    float xrotation = settings.xrotation;
    if (xrotation != 0 && xrotation != 360) {
        GPURenderUtils::waitForRenderCompletion(&buffer);
        int xpivot = settings.xpivot;

        RenderBuffer orig(buffer);
        buffer.Clear();

        float sine = sin((xrotation + 90) * M_PI / 180);
        float pivot = xpivot * buffer.BufferWi / 100;

        for (int x = pivot; x < buffer.BufferWi; ++x) {
            float tox = sine * (x - pivot) + pivot;
            for (int y = 0; y < buffer.BufferHt; ++y) {
                buffer.SetPixel(tox, y, orig.GetPixel(x, y));
            }
        }

        for (int x = pivot - 1; x >= 0; --x) {
            float tox = -1 * sine * (pivot - x) + pivot;
            for (int y = 0; y < buffer.BufferHt; ++y) {
                buffer.SetPixel(tox, y, orig.GetPixel(x, y));
            }
        }
    }
}

void PixelBufferClass::RotateY(RenderBuffer &buffer, GPURenderUtils::RotoZoomSettings &settings)
{
    // Now do the rotation around a point on the y axis
    float yrotation = settings.yrotation;
    if (yrotation != 0 && yrotation != 360) {
        GPURenderUtils::waitForRenderCompletion(&buffer);

        int ypivot = settings.ypivot;
        RenderBuffer orig(buffer);
        buffer.Clear();

        float sine = sin((yrotation + 90) * M_PI / 180);
        float pivot = ypivot * buffer.BufferHt / 100;

        for (int y = pivot; y < buffer.BufferHt; ++y) {
            float toy = sine * (y - pivot) + pivot;
            for (int x = 0; x < buffer.BufferWi; ++x) {
                buffer.SetPixel(x, toy, orig.GetPixel(x, y));
            }
        }

        for (int y = pivot - 1; y >= 0; --y) {
            float toy = -1 * sine * (pivot - y) + pivot;
            for (int x = 0; x < buffer.BufferWi; ++x) {
                buffer.SetPixel(x, toy, orig.GetPixel(x, y));
            }
        }
    }
}

void PixelBufferClass::RotateZAndZoom(RenderBuffer &buffer, GPURenderUtils::RotoZoomSettings &settings)
{
    // Do the Z axis rotate and zoom first
    float zoom = settings.zoom;
    float rotation = settings.zrotation;

    if (rotation != 0.0 || zoom != 1.0) {
        GPURenderUtils::waitForRenderCompletion(&buffer);

        static const float PI_2 = 6.283185307f;
        xlColor c;
        RenderBuffer orig(buffer);
        int q = settings.zoomquality;
        int cx = settings.pivotpointx;
        int cy = settings.pivotpointy;
        float inc = 1.0 / (float)q;

        float angle = PI_2 * -rotation;
        float xoff = (cx * buffer.BufferWi) / 100.0;
        float yoff = (cy * buffer.BufferHt) / 100.0;
        float anglecos = cos(-angle);
        float anglesin = sin(-angle);

        buffer.Clear();
        for (int x = 0; x < buffer.BufferWi; x++) {
            for (int i = 0; i < q; i++) {
                for (int y = 0; y < buffer.BufferHt; y++) {
                    orig.GetPixel(x, y, c);
                    for (int j = 0; j < q; j++) {
                        float xx = (float)x + ((float)i * inc) - xoff;
                        float yy = (float)y + ((float)j * inc) - yoff;
                        float u = xoff + anglecos * xx * zoom + anglesin * yy * zoom;
                        if (u >= 0 && u < buffer.BufferWi) {
                            float v = yoff + -anglesin * xx * zoom + anglecos * yy * zoom;

                            if (v >= 0 && v < buffer.BufferHt) {
                                buffer.SetPixel(u, v, c);
                            }
                        }
                    }
                }
            }
        }
    }
}

void PixelBufferClass::RotoZoom(LayerInfo* layer, float offset)
{
    if (std::isinf(offset)) offset = 1.0;

    GPURenderUtils::RotoZoomSettings settings;
    settings.offset = offset;
    settings.rotationorder = layer->rotationorder;
    
    settings.xrotation = layer->xrotation;
    if (layer->XRotationValueCurve.IsActive()) {
        settings.xrotation = layer->XRotationValueCurve.GetOutputValueAt(offset, layer->buffer.GetStartTimeMS(), layer->buffer.GetEndTimeMS());
    }
    settings.xpivot = layer->xpivot;
    settings.ypivot = layer->ypivot;
    if (settings.xrotation != 0 && settings.xrotation != 360) {
        GPURenderUtils::waitForRenderCompletion(&layer->buffer);
        if (layer->XPivotValueCurve.IsActive()) {
            settings.xpivot = layer->XPivotValueCurve.GetOutputValueAt(offset, layer->buffer.GetStartTimeMS(), layer->buffer.GetEndTimeMS());
        }
    }
    settings.yrotation = layer->yrotation;
    if (layer->YRotationValueCurve.IsActive()) {
        settings.yrotation = layer->YRotationValueCurve.GetOutputValueAt(offset, layer->buffer.GetStartTimeMS(), layer->buffer.GetEndTimeMS());
    }
    if (settings.yrotation != 0 && settings.yrotation != 360) {
        if (layer->YPivotValueCurve.IsActive()) {
            settings.ypivot = layer->YPivotValueCurve.GetOutputValueAt(offset, layer->buffer.GetStartTimeMS(), layer->buffer.GetEndTimeMS());
        }
    }
    
    settings.zoom = layer->zoom;
    if (layer->ZoomValueCurve.IsActive()) {
        settings.zoom = layer->ZoomValueCurve.GetOutputValueAtDivided(offset, layer->buffer.GetStartTimeMS(), layer->buffer.GetEndTimeMS());
    }
    float rotations = layer->rotations;
    if (layer->RotationsValueCurve.IsActive()) {
        rotations = layer->RotationsValueCurve.GetOutputValueAtDivided(offset, layer->buffer.GetStartTimeMS(), layer->buffer.GetEndTimeMS());
    }

    float rotationoffset = offset;
    float offsetperrotation = 1.0f;
    if (rotations > 0) {
        offsetperrotation = 1.0f / rotations;
    }
    while (rotationoffset > offsetperrotation) {
        rotationoffset -= offsetperrotation;
    }
    rotationoffset *= rotations;
    settings.zrotation = (float)layer->rotation / 100.0;
    if (rotations > 0) {
        if (layer->RotationValueCurve.IsActive()) {
            settings.zrotation = layer->RotationValueCurve.GetValueAt(rotationoffset, layer->buffer.GetStartTimeMS(), layer->buffer.GetEndTimeMS());
        }
    }
    settings.zoomquality = layer->zoomquality;
    settings.pivotpointx = layer->pivotpointx;
    if (layer->PivotPointXValueCurve.IsActive()) {
        settings.pivotpointx = layer->PivotPointXValueCurve.GetOutputValueAt(offset, layer->buffer.GetStartTimeMS(), layer->buffer.GetEndTimeMS());
    }
    settings.pivotpointy = layer->pivotpointy;
    if (layer->PivotPointYValueCurve.IsActive()) {
        settings.pivotpointy = layer->PivotPointYValueCurve.GetOutputValueAt(offset, layer->buffer.GetStartTimeMS(), layer->buffer.GetEndTimeMS());
    }
    bool willDoRZ = (settings.xrotation != 0 && settings.xrotation != 360);
    willDoRZ |= (settings.yrotation != 0 && settings.yrotation != 360);
    willDoRZ |= (settings.zrotation != 0.0 || settings.zoom != 1.0);
    
    if (willDoRZ) {
        if (!GPURenderUtils::RotoZoom(&layer->buffer, settings)) {
            for (auto &c : layer->rotationorder) {
                switch(c) {
                case 'X':
                    RotateX(layer->buffer, settings);
                    break;
                case 'Y':
                    RotateY(layer->buffer, settings);
                    break;
                case 'Z':
                    RotateZAndZoom(layer->buffer, settings);
                    break;
                }
            }
        }
    }    
}

bool PixelBufferClass::IsVariableSubBuffer(int layer) const
{
    const std::string &subBuffer = layers[layer]->subBuffer;
    return subBuffer.find("Active=TRUE") != std::string::npos;
}

MixTypes PixelBufferClass::GetMixType(int layer) const
{
    return layers[layer]->mixType;
}

bool PixelBufferClass::IsCanvasMix(int layer) const
{
    return layers[layer]->canvas;
}

void PixelBufferClass::PrepareVariableSubBuffer(int EffectPeriod, int layer)
{
    if (!IsVariableSubBuffer(layer)) return;

    const std::string &subBuffer = layers[layer]->subBuffer;

    int effStartPer, effEndPer;
    layers[layer]->buffer.GetEffectPeriods(effStartPer, effEndPer);
    float offset = 0.0;
    if (effEndPer != effStartPer) {
        offset = ((float)EffectPeriod - (float)effStartPer) / ((float)effEndPer - (float)effStartPer);
    }
    offset = std::min(offset, 1.0f);
    const std::string &type = layers[layer]->type;
    const std::string &camera = layers[layer]->camera;
    const std::string &transform = layers[layer]->transform;
    layers[layer]->buffer.Nodes.clear();
    layers[layer]->BufferOffsetX = 0;
    layers[layer]->BufferOffsetY = 0;
    model->InitRenderBufferNodes(type, camera, transform, layers[layer]->buffer.Nodes, layers[layer]->BufferWi, layers[layer]->BufferHt, layers[layer]->stagger);
    ComputeSubBuffer(subBuffer, layers[layer]->buffer.Nodes, layers[layer]->BufferWi, layers[layer]->BufferHt,
                     layers[layer]->BufferOffsetX, layers[layer]->BufferOffsetY,
                     offset, layers[layer]->buffer.GetStartTimeMS(), layers[layer]->buffer.GetEndTimeMS());
    layers[layer]->buffer.BufferWi = layers[layer]->BufferWi;
    layers[layer]->buffer.BufferHt = layers[layer]->BufferHt;

    if (layers[layer]->buffer.BufferWi == 0) layers[layer]->buffer.BufferWi = 1;
    if (layers[layer]->buffer.BufferHt == 0) layers[layer]->buffer.BufferHt = 1;
    
    layers[layer]->buffer.InitBuffer(layers[layer]->BufferHt, layers[layer]->BufferWi, transform);
    GPURenderUtils::setupRenderBuffer(this, &layers[layer]->buffer);
}

void PixelBufferClass::HandleLayerBlurZoom(int EffectPeriod, int layer) {
    int effStartPer, effEndPer;
    layers[layer]->buffer.GetEffectPeriods(effStartPer, effEndPer);
    float offset = 0.0f;
    if (effEndPer != effStartPer) {
        offset = ((float)EffectPeriod - (float)effStartPer) / ((float)effEndPer - (float)effStartPer);
    }
    offset = std::min(offset, 1.0f);

    if (layers[layer]->freezeAfterFrame > EffectPeriod - effStartPer) {
        // do gausian blur
        if (layers[layer]->BlurValueCurve.IsActive() || layers[layer]->blur > 1) {
            Blur(layers[layer], offset);
        }
        RotoZoom(layers[layer], offset);
    }
    // All rendering is encoded, commit it so the GPU can start
    GPURenderUtils::commitRenderBuffer(&layers[layer]->buffer);
}

void PixelBufferClass::CalcOutput(int EffectPeriod, const std::vector<bool> & validLayers, int saveLayer)
{
    int curStep;

    for(int ii=0; ii < numLayers; ii++) {
        if (!validLayers[ii]) {
            continue;
        }
        if (layers[ii]->use_music_sparkle_count &&
            layers[ii]->buffer.GetMedia() != nullptr) {
            float f = 0.0;
            std::list<float> const * const pf = layers[ii]->buffer.GetMedia()->GetFrameData(layers[ii]->buffer.curPeriod, FRAMEDATA_HIGH, "");
            if (pf != nullptr) {
                f = *pf->cbegin();
            }
            layers[ii]->music_sparkle_count_factor = f;
        } else {
            layers[ii]->use_music_sparkle_count = false;
        }


        double fadeInFactor=1, fadeOutFactor=1;
        layers[ii]->fadeFactor = 1.0;
        layers[ii]->inMaskFactor = 1.0;
        layers[ii]->outMaskFactor = 1.0;
        if (layers[ii]->fadeInSteps > 0 || layers[ii]->fadeOutSteps > 0) {
            int effStartPer, effEndPer;
            layers[ii]->buffer.GetEffectPeriods(effStartPer, effEndPer);
            bool isFirstFrame = (effStartPer == EffectPeriod);

            if (EffectPeriod < (effStartPer)+layers[ii]->fadeInSteps && layers[ii]->fadeInSteps != 0) {
                curStep = EffectPeriod - effStartPer + 1;
                fadeInFactor = (double)curStep/(double)layers[ii]->fadeInSteps;
            }
            if (EffectPeriod > (effEndPer)-layers[ii]->fadeOutSteps && layers[ii]->fadeOutSteps != 0) {
                curStep = EffectPeriod - (effEndPer-layers[ii]->fadeOutSteps);
                fadeOutFactor = 1-(double)curStep/(double)layers[ii]->fadeOutSteps;
            }
            //calc fades
            if (STR_FADE == layers[ii]->inTransitionType) {
                if (fadeInFactor<1) {
                    layers[ii]->fadeFactor = fadeInFactor;
                }
            } else {
                layers[ii]->inMaskFactor = fadeInFactor;
            }
            if ( STR_FADE == layers[ii]->outTransitionType) {
               if (fadeOutFactor < 1) {
                  if (STR_FADE == layers[ii]->inTransitionType && fadeInFactor < 1)
                     layers[ii]->fadeFactor = (fadeInFactor + fadeOutFactor) / 2.0;
                  else
                     layers[ii]->fadeFactor = fadeOutFactor;
               }
            } else {
               layers[ii]->outMaskFactor = fadeOutFactor;
            }
            // this is where it gets tricky, since calculateMask() handles both in and out transitions...
            const RenderBuffer *prevRB = nullptr;
            int fakeLayerIndex = numLayers - 1;
            if ( fakeLayerIndex - ii > 1 )
               prevRB = &layers[ii+1]->buffer;
            layers[ii]->renderTransitions(isFirstFrame, prevRB);
        } else {
           layers[ii]->mask.clear();
        }
        layers[ii]->calculateNodeOutputParams(EffectPeriod);
        
        GPURenderUtils::waitForRenderCompletion(&layers[ii]->buffer);
    }

    // layer calculation and map to output
    size_t NodeCount = layers[0]->buffer.Nodes.size();
    int countValid = 0;
    for (auto x : validLayers) {
        if (x) {
            ++countValid;
        }
    }
    int blockSize = std::max( 5000 / std::max(countValid, 1), 500);
    /*
    //bunch of test code to test the timing of various block sizes to see what impact
    //the block size has
    static int test = 2;
    if (countValid == test) {
        for (int vvv = 1000; vvv < (NodeCount + 1000); vvv += 1000) {
            wxStopWatch timer;
            parallel_for(0, NodeCount, [this, saveLayer, &validLayers, EffectPeriod] (int i) {
                if (!layers[saveLayer]->buffer.Nodes[i]->IsVisible()) {
                    // unmapped pixel - set to black
                    layers[saveLayer]->buffer.Nodes[i]->SetColor(xlBLACK);
                } else {
                    // get blend of two effects
                    xlColor color;
                    GetMixedColor(i,
                                  color,
                                  validLayers, EffectPeriod);

                    // set color for physical output
                    layers[saveLayer]->buffer.Nodes[i]->SetColor(color);
                }
            }, vvv);
            printf("%d\t%d\t%lld\n", test, vvv, timer.TimeInMicro());
        }
        test++;
    }
    */

    std::vector<NodeBaseClassPtr> &Nodes = layers[saveLayer]->buffer.Nodes;
    parallel_for(0, NodeCount, [this, &Nodes, &validLayers, saveLayer, EffectPeriod] (int i) {
        if (!Nodes[i]->IsVisible()) {
            // unmapped pixel - set to black
            Nodes[i]->SetColor(xlBLACK);
        } else {
            // get blend of two effects
            GetMixedColor(i, validLayers, EffectPeriod, saveLayer);
        }
    }, blockSize);
}

static int DecodeType(const std::string &type)
{
    if (type == "Wipe")
    {
        return 1;
    }
    else if (type == "Clock")
    {
        return 2;
    }
    else if (type == "From Middle")
    {
        return 3;
    }
    else if (type == "Square Explode")
    {
        return 4;
    }
    else if (type == "Circle Explode")
    {
        return 5;
    }
    else if (type == "Blinds")
    {
        return 6;
    }
    else if (type == "Blend")
    {
        return 7;
    }
    else if (type == "Slide Checks")
    {
        return 8;
    }
    else if (type == "Slide Bars")
    {
        return 9;
    }

    return 0;
}
void PixelBufferClass::LayerInfo::clear() {
    buffer.Clear();
    if (modelBuffers) {
        for (auto it = modelBuffers->begin();  it != modelBuffers->end(); ++it) {
            (*it)->Clear();
        }
    }
}


void PixelBufferClass::LayerInfo::createFromMiddleMask(bool out) {
    bool reverse = inTransitionReverse;
    float factor = inMaskFactor;
    int adjust = inTransitionAdjust;
    if ( InTransitionAdjustValueCurve.IsActive() )
       adjust = static_cast<int>( InTransitionAdjustValueCurve.GetOutputValueAt( factor, buffer.GetStartTimeMS(), buffer.GetEndTimeMS() ) );

    if (out) {
        reverse = outTransitionReverse;
        factor = outMaskFactor;
        adjust = outTransitionAdjust;
        if ( OutTransitionAdjustValueCurve.IsActive() )
           adjust = static_cast<int>( OutTransitionAdjustValueCurve.GetOutputValueAt( factor, buffer.GetStartTimeMS(), buffer.GetEndTimeMS() ) );
    }
    uint8_t m1 = 255;
    uint8_t m2 = 0;

    if (reverse) {
        factor = 1.0 - factor;
        m1 = 0;
        m2 = 255;
    }

    double w_2 = 0.5 * buffer.BufferWi;
    double h_2 = 0.5 * buffer.BufferHt;
    Vec2D p1( w_2, 0. );
    Vec2D p2( w_2, buffer.BufferHt );

    double angle = interpolate( 0.01 * adjust, 0.0, -M_PI_2, 1.0, M_PI_2, LinearInterpolater() );
    p1 = p1.RotateAbout( angle, Vec2D( w_2, h_2 ) );
    p2 = p2.RotateAbout( angle, Vec2D( w_2, h_2 ) );

    double p1_p2_len = p2.Dist( p1 );
    double y2_less_y1 = p2.y - p1.y;
    double x2_less_x1 = p2.x - p1.x;
    double offset = p2.x * p1.y - p2.y * p1.x;
    uint8_t c = 0;

    double len = ::sqrt( buffer.BufferWi * buffer.BufferWi + buffer.BufferHt * buffer.BufferHt );
    double step = len / 2.0 * factor;

   for (int x = 0; x < BufferWi; ++x )
   {
      for ( int y = 0; y < BufferHt; ++y )
      {
         double dist = std::abs( y2_less_y1 * x - x2_less_x1 * y + offset ) / p1_p2_len;
         c = (dist > step) ? m1 : m2;
         mask[x * BufferHt + y] = c;
      }
   }
}

void PixelBufferClass::LayerInfo::createCircleExplodeMask(bool out) {
    // distance from centre
    // sqrt((x - buffer.BufferWi / 2) ^ 2 + (y - buffer.BufferHt / 2) ^ 2);
    float maxradius = sqrt(((buffer.BufferWi / 2) * (buffer.BufferWi / 2)) + ((buffer.BufferHt / 2) * (buffer.BufferHt / 2)));

    bool reverse = inTransitionReverse;
    float factor = inMaskFactor;
    if (out) {
        reverse = !outTransitionReverse;
        factor = outMaskFactor;
    }

    uint8_t m1 = 255;
    uint8_t m2 = 0;

    if (reverse) {
        factor = 1.0 - factor;
        m1 = 0;
        m2 = 255;
    }

    float rad = maxradius * factor;

    for (int x = 0; x < BufferWi; x++)
    {
        for (int y = 0; y < BufferHt; y++)
        {
            float radius = sqrt((x - (BufferWi / 2)) * (x - (BufferWi / 2)) + (y - (BufferHt / 2)) * (y - (BufferHt / 2)));
            mask[x * BufferHt + y] = radius < rad ? m2 : m1;
        }
    }
}
void PixelBufferClass::LayerInfo::createSquareExplodeMask(bool out)
{
    bool reverse = inTransitionReverse;
    float factor = inMaskFactor;
    if (out) {
        reverse = !outTransitionReverse;
        factor = outMaskFactor;
    }

    uint8_t m1 = 255;
    uint8_t m2 = 0;

    if (reverse) {
        factor = 1.0 - factor;
        m1 = 0;
        m2 = 255;
    }

    float xstep = ((float)BufferWi / 2.0) * (float)factor;
    float ystep = ((float)BufferHt / 2.0) * (float)factor;

    int x1 = BufferWi / 2 - xstep;
    int x2 = BufferWi / 2 + xstep;
    int y1 = BufferHt / 2 - ystep;
    int y2 = BufferHt / 2 + ystep;
    for (int x = 0; x < BufferWi; x++) {
        for (int y = 0; y < BufferHt; y++) {
            uint8_t c;
            if (x < x1 || x > x2 || y < y1 || y > y2) {
                c = m1;
            } else {
                c = m2;
            }
            mask[x * BufferHt + y] = c;
        }
    }
}

static bool isLeft(const wxPoint &a, const wxPoint &b, const wxPoint &test) {
    return ((b.x - a.x)*(test.y - a.y) - (b.y - a.y)*(test.x - a.x)) > 0;
}

void PixelBufferClass::LayerInfo::createWipeMask(bool out)
{
    int adjust = inTransitionAdjust;
    float factor = inMaskFactor;
    if ( InTransitionAdjustValueCurve.IsActive() )
       adjust = static_cast<int>( InTransitionAdjustValueCurve.GetOutputValueAt( factor, buffer.GetStartTimeMS(), buffer.GetEndTimeMS() ) );
    bool reverse = inTransitionReverse;
    if (out) {
        adjust = outTransitionAdjust;
        if ( OutTransitionAdjustValueCurve.IsActive() )
           adjust = static_cast<int>( OutTransitionAdjustValueCurve.GetOutputValueAt( factor, buffer.GetStartTimeMS(), buffer.GetEndTimeMS() ) );
        reverse = outTransitionReverse;
        factor = outMaskFactor;
    }

    if (reverse) {
        adjust += 50;
        if (adjust >= 100) {
            adjust -= 100;
        }
    }

    float angle = 2.0 * M_PI * (float)adjust / 100.0;

    float slope = tan(angle);

    uint8_t m1 = 255;
    uint8_t m2 = 0;

    float curx = std::round(factor * ((float)BufferWi - 1.0));
    float cury = std::round(factor * ((float)BufferHt - 1.0));

    if (angle >= 0 && angle < M_PI_2) {
        curx = BufferWi - curx - 1;
        std::swap(m1, m2);
    } else if (angle >= M_PI_2 && angle < M_PI) {
        curx = BufferWi - curx - 1;
        cury = BufferHt - cury - 1;
    } else if (angle >= M_PI && angle < (M_PI + M_PI_2)) {
        cury = BufferHt - cury - 1;
    } else {
        std::swap(m1, m2);
    }
    float endx = curx == -1 ? -5 : -1;
    float endy = slope * (endx - curx) + cury;
    if (slope > 999) {
        //nearly vertical
        endx = curx;
        endy = cury - 10;
    } else if (slope < -999) {
        //nearly vertical
        endx = curx;
        endy = cury + 10;
    }
    wxPoint start(curx, cury);
    wxPoint end(endx, endy);

    // start bottom left 0, 0
    // y = slope * x + y'
    for (int x = 0; x < BufferWi; x++) {
        for (int y = 0; y < BufferHt; y++) {
            mask[x * BufferHt + y] = isLeft(start, end, wxPoint(x, y)) ? m1 : m2;
        }
    }
}

void PixelBufferClass::LayerInfo::createClockMask(bool out)
{
    bool reverse = inTransitionReverse;
    float factor = inMaskFactor;
    int adjust = inTransitionAdjust;
    if (InTransitionAdjustValueCurve.IsActive())
        adjust = static_cast<int>(InTransitionAdjustValueCurve.GetOutputValueAt(factor, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()));
    uint8_t m1 = 255;
    uint8_t m2 = 0;
    if (out) {
        reverse = outTransitionReverse;
        factor = outMaskFactor;
        adjust = outTransitionAdjust;
        if (OutTransitionAdjustValueCurve.IsActive())
            adjust = static_cast<int>(OutTransitionAdjustValueCurve.GetOutputValueAt(factor, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()));
    }

    float startradians = 2.0 * M_PI * (float)adjust / 100.0;
    float currentradians = 2.0 * M_PI * factor;
    if (reverse) {
        float tmp = startradians;
        startradians = startradians - currentradians;
        currentradians = tmp;
        if (startradians < 0) {
            startradians += 2.0f * (float)M_PI;
            currentradians += 2.0f * (float)M_PI;
        }
    }
    else {
        currentradians = startradians + currentradians;
    }

    for (int x = 0; x < BufferWi; x++) {
        for (int y = 0; y < BufferHt; y++) {
            float radianspixel;
            if (x - BufferWi / 2 == 0 && y - BufferHt / 2 == 0) {
                radianspixel = 0.0;
            }
            else {
                radianspixel = atan2(x - BufferWi / 2,
                    y - BufferHt / 2);
            }
            if (radianspixel < 0) {
                radianspixel += 2.0f * (float)M_PI;
            }
            if (currentradians > 2.0f * (float)M_PI && radianspixel < startradians) {
                radianspixel += 2.0f * (float)M_PI;
            }

            bool s_lt_p = radianspixel > startradians;
            bool c_gt_p = radianspixel < currentradians;
            mask[x * BufferHt + y] = (s_lt_p && c_gt_p) ? m2 : m1;
        }
    }
}

void PixelBufferClass::LayerInfo::createBlindsMask(bool out)
{
    bool reverse = inTransitionReverse;
    float factor = inMaskFactor;
    int adjust = inTransitionAdjust;
    if (InTransitionAdjustValueCurve.IsActive())
        adjust = static_cast<int>(InTransitionAdjustValueCurve.GetOutputValueAt(factor, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()));
    uint8_t m1 = 255;
    uint8_t m2 = 0;
    if (out) {
        reverse = outTransitionReverse;
        factor = outMaskFactor;
        adjust = outTransitionAdjust;
        if (OutTransitionAdjustValueCurve.IsActive())
            adjust = static_cast<int>(OutTransitionAdjustValueCurve.GetOutputValueAt(factor, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()));
    }
    if (adjust == 0) {
        adjust = 1;
    }
    adjust = (buffer.BufferWi / 2) * adjust / 100;
    if (adjust == 0) {
        adjust = 1;
    }

    int per = buffer.BufferWi / adjust;
    if (per < 1) {
        per = 1;
    }
    int blinds = buffer.BufferWi / per;
    while (blinds * per < buffer.BufferWi) {
        blinds++;
    }
    int step = std::round(((float)per) * factor);
    int x = 0;
    while (x < BufferWi) {
        for (int z = 0; z < per && x < BufferWi; z++, x++) {
            int c = z < step ? m2 : m1;
            if (reverse) {
                c = (per - z - 1) < step ? m2 : m1;
            }
            for (int y = 0; y < BufferHt; y++) {
                mask[x * BufferHt + y] = c;
            }
        }
    }
}

void PixelBufferClass::LayerInfo::createBlendMask(bool out) {
    //bool reverse = inTransitionReverse;
    float factor = inMaskFactor;
    int adjust = inTransitionAdjust;
    if ( InTransitionAdjustValueCurve.IsActive() )
       adjust = static_cast<int>( InTransitionAdjustValueCurve.GetOutputValueAt( factor, buffer.GetStartTimeMS(), buffer.GetEndTimeMS() ) );
    uint8_t m1 = 255;
    uint8_t m2 = 0;
    if (out) {
        //reverse = outTransitionReverse;
        factor = outMaskFactor;
        adjust = outTransitionAdjust;
        if ( OutTransitionAdjustValueCurve.IsActive() )
           adjust = static_cast<int>( OutTransitionAdjustValueCurve.GetOutputValueAt( factor, buffer.GetStartTimeMS(), buffer.GetEndTimeMS() ) );
    }

    std::minstd_rand rng(1234);

    int pixels = BufferWi * BufferHt;
    adjust = 10 * adjust / 100;
    if (adjust == 0) {
        adjust = 1;
    }
    int actualpixels = pixels / (adjust * adjust);
    if (actualpixels == 0) actualpixels = 1;
    float step = ((float)pixels / (adjust*adjust)) * factor;

    int xpixels = BufferWi / adjust;
    while (xpixels * adjust < BufferWi) {
        xpixels++;
    }
    int ypixels = BufferHt / adjust;
    while (ypixels * adjust < BufferHt) {
        ypixels++;
    }

    // set all the background first
    for (int x = 0; x < BufferWi; x++) {
        for (int y = 0; y < BufferHt; y++) {
            mask[x * BufferHt + y] = m1;
        }
    }

    for (int i = 0; i < step; i++)
    {
        int jy = rng() % actualpixels;
        int jx = rng() % actualpixels;

        int x = (jx % xpixels) * adjust;
        int y = (jy % ypixels) * adjust;
        if (mask[x * BufferHt + y] == m2) {

            // check if there is anything left to mask
            bool undone = false;
            for (int tx = 0; tx < std::min(xpixels, actualpixels) && undone == false; ++tx)
            {
                for (int ty = 0; ty < std::min(ypixels, actualpixels) && undone == false; ++ty)
                {
                    if (mask[tx * adjust * BufferHt + ty * adjust] == m1)
                    {
                        undone = true;
                    }
                }
            }

            if (undone)
            {
                i--;
            }
            else
            {
                break;
            }
        } else {
            for (int k = 0; k < adjust; ++k) {
                if ((x + k) < BufferWi) {
                    for (int l = 0; l < adjust; l++) {
                        if ((y + l) < BufferHt) {
                            mask[(x + k) * BufferHt + y + l] = m2;
                        }
                    }
                }
            }
        }
    }
}

void PixelBufferClass::LayerInfo::createSlideChecksMask(bool out) {
    bool reverse = inTransitionReverse;
    float factor = inMaskFactor;
    int adjust = inTransitionAdjust;
    if ( InTransitionAdjustValueCurve.IsActive() )
       adjust = static_cast<int>( InTransitionAdjustValueCurve.GetOutputValueAt( factor, buffer.GetStartTimeMS(), buffer.GetEndTimeMS() ) );
    uint8_t m1 = 255;
    uint8_t m2 = 0;
    if (out) {
        reverse = outTransitionReverse;
        factor = outMaskFactor;
        adjust = outTransitionAdjust;
        if ( OutTransitionAdjustValueCurve.IsActive() )
           adjust = static_cast<int>( OutTransitionAdjustValueCurve.GetOutputValueAt( factor, buffer.GetStartTimeMS(), buffer.GetEndTimeMS() ) );
    }

    if (adjust < 2) {
        adjust = 2;
    }
    adjust = (std::max(buffer.BufferWi / 2, buffer.BufferHt /2)) * adjust / 100;
    if (adjust < 2) {
        adjust = 2;
    }
    int xper = buffer.BufferWi * 2 / adjust ;
    if (xper < 1) {
        xper = 1;
    }
    int yper = buffer.BufferHt / adjust;
    if (yper < 1) {
        yper = 1;
    }
    float step = (((float)xper*2.0) * factor);
    for (int x = 0; x < BufferWi; x++) {
        int xb = x / xper;
        int xp = (x - xb * xper) % xper;
        int xpos = x;
        if (reverse) {
            xpos = BufferWi - x - 1;
        }
        for (int y = 0; y < BufferHt; y++) {
            int yb = y / yper;
            if (yb % 2) {
                if (xp >= (xper / 2)) {
                    int xp2 = xp - xper / 2;
                    mask[xpos * BufferHt + y] = xp2 < step ? m2 : m1;
                } else {
                    int step2 = step - (xper / 2);
                    mask[xpos * BufferHt + y] = xp < step2 ? m2 : m1;
                }
            } else {
                mask[xpos * BufferHt + y] = xp < step ? m2 : m1;
            }
        }
    }
}
void PixelBufferClass::LayerInfo::createSlideBarsMask(bool out) {
    //bool reverse = inTransitionReverse;
    float factor = inMaskFactor;
    int adjust = inTransitionAdjust;
    if ( InTransitionAdjustValueCurve.IsActive() )
       adjust = static_cast<int>( InTransitionAdjustValueCurve.GetOutputValueAt( factor, buffer.GetStartTimeMS(), buffer.GetEndTimeMS() ) );
    uint8_t m1 = 255;
    uint8_t m2 = 0;
    if (out) {
        //reverse = outTransitionReverse;
        factor = outMaskFactor;
        adjust = outTransitionAdjust;
        if ( OutTransitionAdjustValueCurve.IsActive() )
           adjust = static_cast<int>( OutTransitionAdjustValueCurve.GetOutputValueAt( factor, buffer.GetStartTimeMS(), buffer.GetEndTimeMS() ) );
    }

    if (adjust == 0) {
        adjust = 1;
    }
    adjust = (BufferHt / 2) * adjust / 100;
    if (adjust == 0) {
        adjust = 1;
    }

    int per = BufferHt / adjust;
    if (per < 1) {
        per = 1;
    }
    int blinds = BufferHt / per;
    while (blinds * per < BufferHt) {
        blinds++;
    }

    float step = (float)BufferWi * factor;
    for (int y = 0; y < BufferHt; y++) {
        int blind = y / per;
        for (int x = 0; x < BufferWi; x++) {
            int xpos = x;
            if ((blind % 2 == 1) == out) {
                xpos = BufferWi - x - 1;
            }
            mask[xpos * BufferHt + y] = x <= step ? m2 : m1;
        }
    }
}

namespace
{
   const std::vector<std::string> transitionNames = {
       STR_FOLD, STR_DISSOLVE, STR_CIRCULAR_SWIRL, STR_BOW_TIE, STR_ZOOM, STR_DOORWAY, STR_BLOBS, STR_PINWHEEL, STR_STAR, STR_SWAP, STR_SHATTER, STR_CIRCLES
   };
   bool nonMaskTransition( const std::string& transitionType )
   {
      return std::find( transitionNames.cbegin(), transitionNames.cend(), transitionType ) != transitionNames.cend();
   }
}

void PixelBufferClass::LayerInfo::renderTransitions(bool isFirstFrame, const RenderBuffer* prevRB) {
    bool hasMask = false;
    if (inMaskFactor < 1.0) {
        mask.resize(BufferHt * BufferWi);
        if ( nonMaskTransition( inTransitionType ) ) {
            ColorBuffer cb( buffer.pixels, buffer.pixelVector.size(), buffer.BufferWi, buffer.BufferHt );

            if ( inTransitionType == STR_FOLD ) {
               foldIn( buffer, cb, prevRB, inMaskFactor, inTransitionReverse );
            } else if ( inTransitionType == STR_DISSOLVE ) {
               dissolveIn( buffer, cb, inMaskFactor );
            } else if ( inTransitionType == STR_CIRCULAR_SWIRL ) {
               Vec2D xy( 0.5, 0.5 );
               double speed = interpolate( 0.2, 0.0, 1.0, 40.0, 9.0, LinearInterpolater() );
               circularSwirl( buffer, cb, xy, speed, 1.f-inMaskFactor );
            } else if ( inTransitionType == STR_BOW_TIE ) {
               int adjust = inTransitionAdjust;
               if ( InTransitionAdjustValueCurve.IsActive() )
                  adjust = static_cast<int>( InTransitionAdjustValueCurve.GetOutputValueAt( inMaskFactor, buffer.GetStartTimeMS(), buffer.GetEndTimeMS() ) );
               bowTie( buffer, cb, prevRB, inMaskFactor, adjust, inTransitionReverse );
            } else if ( inTransitionType == STR_ZOOM ) {
               zoomTransition( buffer, cb, inMaskFactor );
            } else if ( inTransitionType == STR_DOORWAY ) {
               doorway( buffer, cb, prevRB, inMaskFactor );
            } else if ( inTransitionType == STR_BLOBS ) {
               int adjust = inTransitionAdjust;
               if ( InTransitionAdjustValueCurve.IsActive() )
                  adjust = static_cast<int>( InTransitionAdjustValueCurve.GetOutputValueAt( inMaskFactor, buffer.GetStartTimeMS(), buffer.GetEndTimeMS() ) );
               blobs( buffer, cb, prevRB, inMaskFactor, adjust );
            } else if ( inTransitionType == STR_PINWHEEL ) {
               int adjust = inTransitionAdjust;
               if ( InTransitionAdjustValueCurve.IsActive() )
                  adjust = static_cast<int>( InTransitionAdjustValueCurve.GetOutputValueAt( inMaskFactor, buffer.GetStartTimeMS(), buffer.GetEndTimeMS() ) );
               pinwheelTransition( buffer, cb, prevRB, 1.-inMaskFactor, adjust );
            } else if ( inTransitionType == STR_STAR ) {
               int adjust = inTransitionAdjust;
               if ( InTransitionAdjustValueCurve.IsActive() )
                  adjust = static_cast<int>( InTransitionAdjustValueCurve.GetOutputValueAt( inMaskFactor, buffer.GetStartTimeMS(), buffer.GetEndTimeMS() ) );
               starTransition( buffer, cb, prevRB, inMaskFactor, adjust, inTransitionReverse );
            } else if ( inTransitionType == STR_SWAP ) {
               swapTransition( buffer, cb, prevRB, inMaskFactor );
            } else if ( inTransitionType == STR_SHATTER ) {
               shatterTransition( buffer, cb, prevRB, 1.f-inMaskFactor );
            } else if ( inTransitionType == STR_CIRCLES ) {
               int adjust = inTransitionAdjust;
               if ( InTransitionAdjustValueCurve.IsActive() )
                  adjust = static_cast<int>( InTransitionAdjustValueCurve.GetOutputValueAt( inMaskFactor, buffer.GetStartTimeMS(), buffer.GetEndTimeMS() ) );
               circlesTransition( buffer, cb, prevRB, 1.f-inMaskFactor, adjust );
            }
        } else {
           calculateMask(inTransitionType, false, isFirstFrame);
        }
        hasMask = true;
    }
    if (outMaskFactor < 1.0) {
        mask.resize(BufferHt * BufferWi);
        if ( nonMaskTransition( outTransitionType ) ) {
            ColorBuffer cb( buffer.pixels, buffer.pixelVector.size(), buffer.BufferWi, buffer.BufferHt );
            if ( outTransitionType == STR_FOLD ) {
               foldOut( buffer, cb, prevRB, outMaskFactor, outTransitionReverse );
            } else if ( outTransitionType == STR_DISSOLVE ) {
               dissolveOut( buffer, cb, 1.f - outMaskFactor );
            } else if ( outTransitionType == STR_CIRCULAR_SWIRL ) {
               Vec2D xy( 0.5, 0.5 );
               double speed = interpolate( 0.2, 0.0, 1.0, 40.0, 9.0, LinearInterpolater() );
               circularSwirl( buffer, cb, xy, speed, 1.f - outMaskFactor );
            } else if ( outTransitionType == STR_BOW_TIE ) {
               int adjust = outTransitionAdjust;
               if ( OutTransitionAdjustValueCurve.IsActive() )
                  adjust = static_cast<int>( OutTransitionAdjustValueCurve.GetOutputValueAt( outMaskFactor, buffer.GetStartTimeMS(), buffer.GetEndTimeMS() ) );
               bowTie( buffer, cb, prevRB, outMaskFactor, adjust, outTransitionReverse );
            } else if ( outTransitionType == STR_ZOOM ) {
               zoomTransition( buffer, cb, outMaskFactor );
            } else if ( outTransitionType == STR_DOORWAY ) {
               doorway( buffer, cb, prevRB, outMaskFactor );
            } else if ( outTransitionType == STR_BLOBS ) {
               int adjust = outTransitionAdjust;
               if ( OutTransitionAdjustValueCurve.IsActive() )
                  adjust = static_cast<int>( OutTransitionAdjustValueCurve.GetOutputValueAt( outMaskFactor, buffer.GetStartTimeMS(), buffer.GetEndTimeMS() ) );
               blobs( buffer, cb, prevRB, outMaskFactor, adjust );
            } else if ( outTransitionType == STR_PINWHEEL ) {
               int adjust = outTransitionAdjust;
               if ( OutTransitionAdjustValueCurve.IsActive() )
                  adjust = static_cast<int>( OutTransitionAdjustValueCurve.GetOutputValueAt( outMaskFactor, buffer.GetStartTimeMS(), buffer.GetEndTimeMS() ) );
               pinwheelTransition( buffer, cb, prevRB, 1.-outMaskFactor, adjust );
            } else if ( outTransitionType == STR_STAR ) {
               int adjust = outTransitionAdjust;
               if ( OutTransitionAdjustValueCurve.IsActive() )
                  adjust = static_cast<int>( OutTransitionAdjustValueCurve.GetOutputValueAt( outMaskFactor, buffer.GetStartTimeMS(), buffer.GetEndTimeMS() ) );
               starTransition( buffer, cb, prevRB, outMaskFactor, adjust, outTransitionReverse );
            } else if ( outTransitionType == STR_SWAP ) {
               swapTransition( buffer, cb, prevRB, outMaskFactor );
            } else if ( outTransitionType == STR_SHATTER ) {
               shatterTransition( buffer, cb, prevRB, 1.f-outMaskFactor );
            } else if ( outTransitionType == STR_CIRCLES ) {
               int adjust = outTransitionAdjust;
               if ( OutTransitionAdjustValueCurve.IsActive() )
                  adjust = static_cast<int>( OutTransitionAdjustValueCurve.GetOutputValueAt( outMaskFactor, buffer.GetStartTimeMS(), buffer.GetEndTimeMS() ) );
               circlesTransition( buffer, cb, prevRB, 1.f-outMaskFactor, adjust );
            }
        } else {
           calculateMask(outTransitionType, true, isFirstFrame);
        }
        hasMask = true;
    }
    if (!hasMask) {
        mask.clear();
    }
}

void PixelBufferClass::LayerInfo::calculateMask(const std::string &type, bool mode, bool isFirstFrame) {
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    switch (DecodeType(type)) {
        case 1:
            createWipeMask(mode);
            break;
        case 2:
            createClockMask(mode);
            break;
        case 3:
            createFromMiddleMask(mode);
            break;
        case 4:
            createSquareExplodeMask(mode);
            break;
        case 5:
            createCircleExplodeMask(mode);
            break;
        case 6:
            createBlindsMask(mode);
            break;
        case 7:
            createBlendMask(mode);
            break;
        case 8:
            createSlideChecksMask(mode);
            break;
        case 9:
            createSlideBarsMask(mode);
            break;
        default:
            if (isFirstFrame)
            {
                logger_base.warn("Unrecognised transition type '%s'.", (const char *)type.c_str());
            }
            break;
    }
}

void PixelBufferClass::LayerInfo::calculateNodeOutputParams(int EffectPeriod) {
    int effStartPer, effEndPer;
    buffer.GetEffectPeriods(effStartPer, effEndPer);
    float offset = ((float)(EffectPeriod - effStartPer)) / ((float)(effEndPer - effStartPer));
    offset = std::min(offset, 1.0f);

    if (HueAdjustValueCurve.IsActive()) {
        outputHueAdjust = HueAdjustValueCurve.GetOutputValueAt(offset, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()) / 100.0;
    } else {
        outputHueAdjust = (float)hueadjust / 100.0;
    }
    if (SaturationAdjustValueCurve.IsActive()) {
        outputSaturationAdjust = SaturationAdjustValueCurve.GetOutputValueAt(offset, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()) / 100.0;
    } else {
        outputSaturationAdjust = (float)saturationadjust / 100.0;
    }
    if (ValueAdjustValueCurve.IsActive()) {
        outputValueAdjust = ValueAdjustValueCurve.GetOutputValueAt(offset, buffer.GetStartTimeMS(), buffer.GetEndTimeMS()) / 100.0;
    } else {
        outputValueAdjust = (float)valueadjust / 100.0;
    }

    // adjust for HSV adjustments
    needsHSVAdjust = (outputHueAdjust != 0 || outputSaturationAdjust != 0 || outputValueAdjust != 0);

    outputSparkleCount = sparkle_count;
    if (SparklesValueCurve.IsActive()) {
        outputSparkleCount = (int)SparklesValueCurve.GetOutputValueAt(offset, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    }
    if (use_music_sparkle_count) {
        outputSparkleCount = (int)(music_sparkle_count_factor * (float)outputSparkleCount);
    }

    if (BrightnessValueCurve.IsActive()) {
        outputBrightnessAdjust = (int)BrightnessValueCurve.GetOutputValueAt(offset, buffer.GetStartTimeMS(), buffer.GetEndTimeMS());
    } else {
        outputBrightnessAdjust = brightness;
    }

    outputEffectMixThreshold = effectMixThreshold;
    if (effectMixVaries) {
        //vary mix threshold gradually during effect interval -DJ
        outputEffectMixThreshold = buffer.GetEffectTimeIntervalPosition();
    }
    if (outputEffectMixThreshold < 0) {
        outputEffectMixThreshold = 0;
    }
}


bool PixelBufferClass::LayerInfo::isMasked(int x, int y) {
    int idx = x*BufferHt + y;
    if (idx < mask.size()) {
        return mask[idx] > 0;
    }
    return false;
}

int PixelBufferClass::GetLayerCount() const {
    return layers.size();
}
