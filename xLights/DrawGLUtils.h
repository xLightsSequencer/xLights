#include "Color.h"

namespace DrawGLUtils
{
    void DrawPoint(const xlColor &color, wxDouble x, wxDouble y);
    void DrawLine(const xlColor &color, wxByte alpha,int x1, int y1,int x2, int y2,float width);
    void DrawRectangle(const xlColor &color, bool dashed, int x1, int y1,int x2, int y2);
    void DrawFillRectangle(const xlColor &color, wxByte alpha, int x, int y,int width, int height);
    void CreateOrUpdateTexture(char** p_XPM, GLuint* texture);
}

