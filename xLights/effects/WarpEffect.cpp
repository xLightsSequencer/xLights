#include "WarpEffect.h"
#include "OnPanel.h"
#include "WarpPanel.h"

#include "../sequencer/Effect.h"
#include "../RenderBuffer.h"
#include "../UtilClasses.h"
#include "../xLightsMain.h" //xLightsFrame
#include "../OpenGLShaders.h"
#include "../DissolveTransitionPattern.h"
#include "../DrawGLUtils.h"

#ifndef __WXMAC__
#include <GL/gl.h>
#ifdef _MSC_VER
#include "GL\glext.h"
#else
#include <GL/glext.h>
#endif

extern PFNGLGENBUFFERSPROC glGenBuffers;
extern PFNGLBINDBUFFERPROC glBindBuffer;
extern PFNGLBUFFERDATAPROC glBufferData;
extern PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;
extern PFNGLACTIVETEXTUREPROC glActiveTexture;
extern PFNGLDELETEPROGRAMPROC glDeleteProgram;
extern PFNGLUSEPROGRAMPROC glUseProgram;
extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
extern PFNGLUNIFORMMATRIX2FVPROC glUniformMatrix4fv;
extern PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
extern PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
extern PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
extern PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
extern PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
extern PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
extern PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;
extern PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;
extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
extern PFNGLUNIFORM1IPROC glUniform1i;
extern PFNGLUNIFORM1FPROC glUniform1f;
extern PFNGLUNIFORM4FPROC glUniform4f;
#else
#include "OpenGL/gl.h"
#endif

#include "../../include/warp-64.xpm"
#include "../../include/warp-48.xpm"
#include "../../include/warp-32.xpm"
#include "../../include/warp-24.xpm"
#include "../../include/warp-16.xpm"

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

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

   GLuint RenderBufferTexture( int w, int h )
   {
      GLuint texId = 0;

      glGenTextures( 1, &texId );
      glBindTexture( GL_TEXTURE_2D, texId );

      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr );

      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

      glBindTexture( GL_TEXTURE_2D, 0 );

      return texId;
   }

   GLuint NoiseTexture()
   {
      GLuint texId = 0;

      glGenTextures( 1, &texId );
      glBindTexture( GL_TEXTURE_2D, texId );

      glTexImage2D( GL_TEXTURE_2D, 0, GL_R8, DissolvePatternWidth, DissolvePatternHeight, 0, GL_RED, GL_UNSIGNED_BYTE, DissolveTransitonPattern );

      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
      glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

      glBindTexture( GL_TEXTURE_2D, 0 );

      return texId;
   }

   bool createOpenGLRenderBuffer( int width, int height, GLuint *rbID, GLuint *fbID )
   {
      glGenRenderbuffers(1, rbID);
      glBindRenderbuffer(GL_RENDERBUFFER, *rbID);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, width, height );

      glGenFramebuffers(1, fbID );
      glBindFramebuffer(GL_FRAMEBUFFER, *fbID);
      glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, *rbID);
      glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, *rbID);

      glBindFramebuffer( GL_FRAMEBUFFER, 0 );

      return *rbID != 0 && *fbID != 0;
   }

   const char *vsSrc =
      "#version 330 core\n"
      "in vec2 vpos;\n"
      "in vec2 tpos;\n"
      "out vec2 texCoord;\n"
      "void main(){\n"
      "    gl_Position = vec4(vpos,0,1);\n"
      "    texCoord = tpos;\n"
      "}\n";

   const char *psDissolveIn =
      "#version 330\n"
      "in vec2 texCoord;\n"
      "out vec4 color;\n"
      "uniform sampler2D texSampler;\n"
      "uniform float progress;\n"
      "uniform sampler2D noiseSampler;\n"
      "void main(){\n"
      "    float red = texture(noiseSampler, texCoord).r;\n"
      "    color = (red <= progress)\n"
      "        ? texture(texSampler, texCoord)\n"
      "        : vec4(0, 0, 0, 1);\n"
      "}\n";
   const char *psDissolveOut =
      "#version 330\n"
      "in vec2 texCoord;\n"
      "out vec4 color;\n"
      "uniform sampler2D texSampler;\n"
      "uniform float progress;\n"
      "uniform sampler2D noiseSampler;\n"
      "void main(){\n"
      "    float red = texture( noiseSampler, texCoord).r;\n"
      "    color = (red > progress)\n"
      "        ? texture(texSampler, texCoord)\n"
      "        : vec4(0, 0, 0, 1);\n"
      "}\n";

   const char *psCircleRevealIn =
      "#version 330\n"
      "in vec2 texCoord;\n"
      "out vec4 color;\n"
      "uniform sampler2D texSampler;\n"
      "uniform float progress;\n"
      "uniform vec4 xy;\n"
      "void main(){\n"
      "    float FuzzyAmount = 0.04;\n"
      "    float CircleSize = 0.60;\n"
      "    vec2 CenterPt = vec2(xy.x, xy.y);\n"
      "\n"
      "    float radius = -FuzzyAmount + progress * (CircleSize + 2.0 * FuzzyAmount);\n"
      "    float fromCenter = length(texCoord - CenterPt);\n"
      "    float distFromCircle = fromCenter - radius;\n"
      "\n"
      "    vec4 c = texture(texSampler, texCoord);\n"
      "    float p = clamp((distFromCircle + FuzzyAmount) / (2.0 * FuzzyAmount), 0, 1);\n"
      "    color = mix( c, vec4(0,0,0,1), p );\n"
      "}\n";
   const char *psCircleRevealOut =
      "#version 330\n"
      "in vec2 texCoord;\n"
      "out vec4 color;\n"
      "uniform sampler2D texSampler;\n"
      "uniform float progress;\n"
      "uniform vec4 xy;\n"
      "void main(){\n"
      "    float FuzzyAmount = 0.04;\n"
      "    float CircleSize = 0.60;\n"
      "    vec2 CenterPt = vec2(xy.x, xy.y);\n"
      "\n"
      "    float radius = -FuzzyAmount + (1-progress) * (CircleSize + 2.0 * FuzzyAmount);\n"
      "    float fromCenter = length(texCoord - CenterPt);\n"
      "    float distFromCircle = fromCenter - radius;\n"
      "\n"
      "    vec4 c = texture(texSampler, texCoord);\n"
      "    float p = clamp((distFromCircle + FuzzyAmount) / (2.0 * FuzzyAmount), 0, 1 );\n"
      "    color = mix( c, vec4(0,0,0,1), p );\n"
      "}\n";

   const char *psBandedSwirlIn =
      "#version 330\n"
      "in vec2 texCoord;\n"
      "out vec4 color;\n"
      "uniform sampler2D texSampler;\n"
      "uniform float progress;\n"
      "uniform vec4 xy;\n"
      "uniform float frequency;\n"
      "void main() {\n"
      "    vec2 center = vec2(xy.x, xy.y);\n"
      "    float twistAmount = 1.6f;\n"
      "\n"
      "    vec2 toUV = vec2(texCoord - center);\n"
      "    float distanceFromCenter = length(toUV);\n"
      "    vec2 normToUV = toUV / distanceFromCenter;\n"
      "    float angle = atan(normToUV.y, normToUV.x);\n"
      "\n"
      "    angle += sin(distanceFromCenter * frequency) * twistAmount * (1-progress);\n"
      "    vec2 newUV = vec2(cos(angle), sin(angle));\n"
      "    newUV = newUV * distanceFromCenter + center;\n"
      "\n"
      "    vec4 c1 = texture(texSampler, fract(newUV));\n"
      "    vec4 c2 = texture(texSampler, texCoord);\n"
      "    color = mix(c1, c2, 1-progress);\n"
      "}\n";
   const char *psBandedSwirlOut =
      "#version 330\n"
      "in vec2 texCoord;\n"
      "out vec4 color;\n"
      "uniform sampler2D texSampler;\n"
      "uniform float progress;\n"
      "uniform vec4 xy;\n"
      "uniform float frequency;\n"
      "void main() {\n"
      "    vec2 center = vec2(xy.x, xy.y);\n"
      "    float twistAmount = 1.6f;\n"
      "\n"
      "    vec2 toUV = vec2(texCoord - center);\n"
      "    float distanceFromCenter = length(toUV);\n"
      "    vec2 normToUV = toUV / distanceFromCenter;\n"
      "    float angle = atan(normToUV.y, normToUV.x);\n"
      "\n"
      "    angle += sin(distanceFromCenter * frequency) * twistAmount * progress;\n"
      "    vec2 newUV = vec2(cos(angle), sin(angle));\n"
      "    newUV = newUV * distanceFromCenter + center;\n"
      "\n"
      "    vec4 c1 = texture(texSampler, fract(newUV));\n"
      "    vec4 c2 = texture(texSampler, texCoord);\n"
      "    color = mix(c2, c1, progress);\n"
      "}\n";

   const char *psRippleIn =
      "#version 330\n"
      "in vec2 texCoord;\n"
      "out vec4 color;\n"
      "uniform sampler2D texSampler;\n"
      "uniform float progress;\n"
      "uniform vec4 xy;\n"
      "uniform float speed;\n"
      "uniform float frequency;\n"
      "void main() {\n"
      "    vec2 center = vec2(xy.x, xy.y);\n"
      "    float amplitude = 0.15;\n"
      "\n"
      "    vec2 toUV = texCoord - center;\n"
      "    float distanceFromCenter = length(toUV);\n"
      "    vec2 normToUV = toUV / distanceFromCenter;\n"
      "\n"
      "    float wave = cos(frequency * distanceFromCenter - speed * progress);\n"
      "    float offset = progress * wave * amplitude;\n"
      "    vec2 newUV = center + normToUV * (distanceFromCenter + offset);\n"
      "\n"
      "    vec4 c1 = texture(texSampler, texCoord);\n"
      "    vec4 c2 = texture(texSampler, newUV);\n"
      "\n"
      "    color = mix( c2, c1, progress );\n"
      "}\n";
      const char *psRippleOut =
      "#version 330\n"
      "in vec2 texCoord;\n"
      "out vec4 color;\n"
      "uniform sampler2D texSampler;\n"
      "uniform float progress;\n"
      "uniform vec4 xy;\n"
      "uniform float speed;\n"
      "uniform float frequency;\n"
      "void main() {\n"
      "    vec2 center = vec2(xy.x, xy.y);\n"
      "    float amplitude = 0.15;\n"
      "\n"
      "    vec2 toUV = texCoord - center;\n"
      "    float distanceFromCenter = length(toUV);\n"
      "    vec2 normToUV = toUV / distanceFromCenter;\n"
      "\n"
      "    float wave = cos(frequency * distanceFromCenter - speed * progress);\n"
      "    float offset = progress * wave * amplitude;\n"
      "    vec2 newUV = center + normToUV * (distanceFromCenter + offset);\n"
      "\n"
      "    vec4 c1 = texture(texSampler, texCoord);\n"
      "    vec4 c2 = texture(texSampler, newUV);\n"
      "\n"
      "    color = mix( c1, c2, progress );\n"
      "}\n";

   const char *psWaterDrops =
      "#version 330\n"
      "precision highp float;\n"
      "in vec2 texCoord;\n"
      "out vec4 color;\n"
      "uniform float speed;\n"
      "uniform float progress;\n"
      "uniform sampler2D texSampler;\n"
      "uniform vec4 xy;\n"
      "\n"
      "const float PI = 3.14159265359;\n"
      "#define time (-progress * 35.0)\n"
      "\n"
      "float genWave( float len )\n"
      "{\n"
      "   float wave = sin( speed * PI * len + time );\n"
      "   wave = ( wave + 1.0 ) * 0.5;\n"
      "   wave -= 0.3;\n"
      "   wave *= wave * wave;\n"
      "   return wave;\n"
      "}\n"
      "\n"
      "void main() {\n"
      "   vec2 so = vec2(xy.x, xy.y); // droplet pt\n"
      "   vec2 pos2 = vec2( texCoord - so );\n"
      "   vec2 pos2n = normalize( pos2 );\n"
      "\n"
      "   float len = length( pos2 );\n"
      "   float wave = genWave( len );\n"
      "\n"
      "   vec2 uv2 = -pos2n * wave / ( 1.0 + 5.0 * len );\n"
      "\n"
      "   color = texture( texSampler, texCoord + uv2 );\n"
      "}\n";

   bool useOpenGL = false;
}

bool WarpEffect::s_shadersInit = false;
unsigned WarpEffect::s_programId_dissolve_in = 0;
unsigned WarpEffect::s_programId_dissolve_out = 0;
unsigned WarpEffect::s_programId_circleReveal_in = 0;
unsigned WarpEffect::s_programId_circleReveal_out = 0;
unsigned WarpEffect::s_programId_bandedSwirl_in = 0;
unsigned WarpEffect::s_programId_bandedSwirl_out = 0;
unsigned WarpEffect::s_programId_ripple_in = 0;
unsigned WarpEffect::s_programId_ripple_out = 0;
unsigned WarpEffect::s_programId_waterdrops = 0;
unsigned WarpEffect::s_noiseTexId = 0;
unsigned WarpEffect::s_vertexArrayId = 0;
unsigned WarpEffect::s_vertexBufferId = 0;
unsigned WarpEffect::s_fbId = 0;
unsigned WarpEffect::s_rbId = 0;
unsigned WarpEffect::s_rbTex = 0;
int WarpEffect::s_rbWidth = 0;
int WarpEffect::s_rbHeight = 0;

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

void WarpEffect::SetDefaultParameters(Model *cls)
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
       ret << "E_TEXTCTRL_Warp_Freq=" << p->TextCtrl_Warp_Frequency->GetValue().ToStdString() << ",";

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
    if ( settingsMap.Get( "E_TEXTCTRL_Warp_Freq", "" )== "20" )
      settingsMap.erase( "E_TEXTCTRL_Warp_Freq" );

    RenderableEffect::RemoveDefaults(version, effect);
}

void WarpEffect::Render(Effect *eff, SettingsMap &SettingsMap, RenderBuffer &buffer)
{
   std::string warpType = SettingsMap.Get( "CHOICE_Warp_Type", "water drops" );
   std::string warpTreatment = SettingsMap.Get( "CHOICE_Warp_Treatment", "constant");
   std::string warpStrX = SettingsMap.Get( "TEXTCTRL_Warp_X", "50" );
   std::string warpStrY = SettingsMap.Get( "TEXTCTRL_Warp_Y", "50" );
   std::string speedStr = SettingsMap.Get( "TEXTCTRL_Warp_Speed", "20" );
   std::string freqStr = SettingsMap.Get( "TEXTCTRL_Warp_Freq", "20" );
   float x = 0.01f * std::stoi( warpStrX );
   float y = 0.01f * std::stoi( warpStrY );
   float speed = std::stof( speedStr );
   float freq = std::stof( freqStr );
   float progress = buffer.GetEffectTimeIntervalPosition(1.f);

   if ( useOpenGL && OpenGLShaders::HasShaderSupport() && OpenGLShaders::HasFramebufferObjects() )
   {
      WarpPanel *p = (WarpPanel *)panel;
      p->_preview->SetCurrentGLContext();

      sizeForRenderBuffer( buffer );

      glBindFramebuffer( GL_FRAMEBUFFER, s_fbId );
      glViewport( 0, 0, buffer.BufferWi, buffer.BufferHt );

      glClearColor( 0.f, 0.f, 0.f, 0.f );
      glClear( GL_COLOR_BUFFER_BIT );

      glActiveTexture( GL_TEXTURE0 );
      glBindTexture( GL_TEXTURE_2D, s_rbTex );
      glTexSubImage2D( GL_TEXTURE_2D, 0, 0,0, buffer.BufferWi,buffer.BufferHt, GL_RGBA, GL_UNSIGNED_BYTE, &buffer.pixels[0] );

      glActiveTexture( GL_TEXTURE0+1 );
      glBindTexture( GL_TEXTURE_2D, s_noiseTexId );

      glBindVertexArray( s_vertexArrayId );
      glBindBuffer( GL_ARRAY_BUFFER, s_vertexBufferId );

      GLuint programId = 0;
      if ( warpType == "water drops" )
         programId = s_programId_waterdrops;
      else if ( warpType == "dissolve" )
         programId = warpTreatment == "in" ? s_programId_dissolve_in : s_programId_dissolve_out;
      else if ( warpType == "circle reveal" )
         programId = warpTreatment == "in" ? s_programId_circleReveal_in : s_programId_circleReveal_out;
      else if ( warpType == "banded swirl" )
         programId = warpTreatment == "in" ? s_programId_bandedSwirl_in : s_programId_bandedSwirl_out;
      else if ( warpType == "ripple" )
         programId = warpTreatment == "in" ? s_programId_ripple_in : s_programId_ripple_out;
      glUseProgram( programId );

      int loc = glGetUniformLocation( programId, "texSampler" );
      glUniform1i( loc, 0 );

      loc = glGetUniformLocation( programId, "progress" );
      glUniform1f( loc, progress );

      loc = glGetUniformLocation( programId, "noiseSampler" );
      if ( loc >= 0 )
         glUniform1i( loc, 1 );

      loc = glGetUniformLocation( programId, "xy" );
      if ( loc >= 0 )
         glUniform4f( loc, x, y, 0.f, 1.f );

      loc = glGetUniformLocation( programId, "speed" );
      if ( loc >= 0 )
         glUniform1f( loc, speed );

      loc = glGetUniformLocation( programId, "frequency" );
      if ( loc >= 0 )
         glUniform1f( loc, freq );

      GLuint vattrib = glGetAttribLocation( programId, "vpos" );
      glVertexAttribPointer( vattrib, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTex), reinterpret_cast<void *>( offsetof(VertexTex, v) ) );
      glEnableVertexAttribArray( vattrib );

      GLuint tattrib = glGetAttribLocation( programId, "tpos" );
      glVertexAttribPointer( tattrib, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTex), reinterpret_cast<void *>( offsetof(VertexTex, t ) ) );
      glEnableVertexAttribArray( tattrib );

      glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

      glDisableVertexAttribArray( vattrib );
      glDisableVertexAttribArray( tattrib );

      glBindVertexArray( 0 );
      glBindBuffer( GL_ARRAY_BUFFER, 0 );

      xlColorVector& cv( buffer.pixels );
      glReadPixels( 0, 0, buffer.BufferWi, buffer.BufferHt, GL_RGBA, GL_UNSIGNED_BYTE, &cv[0] );
   }
   else
   {
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
}

bool WarpEffect::CanRenderOnBackgroundThread( Effect *effect, const SettingsMap &settings, RenderBuffer &buffer )
{
   return !( useOpenGL && OpenGLShaders::HasShaderSupport() && OpenGLShaders::HasFramebufferObjects() );
}

void WarpEffect::sizeForRenderBuffer( const RenderBuffer& rb )
{
   if ( !s_shadersInit )
   {
      s_programId_dissolve_in = OpenGLShaders::compile( vsSrc, psDissolveIn );
      s_programId_dissolve_out = OpenGLShaders::compile( vsSrc, psDissolveOut );
      s_programId_circleReveal_in = OpenGLShaders::compile( vsSrc, psCircleRevealIn );
      s_programId_circleReveal_out = OpenGLShaders::compile( vsSrc, psCircleRevealOut );
      s_programId_bandedSwirl_in = OpenGLShaders::compile( vsSrc, psBandedSwirlIn );
      s_programId_bandedSwirl_out = OpenGLShaders::compile( vsSrc, psBandedSwirlOut );
      s_programId_ripple_in = OpenGLShaders::compile( vsSrc, psRippleIn );
      s_programId_ripple_out = OpenGLShaders::compile( vsSrc, psRippleOut );
      s_programId_waterdrops = OpenGLShaders::compile( vsSrc, psWaterDrops );
      s_noiseTexId = NoiseTexture();

      VertexTex vt[4] =
      {
         { {  1.f, -1.f }, { 1.f, 0.f } },
         { { -1.f, -1.f }, { 0.f, 0.f } },
         { {  1.f,  1.f }, { 1.f, 1.f } },
         { { -1.f,  1.f }, { 0.f, 1.f } }
      };
      glGenVertexArrays( 1, &s_vertexArrayId );
      glGenBuffers( 1, &s_vertexBufferId );

      glBindVertexArray( s_vertexArrayId );
      glBindBuffer( GL_ARRAY_BUFFER, s_vertexBufferId );
      glBufferData( GL_ARRAY_BUFFER, sizeof(VertexTex[4]), vt, GL_STATIC_DRAW );

      glBindVertexArray( 0 );
      glBindBuffer( GL_ARRAY_BUFFER, 0 );

      createOpenGLRenderBuffer( rb.BufferWi, rb.BufferHt, &s_rbId, &s_fbId );

      s_rbTex = RenderBufferTexture( rb.BufferWi, rb.BufferHt );

      s_rbWidth = rb.BufferWi;
      s_rbHeight = rb.BufferHt;
      s_shadersInit = true;
   }
   else if ( rb.BufferWi > s_rbWidth || rb.BufferHt > s_rbHeight )
   {
      glBindFramebuffer( GL_FRAMEBUFFER, 0 );
      if ( s_fbId )
         glDeleteFramebuffers( 1, &s_fbId );
      if ( s_rbId )
      {
         glBindRenderbuffer( GL_RENDERBUFFER, 0 );
         glDeleteRenderbuffers( 1, &s_rbId );
      }
      if ( s_rbTex )
         glDeleteTextures( 1, &s_rbTex );
      createOpenGLRenderBuffer( rb.BufferWi, rb.BufferHt, &s_rbId, &s_fbId );
      s_rbTex = RenderBufferTexture( rb.BufferWi, rb.BufferHt );

      s_rbWidth = rb.BufferWi;
      s_rbHeight = rb.BufferHt;
   }
}
