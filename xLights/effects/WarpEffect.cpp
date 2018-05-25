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

   xlColor rippleIn( const ColorBuffer& cb, double s, double t, double progress )
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
   xlColor rippleOut( const ColorBuffer& cb, double s, double t, double progress )
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

   xlColor dissolveIn( const ColorBuffer& cb, double s, double t, double progress )
   {
      xlColor dissolveColor = dissolveTex( s, t );
      unsigned char byteProgress = (unsigned char)( 255 * progress );
      return (dissolveColor.red <= byteProgress) ? tex2D( cb, s, t ) : xlBLACK;
   }
   xlColor dissolveOut( const ColorBuffer& cb, double s, double t, double progress )
   {
      xlColor dissolveColor = dissolveTex( s, t );
      unsigned char byteProgress = (unsigned char)( 255 * progress );
      return (dissolveColor.red > byteProgress) ? tex2D( cb, s, t ) : xlBLACK;
   }

   xlColor circleRevealIn( const ColorBuffer& cb, double s, double t, double progress )
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
   xlColor circleRevealOut( const ColorBuffer& cb, double s, double t, double progress )
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

   xlColor bandedSwirlIn( const ColorBuffer& cb, double s, double t, double progress )
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
   xlColor bandedSwirlOut( const ColorBuffer& cb, double s, double t, double progress )
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
      "uniform sampler2D texSampler;\n"
      "uniform float progress;\n"
      "uniform sampler2D noiseSampler;\n"
      "in vec2 texCoord;\n"
      "void main(){\n"
      "    float red = texture( noiseSampler, texCoord).r;\n"
      "    gl_FragColor = (red <= progress )\n"
      "        ? texture( texSampler, texCoord )\n"
      "        : vec4( 0, 0, 0, 1 );\n"
      "}\n";
   const char *psDissolveOut =
      "#version 330\n"
      "uniform sampler2D texSampler;\n"
      "uniform float progress;\n"
      "uniform sampler2D noiseSampler;\n"
      "in vec2 texCoord;\n"
      "void main(){\n"
      "    float red = texture( noiseSampler, texCoord).r;\n"
      "    gl_FragColor = (red > progress )\n"
      "        ? texture( texSampler, texCoord )\n"
      "        : vec4( 0, 0, 0, 1 );\n"
      "}\n";

   const char *psCircleRevealIn =
      "#version 330\n"
      "uniform sampler2D texSampler;\n"
      "uniform float progress;\n"
      "in vec2 texCoord;\n"
      "void main(){\n"
      "    float FuzzyAmount = 0.04;\n"
      "    float CircleSize = 0.60;\n"
      "    vec2 CenterPt = vec2(0.5, 0.5);\n"
      "\n"
      "    float radius = -FuzzyAmount + progress * (CircleSize + 2.0 * FuzzyAmount);\n"
      "    float fromCenter = length(texCoord - CenterPt);\n"
      "    float distFromCircle = fromCenter - radius;\n"
      "\n"
      "    vec4 c = texture(texSampler, texCoord);\n"
      "    float p = clamp((distFromCircle + FuzzyAmount) / (2.0 * FuzzyAmount), 0, 1 );\n"
      "    gl_FragColor = mix( c, vec4(0,0,0,1), p );\n"
      "}\n";
   const char *psCircleRevealOut =
      "#version 330\n"
      "uniform sampler2D texSampler;\n"
      "uniform float progress;\n"
      "in vec2 texCoord;\n"
      "void main(){\n"
      "    float FuzzyAmount = 0.04;\n"
      "    float CircleSize = 0.60;\n"
      "    vec2 CenterPt = vec2(0.5, 0.5);\n"
      "\n"
      "    float radius = -FuzzyAmount + (1-progress) * (CircleSize + 2.0 * FuzzyAmount);\n"
      "    float fromCenter = length(texCoord - CenterPt);\n"
      "    float distFromCircle = fromCenter - radius;\n"
      "\n"
      "    vec4 c = texture(texSampler, texCoord);\n"
      "    float p = clamp((distFromCircle + FuzzyAmount) / (2.0 * FuzzyAmount), 0, 1 );\n"
      "    gl_FragColor = mix( c, vec4(0,0,0, 1), p );\n"
      "}\n";

   const char *psBandedSwirlIn =
      "#version 330\n"
      "uniform sampler2D texSampler;\n"
      "uniform float progress;\n"
      "in vec2 texCoord;\n"
      "void main() {\n"
      "    vec2 center = vec2( 0.5, 0.5);\n"
      "    float frequency = 20.f;\n"
      "    float twistAmount = 1.6f;\n"
      "\n"
      "    vec2 toUV = vec2( texCoord - center );\n"
      "    float distanceFromCenter = length(toUV);\n"
      "    vec2 normToUV = toUV / distanceFromCenter;\n"
      "    float angle = atan(normToUV.y, normToUV.x);\n"
      "\n"
      "    angle += sin(distanceFromCenter * frequency) * twistAmount * (1-progress);\n"
      "    vec2 newUV = vec2( cos(angle), sin(angle) );\n"
      "    newUV = newUV * distanceFromCenter + center;\n"
      "\n"
      "    vec4 c1 = texture(texSampler, fract(newUV));\n"
      "    vec4 c2 = texture(texSampler, texCoord);\n"
      "    gl_FragColor = mix(c1, c2, 1-progress);\n"
      "}\n";
   const char *psBandedSwirlOut =
      "#version 330\n"
      "uniform sampler2D texSampler;\n"
      "uniform float progress;\n"
      "in vec2 texCoord;\n"
      "void main() {\n"
      "    vec2 center = vec2( 0.5, 0.5);\n"
      "    float frequency = 20.f;\n"
      "    float twistAmount = 1.6f;\n"
      "\n"
      "    vec2 toUV = vec2( texCoord - center );\n"
      "    float distanceFromCenter = length(toUV);\n"
      "    vec2 normToUV = toUV / distanceFromCenter;\n"
      "    float angle = atan(normToUV.y, normToUV.x);\n"
      "\n"
      "    angle += sin(distanceFromCenter * frequency) * twistAmount * progress;\n"
      "    vec2 newUV = vec2( cos(angle), sin(angle) );\n"
      "    newUV = newUV * distanceFromCenter + center;\n"
      "\n"
      "    vec4 c1 = texture(texSampler, fract(newUV));\n"
      "    vec4 c2 = texture(texSampler, texCoord);\n"
      "    gl_FragColor = mix(c2, c1, progress);\n"
      "}\n";

   const char *psRippleIn =
      "#version 330\n"
      "uniform sampler2D texSampler;\n"
      "uniform float progress;\n"
      "in vec2 texCoord;\n"
      "void main() {\n"
      "    vec2 center = vec2( 0.5, 0.5 );\n"
      "    float frequency = 20;\n"
      "    float speed = 10;\n"
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
      "    gl_FragColor = mix( c2, c1, progress );\n"
      "}\n";
      const char *psRippleOut =
      "#version 330\n"
      "uniform sampler2D texSampler;\n"
      "uniform float progress;\n"
      "in vec2 texCoord;\n"
      "void main() {\n"
      "    vec2 center = vec2( 0.5, 0.5 );\n"
      "    float frequency = 20;\n"
      "    float speed = 10;\n"
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
      "    gl_FragColor = mix( c1, c2, progress );\n"
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
    p->Choice_Warp_Effect->SetValue( "dissolve" );
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

    if ( settingsMap.Get( "CHOICE_Warp_Effect", "" )== "dissolve" )
      settingsMap.erase( "CHOICE_Warp_Effect" );
    if ( settingsMap.Get( "CHOICE_Warp_Type", "" )== "in" )
      settingsMap.erase( "CHOICE_Warp_Type" );

    RenderableEffect::RemoveDefaults(version, effect);
}

void WarpEffect::Render(Effect *eff, SettingsMap &SettingsMap, RenderBuffer &buffer)
{
   std::string warpEffect = SettingsMap.Get( "CHOICE_Warp_Effect", "dissolve" );
   std::string warpType = SettingsMap.Get( "CHOICE_Warp_Type", "in");
   float progress = buffer.GetEffectTimeIntervalPosition(1.f);

   if ( useOpenGL && OpenGLShaders::HasShaderSupport() && OpenGLShaders::HasFramebufferObjects() )
   {
      WarpPanel *p = (WarpPanel *)panel;
      p->_preview->SetCurrentGLContext();

      sizeForRenderBuffer( buffer );

      glBindFramebuffer( GL_FRAMEBUFFER, /*fbId*/s_fbId );
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
      if ( warpEffect == "dissolve" )
         programId = warpType == "in" ? s_programId_dissolve_in : s_programId_dissolve_out;
      else if ( warpEffect == "circle reveal" )
         programId = warpType == "in" ? s_programId_circleReveal_in : s_programId_circleReveal_out;
      else if ( warpEffect == "banded swirl" )
         programId = warpType == "in" ? s_programId_bandedSwirl_in : s_programId_bandedSwirl_out;
      else if ( warpEffect == "ripple" )
         programId = warpType == "in" ? s_programId_ripple_in : s_programId_ripple_out;

      glUseProgram( programId );

      int loc = glGetUniformLocation( programId, "texSampler" );
      glUniform1i( loc, 0 );

      loc = glGetUniformLocation( programId, "progress" );
      glUniform1f( loc, progress );

      loc = glGetUniformLocation( programId, "noiseSampler" );
      if ( loc >= 0 )
         glUniform1i( loc, 1 );

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
      double adjust = buffer.GetEffectTimeIntervalPosition(1.f);
      if ( warpEffect == "ripple" )
         RenderPixelTransform( warpType == "in" ? rippleIn : rippleOut, adjust, buffer );
      else if ( warpEffect == "dissolve" )
         RenderPixelTransform( warpType == "in" ? dissolveIn : dissolveOut, adjust, buffer );
      else if ( warpEffect == "banded swirl" )
         RenderPixelTransform( warpType == "in" ? bandedSwirlIn : bandedSwirlOut, adjust, buffer );
      else if ( warpEffect == "circle reveal" )
         RenderPixelTransform( warpType == "in" ? circleRevealIn : circleRevealOut, adjust, buffer );
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
