

#include "wx/wx.h"


#ifdef __WXMAC__
 #include "OpenGL/gl.h"
#else
	#ifdef _MSC_VER
		#include "GL/glut.h"
	#else
		#include <GL/gl.h>
	#endif
#endif

#include <wx/bitmap.h>
#include "DrawGLUtils.h"
#include "osxMacUtils.h"
#include <wx/graphics.h>
#include <wx/dcgraph.h>

#include <map>
#include "Image.h"
const double PI  =3.141592653589793238463;


static DrawGLUtils::xlGLCacheInfo *currentCache;

class OpenGL15Cache : public DrawGLUtils::xlGLCacheInfo {
public:
    OpenGL15Cache() {
        data.PreAlloc(128);
    };
    virtual ~OpenGL15Cache() {
    };

    virtual void addVertex(float x, float y, const xlColor &c) override {
        data.PreAlloc(1);
        data.AddVertex(x, y, c);
    }
    virtual void ensureSize(unsigned int s) override {
        data.PreAlloc(s);
    }
    virtual unsigned int vertexCount() override {
        return data.count;
    }
    virtual void flush(int type, int enableCapability) override {
        Draw(data, type, enableCapability);
        data.Reset();
    }


    void Draw(DrawGLUtils::xlVertexAccumulator &va, const xlColor & color, int type, int enableCapability) override {
        if (enableCapability != 0) {
            glEnable(enableCapability);
        }
        glColor4ub(color.Red(), color.Green(), color.Blue(), color.Alpha());
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(2, GL_FLOAT, 0, &va.vertices[0]);
        glDrawArrays(type, 0, va.count);

        glDisableClientState(GL_VERTEX_ARRAY);
        if (enableCapability != 0) {
            glDisable(enableCapability);
        }
    }
    void Draw(DrawGLUtils::xlVertexColorAccumulator &va, int type, int enableCapability) override {
        if (enableCapability != 0) {
            glEnable(enableCapability);
        }
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);

        glColorPointer(4, GL_UNSIGNED_BYTE, 0, &va.colors[0]);
        glVertexPointer(2, GL_FLOAT, 0, &va.vertices[0]);
        glDrawArrays(type, 0, va.count);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        if (enableCapability != 0) {
            glDisable(enableCapability);
        }
    }
    void Draw(DrawGLUtils::xlVertexTextureAccumulator &va, int type, int enableCapability)  override {
        if (enableCapability != 0) {
            glEnable(enableCapability);
        }

        glBindTexture(GL_TEXTURE_2D, va.id);
        glEnable(GL_TEXTURE_2D);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        glTexCoordPointer(2, GL_FLOAT, 0, &va.tvertices[0]);
        glVertexPointer(2, GL_FLOAT, 0, &va.vertices[0]);
        
        if (va.alpha != 255) {
            float intensity = va.alpha;
            intensity /= 255;
            if (intensity > 1.0) {
                intensity = 1.0;
            } else if (intensity < 0) {
                intensity = 0;
            }
            glColor4f(1.0, 1.0, 1.0, intensity);
            glDrawArrays(type, 0, va.count);
            glColor4f(1.0, 1.0, 1.0, 1.0);
        } else {
            glDrawArrays(type, 0, va.count);
        }
        
        

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        if (enableCapability != 0) {
            glDisable(enableCapability);
        }
        glDisable(GL_TEXTURE_2D);
    }


    virtual void Ortho(int topleft_x, int topleft_y, int bottomright_x, int bottomright_y) override {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        glOrtho(topleft_x, bottomright_x, bottomright_y, topleft_y, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }

    void PushMatrix() override {
        glPushMatrix();
    }
    void PopMatrix() override {
        glPopMatrix();
    }
    void Translate(float x, float y, float z) override {
        glTranslatef(x, y, z);
    }
    void Rotate(float angle, float x, float y, float z) override {
        glRotatef(angle, x, y, z);
    }
    void Scale(float w, float h, float z) override {
        glScalef(w, h, z);
    }

    void DrawTexture(GLuint* texture, float x, float y, float x2, float y2,
                     float tx, float ty, float tx2, float ty2) override {


        DrawGLUtils::xlVertexTextureAccumulator va(*texture);
        va.PreAlloc(6);

        va.AddVertex(x - 0.4, y, tx, ty);
        va.AddVertex(x - 0.4, y2, tx, ty2);
        va.AddVertex(x2 - 0.4, y2, tx2, ty2);
        va.AddVertex(x2 - 0.4, y2, tx2, ty2);
        va.AddVertex(x2 - 0.4, y, tx2, ty);
        va.AddVertex(x - 0.4, y, tx, ty);

        Draw(va, GL_TRIANGLES, 0);
    }

protected:
    DrawGLUtils::xlVertexColorAccumulator data;
    unsigned int curCount;
};



DrawGLUtils::xlGLCacheInfo *Create31Cache();
DrawGLUtils::xlGLCacheInfo *Create21Cache();


DrawGLUtils::xlGLCacheInfo *DrawGLUtils::CreateCache() {
    const GLubyte* str = glGetString(GL_VERSION);
    DrawGLUtils::xlGLCacheInfo *ret = nullptr;
    if (str[0] >= '3') {
        ret = Create31Cache();
    }
    if (ret == nullptr && str[0] == '2') {
        //ret = Create21Cache();
    }
    if (ret == nullptr) {
        ret = new OpenGL15Cache();
    }
    return ret;
}
void DrawGLUtils::SetCurrentCache(xlGLCacheInfo *c) {
    currentCache = c;
}
void DrawGLUtils::DestroyCache(xlGLCacheInfo *cache) {
    if (currentCache == cache) {
        currentCache = nullptr;
    }
    delete cache;
}

void DrawGLUtils::SetViewport(xlGLCanvas &win, int topleft_x, int topleft_y, int bottomright_x, int bottomright_y) {
    int x, y, x2, y2;
    x = topleft_x;
    y = topleft_y;
    x2 = bottomright_x;
    y2 = bottomright_y;

    xlSetRetinaCanvasViewport(win, x,y,x2,y2);
    glViewport(x,y,x2-x,y2-y);
    currentCache->Ortho(topleft_x, topleft_y, bottomright_x, bottomright_y);
}

void DrawGLUtils::PushMatrix() {
    currentCache->PushMatrix();
}
void DrawGLUtils::PopMatrix() {
    currentCache->PopMatrix();
}
void DrawGLUtils::Translate(float x, float y, float z) {
    currentCache->Translate(x, y, z);
}
void DrawGLUtils::Rotate(float angle, float x, float y, float z) {
    currentCache->Rotate(angle, x, y, z);
}
void DrawGLUtils::Scale(float w, float h, float z) {
    currentCache->Scale(w, h, z);
}
void DrawGLUtils::Draw(xlVertexAccumulator &va, const xlColor & color, int type, int enableCapability) {
    currentCache->Draw(va, color, type, enableCapability);
}
void DrawGLUtils::Draw(xlVertexColorAccumulator &va, int type, int enableCapability) {
    currentCache->Draw(va, type, enableCapability);
}
void DrawGLUtils::Draw(xlVertexTextureAccumulator &va, int type, int enableCapability) {
    currentCache->Draw(va, type, enableCapability);
}




void DrawGLUtils::PreAlloc(int verts) {
    currentCache->ensureSize(verts);
}
int DrawGLUtils::VertexCount() {
    return currentCache->vertexCount();
}
void DrawGLUtils::AddVertex(double x, double y, const xlColor &c, int transparency) {
    xlColor color(c);
    if (transparency) {
        double t = 100.0 - transparency;
        t *= 2.55;
        transparency = t;
        color.alpha = transparency > 255 ? 255 : (transparency < 0 ? 0 : transparency);
    }
    currentCache->addVertex(x, y, color);
}

void DrawGLUtils::AddRectAsTriangles(double x1, double y1,
                                     double x2, double y2,
                                     const xlColor &c, int transparency) {
    xlColor color(c);
    if (transparency) {
        double t = 100.0 - transparency;
        t *= 2.55;
        transparency = t;
        color.alpha = transparency > 255 ? 255 : (transparency < 0 ? 0 : transparency);
    }
    PreAlloc(6);
    currentCache->addVertex(x1, y1, color);
    currentCache->addVertex(x1, y2, color);
    currentCache->addVertex(x2, y2, color);

    currentCache->addVertex(x2, y2, color);
    currentCache->addVertex(x2, y1, color);
    currentCache->addVertex(x1, y1, color);
}
void DrawGLUtils::AddRect(double x1, double y1,
                          double x2, double y2,
                          const xlColor &c, int transparency) {
    xlColor color(c);
    if (transparency) {
        double t = 100.0 - transparency;
        t *= 2.55;
        transparency = t;
        color.alpha = transparency > 255 ? 255 : (transparency < 0 ? 0 : transparency);
    }
    currentCache->ensureSize(4);
    currentCache->addVertex(x1, y1, color);
    currentCache->addVertex(x1, y2, color);
    currentCache->addVertex(x2, y2, color);
    currentCache->addVertex(x2, y1, color);
}

void DrawGLUtils::End(int type, int enableCapability) {
    currentCache->flush(type, enableCapability);
}

void DrawGLUtils::DrawPoint(const xlColor &color, double x, double y)
{
    currentCache->addVertex(x, y, color);
    currentCache->flush(GL_POINTS);
}

void DrawGLUtils::DrawCircle(const xlColor &color, double cx, double cy, double r, int ctransparency, int etransparency)
{
    xlColor c(color);

    if (ctransparency) {
        double t = 100.0 - ctransparency;
        t *= 2.56;
        ctransparency = t;
        c.alpha = ctransparency;
    }

    int num_segments = r / 2;
    if (num_segments < 16) {
        num_segments = 16;
    }
    currentCache->ensureSize(num_segments * 2 + 4);
    float theta = 2 * 3.1415926 / float(num_segments);
    float tangetial_factor = tanf(theta);//calculate the tangential factor

    float radial_factor = cosf(theta);//calculate the radial factor

    float x = r;//we start at angle = 0

    float y = 0;

    currentCache->addVertex(cx, cy, c);

    if (etransparency) {
        double t = 100.0 - etransparency;
        t *= 2.56;
        etransparency = t;
        c.alpha = etransparency;
    } else {
        c.alpha = 255;
    }

    for(int ii = 0; ii < num_segments; ii++)
    {
        currentCache->addVertex(x + cx, y + cy, c);

        //calculate the tangential vector
        //remember, the radial vector is (x, y)
        //to get the tangential vector we flip those coordinates and negate one of them

        float tx = -y;
        float ty = x;

        //add the tangential vector

        x += tx * tangetial_factor;
        y += ty * tangetial_factor;

        //correct using the radial factor

        x *= radial_factor;
        y *= radial_factor;
    }
    currentCache->addVertex(x + cx, y + cy, c);
    currentCache->flush(GL_TRIANGLE_FAN);
}

void DrawGLUtils::DrawCircleUnfilled(const xlColor &color, double cx, double cy, double r, float width )
{
    static const double inc = PI / 12;
    static const double max = 2 * PI;
    glLineWidth(width);
    for(double d = 0; d < max; d += inc) {
        currentCache->addVertex(cos(d) * r + cx, sin(d) * r + cy, color);
    }
    currentCache->flush(GL_LINE_LOOP);
}

void DrawGLUtils::DrawLine(const xlColor &color, wxByte alpha,int x1, int y1, int x2, int y2, float width)
{
    glLineWidth(width);
    currentCache->ensureSize(4);
    currentCache->addVertex(x1, y1, color);
    currentCache->addVertex(x2, y2, color);
    currentCache->flush(GL_LINES);
}

void DrawGLUtils::DrawRectangle(const xlColor &color, bool dashed, int x1, int y1, int x2, int y2)
{
    if (!dashed)
    {
        currentCache->ensureSize(16);
        currentCache->addVertex(x1, y1, color);
        currentCache->addVertex(x2, y1, color);

        currentCache->addVertex(x2, y1, color);
        currentCache->addVertex(x2, y2 - 0.4f, color);

        currentCache->addVertex(x2, y2 - 0.4f, color);
        currentCache->addVertex(x1, y2 - 0.4f, color);

        currentCache->addVertex(x1, y1, color);
        currentCache->addVertex(x1, y2, color);
        currentCache->flush(GL_LINES);
    }
    else
    {
        // Line 1
        int xs = x1<x2?x1:x2;
        int xf = x1>x2?x1:x2;
        for(int x=xs;x<=xf;x++)
        {
            if(x%8<4)
            {
                currentCache->addVertex(x, y1, color);
            }
        }
        // Line 2
        int ys = y1<y2?y1:y2;
        int yf = y1>y2?y1:y2;
        for(int y=ys;y<=yf;y++)
        {
            if(y%8<4)
            {
                currentCache->addVertex(x2, y, color);
            }
        }
        // Line 3
        xs = x1<x2?x1:x2;
        xf = x1>x2?x1:x2;
        for(int x=xs;x<=xf;x++)
        {
            if(x%8<4)
            {
                currentCache->addVertex(x, y2, color);
            }
        }
        // Line 4
        ys = y1<y2?y1:y2;
        yf = y1>y2?y1:y2;
        for(int y=ys;y<=yf;y++)
        {
            if(y%8<4)
            {
                currentCache->addVertex(x1, y, color);
            }
        }
        currentCache->flush(GL_POINTS);
    }
}

void DrawGLUtils::DrawFillRectangle(const xlColor &color, wxByte alpha, int x, int y,int width, int height)
{
    xlColor c(color);
    c.alpha = alpha;
    AddRectAsTriangles(x, y, x + width, y + height, c);
    currentCache->flush(GL_TRIANGLES);
}

void DrawGLUtils::DrawRectangleArray(double y1, double y2, double x, std::vector<double> &xs, std::vector<xlColor> & colors, bool flush) {
    // each rect is 6 vertices (using GL_TRIANGLES) of x/y
    currentCache->ensureSize(colors.size() * 6);
    for (int n = 0; n < xs.size(); n++) {
        int x2 = xs[n];
        currentCache->addVertex(x, y1, colors[n]);
        currentCache->addVertex(x, y2, colors[n]);
        currentCache->addVertex(x2, y2, colors[n]);

        currentCache->addVertex(x, y1, colors[n]);
        currentCache->addVertex(x2, y2, colors[n]);
        currentCache->addVertex(x2, y1, colors[n]);
        x = x2;
    }
    if (flush) {
        currentCache->flush(GL_TRIANGLES);
    }
}


void DrawGLUtils::DrawHBlendedRectangle(const xlColor &left, const xlColor &right, int x1, int y1,int x2, int y2, bool flush) {
    currentCache->addVertex(x1, y1, left);
    currentCache->addVertex(x1, y2, left);
    currentCache->addVertex(x2, y2, right);

    currentCache->addVertex(x2, y2, right);
    currentCache->addVertex(x2, y1, right);
    currentCache->addVertex(x1, y1, left);
    if (flush) {
        currentCache->flush(GL_TRIANGLES);
    }
}
void DrawGLUtils::DrawHBlendedRectangle(const xlColorVector &colors, int x1, int y1,int x2, int y2, int offset) {
    xlColor start;
    xlColor end;
    int cnt = colors.size();
    if (cnt == 0) {
        return;
    }
    start = colors[0+offset];
    if (cnt == 1) {
        DrawGLUtils::DrawHBlendedRectangle(start, start, x1, y1, x2, y2);
        return;
    }
    int xl = x1;
    start = colors[0+offset];
    for (int x = 1+offset; x < cnt; x++) {
        end =  colors[x];
        int xr = x1 + (x2 - x1) * x / (cnt  - 1);
        if (x == (cnt - 1)) {
            xr = x2;
        }
        DrawGLUtils::DrawHBlendedRectangle(start, end, xl, y1, xr, y2, false);
        start = end;
        xl = xr;
    }
    currentCache->flush(GL_TRIANGLES);
}


void DrawGLUtils::DrawDisplayList(float xOffset, float yOffset,
                                  float width, float height,
                                  const DrawGLUtils::xlDisplayList & dl,
                                  xlVertexColorAccumulator &bg) {
    std::lock_guard<std::recursive_mutex> lock(dl.lock);
    if (dl.empty()) {
        return;
    }
    bg.PreAlloc(dl.size());
    for (int idx = 0; idx < dl.size(); idx++) {
        const DisplayListItem &item = dl[idx];
        bg.AddVertex(xOffset + item.x * width, yOffset + item.y * height, item.color);
    }
}


static void addMipMap(GLuint* texture, const wxImage &l_Image, int &level) {
    if (l_Image.IsOk() == true)
    {
        glTexImage2D(GL_TEXTURE_2D, level, GL_RGB, (GLsizei)l_Image.GetWidth(), (GLsizei)l_Image.GetHeight(),
                     0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)l_Image.GetData());
        int err = glGetError();
        if (err == GL_NO_ERROR) {
            level++;
        }
    }
}

void DrawGLUtils::CreateOrUpdateTexture(const wxBitmap &bmp48,
                                        const wxBitmap &bmp32,
                                        const wxBitmap &bmp16,
                                        GLuint* texture)
{
    int level = 0;
    glGenTextures(1,texture);
    glBindTexture(GL_TEXTURE_2D, *texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    addMipMap(texture, bmp48.ConvertToImage().Rescale(64, 64, wxIMAGE_QUALITY_HIGH), level);
    addMipMap(texture, bmp32.ConvertToImage(), level);
    addMipMap(texture, bmp16.ConvertToImage(), level);
    addMipMap(texture, bmp16.ConvertToImage().Rescale(8, 8, wxIMAGE_QUALITY_HIGH), level);
    addMipMap(texture, bmp16.ConvertToImage().Rescale(4, 4, wxIMAGE_QUALITY_HIGH), level);
    addMipMap(texture, bmp16.ConvertToImage().Rescale(2, 2, wxIMAGE_QUALITY_HIGH), level);
    addMipMap(texture, bmp16.ConvertToImage().Rescale(1, 1, wxIMAGE_QUALITY_HIGH), level);
}

void DrawGLUtils::DrawTexture(GLuint* texture,
                         float x, float y, float x2, float y2,
                         float tx, float ty, float tx2, float ty2)
{
    currentCache->DrawTexture(texture, x, y, x2, y2, tx, ty, tx2, ty2);
}

void DrawGLUtils::UpdateTexturePixel(GLuint* texture, double x, double y, xlColor& color, bool hasAlpha)
{
    int bytesPerPixel = hasAlpha ?  4 : 3;
    GLubyte *imageData=new GLubyte[bytesPerPixel];
    imageData[0] = color.red;
    imageData[1] = color.green;
    imageData[2] = color.blue;
    if( hasAlpha ) {
        imageData[3] = color.alpha;
    }
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,*texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, x, y, 1, 1, hasAlpha ?  GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, imageData);
    delete [] imageData;
    glDisable(GL_TEXTURE_2D);
}


class FontInfoStruct {
public:
    FontInfoStruct(const char * const *d,
                   float mh, float mw, float md,
                   std::vector<float> w)
        : data(d), maxH(mh), maxW(mw), maxD(md), widths(w) {
        
    }
    
    const char * const *data;
    float maxH;
    float maxW;
    float maxD;
    std::vector<float> widths;
};
#include "fonts/fonts.h"

#ifndef LINUX
#define USEAA true
#else
#define USEAA false
#endif

#define NUMLINES 6
#define NUMCHARS 16

#define CASEFONT(x) case x: Load(font##x); break;

class FontTexture {
public:
    FontTexture() { id = 0;};
    ~FontTexture() {};

    bool Valid() { return id != 0;}
    
    void Create(int size) {
        switch (size) {
            CASEFONT(10);
            CASEFONT(12);
            CASEFONT(16);
            CASEFONT(18);
            CASEFONT(20);
            CASEFONT(24);
            CASEFONT(28);
            CASEFONT(40);
            CASEFONT(44);
            CASEFONT(56);
            CASEFONT(88);
            default:
                printf("No FONT!!!! %d\n", size);
                ForceCreate(size);
                break;
        }
    }
    void Load(const FontInfoStruct &fi) {
        maxW = fi.maxW;
        maxH = fi.maxH;
        maxD = fi.maxD;
        widths = fi.widths;
        wxImage cimg(fi.data);
        cimg.InitAlpha();
        InitializeImage(cimg);
    }

    void InitializeImage(wxImage &cimg) {
        for (int x = 0; x < cimg.GetWidth(); x++) {
            for (int y = 0; y < cimg.GetHeight(); y++) {
                int alpha = cimg.GetRed(x, y);
                if (alpha) {
                    cimg.SetRGB(x, y, 0, 0, 0);
                    cimg.SetAlpha(x, y, alpha);
                } else {
                    cimg.SetRGB(x, y, 0, 0, 0);
                    cimg.SetAlpha(x, y, 0);
                }
            }
        }
        image.load(cimg);
        id = *image.getID();
    }
    void ForceCreate(int size) {
        wxString faceName = "Gil Sans";
        
        bool useAA = USEAA;
        if (size <= 12)  {
            useAA = false;
        }

        wxGraphicsContext *ctx = wxGraphicsContext::Create();
        ctx->SetInterpolationQuality(wxInterpolationQuality::wxINTERPOLATION_BEST);
        ctx->SetCompositionMode(wxCompositionMode::wxCOMPOSITION_OVER);
        if (useAA) {
            ctx->SetAntialiasMode(wxANTIALIAS_DEFAULT);
            ctx->SetFont(ctx->CreateFont(size, faceName, wxFONTFLAG_ANTIALIASED, *wxWHITE));
        } else {
            ctx->SetAntialiasMode(wxANTIALIAS_NONE);
            ctx->SetFont(ctx->CreateFont(size, faceName, wxFONTFLAG_NOT_ANTIALIASED, *wxWHITE));
        }
        maxW = 0;
        maxH = 0;
        maxD = 0;
        for (char c = ' '; c <= '~'; c++) {
            wxString s = c;
            double width, height, desc, el;
            ctx->GetTextExtent(s, &width, &height, &desc, &el);
            maxW = std::max(maxW, (float)width);
            maxH = std::max(maxH, (float)height);
            maxD = std::max(maxD, (float)desc);
        }
        delete ctx;
        
        maxW += 1.0; // allow room to possibly have to handle some kerning
        
        
        int imgh = (maxH + 5) * 6;
        int imgw = (maxW + 5) * 16;
        float power_of_two_that_gives_correct_width=std::log((float)imgw)/std::log(2.0);
        float power_of_two_that_gives_correct_height=std::log((float)imgh)/std::log(2.0);
        imgw=(int)std::pow( 2.0, (int)(std::ceil(power_of_two_that_gives_correct_width)) );
        imgh=(int)std::pow( 2.0, (int)(std::ceil(power_of_two_that_gives_correct_height)) );

        
        wxImage cimg(imgw, imgh);
        cimg.InitAlpha();

        /*
        ctx = wxGraphicsContext::Create(cimg);
        ctx->SetAntialiasMode(wxANTIALIAS_NONE);
        ctx->SetPen( wxPen(wxColor(92, 92, 92), 0) );
        for (int l = 0; l < NUMLINES; l++) {
            for (int x = 0; x < (maxH + 5); x += 2) {
                wxPoint2DDouble beginPoints(0, x + l * (maxH+5));
                wxPoint2DDouble endPoints((maxW + 5)*NUMCHARS, x + l * (maxH+5));
                ctx->StrokeLines(1, &beginPoints, &endPoints);
            }
        }
        for (int l = 0; l < NUMCHARS; l++) {
            wxPoint2DDouble beginPoints(l * (maxW+5), 0);
            wxPoint2DDouble endPoints(l * (maxW + 5), (NUMLINES + 1) * (maxH+5));
            ctx->StrokeLines(1, &beginPoints, &endPoints);
        }
        delete ctx;
         */
        
        
        ctx = wxGraphicsContext::Create(cimg);
        ctx->SetPen( *wxWHITE_PEN );
        ctx->SetInterpolationQuality(wxInterpolationQuality::wxINTERPOLATION_BEST);
        ctx->SetCompositionMode(wxCompositionMode::wxCOMPOSITION_OVER);

        if (useAA) {
            ctx->SetAntialiasMode(wxANTIALIAS_DEFAULT);
            ctx->SetFont(ctx->CreateFont(size, faceName, wxFONTFLAG_ANTIALIASED, *wxWHITE));
        } else {
            ctx->SetAntialiasMode(wxANTIALIAS_NONE);
            ctx->SetFont(ctx->CreateFont(size, faceName, wxFONTFLAG_NOT_ANTIALIASED, *wxWHITE));
        }
        
        
        int count = 0;
        int line = 0;
        widths.clear();
        widths.resize('~' - ' ' + 1);
        for (char c = ' '; c <= '~'; c++) {
            wxString s = c;
            ctx->DrawText(s, 2 + (count * (maxW + 5)), 2 + line * (maxH + 5));
            
            double width, height, desc, el;
            ctx->GetTextExtent(s, &width, &height, &desc, &el);
            widths[c - ' '] = width;
            count++;
            if (count == 16) {
                count = 0;
                line++;
            }
        }
        delete ctx;

        
        for (int l = 0; l < NUMLINES; l++) {
            for (int c = 0; c < NUMCHARS; c++) {
                bool kerned = false;
                char ch = (l * NUMCHARS + c);
                for (int y = 0; y < (maxH + 3) && !kerned; y++) {
                    int notblack = cimg.GetRed(2 + c * (maxW + 5), y + 1 + l * (maxH + 5));
                    if (notblack) {
                        //the antialiasing has caused part of the pixel to bleed to the left
                        //we need to move the entire thing to the right
                        kerned = true;
                        //printf("Kerned: %c\n", (ch + ' '));
                    }
                }
                if (kerned) {
                    for (int x = (maxW-1); x > 1; x--) {
                        int xpos = c * (maxW + 5) + x;
                        for (int y = 1; y <= (maxH + 4); y++) {
                            int ypos = y  + l * (maxH + 5);
                            int clr = cimg.GetRed(xpos - 1, ypos);
                            cimg.SetRGB(xpos, ypos, clr, clr, clr);
                        }
                    }
                    for (int y = 1; y <= (maxH + 4); y++) {
                        int ypos = y  + l * (maxH + 5);
                        cimg.SetRGB(c * (maxW + 5) + 1, ypos, 0, 0, 0);
                    }
                    widths[ch] += 1.0;
                }
                /*
                if ((ch + ' ') == 'j') {
                    printf("%f  %f\n", c * (maxW + 5) + 1, widths[ch]);
                }
                 */
            }
        }
        
        
         //used to output data that can be used to static generation above
        /*
        wxString fn = wxString::Format("/tmp/font_%d.png", size);
        cimg.SaveFile(fn, wxBITMAP_TYPE_PNG);
        printf("#include \"font_%d.xpm\"\n", size);
        if (size == 10) {
            widths['b' - ' '] += 0.5;
            widths['o' - ' '] += 0.5;
            widths['p' - ' '] += 0.5;
            widths['w' - ' '] += 0.5;
            widths['x' - ' '] += 0.75;
            widths['y' - ' '] += 0.75;
            widths['m' - ' '] -= 0.75;
            widths['A' - ' '] += 0.75;
            widths['C' - ' '] -= 0.5;
            widths['W' - ' '] += 0.5;
            widths['P' - ' '] += 0.5;
            widths['|' - ' '] -= 0.5;
        }
        if (size == 12) {
            widths['p' - ' '] += 0.5;
            widths['w' - ' '] += 0.75;
            widths['m' - ' '] -= 0.5;
            widths['e' - ' '] -= 0.5;
            widths['t' - ' '] += 0.5;
        }
        printf("static FontInfoStruct font%d(font_%d_xpm, %f, %f, %f, {\n", size, size, maxH, maxW, maxD);
        printf("%f", widths[0]);
        for (int x = 1; x < widths.size(); x++) {
            printf(", %f", widths[x]);
        }
        printf("});\n");
        */
        
        
        InitializeImage(cimg);
    }
    void Populate(float x, float yBase, const wxString &text, float factor, DrawGLUtils::xlVertexTextureAccumulator &va) {
        va.PreAlloc(6 * text.Length());
        va.id = id;
        //DrawGLUtils::DrawLine(xlBLUE, 255, x, yBase, x+3, yBase, 1);
        //DrawGLUtils::DrawLine(xlBLUE, 255, x, yBase - (float(maxH) + 2) / factor, x+3, yBase - (float(maxH) + 2) / factor, 1);
        for (int idx = 0; idx < text.Length(); idx++) {
            char ch = text[idx];
            if (ch >= ' ' && ch <= '~') {
                if (ch == ' ') {
                    x += widths[0] / factor;
                    continue;
                }
                int linei = ch;
                linei -= ' ';
                linei /= 16.0;
                float line = linei;

                float pos = ch - ' ';
                pos -= linei * 16.0;

                float tx = 1 + (pos * (maxW + 5));
                float tx2 = tx + widths[ch - ' '];
                
                float x2 = x + float(widths[ch - ' ']) / factor;
                
                float ty2 = image.textureHeight - 3 - (line * (maxH + 5));
                float ty = ty2 - maxH;
                
                float y = yBase;
                float y2 = yBase - float(maxH) / factor;
                /*
                if (ch == 'p') {
                    printf("%c   %f %f    %f %f     %f %f\n", ch, x, x2, tx, tx2,  (x2-x), (tx2-tx));
                }
                 */
                
                tx /= image.textureWidth;
                tx2 /= image.textureWidth;
                
                ty2 /= image.textureHeight;
                ty /= image.textureHeight;
                
                //samples need to be from the MIDDLE of the pixel
                ty -= 0.5 / image.textureHeight;
                ty2 += 0.75 / image.textureHeight;
                
                tx += 0.5 / image.textureWidth;
                tx2 += 1.0 / image.textureWidth;
                
                y += 0.25/factor;
                y2 -= 0.75/factor;
                x -= 0.25/factor;
                x2 += 0.25/factor;
                
                /*
                if (ch == '1') {
                    DrawGLUtils::DrawFillRectangle(xlWHITE, 255,x,y, image.textureWidth / factor, image.textureHeight / factor);
                    
                    ty = 0.0;
                    ty2 = 1.0 - ty;
                    tx = 0.0;
                    tx2 = 1.0 - tx;
                    y = yBase + (float)(image.textureHeight)/factor;
                    x2 = x + (float)(image.textureWidth)/ factor;
                    y2 = yBase;
                }
                */
                //printf("%c   %f %f    %f %f\n", ch, tx, tx2, ty, ty2);
                
                va.AddVertex(x, y, tx, ty);
                va.AddVertex(x, y2, tx, ty2);
                va.AddVertex(x2, y2, tx2, ty2);
                va.AddVertex(x2, y2, tx2, ty2);
                va.AddVertex(x2, y, tx2, ty);
                va.AddVertex(x, y, tx, ty);

                x += widths[ch - ' '] / factor;
            }
        }

    }
    void Draw(float x, float yBase, const wxString &text, float factor) {
        glDisable(GL_DEPTH_TEST);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        DrawGLUtils::xlVertexTextureAccumulator va(id);
        Populate(x, yBase, text, factor, va);
        DrawGLUtils::Draw(va, GL_TRIANGLES, GL_BLEND);
    }

    float TextWidth(const wxString &text, float factor) {
        float w = 0;
        for (int idx = 0; idx < text.Length(); idx++) {
            char ch = text[idx];
            if (ch >= ' ' && ch <= '~') {
                w += widths[ch - ' '];
            }
        }
        return w / factor;
    }
    
    Image image;
    float maxD, maxW, maxH;
    std::vector<float> widths;
    GLuint id;
};
static std::map<unsigned int, FontTexture> FONTS;

void DrawGLUtils::DrawText(double x, double y, double size, const wxString &text, double factor) {
    int tsize = size * factor;
    if (!FONTS[tsize].Valid()) {
        FONTS[tsize].Create(tsize);
    }
    FONTS[tsize].Draw(x, y, text, factor);
}
int DrawGLUtils::GetTextWidth(double size, const wxString &text, double factor) {
    int tsize = size * factor;
    if (!FONTS[tsize].Valid()) {
        FONTS[tsize].Create(tsize);
    }
    return FONTS[tsize].TextWidth(text, factor);
}

void DrawGLUtils::Draw(DrawGLUtils::xlVertexTextAccumulator &va, int size, float factor, int enableCapability) {
    int tsize = size * factor;
    if (!FONTS[tsize].Valid()) {
        FONTS[tsize].Create(tsize);
    }
    /*
    if (tsize == 10 || tsize == 12) {
        for (int i = 0; i < va.count; i++) {
            
        }
        return;
    }
     */
    DrawGLUtils::xlVertexTextureAccumulator vat;
    for (int x = 0; x < va.count; x++) {
        FONTS[tsize].Populate(va.vertices[x*2], va.vertices[x*2 + 1], va.text[x], factor, vat);
    }
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    Draw(vat, GL_TRIANGLES, enableCapability);
}

