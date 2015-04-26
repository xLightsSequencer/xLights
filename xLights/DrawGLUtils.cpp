
#ifdef __WXMAC__
 #include "OpenGL/glu.h"
 #include "OpenGL/gl.h"
#else
 #include <GL/glu.h>
 #include <GL/gl.h>
#endif

#include <wx/bitmap.h>
#include "DrawGLUtils.h"

void DrawGLUtils::StartPoints(const xlColor &color, int transparency) {
    if (transparency) {
        double t = 100.0 - transparency;
        t *= 2.56;
        transparency = t;
        glColor4ub(color.Red(), color.Green(),color.Blue(), transparency);
    } else {
        glColor3ub(color.Red(), color.Green(),color.Blue());
    }
    glBegin(GL_POINTS);
}
void DrawGLUtils::AddPoint(double x, double y) {
    glVertex2f(x, y);
}
void DrawGLUtils::EndPoints() {
    glEnd();
}



void DrawGLUtils::DrawPoint(const xlColor &color, double x, double y)
{
    glColor3ub(color.Red(), color.Green(),color.Blue());
    glBegin(GL_POINTS);
    glVertex2f(x, y);
    glEnd();
}

void DrawGLUtils::DrawCircle(const xlColor &color, double cx, double cy, double r, int ctransparency, int etransparency)
{
    if (ctransparency) {
        double t = 100.0 - ctransparency;
        t *= 2.56;
        ctransparency = t;
        glColor4ub(color.Red(), color.Green(),color.Blue(), ctransparency);
    } else {
        glColor3ub(color.Red(), color.Green(),color.Blue());
    }
    
    int num_segments = r / 2;
    if (num_segments < 16) {
        num_segments = 16;
    }
    float theta = 2 * 3.1415926 / float(num_segments);
    float tangetial_factor = tanf(theta);//calculate the tangential factor
    
    float radial_factor = cosf(theta);//calculate the radial factor
    
    float x = r;//we start at angle = 0
    
    float y = 0;
    
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy); //center vertex
    
    if (etransparency) {
        double t = 100.0 - etransparency;
        t *= 2.56;
        etransparency = t;
        glColor4ub(color.Red(), color.Green(),color.Blue(), etransparency);
    } else {
        glColor3ub(color.Red(), color.Green(),color.Blue());
    }

    for(int ii = 0; ii < num_segments; ii++)
    {
        glVertex2f(x + cx, y + cy);//output vertex
        
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
    glVertex2f(x + cx, y + cy);//output vertex
    glEnd();
}


void DrawGLUtils::DrawLine(const xlColor &color, wxByte alpha,int x1, int y1, int x2, int y2, float width)
{
    glLineWidth(width);
    glColor4ub(color.Red(), color.Green(),color.Blue(),alpha);
    glBegin(GL_LINES);
    glVertex2i(x1, y1);
    glVertex2i(x2, y2);
    glEnd();
}

void DrawGLUtils::DrawRectangle(const xlColor &color, bool dashed, int x1, int y1, int x2, int y2)
{
    glColor3ub(color.Red(), color.Green(),color.Blue());
    if (!dashed)
    {
        glBegin(GL_LINES);
        glVertex2f(x1, y1);
        glVertex2f(x2, y1);

        glVertex2f(x2, y1);
        glVertex2f(x2, y2 - 0.4f);

        glVertex2f(x2, y2 - 0.4f);
        glVertex2f(x1, y2 - 0.4f);

        glVertex2f(x1, y1);
        glVertex2f(x1, y2);
        glEnd();
    }
    else
    {
        glBegin(GL_POINTS);
        // Line 1
        int xs = x1<x2?x1:x2;
        int xf = x1>x2?x1:x2;
        for(int x=xs;x<=xf;x++)
        {
            if(x%8<4)
            {
                glVertex2f(x, y1);
            }
        }
        // Line 2
        int ys = y1<y2?y1:y2;
        int yf = y1>y2?y1:y2;
        for(int y=ys;y<=yf;y++)
        {
            if(y%8<4)
            {
                glVertex2f(x2,y);
            }
        }
        // Line 3
        xs = x1<x2?x1:x2;
        xf = x1>x2?x1:x2;
        for(int x=xs;x<=xf;x++)
        {
            if(x%8<4)
            {
                glVertex2f(x, y2);
            }
        }
        // Line 4
        ys = y1<y2?y1:y2;
        yf = y1>y2?y1:y2;
        for(int y=ys;y<=yf;y++)
        {
            if(y%8<4)
            {
                glVertex2f(x1,y);
            }
        }
        glEnd();
    }
}

void DrawGLUtils::DrawFillRectangle(const xlColor &color, wxByte alpha, int x, int y,int width, int height)
{
    glColor4ub(color.Red(), color.Green(),color.Blue(),alpha);
    //glColor3ub(color.Red(), color.Green(),color.Blue());
    glBegin(GL_QUADS);
    glVertex2f(x, y);
    glVertex2f(x+width, y);
    glVertex2f(x+width, y+height);
    glVertex2f(x, y+height);
    glEnd();
}

void DrawGLUtils::DrawRectangleArray(double y1, double y2, double x, std::vector<double> &xs, std::vector<xlColor> & colors) {
    glBegin(GL_QUADS);
    glColor3ub(colors[0].Red(), colors[0].Green(),colors[0].Blue());

    for (int n = 0; n < xs.size(); n++) {
        glColor3ub(colors[n].Red(), colors[n].Green(),colors[n].Blue());
        glVertex2f(x, y1);
        glVertex2f(x, y2);
        int x2 = xs[n];
        glVertex2f(x2, y2);
        glVertex2f(x2, y1);
        x = x2;
    }
    glEnd();
}


void DrawGLUtils::DrawHBlendedRectangle(const xlColor &left, const xlColor &right, int x1, int y1,int x2, int y2) {
    glColor3ub(left.Red(), left.Green(), left.Blue());
    glBegin(GL_QUADS);
    glVertex2f(x1, y1);
    glVertex2f(x1, y2);
    glColor3ub(right.Red(), right.Green(),right.Blue());
    glVertex2f(x2, y2);
    glVertex2f(x2, y1);
    glEnd();
}
void DrawGLUtils::DrawHBlendedRectangle(const xlColorVector &colors, int x1, int y1,int x2, int y2) {
    xlColor start;
    xlColor end;
    int cnt = colors.size();
    if (cnt == 0) {
        return;
    }
    start = colors[0];
    if (cnt == 1) {
        DrawGLUtils::DrawHBlendedRectangle(start, start, x1, y1, x2, y2);
        return;
    }
    int xl = x1;
    start = colors[0];
    for (int x = 1; x < cnt; x++) {
        end =  colors[x];
        int xr = x1 + (x2 - x1) * x / (cnt  - 1);
        if (x == (cnt - 1)) {
            xr = x2;
        }
        DrawGLUtils::DrawHBlendedRectangle(start, end, xl, y1, xr, y2);
        start = end;
        xl = xr;
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
